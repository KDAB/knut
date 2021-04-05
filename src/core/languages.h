#pragma once

namespace Core {

enum Languages {
    Language_CXX = 1,
    Language_C = 2,
    Language_QML = 4,
    Language_UI = 8,
    Language_QRC = 16, // Qt resource file
    Language_RC = 32, // Windows MFC resource file
};

}
