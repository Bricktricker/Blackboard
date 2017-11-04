#include "Blackboard.h"
#include <iostream>

void call(const int& value) {
	//__debugbreak();
	int i = 0;
}

int main() {
	Util::Blackboard b;
	b.subscribe<int>("key", &call);
	b.subscribe<int>("key", [](const std::string& str, const int& val) {
		__debugbreak();
	});
	b.write("key", 5, true);

	auto value = b.read<int>("key");

	//b.unsubscribeAll("key");
	b.wipeTypeKey<int>("key");
	return 0;
}