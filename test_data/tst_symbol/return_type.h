#pragma once

#include <string>

using namespace std;

void *freePtr();
void *freePtrImpl() { return nullptr; }

const std::string &freeReference();
const std::string &freeReferenceImpl() {
  return "";
}

const class T *const freeConstPtr();
const class T *const freeConstPtrImpl() { return nullptr; }

class MyClass {
  MyClass() = default();
  ~MyClass() = default();

  void *memberPtr();
  const string& memberReference();
  const class T*    const memberConstPtr();
};

void*MyClass::memberPtrImpl() {
  return nullptr;
}

const std::string& MyClass::memberReferenceImpl() {
  return m_thing;
}

const
class
T
*
const
MyClass::memberConstPtrImpl() {
  return nullptr;
}

MyClassImpl::MyClassImpl()
{
}

MyClassImpl::~MyClassImpl()
{
}
