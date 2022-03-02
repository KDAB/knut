#include "myobject.h"

#include <iostream>


MyObject::MyObject(const std::string& message)
  : m_message(message)
{}

void MyObject::sayMessage() {
  std::cout << m_message << std::endl;
}
