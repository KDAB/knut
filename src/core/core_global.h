#pragma once

#include <QString>

namespace Core {

inline QString testDataPath()
{
#if defined(TEST_DATA_PATH)
    return TEST_DATA_PATH;
#else
    return "";
#endif
}

}
