#pragma once

class Foo;
class Bar;

class MyClass {
public:
    MyClass();
    virtual ~MyClass();

    Foo foo() const;
    void setFoo(Foo* foo);

    virtual void setFooBar(Foo* foo, Bar* bar);

    Foo &fooRef() const;
    Bar *barPtr() const;
    const Foo *&fooPtrRef() const;
    Bar **barPtrPtr() const;

private:
    double m_double = 1.1;
    const int m_constInt = 2;
    Foo &m_fooRef;
    Bar *m_barPtr = nullptr;
    const Foo *& m_fooPtrRef;
    Bar **m_barPtrPtr;
};
