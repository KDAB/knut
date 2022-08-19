#pragma once

#include <string>

class MyObject {
public:
    MyObject(const std::string& message);
    ~MyObject();
    void sayMessage();
    void sayMessage(const std::string& test);

    enum class MyEnum {
        A = 0x01,
        B = 0x02,
        C = A | B
    };

private:
    std::string m_message;
    MyEnum m_enum;
};
