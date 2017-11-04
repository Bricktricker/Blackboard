# Blackboard
This thread save data structure allows you to store data of all types in one data structure.
It requires c++ 11. This repository is a fork of [MitchCroft/Blackboard](https://github.com/MitchCroft/Blackboard),
but I removed the singleton feature and optimized it a bit, so that you can create multiple instances of the blackboard.

### Usage:
```cpp
#include "Blackboard.h"

int main(){
    Util::Blackboard b;
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
    Util::Blackboard b;
    Test t1;
    b.write("key", t);
    Test t2 = b.read<Test>("key");
    return 0;
}
```

You can also define callbacks, whitch get called when a value gets added or changed on the blackboard. By passing `false` as the third argument, no listener gets called. There are three diffrent types of functions you can define as callbacks:

1. void foo(const std::string& key);
2. void foo(const T& val );
3. void foo(const std::string& key, const T& val);

```cpp
    #include "Blackboard.h"
    
    int main(){
        Util::Blackboard b;
        b.subscribe<int>("key",
        [](const std::string& key, const int& val) {
		    std::cout << "Entry with the key " << key <<
		    " changed to " << val << '\n';
	    });
	    
        b.write("key", 5); //lamda function gets called
        b.write("key", 6, false); //no call to lamda function
        
        return 0;
    }
```
