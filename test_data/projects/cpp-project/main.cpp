#include <iostream>

#include "myobject.h"

int freeFunction(unsigned, long long);

int main(int argc, char *argv[]) {
    MyObject object("Hello World!");

    object.sayMessage();
    object.sayMessage("Another message" /*a comment*/);

    freeFunction(1, 1);

    return 0;
}

// Test functions with/without named parameters
using namespace std;
string myFreeFunction(
        unsigned,
        unsigned int,
        long long,
        const string,
        const std::string&,
        long long (*)(unsigned, const std::string&)) {
    return "hello";
}

int myOtherFreeFunction(
        unsigned a,
        unsigned int b,
        long long c,
        const string d,
        const std::string& e_123,
        long long (*f)(unsigned, const std::string&)
        ) {
    return 42;
}

int freeFunction(unsigned, long long)
{
        return 5;
}
