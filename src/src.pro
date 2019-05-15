include(../knut.pri)

TEMPLATE = app
TARGET   = $$KNUT_APP_TARGET
DESTDIR  = $$KNUT_APP_PATH

QT       += core gui widgets qml

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

include (core/core.pri)
include (models/models.pri)

SOURCES += \
        actiondialog.cpp \
        contenttree.cpp \
        fileselector.cpp \
        jsrunner.cpp \
        main.cpp \
        mainwindow.cpp \
        menudialog.cpp \
        overviewfiltermodel.cpp \
        overviewmodel.cpp \
        overviewtree.cpp \
        rcsyntaxhighlighter.cpp \
        resultwidget.cpp\
        textedit.cpp \
	toolbardialog.cpp

HEADERS += \
        actiondialog.h \
        contenttree.h \
        fileselector.h \
        jsrunner.h \
        mainwindow.h \
        menudialog.h \
        overviewfiltermodel.h \
        overviewmodel.h \
        overviewtree.h \
        rcsyntaxhighlighter.h \
        resultwidget.h\
        textedit.h \
	toolbardialog.h

FORMS += \
        actiondialog.ui \
        mainwindow.ui \
        menudialog.ui \
	toolbardialog.ui
