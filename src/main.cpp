#include <QApplication>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cxxopts.hpp>

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

    try {
        cxxopts::Options options("Knut", "Automation tool for kdabians");

        options.add_options()("h,help", "Show this help");
        options.add_options("Tests")("t,tests", "Run internal tests, all options are then passed to doctest");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help({"", "Tests"}) << std::endl;
            exit(0);
        }
    } catch (const cxxopts::OptionException &e) {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return app.exec();
}
