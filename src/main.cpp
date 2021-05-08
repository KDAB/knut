#include <QApplication>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Run tests if there's no parameters or the first parameter is --tests
    bool runTests = (argc == 1);
    if (!runTests && (argc >= 2 && (!strcmp(argv[1], "--tests") || !strcmp(argv[1], "-t")))) {
        runTests = true;
        argc -= 2;
        argv = &(argv[2]);
    }

    if (runTests) {
        doctest::Context context;
        context.setOption("no-breaks", false); // break in debugger
        context.applyCommandLine(argc, argv);
        return context.run();
    }

    return app.exec();
}
