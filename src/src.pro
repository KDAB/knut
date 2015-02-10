TEMPLATE = app
SOURCES += main.cpp \
    document.cpp \
    converter.cpp
FLEXSOURCES = rclexer.l
BISONSOURCES = rcparser.y
CONFIG += c++11 console
TARGET = ../bin/knut

flex.input = FLEXSOURCES
flex.output = rclexer.cpp
flex.commands = flex --header-file=rclexer.h -o rclexer.cpp ${QMAKE_FILE_IN}
win32-msvc*:flex.commands = win_flex --wincompat --header-file=rclexer.h -o rclexer.cpp ${QMAKE_FILE_IN}
flex.variable_out = SOURCES
flex.name = Flex ${QMAKE_FILE_IN}
flex.depends = rcparser.cpp
flex.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flex

flexheader.input = FLEXSOURCES
flexheader.output = rclexer.h
flexheader.commands = @true
msvc:flexheader.commands = @echo >NUL
flexheader.variable_out = SOURCES
flexheader.name = Flex ${QMAKE_FILE_IN}
flexheader.depends = rcparser.cpp
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += flex

bison.input = BISONSOURCES
bison.output = rcparser.cpp
bison.commands = bison --debug --verbose --defines=rcparser.h -o rcparser.cpp ${QMAKE_FILE_IN}
win32-msvc*:bison.commands = win_bison --debug --verbose --defines=rcparser.h -o rcparser.cpp ${QMAKE_FILE_IN}
bison.variable_out = SOURCES
bison.name = Bison {$QMAKE_FILE_IN}
bison.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bison

bisonheader.input = BISONSOURCES
bisonheader.output = rcparser.h
bisonheader.commands = @true
msvc:bisonheader.commands = @echo >NUL
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers {$QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader

HEADERS += \
    document.h \
    converter.h

