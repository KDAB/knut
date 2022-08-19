#include "myobject.h"

#include <iostream>


MyObject::MyObject(const std::string& message)
    : m_message(message)
{}

MyObject::~MyObject()
{}

void MyObject::sayMessage() {
    std::cout << m_message << std::endl;
}

void MyObject::sayMessage(const std::string& test) {
    m_enum = MyEnum::C;
    std::cout << test << std::endl;
}
