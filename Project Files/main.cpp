#include "Blackboard.h"
#include <iostream>

struct Test {
	int m_val = 0;
};

int main() {
	Util::Blackboard b;

	b.subscribe<int>("key", [](const std::string& key, const int& val) {
		std::cout << "Entry with the key " << key << " changed to " << val << '\n';
	});

	b.write("key", 5, true);
	Test t;
	b.write("class", t);

	auto value = b.read<int>("key");
	const Test& t2 = b.read<Test>("class");
	auto v2 = b.read<int>("key");
	b.write<std::string>("str", "val");

	auto s = b.read<std::string>("str");

	return 0;
}