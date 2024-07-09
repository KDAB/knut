/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptrunner.h"
#include "classsymbol.h"
#include "cppdocument.h"
#include "dir.h"
#include "file.h"
#include "fileinfo.h"
#include "functionsymbol.h"
#include "mark.h"
#include "message.h"
#include "project.h"
#include "qttsdocument.h"
#include "qtuidocument.h"
#include "rcdocument.h"
#include "scriptdialogitem.h"
#include "scriptitem.h"
#include "settings.h"
#include "symbol.h"
#include "testutil.h"
#include "textdocument.h"
#include "textrange.h"
#include "userdialog.h"
#include "utils.h"
#include "utils/log.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>
#include <QtQml/private/qqmlengine_p.h>
#include <kdalgorithms.h>

namespace Core {

static constexpr int NormalExitCode = 0;
static constexpr int ErrorCode = -1;

template <typename Object>
void addProperties(QSet<QString> &properties)
{
    for (int i = 0; i < Object::staticMetaObject.propertyCount(); ++i) {
        QString className = Object::staticMetaObject.className();
        properties.insert(
            QString("%1::%2").arg(className.split("::").last()).arg(Object::staticMetaObject.property(i).name()));
    }
}

ScriptRunner::ScriptRunner(QObject *parent)
    : QObject(parent)
{
    // Knut objects registrations
    qRegisterMetaType<FunctionArgument>();
    qRegisterMetaType<ClassSymbol>();
    qRegisterMetaType<FunctionSymbol>();
    qRegisterMetaType<QDirValueType>();
    qRegisterMetaType<QFileInfoValueType>();
    qRegisterMetaType<Symbol>();
    qRegisterMetaType<TextRange>();

    // Knut QML module
    qmlRegisterSingletonType<Dir>("Knut", 1, 0, "Dir", [](QQmlEngine *engine, QJSEngine *) {
        return new Dir(engine->property("scriptPath").toString());
    });
    qmlRegisterSingletonType<FileInfo>("Knut", 1, 0, "FileInfo", [](QQmlEngine *, QJSEngine *) {
        return new FileInfo();
    });
    qmlRegisterSingletonType<File>("Knut", 1, 0, "File", [](QQmlEngine *, QJSEngine *) {
        return new File();
    });
    qmlRegisterSingletonType<Message>("Knut", 1, 0, "Message", [](QQmlEngine *, QJSEngine *) {
        return new Message();
    });
    qmlRegisterSingletonType<Utils>("Knut", 1, 0, "Utils", [](QQmlEngine *, QJSEngine *) {
        return new Utils();
    });
    qmlRegisterSingletonType<UserDialog>("Knut", 1, 0, "UserDialog", [](QQmlEngine *, QJSEngine *) {
        return new UserDialog();
    });
    qmlRegisterSingletonType<Project>("Knut", 1, 0, "Project", [](QQmlEngine *engine, QJSEngine *) {
        engine->setObjectOwnership(Project::instance(), QQmlEngine::CppOwnership);
        return Project::instance();
    });
    qmlRegisterSingletonType<Settings>("Knut", 1, 0, "Settings", [](QQmlEngine *engine, QJSEngine *) {
        engine->setObjectOwnership(Settings::instance(), QQmlEngine::CppOwnership);
        return Settings::instance();
    });

    qmlRegisterUncreatableType<Document>("Knut", 1, 0, "Document", "Abstract class");
    qmlRegisterType<ScriptDialogItem>("Knut", 1, 0, "ScriptDialog");
    qmlRegisterType<ScriptItem>("Knut", 1, 0, "Script");
    qmlRegisterType<TextDocument>("Knut", 1, 0, "TextDocument");
    qmlRegisterType<QtUiDocument>("Knut", 1, 0, "QtUiDocument");
    qmlRegisterUncreatableType<QtUiWidget>("Knut", 1, 0, "QtUiWidget", "Only created by QtUiDocument");
    qmlRegisterType<CppDocument>("Knut", 1, 0, "CppDocument");
    qmlRegisterUncreatableType<Core::Symbol>("Knut", 1, 0, "Symbol", "Only created by CodeDocument");
    qmlRegisterType<RcDocument>("Knut", 1, 0, "RcDocument");
    qmlRegisterType<QtTsDocument>("Knut", 1, 0, "QtTsDocument");
    qmlRegisterUncreatableType<QtTsMessage>("Knut", 1, 0, "QtTsMessage", "Only created by QtTsDocument");

    // RcCore
    qRegisterMetaType<RcCore::Asset>();
    qRegisterMetaType<QList<RcCore::Asset>>();
    qRegisterMetaType<RcCore::ToolBarItem>();
    qRegisterMetaType<QList<RcCore::ToolBarItem>>();
    qRegisterMetaType<RcCore::ToolBar>();
    qRegisterMetaType<QList<RcCore::ToolBar>>();
    qRegisterMetaType<RcCore::Widget>();
    qRegisterMetaType<QList<RcCore::Widget>>();
    qRegisterMetaType<RcCore::MenuItem>();
    qRegisterMetaType<QList<RcCore::MenuItem>>();
    qRegisterMetaType<RcCore::Menu>();
    qRegisterMetaType<QList<RcCore::Menu>>();
    qRegisterMetaType<RcCore::Shortcut>();
    qRegisterMetaType<QList<RcCore::Shortcut>>();
    qRegisterMetaType<RcCore::Action>();
    qRegisterMetaType<QList<RcCore::Action>>();
    qRegisterMetaType<RcCore::RibbonElement>();
    qRegisterMetaType<QList<RcCore::RibbonElement>>();
    qRegisterMetaType<RcCore::RibbonPanel>();
    qRegisterMetaType<QList<RcCore::RibbonPanel>>();
    qRegisterMetaType<RcCore::RibbonCategory>();
    qRegisterMetaType<QList<RcCore::RibbonCategory>>();
    qRegisterMetaType<RcCore::RibbonContext>();
    qRegisterMetaType<QList<RcCore::RibbonContext>>();
    qRegisterMetaType<RcCore::RibbonMenu>();
    qRegisterMetaType<RcCore::Ribbon>();
    qRegisterMetaType<QList<RcCore::Ribbon>>();

    // Knut.Test
    qmlRegisterSingletonType<TestUtil>("Knut.Test", 1, 0, "TestUtil", [](QQmlEngine *, QJSEngine *) {
        return new TestUtil();
    });

    // Properties
    addProperties<FunctionArgument>(m_properties);
    addProperties<ClassSymbol>(m_properties);
    addProperties<FunctionSymbol>(m_properties);
    addProperties<QDirValueType>(m_properties);
    addProperties<QFileInfoValueType>(m_properties);
    addProperties<Symbol>(m_properties);
    addProperties<TextRange>(m_properties);
    addProperties<Dir>(m_properties);
    addProperties<FileInfo>(m_properties);
    addProperties<File>(m_properties);
    addProperties<Message>(m_properties);
    addProperties<Utils>(m_properties);
    addProperties<UserDialog>(m_properties);
    addProperties<Document>(m_properties);
    addProperties<TextDocument>(m_properties);
    addProperties<Mark>(m_properties);
    addProperties<QtUiDocument>(m_properties);
    addProperties<QtUiWidget>(m_properties);
    addProperties<CppDocument>(m_properties);
    addProperties<RcDocument>(m_properties);
    addProperties<QtTsDocument>(m_properties);
    addProperties<QtTsMessage>(m_properties);
}

ScriptRunner::~ScriptRunner() = default;

QVariant ScriptRunner::runScript(const QString &fileName, const std::function<void()> &endCallback)
{
    const QFileInfo fi(fileName);

    const auto fullName = fi.absoluteFilePath();

    m_hasError = false;

    QVariant result;
    if (fi.exists() && fi.isReadable()) {
        // TODO set the current project directory as the current path before running the script

        // Run the script
        auto engine = getEngine(fullName);
        if (endCallback)
            connect(engine, &QObject::destroyed, this, endCallback);

        if (fi.suffix() == "js") {
            result = runJavascript(fullName, engine);
        } else {
            result = runQml(fullName, engine);
        }
        // engine is deleted in runJavascript or runQml
    } else {
        spdlog::error("File {} doesn't exist", fileName);
        return QVariant(ErrorCode);
    }

    return result;
}

bool ScriptRunner::isProperty(const QString &apiCall)
{
    return m_properties.contains(apiCall);
}

QQmlEngine *ScriptRunner::getEngine(const QString &fileName)
{
    const QFileInfo fi(fileName);

    auto engine = new QQmlEngine(this);
    currentScriptPath = fi.absoluteFilePath();
    engine->setProperty("scriptPath", fi.absolutePath());
    engine->setProperty("scriptWindow", false);
    engine->addImportPath("qrc:/qml");

    auto logWarnings = [this](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
            if (warning.description().contains("error", Qt::CaseInsensitive))
                spdlog::error("{}({}): {}", warning.url().toLocalFile(), warning.line(), warning.description());
            else
                spdlog::warn("{}({}): {}", warning.url().toLocalFile(), warning.line(), warning.description());
            m_hasError = true;
        }
    };

    connect(engine, &QQmlEngine::warnings, this, logWarnings);
    engine->setOutputWarningsToStandardError(false);

    return engine;
}

QVariant ScriptRunner::runJavascript(const QString &fileName, QQmlEngine *engine)
{
    const QString text =
        QStringLiteral(
            "import QtQml\n"
            "import Knut\n"
            "import \"%1\" as MyScript\n"
            "QtObject { property var _scriptResult; Component.onCompleted : _scriptResult = MyScript.main() }")
            .arg(QUrl::fromLocalFile(fileName).toString());

    QQmlComponent component(engine);
    component.setData(text.toLatin1(), QUrl::fromLocalFile(fileName));

    auto *result = qobject_cast<QObject *>(component.create());
    engine->deleteLater();
    m_hasError = component.isError();
    if (component.isReady() && !m_hasError)
        return result->property("_scriptResult");

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
                static_cast<QObject *>(qxView)->setParent(engine);
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
            } else if (auto dialog = qobject_cast<ScriptDialogItem *>(topLevel)) {
                dialog->show();
                connect(dialog, &ScriptDialogItem::scriptFinished, engine, &QObject::deleteLater);
            }
            // Make sure calling `Qt.quit()` in QML deletes everything
            auto cleanup = [engine, topLevel]() {
                engine->deleteLater();
                if (topLevel)
                    topLevel->deleteLater();
            };
            connect(engine, &QQmlEngine::quit, engine, cleanup);

            // Start the init function if it exists.
            if (topLevel->metaObject()->indexOfMethod("init()") != -1)
                QMetaObject::invokeMethod(topLevel, "init", Qt::DirectConnection);

            // Run the test (in testMode) or run method if it exists
            QString methodName;
            if (Settings::instance()->isTesting() && topLevel->metaObject()->indexOfMethod("test()") != -1) {
                methodName = "test";
            } else if (topLevel->metaObject()->indexOfMethod("run()") != -1) {
                methodName = "run";
            }
            if (!methodName.isEmpty()) {
                QMetaObject::invokeMethod(topLevel, qPrintable(methodName), Qt::DirectConnection);
                if (m_hasError)
                    return ErrorCode;
                QVariant result = topLevel->property("failed");
                return result;
            }

            return NormalExitCode;
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
    auto isValidErrorOnLocalFile = [](const QQmlError &error) {
        return error.line() >= 0 && QFileInfo(error.url().toLocalFile()).isFile();
    };
    kdalgorithms::filter(errors, isValidErrorOnLocalFile);
    m_errors = errors;
}

} // namespace Core
