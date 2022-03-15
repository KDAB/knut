#pragma once

namespace Base {

class TestDocument
{
public:
    TestDocument();

    bool isParent() const;
    int childrenCount(const int &root);

    int rows;
    int columns;
};

} // namespace Core
