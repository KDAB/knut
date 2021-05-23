#include "core/settings.h"
#include "interface/scriptmanager.h"

#include <QApplication>
#include <QCommandLineParser>

#include <cxxopts.hpp>

std::stringstream &operator>>(std::stringstream &out, QString &s)
{
    std::string text;
    out >> text;
    s = QString::fromStdString(text);
    return out;
}

class OptionParser
{
public:
    OptionParser(int argc, char *argv[])
    {
        m_hasTests = (argc == 1);
        for (int i = 0; i < argc; ++i) {
            if (strncmp(argv[i], "--dt-", strlen("--dt-")) == 0) {
                m_hasTests = true;
                return;
            }
        }

        try {
            cxxopts::Options options("knut", "Automation tool for kdabians");

            // clang-format off
            options.add_options()
                    ("h,help", "Display available options")
                    ("v,version", "Display the version of this program")
                    ("s,script", "Run given script then exit", cxxopts::value<QString>())
                    ("r,root", "Root directory of the project", cxxopts::value<QString>());
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
                m_scriptName = result["script"].as<QString>();
            if (result.count("root"))
                m_rootDir = result["root"].as<QString>();

        } catch (const cxxopts::OptionException &e) {
            std::cout << "Error parsing options: " << e.what() << std::endl;
            exit(1);
        }
    }

    bool hasTests() const { return m_hasTests; }
    QString scriptName() const { return m_scriptName; }
    QString rootDir() const { return m_rootDir; }

private:
    bool m_hasTests = false;
    QString m_scriptName;
    QString m_rootDir;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(KNUT_VERSION);

    Q_INIT_RESOURCE(core);
    Q_INIT_RESOURCE(interface);

    OptionParser options(argc, argv);

    if (!options.rootDir().isEmpty())
        Core::Settings::instance()->loadProjectSettings(options.rootDir());

    if (!options.scriptName().isEmpty()) {
        auto sm = Interface::ScriptManager::instance();
        QObject::connect(sm, &Interface::ScriptManager::scriptFinished, &app, [](const QVariant &result) {
            QApplication::exit(result.toInt());
        });
        sm->runScript(options.scriptName());
    }

    return app.exec();
}
