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
