TEMPLATE = app
TARGET   = step1

DEFINES -= UNICODE
DEFINES += _AFXDLL WINVER=0x0501
QMAKE_LIBS_QT_ENTRY =

SOURCES = MFCDialogApp.cpp \
    MFCDialogDlg.cpp \
    ModalDlg.cpp \
    ModelessDlg.cpp \
    stdafx.cpp
HEADERS = MFCDialogApp.h \
    ModalDlg.h \
    Resource.h \
    targetver.h \
    MFCDialogDlg.h \
    ModelessDlg.h \
    stdafx.h
RC_FILE = MFCDialog.rc

FORMS += \
    ModalDlg.ui \
    ModelessDlg.ui

include (../../../qtwinmigrate/qtwinmigrate.pri)
