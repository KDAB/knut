include (../../knut.pri)
include (../../samples/samples.pri)
include (../../js-samples/js-samples.pri)

QT += testlib uitools

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DESTDIR  = $$KNUT_APP_PATH

include (../../src/core/core.pri)

SOURCES +=  tst_convertertest.cpp
