#include "myclass.h"

MyClass::MyClass()
{}

MyClass::~MyClass()
{}

Foo MyClass::foo() const
{ 
    return m_fooRef; 
}

void MyClass::setFoo(Foo* foo)
{
    // Do something
}

void MyClass::setFooBar(Foo* foo, Bar* bar)
{
    // Do Something
}

Foo &MyClass::fooRef() const
{
    return m_fooRef;
}

Bar *MyClass::barPtr() const
{
    return m_barPtr;
}

const Foo *&MyClass::fooPtrRef() const
{
    return m_fooPtrRef;
}

Bar **MyClass::barPtrPtr() const
{
    return m_barPtrPtr;
}
