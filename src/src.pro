TEMPLATE = app
SOURCES += main.cpp
FLEXSOURCES = lexer.l
BISONSOURCES = parser.y
CONFIG += C++11

OTHER_FILES += \
    $$FLEXSOURCES \
    $$BISONSOURCES

flex.input = FLEXSOURCES
flex.output = ${QMAKE_FILE_BASE}.cpp
flex.commands = flex -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flex.variable_out = SOURCES
flex.name = Flex ${QMAKE_FILE_IN}
flex.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flex

flexheader.input = FLEXSOURCES
flexheader.output = ${QMAKE_FILE_BASE}.h
flexheader.commands = @true
flexheader.variable_out = HEADERS
flexheader.name = Flex Headers ${QMAKE_FILE_IN}
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += flexheader

bison.input = BISONSOURCES
bison.output = ${QMAKE_FILE_BASE}.cpp
bison.commands = bison --debug --verbose --defines=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
bison.variable_out = SOURCES
bison.name = Bison {$QMAKE_FILE_IN}
bison.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bison

bisonheader.input = BISONSOURCES
bisonheader.output = ${QMAKE_FILE_BASE}.h
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers {$QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader
