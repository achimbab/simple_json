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

    Stats() : 
        strength(0),
        vitality(0),
        intelligence(0),
        magReg(0),
        phyReg(0) {
    }

    Stats(const Stats& other) : 
        strength(other.strength),
        vitality(other.vitality),
        intelligence(other.intelligence),
        magReg(other.magReg),
        phyReg(other.phyReg) {
    }

    Stats& operator+=(const Stats& rhs) {
        strength += rhs.strength;
        vitality += rhs.vitality;
        intelligence += rhs.intelligence;
        magReg += rhs.magReg;
        phyReg += rhs.phyReg;
        return *this;
    }

    Stats operator+(const Stats& rhs) {
        Stats stats(*this);
        stats.strength += rhs.strength;
        stats.vitality += rhs.vitality;
        stats.intelligence += rhs.intelligence;
        stats.magReg += rhs.magReg;
        stats.phyReg += rhs.phyReg;
        return stats;
    }

    friend ostream& operator<<(ostream& os, const Stats& s) {
        os << "S:" << s.strength << ", V:" << s.vitality << ", I:" << s.intelligence << ", M:" << s.magReg << ", P:" << s.phyReg;
        return os;
    }
};

class Equip {
public:
    equip_part_t part;
    string set;
    Stats stats;

    Equip(JSONValue v) {
        auto o = v.v_.o_;
        set = o->find("Set")->second.v_.s_;
        part = fromString(o->find("Part")->second.v_.s_);
        stats.strength = o->find("Strength")->second.v_.i_;
        stats.vitality = o->find("Vitality")->second.v_.i_;
        stats.intelligence = o->find("Intelligence")->second.v_.i_;
        stats.magReg = o->find("MagReg")->second.v_.i_;
        stats.phyReg = o->find("PhyReg")->second.v_.i_;
    }

    friend ostream& operator<<(ostream& os, const Equip& e) {
        os << e.set << ", " << e.part << ", " << e.stats;
        return os;
    }
};

class SetOption {
public:
    string set;
    Stats stats[5];
    int no;

    SetOption(JSONValue v) : no(0) {
        auto o = v.v_.o_;
        set = o->find("Set")->second.v_.s_;
        memset(&stats, 0, sizeof(stats));
        add(v);
    }

    void add(JSONValue v) {
        auto o = v.v_.o_;
        int count = o->find("Count")->second.v_.i_;
        stats[count - 1].strength = o->find("Strength")->second.v_.i_;
        stats[count - 1].vitality = o->find("Vitality")->second.v_.i_;
        stats[count - 1].intelligence = o->find("Intelligence")->second.v_.i_;
        stats[count - 1].magReg = o->find("MagReg")->second.v_.i_;
        stats[count - 1].phyReg = o->find("PhyReg")->second.v_.i_;
        no = max(count, no);
    }

    friend ostream& operator<<(ostream& os, const SetOption& o) {
        os << o.set << endl;
        for (int i = 0; i < o.no; i++) {
            os << o.stats[i] << endl;
        }
        return os;
    }
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
    map<string, SetOption> setOptions;

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
    auto setOptionsSource = value.v_.o_->find("SetOptions");
    if (setOptionsSource == value.v_.o_->end()) {
        return 1;
    }
    for (auto o : *(setOptionsSource->second.v_.a_)) {
        string set = o.v_.o_->find("Set")->second.v_.s_;
        auto s = setOptions.find(set);
        if (s == setOptions.end()) {
            setOptions.insert(make_pair(set, SetOption(o)));
        } else {
            s->second.add(o);
        }
    }

    for (auto o : setOptions) {
        cout << o.second << endl;
    }

    // combine
    vector<vector<Equip>> ec;
    // body
    for (auto b : equips) {
        if (b.part == BODY) {
            for (auto h : equips) {
                if (h.part == HEAD) {
                    for (auto g : equips) {
                        if (g.part == GLOVES) {
                            for (auto w : equips) {
                                if (w.part == GLOVES) {
                                    for (auto s : equips) {
                                        if (s.part == GLOVES) {
                                            Stats total = b.stats + h.stats + g.stats + w.stats + s.stats;
                                            cout << "\nTOTAL: " << total << endl;
                                            cout << b << endl  << h << endl << g << endl << w << endl << s << endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // head
    // gloves
    // waist
    // shoes

    return 0;
}

int main()
{
    return combine_equips();
}


