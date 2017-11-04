//#define BB_NO_THREAD
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
	b.write("t", 6);
	Test t;
	t.m_val = 1;
	b.write("class", t);
	b.write("temp", Test());

	auto value = b.read<int>("key");
	Test& t2 = b.read<Test>("class");
	b.write<std::string>("str", "val");

	std::string str = b.read<std::string>("str");

	return 0;
}