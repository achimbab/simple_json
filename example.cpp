#include <iostream>
#include <fstream>
#include <iomanip>

#include "json.h"

using namespace std;
using namespace json;

void print(JSONValue value, int indent) {
    switch (value.type_) {
        case OBJECT:
            cout << setw(indent*10) << "OBJECT\n";
            indent++;
            for (auto kv : *(value.v_.o_)) {
                cout << setw(indent*10) << kv.first << ": ";
                switch (kv.second.type_) {
                case STRING:
                    cout << kv.second.v_.s_ << endl;
                    break;
                case INTEGER:
                    cout << kv.second.v_.i_ << endl;
                    break;
                case ARRAY:
                    print(kv.second, indent+1);
                    break;
                }
            }
            break;
        case ARRAY:
            cout << setw(indent*10) << "ARRAY\n";
            for (auto v : *(value.v_.a_)) {
                print(v, indent+1);
            }
            break;
    }
}

typedef enum {
    BODY = 0,
    HEAD,
    GLOVES,
    WAIST,
    SHOES,

    EQUIP_MAX,
    UNKOWN,
} equip_part_t;

equip_part_t fromString(const char* str) {
    if (strncmp(str, "Body", 4) == 0) {
        return BODY;
    } else if (strncmp(str, "Head", 4) == 0) {
        return HEAD;
    } else if (strncmp(str, "Gloves", 6) == 0) {
        return GLOVES;
    } else if (strncmp(str, "Waist", 5) == 0) {
        return WAIST;
    } else if (strncmp(str, "Shoes", 5) == 0) {
        return SHOES;
    } else {
        return UNKOWN;
    }
}

class Stats {
public:
    int strength;
    int vitality;
    int intelligence;
    int magReg;
    int phyReg;

    friend ostream& operator<<(ostream& os, const Stats& s) {
        os << "S:" << s.strength << ", V:" << s.vitality << ", I:" << s.intelligence << ", M:" << s.magReg << ", P:" << s.phyReg;
        return os;
    }
};

class Equip {
public:
    equip_part_t part;
    string set;
    Stats stat;

    Equip(JSONValue v) {
        auto o = v.v_.o_;
        set = o->find("Set")->second.v_.s_;
        part = fromString(o->find("Part")->second.v_.s_);
        stat.strength = o->find("Strength")->second.v_.i_;
        stat.vitality = o->find("Vitality")->second.v_.i_;
        stat.intelligence = o->find("Intelligence")->second.v_.i_;
        stat.magReg = o->find("MagReg")->second.v_.i_;
        stat.phyReg = o->find("PhyReg")->second.v_.i_;
    }

    friend ostream& operator<<(ostream& os,  const Equip& e) {
        os << e.set << ", " << e.part << ", " << e.stat;
        return os;
    }
};

class SetOption {
public:
    string set;
    Stats* stats[5];
};

int combine_equips() {
    // Read JSON from a file
    string buf;
    string line;

    ifstream in("../equipments.json");
    if (!in.is_open()) {
        return 1;
    }

    while (getline(in, line)) {
        buf += line;
    }
    in.close();

    // Parse JSON to Objects
    cout << buf.c_str() << endl;
    JSONParser parser(buf.c_str());
    JSONValue value = parser.readValue();
    print(value, 0);

    vector<Equip> equips;
    vector<SetOption> setoptions;

    // Load equipments
    auto equipsSource = value.v_.o_->find("Equipments");
    if (equipsSource == value.v_.o_->end()) {
        return 1;
    }
    for (auto e : *(equipsSource->second.v_.a_)) {
        equips.push_back(Equip(e));
    }

    for (auto e : equips) {
        cout << e << endl;
    }

    // Load setoptions


    return 0;
}

int main()
{
    return combine_equips();
}


