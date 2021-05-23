#include "scriptrunner.h"

#include "testutil.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/fileinfo.h"
#include "core/scriptitem.h"
#include "core/settings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>

#include <QtQml/private/qqmlengine_p.h>

namespace Interface {

static constexpr int ErrorCode = -1;

// Dir singleton function provider
static QObject *dir_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)
    auto dir = new Core::Dir();
    dir->setProperty("scriptPath", engine->property("scriptPath"));
    return dir;
}

// FileInfo singleton function provider
static QObject *fileinfo_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Core::FileInfo();
}

// File Singletion function provider
static QObject *file_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Core::File();
}

ScriptRunner::ScriptRunner(QObject *parent)
    : QObject(parent)
{
    m_logger = spdlog::get("script");

    // Script objects registrations
    qRegisterMetaType<Core::QDirValueType>();
    qRegisterMetaType<Core::QFileInfoValueType>();

    // Script
    qmlRegisterSingletonType<Core::Dir>("Script", 1, 0, "Dir", dir_provider);
    qmlRegisterSingletonType<Core::FileInfo>("Script", 1, 0, "FileInfo", fileinfo_provider);
    qmlRegisterSingletonType<Core::File>("Script", 1, 0, "File", file_provider);
    qmlRegisterSingletonInstance<Core::Settings>("Script", 1, 0, "Settings", Core::Settings::instance());

    qmlRegisterType<Core::ScriptItem>("Script", 1, 0, "Script");

    // Script.Test
    qmlRegisterType<TestUtil>("Script.Test", 1, 0, "TestUtil");
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
        if (fi.suffix() == QLatin1String("js"))
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
    engine->addImportPath(QLatin1String("qrc:/qml"));

    auto logWarnings = [this](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
            m_logger->warn("{}({}): warning: {}", warning.url().toLocalFile().toStdString(), warning.line(),
                           warning.description().toStdString());
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
                return topLevel->property("failed");
            }

            return QVariant(0);
        }
    }

    // Error handling
    m_hasError = true;
    filterErrors(*component);
    engine->deleteLater();
    return QVariant(ErrorCode);
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

}
