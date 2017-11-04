#include "Blackboard.h"
using Utilities::Blackboard;
#include <iostream>

void call(const int& value) {
	//__debugbreak();
	int i = 0;
}

int main() {
	Blackboard b;
	b.subscribe<int>("key", &call);
	b.write("key", 5, true);

	auto value = b.read<int>("key");

	//b.unsubscribeAll("key");
	b.wipeTypeKey<int>("key");
	return 0;
}