#include <QApplication>
#include <QTimer>

#include <core/settings.h>
#include <lsp/client.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char *argv[])
{
    doctest::Context context;
    context.setOption("no-breaks", false); // break in debugger
#ifndef QT_DEBUG
    // Don't run by default in release mode
    // use --no-run=false if you want to run the tests
    context.setOption("no-run", true);
#endif
    context.applyCommandLine(argc, argv);

    QApplication app(argc, argv);

    // Run the test after starting the QApplication
    QTimer::singleShot(0, &app, [&]() {
        int res = context.run();
        if (context.shouldExit())
            QCoreApplication::exit(res);
    });

    return app.exec();
}
