#include <iostream>

#include "json.h"

int main()
{
	const char* source = "{\"key\":\"value\",\"key2\":\"value2\",\"key3\":12345}";
	json::JSONParser parser(source);
	json::JSONValue value = parser.readValue();
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
    return 0;
}


