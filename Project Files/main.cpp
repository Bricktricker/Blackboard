#include "Blackboard.h"
using Utilities::Blackboard;
#include <iostream>

void call(const int& value) {
	__debugbreak();
}

int main() {
	Blackboard b;
	b.subscribe<int>("key", &call);
	b.write("key", 5, true);

	auto value = b.read<int>("key");

	return 0;
}