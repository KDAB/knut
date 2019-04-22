include (../../knut.pri)

QT += testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DESTDIR  = $$KNUT_APP_PATH

include (../../src/core/core.pri)

SOURCES += tst_lexertest.cpp
