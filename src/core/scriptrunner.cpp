#include "scriptrunner.h"

#include "cppdocument.h"
#include "dir.h"
#include "file.h"
#include "fileinfo.h"
#include "message.h"
#include "project.h"
#include "rcdocument.h"
#include "scriptitem.h"
#include "settings.h"
#include "testutil.h"
#include "textdocument.h"
#include "uidocument.h"
#include "userdialog.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>

#include <QtQml/private/qqmlengine_p.h>

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Core {

static constexpr int ErrorCode = -1;

ScriptRunner::ScriptRunner(QObject *parent)
    : QObject(parent)
{
    m_logger = spdlog::get("script");
    if (!m_logger) {
        m_logger = spdlog::stdout_color_mt("script");
        m_logger->set_level(spdlog::level::debug);
    }

    // Script objects registrations
    qRegisterMetaType<QDirValueType>();
    qRegisterMetaType<QFileInfoValueType>();
    qRegisterMetaType<RcCore::Asset>();
    qRegisterMetaType<QVector<RcCore::Asset>>();
    qRegisterMetaType<RcCore::ToolBarItem>();
    qRegisterMetaType<QVector<RcCore::ToolBarItem>>();
    qRegisterMetaType<RcCore::ToolBar>();
    qRegisterMetaType<QVector<RcCore::ToolBar>>();
    qRegisterMetaType<RcCore::Widget>();
    qRegisterMetaType<QVector<RcCore::Widget>>();
    qRegisterMetaType<RcCore::MenuItem>();
    qRegisterMetaType<QVector<RcCore::MenuItem>>();
    qRegisterMetaType<RcCore::Menu>();
    qRegisterMetaType<QVector<RcCore::Menu>>();
    qRegisterMetaType<RcCore::Shortcut>();
    qRegisterMetaType<QVector<RcCore::Shortcut>>();
    qRegisterMetaType<RcCore::Action>();
    qRegisterMetaType<QVector<RcCore::Action>>();

    // Script
    qmlRegisterSingletonType<Dir>("Script", 1, 0, "Dir", [](QQmlEngine *engine, QJSEngine *) {
        return new Dir(engine->property("scriptPath").toString());
    });
    qmlRegisterSingletonType<FileInfo>("Script", 1, 0, "FileInfo", [](QQmlEngine *, QJSEngine *) {
        return new FileInfo();
    });
    qmlRegisterSingletonType<File>("Script", 1, 0, "File", [](QQmlEngine *, QJSEngine *) {
        return new File();
    });
    qmlRegisterSingletonType<Message>("Script", 1, 0, "Message", [](QQmlEngine *, QJSEngine *) {
        return new Message();
    });
    qmlRegisterSingletonType<Utils>("Script", 1, 0, "Utils", [](QQmlEngine *, QJSEngine *) {
        return new Utils();
    });
    qmlRegisterSingletonType<UserDialog>("Script", 1, 0, "UserDialog", [](QQmlEngine *, QJSEngine *) {
        return new UserDialog();
    });

    qmlRegisterUncreatableType<Document>("Script", 1, 0, "Document", "Abstract class");
    qmlRegisterType<ScriptItem>("Script", 1, 0, "Script");
    qmlRegisterType<TextDocument>("Script", 1, 0, "TextDocument");
    qmlRegisterType<UiDocument>("Script", 1, 0, "UiDocument");
    qmlRegisterUncreatableType<UiWidget>("Script", 1, 0, "UiWidget", "Only created by UiDocument");
    qmlRegisterType<CppDocument>("Script", 1, 0, "CppDocument");
    qmlRegisterType<RcDocument>("Script", 1, 0, "RcDocument");

    // Script.Test
    qmlRegisterSingletonType<TestUtil>("Script.Test", 1, 0, "TestUtil", [](QQmlEngine *, QJSEngine *) {
        return new TestUtil();
    });
}

ScriptRunner::~ScriptRunner() { }

QVariant ScriptRunner::runScript(const QString &fileName, std::function<void()> endCallback)
{
    QFileInfo fi(fileName);

    auto fullName = fi.absoluteFilePath();

    m_hasError = false;

    QVariant result;
    if (fi.exists() && fi.isReadable()) {
        // TODO set the current project directory as the current path before running the script

        // Run the script
        auto engine = getEngine(fullName);
        if (endCallback)
            connect(engine, &QObject::destroyed, this, endCallback);
        if (fi.suffix() == "js")
            result = runJavascript(fullName, engine);
        else
            result = runQml(fullName, engine);
        // engine is deleted in runJavascript or runQml
    } else {
        m_logger->error("File {} doesn't exist", fileName.toStdString());
        return QVariant(ErrorCode);
    }

    return result;
}

QQmlEngine *ScriptRunner::getEngine(const QString &fileName)
{
    QFileInfo fi(fileName);

    auto engine = new QQmlEngine(this);
    engine->setProperty("scriptPath", fi.absolutePath());
    engine->setProperty("scriptWindow", false);
    engine->addImportPath("qrc:/qml");

    engine->rootContext()->setContextProperty("Settings", Settings::instance());
    engine->rootContext()->setContextProperty("Project", Project::instance());

    auto logWarnings = [this](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
            m_logger->warn("{}({}): {}", QDir::toNativeSeparators(warning.url().toLocalFile()).toStdString(),
                           warning.line(), warning.description().toStdString());
            m_hasError = true;
        }
    };

    connect(engine, &QQmlEngine::warnings, this, logWarnings);
    engine->setOutputWarningsToStandardError(false);

    return engine;
}

QVariant ScriptRunner::runJavascript(const QString &fileName, QQmlEngine *engine)
{
    const QString text = QStringLiteral("import QtQml 2.12\n"
                                        "import Script 1.0\n"
                                        "import \"%1\" as MyScript\n"
                                        "QtObject { property var result: MyScript.main() }")
                             .arg(QUrl::fromLocalFile(fileName).toString());

    QQmlComponent component(engine);
    component.setData(text.toLatin1(), QUrl::fromLocalFile(fileName));

    auto *result = qobject_cast<QObject *>(component.create());
    engine->deleteLater();
    m_hasError = component.isError();
    if (component.isReady() && !m_hasError)
        return result->property("result");

    filterErrors(component);
    return QVariant(ErrorCode);
}

QVariant ScriptRunner::runQml(const QString &fileName, QQmlEngine *engine)
{
    auto component = new QQmlComponent(engine, engine);
    component->loadUrl(QUrl::fromLocalFile(fileName));

    if (component->isReady()) {
        QObject *topLevel = component->create();
        if (topLevel && !component->isError()) {
            auto window = qobject_cast<QQuickWindow *>(topLevel);
            if (window) {
                engine->setIncubationController(window->incubationController());
            } else if (auto contentItem = qobject_cast<QQuickItem *>(topLevel)) {
                auto qxView = new QQuickView(engine, nullptr);
                window = qxView;
                qxView->setResizeMode(QQuickView::SizeViewToRootObject);
                qxView->setContent(fileName, component, contentItem);
            }

            if (window) {
                engine->setProperty("scriptWindow", true);
                // Fix window flags unless set by QML
                if (window->flags() == Qt::Window)
                    window->setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint
                                     | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint
                                     | Qt::WindowFullscreenButtonHint);

                // Delete on close, and clean up the engine
                connect(window, SIGNAL(closing(QQuickCloseEvent *)), engine,
                        SLOT(deleteLater())); // clazy:excludeall=connect-not-normalized

                window->show();
            } else {
                // Handle quit for non-gui qml, and make sure everything is deleted
                topLevel->setParent(engine);
                connect(engine, &QQmlEngine::quit, engine, &QObject::deleteLater);
            }

            // Start the init function if it exists.
            if (topLevel->metaObject()->indexOfMethod("init()") != -1)
                QMetaObject::invokeMethod(topLevel, "init", Qt::QueuedConnection);

            // Run tests if any
            if (topLevel->metaObject()->indexOfMethod("runTests()") != -1) {
                QMetaObject::invokeMethod(topLevel, "runTests", Qt::DirectConnection);
                if (m_hasError)
                    return ErrorCode;
                QVariant result = topLevel->property("failed");
                delete engine;
                return result;
            }

            return QVariant(0);
        }
    }

    // Error handling
    m_hasError = true;
    filterErrors(*component);
    engine->deleteLater();
    return ErrorCode;
}

void ScriptRunner::filterErrors(const QQmlComponent &component)
{
    auto errors = component.errors();
    auto it = errors.begin();
    while (it != errors.end()) {
        if (it->line() < 0) {
            it = errors.erase(it);
        } else {
            QFileInfo fi(it->url().toLocalFile());
            if (!fi.isFile())
                it = errors.erase(it);
            else
                ++it;
        }
    }
    m_errors = errors;
}

} // namespace Core
