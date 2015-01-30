INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
LIBS += -luser32

SOURCES += $$PWD/src/qwinwidget.cpp $$PWD/src/qwinhost.cpp $$PWD/src/qmfcapp.cpp
HEADERS += $$PWD/src/qwinwidget.h $$PWD/src/qwinhost.h $$PWD/src/qmfcapp.h

INCLUDEPATH += $$PWD/src

contains(QT_MAJOR_VERSION, 5): QT += widgets gui-private
