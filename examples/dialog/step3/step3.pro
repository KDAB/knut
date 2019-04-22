TEMPLATE = app
TARGET   = step3

DEFINES -= UNICODE
DEFINES += _AFXDLL WINVER=0x0501
QMAKE_LIBS_QT_ENTRY =

SOURCES = stdafx.cpp \
    TutorialApp.cpp \
    TutorialDlg.cpp
HEADERS = Resource.h \
    stdafx.h \
    TutorialApp.h \
    TutorialDlg.h
RC_FILE = MFC_UpdateGUI.rc

FORMS += \
    TutorialDlg.ui

include(../../../qtwinmigrate/qtwinmigrate.pri)
