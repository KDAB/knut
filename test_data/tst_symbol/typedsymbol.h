#pragma once

#include <string>

class TestClass {
private:
  void   *m_ptr;
  const std::string &m_lvalue_reference;
  std::string&& m_rvalue_reference;
  const class T*const m_const_ptr;
};
