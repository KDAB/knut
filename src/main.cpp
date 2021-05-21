#include "script/scriptmanager.h"

#include <QApplication>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cxxopts.hpp>

class OptionParser
{
public:
    OptionParser(int argc, char *argv[])
    {
        m_hasTests = (argc == 1);
        if (!m_hasTests && (argc >= 2 && (!strcmp(argv[1], "--tests") || !strcmp(argv[1], "-t")))) {
            m_hasTests = true;
            return;
        }

        try {
            cxxopts::Options options("knut", "Automation tool for kdabians");

            // clang-format off
            options.add_options()
                    ("h,help", "Display available options")
                    ("v,version", "Display the version of this program")
                    ("s,script", "Run given script then exit", cxxopts::value<std::string>());
            options.add_options("Tests")("t,tests", "Run internal tests, all options are then passed to doctest");
            // clang-format on

            auto result = options.parse(argc, argv);

            if (result.count("help")) {
                std::cout << options.help({"", "Tests"}) << std::endl;
                exit(0);
            }
            if (result.count("version")) {
                std::cout << "knut version " KNUT_VERSION << std::endl;
                exit(0);
            }

            if (result.count("script"))
                m_scriptName = QString::fromStdString(result["script"].as<std::string>());

        } catch (const cxxopts::OptionException &e) {
            std::cout << "Error parsing options: " << e.what() << std::endl;
            exit(1);
        }
    }

    bool hasTests() const { return m_hasTests; }
    QString scriptName() const { return m_scriptName; }

private:
    bool m_hasTests = false;
    QString m_scriptName;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("Knut");
    QApplication::setApplicationVersion(KNUT_VERSION);

    OptionParser options(argc, argv);

    if (options.hasTests()) {
        // First remove -t option
        if (argc > 1) {
            argc -= 2;
            argv = &(argv[2]);
        }
        doctest::Context context;
        context.setOption("no-breaks", false); // break in debugger
        context.applyCommandLine(argc, argv);
        return context.run();
    }

    if (!options.scriptName().isEmpty()) {
        auto sm = Script::ScriptManager::instance();
        QObject::connect(sm, &Script::ScriptManager::scriptFinished, &app, [](const QVariant &result) {
            QApplication::exit(result.toInt());
        });
        sm->runScript(options.scriptName());
    }

    return app.exec();
}
