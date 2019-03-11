#ifndef __JSON_H__
#define __JSON_H__

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

#include <cstring>

/**
 * TODO
 * use move semantics
 */

namespace json {

    class JSONValue;

    typedef std::map<std::string, JSONValue> object_t;
    typedef std::vector<JSONValue> array_t;

    typedef enum {
        OBJECT,
        ARRAY,
        STRING,
        INTEGER    
    } type_t;

    typedef enum {
        VALUE_STRING,
        VALUE_INTEGER, 
        BEGIN_ARRAY, 
        BEGIN_OBJECT,
        END_ARRAY, 
        END_OBJECT, 
        NAME_SEPARATOR,
        VALUE_SEPARATOR, 
        END_OF_INPUT,  
        ERROR,
    } token_t;

    class Source {
    public:
        Source(const char* source) : source_(source), p_(source) { 
        }

        void skipWhitespaces() {
            while (*p_ == '\0' || *p_ == ' ') {
                p_++;
            }
        }

        char peekChar() {
            skipWhitespaces();
            return *p_;
        }

        char nextChar() {
            skipWhitespaces();
            return *p_++;
        }

        void eat() {
            p_++;
        }

        char* nextString() {
            const char* start = p_;
            while (*p_ != 0 && *p_ != '"') {
                // TODO escape
                p_++;
            }
            const size_t length = p_ - start;
            char* s = new char[length + 1];
            strncpy(s, start, length);
            s[length] = 0;
            p_++;    // skip "
            return s;
        }

        int nextInt() {
            int i = 0;
            while ('0' <= *p_ and *p_ <= '9' && *p_ != 0) {
                i *= 10;
                i += *p_ - '0';
                p_++;
            }
            return i;
        }

    private:
        const char* source_;
        const char* p_;
    };

    // TODO: Token string, Token integer
    class Token {
    public:
        Token(token_t type, Source& source) : type_(type) {
            switch (type_) {
            case VALUE_STRING:
                source.eat();
                value_.s_ = source.nextString();
                break;
            case VALUE_INTEGER:
                value_.i_ = source.nextInt();
                break;
            default:
                source.eat();
                break;
            }
        }
        token_t getType() { return type_; }

    //private:
        token_t type_;

        union {
            const char* s_;
            int i_;
        } value_;
    };

    class Scanner {
    public:
        Scanner(const char* source) : source_(source) {
        }

        Token nextToken(token_t expected) {
            Token token = nextToken();
            if (token.getType() != expected) {
                // TODO exception
                return token;
            }
            return token;
        }

        Token nextToken() {
            char c = source_.peekChar();
            switch (c) {
            case '{':
                return Token(BEGIN_OBJECT, source_);
            case '}':
                return Token(END_OBJECT, source_);
            case '[':
                return Token(BEGIN_ARRAY, source_);
            case ']':
                return Token(END_ARRAY, source_);
            case '"':
                return Token(VALUE_STRING, source_);
            case ':':
                return Token(NAME_SEPARATOR, source_);
            case ',':
                return Token(VALUE_SEPARATOR, source_);
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return Token(VALUE_INTEGER, source_);
            case '\0':
                return Token(END_OF_INPUT, source_);
            default:
                // TODO : exception
                return Token(ERROR, source_);
            }
        }

    private:
        Source source_;
    };

    class JSONValue {
    public:
        JSONValue(type_t type) : type_(type) {
            switch (type_) {
            case OBJECT:
                v_.o_ = new object_t;
                break;
            case ARRAY:
                v_.a_ = new array_t;
                break;
            case STRING:
                v_.s_ = NULL;
                break;
            }
        }

        JSONValue(const JSONValue& other) : type_(other.type_) {
            switch (type_) {
            case OBJECT:
                v_.o_ = new object_t;
                for(auto kv : *(other.v_.o_)) {
                    put(kv.first, kv.second);
                }
                break;
            case ARRAY:
                v_.a_ = new array_t;
                for(auto v : *(other.v_.a_)) {
                    add(v);
                }
                break;
            case STRING:
                set(strdup(other.v_.s_));
                break;
            case INTEGER:
                set(other.v_.i_);
                break;
            }
        }

        JSONValue& operator=(const JSONValue& other) {
            if(&other == this) {
                return *this;
            }

            switch (type_) {
            case OBJECT:
                v_.o_ = new object_t;
                for(auto kv : *(other.v_.o_)) {
                    put(kv.first, kv.second);
                }
                break;
            case ARRAY:
                v_.a_ = new array_t;
                for(auto v : *(other.v_.a_)) {
                    add(v);
                }
                break;
            case STRING:
                set(strdup(other.v_.s_));
                break;
            case INTEGER:
                set(other.v_.i_);
                break;
            }

            return *this;
        }

        ~JSONValue() {
            switch (type_) {
            case OBJECT:
                delete v_.o_;
                break;
            case ARRAY:
                delete v_.a_;
                break;
            case STRING:
                delete v_.s_;
                break;
            }
        }

        void put(std::string s, JSONValue v) {
            v_.o_->insert(std::make_pair(s, v));
        }

        void add(JSONValue v) {
            v_.a_->push_back(v);
        }

        void set(const char* s) {
            v_.s_ = s;
        }

        void set(const int i) {
            v_.i_ = i;
        }

    //private:
        const type_t type_;
        union {
            object_t* o_;
            array_t* a_;
            const char* s_;
            int i_;
        } v_;
    };

    class JSONParser {
    public:
        JSONParser(const char *source) :
            scanner_(source)
        {
        }

        JSONValue readValue() {
            Token t = scanner_.nextToken();

            switch (t.getType()) {
            case BEGIN_OBJECT:
                return readObject();
            case BEGIN_ARRAY:
                return readArray();
            case VALUE_STRING: {
               JSONValue v(STRING);
               v.set(t.value_.s_);
               return v;
            }
            case VALUE_INTEGER: {
                JSONValue v(INTEGER);
                v.set(t.value_.i_);
                return v;
            }
            default:
                // TODO: exception
                return JSONValue(INTEGER);
            }
        }


    private:
        JSONValue readObject() {
            JSONValue v(OBJECT);

            while (true) {
                Token key = scanner_.nextToken(VALUE_STRING);
                Token nsep = scanner_.nextToken(NAME_SEPARATOR);
                JSONValue value = readValue();
                v.put(key.value_.s_, value);
                Token vsep = scanner_.nextToken();
                if (vsep.getType() == END_OBJECT) {
                    break;
                } else if (vsep.getType() == VALUE_SEPARATOR) {
                    continue;
                } else {
                    // TODO : exception
                    return v;
                }
            }

            return v;
        }

        JSONValue readArray() {
            JSONValue v(ARRAY);

            while (true) {
                JSONValue value = readValue();
                v.add(value);
                Token vsep = scanner_.nextToken();
                if (vsep.getType() == BEGIN_ARRAY) {
                    break;
                } else if (vsep.getType() == VALUE_SEPARATOR) {
                    continue;
                } else {
                    // TODO: exception
                    return v;
                }
            }

            return v;
        }

        Scanner scanner_;
    };

}

#endif

