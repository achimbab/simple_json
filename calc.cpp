#include <iostream>

#include "json.h"

void print(json::JSONValue value) {
    switch (value.type_) {
        case json::OBJECT:
            std::cout << "json::OBJECT" << std::endl;
            for (auto kv : *(value.v_.o_)) {
                std::cout << kv.first << std::endl;
                switch (kv.second.type_) {
                case json::STRING:
                    std::cout << kv.second.v_.s_ << std::endl;
                    break;
                case json::INTEGER:
                    std::cout << kv.second.v_.i_ << std::endl;
                    break;
                }
            }
            break;
        case json::ARRAY:
            std::cout << "json::ARRAY" << std::endl;
            for (auto v : *(value.v_.a_)) {
                print(v);
            }
            break;
    }
}

void test(const char* source) {
	json::JSONParser parser(source);
	json::JSONValue value = parser.readValue();
    print(value);
}

int main()
{
    const char* sources[] = {
        "{\"key\":\"value\",\"key2\":\"value2\",\"key3\":12345}",
        "[{\"key\":\"value\"},{\"key2\":\"value2\"}]",
    };
    for (auto source : sources) {
        std::cout << source << std::endl;
        test(source);
        std::cout << std::endl;
    }
    return 0;
}


