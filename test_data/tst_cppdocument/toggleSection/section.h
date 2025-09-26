#pragma once

#include <string>

class Section {
public:
    Section();
    ~Section();
    void foo();
    int bar();

    static Section *create();
    Section &reference();
private:
    Section *m_pointer;
};
