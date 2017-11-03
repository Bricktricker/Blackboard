#define _BLACKBOARD_
#include "Blackboard.h"
using Utilities::Blackboard;

int main() {
	Blackboard b;
	b.write("key", 5, false);

	return 0;
}