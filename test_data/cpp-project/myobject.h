#pragma once

#include <string>

class MyObject {
  public:
    MyObject(const std::string& message);

    void sayMessage();

  private:
    std::string m_message;
};
