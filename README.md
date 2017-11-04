# Blackboard
This data structure allows you to store data of all types in one data structure.
It requires c++ 11. This repository is a fork of [MitchCroft/Blackboard](https://github.com/MitchCroft/Blackboard),
but I removed the singleton feature, so that you can create multiple instances of the blackboard.

### Basic usage:
```cpp
#include "Blackboard.h"

int main(){
    Blackboard b;
    b.write("key", 5);
    int value = b.read<int>("key");
    return 0;
}
```

It is also possible to store your own data types, but they need to have valid default and copy constructures defined, as well as the assignment operator.

```cpp
#include "Blackboard.h"

class Test{
    int m_val;
public:
    Test(){
        m_val = 1;
    }
};

int main(){
    Blackboard b;
    Test t1;
    b.write("key", t);
    Test t2 = b.read<Test>("key");
    return 0;
}
```

You can also define callbacks, whitch get called when a value gets added or changed on the blackboard.

```cpp
    TODO: Add example
```
