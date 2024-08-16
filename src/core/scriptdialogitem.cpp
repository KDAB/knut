/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptdialogitem.h"
#include "loghighlighter.h"
#include "scriptdialogitem_p.h"
#include "scriptprogressdialog.h"
#include "scriptrunner.h"
#include "settings.h"
#include "utils/log.h"

#include <definition.h>
#include <repository.h>
#include <syntaxhighlighter.h>
#include <theme.h>

#include <QAbstractItemModel>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QQmlContext>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolButton>
#include <QUiLoader>
#include <QVBoxLayout>
#include <spdlog/sinks/qt_sinks.h>

namespace Core {

/*!
 * \qmltype ScriptDialog
 * \brief QML Item for writing visual scripts.
 * \ingroup Items
 *
 * The `ScriptDialog` allows creating a script dialog based on a ui file. It requires creating a ui file with the same
 * name as the qml script.
 *
 * Inside the dialog, all widget's main property is mapped to a property inside the data property, using the same
 * name as the `objectName`. For example, the text of a `QLineEdit` with `objectName` set to `lineEdit` will be mapped
 * to `data.lineEdit`.
 *
 * Buttons (`QPushButton` or `QToolButton`) `clicked` signal is available through the `onClikced` signal handler, with
 * the button `objectName` as parameter.
 *
 * `QDialogButtonBox` `accepted` or `rejected` signals are
 * automatically connected and available through the `onAccepted` and `onRejected` signal handlers.
 *
 * ```qml
 * import Knut
 *
 * ScriptDialog {
 *     property string text1: data.lineEdit
 *     property string text2: data.comboBox
 *     property int number: data.spinBox
 *     property int number2: data.doubleSpinBox
 *     property bool check: data.checkBox
 *     property bool check2: data.radioButton
 *     onClicked: (name) => {
 *         if (name == "pushButton" || name == "toolButton")
 *             console.log(name)
 *     }
 *     onAccepted: {
 *         console.log("Accepted")
 *     }
 *     onRejected: {
 *         console.log("Rejected")
 *     }
 * }
 * ```
 *
 * You can also integrate unit-tests as part of the script dialog. This is done by adding methods prefixed with `test_`.
 * Those methods will be executed when the script is run in test mode (using `--test` command line option). If a test is
 * failing, the script will show a critical log message, and the script will return a non-zero exit code.
 *
 * ```qml
 * import Knut
 *
 * ScriptDialog {
 *     function test_foo() {
 *         compare(1, 2, "test should failed")
 *     }
 *     function test_bar() {
 *         verify(1 == 1, "test should pass")
 *     }
 * }
 * ```
 */

/*!
 * \qmlproperty var ScriptDialog::data
 * This read-only property contains all properties mapping the widgets.
 *
 * Use `data.objectName` to access the main property of a widget (the text for a `QLineEdit` for example).
 */

/*!
 * \qmlproperty bool ScriptDialog::interactive
 * If set to false, `runSteps` will not ask for user input, the entire script will be run at once.
 * This is especially useful for testing.
 */

/*!
 * \qmlproperty int ScriptDialog::stepCount
 * Number of steps to display in the progress bar.
 */

/*!
 * \qmlsignal ScriptDialog::onClicked(string name)
 * This handler is called when a button is cliked, the `name` is the name of the button.
 */

/*!
 * \qmlsignal ScriptDialog::onAccepted()
 * This handler is called when a button with an accept role from a `QDialogButtonBox` is pressed (usually the OK
 * button).
 */

/*!
 * \qmlsignal ScriptDialog::onRejected()
 * This handler is called when a button with a reject role from a `QDialogButtonBox` is pressed (usually the Cancel
 * button).
 */

ScriptDialogItem::ScriptDialogItem(QWidget *parent)
    : QDialog(parent)
    , m_data(new DynamicObject(this))
{
    setAttribute(Qt::WA_DeleteOnClose, false);

    Q_ASSERT(!ScriptRunner::currentScriptPath.isEmpty());
    QFileInfo fi(ScriptRunner::currentScriptPath);
    setUiFile(fi.absolutePath() + '/' + fi.baseName() + ".ui");

    m_data->registerDataChangedCallback([this](const QString &key, const QVariant &value) {
        changeValue(key, value);
    });
}

void ScriptDialogItem::initialize(nlohmann::json &&jsonData)
{
    for (auto it = jsonData.cbegin(); it != jsonData.cend(); ++it) {
        const QString key = QString::fromStdString(it.key());

        QVariant value;
        if (it.value().is_string()) {
            value = QString::fromStdString(it.value().get<std::string>());
        } else if (it.value().is_boolean()) {
            value = it.value().get<bool>();
        } else if (it.value().is_number_integer()) {
            value = it.value().get<int>();
        } else if (it.value().is_number_float()) {
            value = it.value().get<double>();
        } else {
            spdlog::error("{}: Unsupported data type for key '{}'", FUNCTION_NAME, it.key());
            value = QString::fromStdString(it.value().dump());
        }

        changeValue(key, value);
    }
}

void ScriptDialogItem::done(int code)
{
    // When showing progress, the `onAccepted` handler will likely call `QCoreApplication::processEvents` multiple
    // times. This will cause other slots that are connected to `finished` to be evaluated as well. Therefore we use the
    // additional `scriptFinished` signal for cleanup. This signal is only emitted once all handlers of
    // `QDialog::done` have finished.
    QDialog::done(code);
    if (!m_stepGenerator.has_value()) {
        finishScript();
    }
}

bool ScriptDialogItem::isInteractive() const
{
    return m_interactive && !Settings::instance()->isTesting();
}

void ScriptDialogItem::setInteractive(bool interactive)
{
    if (m_interactive != interactive) {
        m_interactive = interactive;
        emit interactiveChanged(isInteractive());
    }
}

int ScriptDialogItem::stepCount() const
{
    return m_stepCount;
}

/**
 * \qmlmethod ScriptDialog::setStepCount(int stepCount)
 *
 * Sets the number of steps to show in the progress bar.
 *
 * By default the value is 0, meaning there are no steps set. This will show an indeterminate progress bar. You can use
 * the `stepCount` property to set the number of steps too.
 */
void ScriptDialogItem::setStepCount(int stepCount)
{
    if (m_stepCount == stepCount)
        return;
    m_stepCount = stepCount;
    if (m_progressDialog) {
        m_progressDialog->setMaximum(stepCount);
    }
    emit stepCountChanged(m_stepCount);
}

/**
 * \qmlmethod ScriptDialog::firstStep(string firstStep)
 *
 * Starts a progress bar with the given `firstStep` title.
 *
 * The number of following `nextStep` calls (or yield calls if using runSteps) should be one less than the number of
 * steps set here.
 */
void ScriptDialogItem::firstStep(const QString &firstStep)
{
    showProgressDialog();
    m_currentStep = 0;
    nextStep(firstStep);
    updateProgress();
}

/**
 * \qmlmethod ScriptDialog::nextStep(string title)
 *
 * Changes the progression to a new progress step.
 *
 * This will update the progress bar and the title of the progress dialog.
 * Make sure that the number of steps is set correctly before calling this method.
 * \sa startProgress
 */
void ScriptDialogItem::nextStep(const QString &title)
{
    // Only update the number after updating the title & progress bar.
    // Otherwise the first call to `nextStep` would already update the progress bar to 1.
    // And we'd have the progress bar show 100% when the last step started, not when it finished.
    //
    // Also, never allow the progress bar to actually reach 100%.
    // That always looks silly. "Hey I'm 100% done, but still not finished!"
    //
    // This is likely just caused by a script that's indicating too few progress steps.
    // So just increase the maximum.
    if (!title.isEmpty()) {
        m_currentStepTitle = title;
    }

    if (m_stepCount != 0 && m_currentStep >= m_stepCount) {
        setStepCount(m_stepCount + 1);
    }

    if (m_progressDialog) {
        const auto title = m_stepCount == 0
            ? m_currentStepTitle
            : QStringLiteral("%1 (%2/%3)").arg(m_currentStepTitle).arg(m_currentStep + 1).arg(m_stepCount);
        m_progressDialog->setTitle(title);
        m_progressDialog->setValue(m_currentStep);
    }
    ++m_currentStep;
}

void ScriptDialogItem::continueScript()
{
    if (m_progressDialog)
        m_progressDialog->setReadOnly(true);
    nextStep(m_nextStepTitle);
    runNextStep();
}

void ScriptDialogItem::abortScript()
{
    spdlog::info("{}: Script aborted.", FUNCTION_NAME);
    finishScript();
}

void ScriptDialogItem::finishScript()
{
    m_stepGenerator.reset();
    cleanupProgressDialog();
    emit scriptFinished();
}

void ScriptDialogItem::runNextStep()
{
    showProgressDialog();

    const auto result = m_stepGenerator->property("next").callWithInstance(m_stepGenerator.value());
    const auto done = result.property("done").toBool();
    m_nextStepTitle = result.property("value").toString();

    spdlog::info("{}: {} done.", FUNCTION_NAME, m_currentStepTitle);

    if (done) {
        finishScript();
        return;
    }

    if (!isInteractive()) {
        continueScript();
        return;
    }

    const auto title = QString("%1 (DONE)").arg(m_currentStepTitle);

    m_progressDialog->setTitle(title);
    m_progressDialog->setValue(m_currentStep);
    // Make sure it is displayed in front of the other widgets
    m_progressDialog->raise();
    m_progressDialog->setReadOnly(false);
}

static bool isGenerator(const QJSValue &generator)
{
    return generator.property("next").isCallable();
}

/**
 * \qmlmethod ScriptDialog::runSteps(function generator)
 *
 * Runs a script in multiple (interactive) steps.
 *
 * The argument to this function must be a JavaScript generator object
 * ([See this documentation on JS
 * Generators](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator)).
 *
 * The generator should yield a string with the next step title, whenever the user should be able to pause the script
 * and inspect the changes. This will behave the same as calling `nextStep`, but pauses the script, until the user
 * continues or aborts the script.
 *
 * You can also mix and match between `yield` and `nextStep` calls.
 *
 * For the best experience, we recommend to use `setStepCount`, `firstStep` and `yield` to indicate the remaining
 * progress.
 *
 * Example:
 * ```javascript
 * function *conversionSteps() {
 *    setStepCount(2)            // <--- Initialize the number of steps
 *    firstStep("Adding member") // <--- Start the first step
 *    document.addMember("test", "int", CppDocument.Public)
 *
 *    yield "Inserting include"  // <--- The user can check that the member was inserted correctly
 *    document.insertInclude("<iostream>")
 * }
 *
 * function convert() {
 *    runSteps(conversionSteps())
 * }
 * ```
 */
void ScriptDialogItem::runSteps(const QJSValue &generator)
{
    if (!isGenerator(generator)) {
        spdlog::error("ScriptDialogItem::runSteps: Argument is not a generator!\n"
                      "Make sure to create a generator function using function* myGenerator() { ... } and call the "
                      "generator with e.g. runSteps(myGenerator())!\n"
                      "See also:"
                      "https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator");
        return;
    }

    m_stepGenerator = generator;
    runNextStep();
}

/*!
 * \qmlmethod ScriptDialog::compare(var actual, var expected, string message = {})
 * Compare `actual` with `expected` and log an error `message` if they are not equal.
 *
 * This method will increment the number of failed test internally, and when the script is finished, the test runner
 * will return the number of failed tests.
 *
 * This is usually used in tests method like that:
 *
 * ```qml
 * ScriptDialog {
 *     function test_foo() {
 *         compare(1, 2, "1 should be equal to 2"); // This will log an error
 *     }
 * }
 * ```
 */
void ScriptDialogItem::compare(const QJSValue &actual, const QJSValue &expected, QString message)
{
    ScriptRunner::compare(this, actual, expected, message);
}

/*!
 * \qmlmethod ScriptDialog::verify(bool value, string message = {})
 * Compare `actual` with `expected` and log an error `message` if they are not equal.
 *
 * This method will increment the number of failed test internally, and when the script is finished, the test runner
 * will return the number of failed tests.
 *
 * This is usually used in tests method like that:
 *
 * ```qml
 * ScriptDialog {
 *     function test_foo() {
 *         let answerToEverything = 42;
 *         verify(answerToEverything == 42, "What else?");
 *     }
 * }
 * ```
 */
void ScriptDialogItem::verify(bool value, QString message)
{
    ScriptRunner::verify(this, value, message);
}

/*!
 * \qmlmethod ScriptDialog::setDisplayLogs(const QString &level)
 * This method enables real-time logging display in the progress dialog by configuring a QPlainTextEdit widget as a log
 * output panel. It filters the logs to show only those that are of the specified log level or higher.
 */
void ScriptDialogItem::setDisplayLogs(const QString &level)
{
    showProgressDialog();

    if (!m_progressDialog)
        return;

    QPlainTextEdit *logPanel = m_progressDialog->logsWidget();
    logPanel->show();
    logPanel->setReadOnly(true);

    new Core::LogHighlighter(logPanel->document());

    const std::vector<QString> levels = {"trace", "debug", "info", "warn", "error", "critical"};
    const auto it = std::find(levels.begin(), levels.end(), level);
    const int index = (it != levels.end()) ? std::distance(levels.begin(), it) : 0;

    spdlog::level::level_enum logLevel = static_cast<spdlog::level::level_enum>(index);

    auto logger = spdlog::default_logger();

    auto currentLogLevel = logger->level();
    auto logLevelMin = static_cast<spdlog::level::level_enum>(std::min(static_cast<int>(currentLogLevel), index));

    logger->set_level(logLevelMin);

    auto sink = std::make_shared<spdlog::sinks::qt_sink_mt>(logPanel, "appendPlainText");
    sink->set_level(logLevel);
    logger->sinks().push_back(sink);
}

void ScriptDialogItem::showProgressDialog()
{
    // If there's no interaction or the progress bar is already displayed, do nothing
    if (!isInteractive() || m_progressDialog)
        return;

    m_progressDialog = new ScriptProgressDialog();
    m_progressDialog->setInteractive(m_stepGenerator.has_value());
    m_progressDialog->setWindowTitle(windowTitle());
    // Using min,max,value of 0 causes an undetermined progress bar
    // As we don't know how long a script may take without the script telling us, this is the default.
    m_progressDialog->setMinimum(0);
    m_progressDialog->setMaximum(m_stepCount);
    m_progressDialog->setValue(m_currentStep);
    m_progressDialog->setReadOnly(true);

    connect(m_progressDialog, &ScriptProgressDialog::apply, this, &ScriptDialogItem::continueScript);
    connect(m_progressDialog, &ScriptProgressDialog::abort, this, &ScriptDialogItem::abortScript);

    m_progressDialogs.push_back(m_progressDialog);
    m_progressDialog->show();
    updateProgress();
}

void ScriptDialogItem::cleanupProgressDialog()
{
    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialogs.removeAll(m_progressDialog);
        m_progressDialog = nullptr;
    }
}

void ScriptDialogItem::updateProgress()
{
    if (!m_progressDialogs.empty()) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

QObject *ScriptDialogItem::data() const
{
    return m_data;
}

QQmlListProperty<QObject> ScriptDialogItem::childrenData()
{
    return QQmlListProperty<QObject>(this, this, &ScriptDialogItem::appendChild, &ScriptDialogItem::countChildren,
                                     &ScriptDialogItem::atChild, &ScriptDialogItem::clearChildren);
}

void ScriptDialogItem::setUiFile(const QString &fileName)
{
    QUiLoader loader;
    QFile file(fileName);

    if (file.open(QFile::ReadOnly)) {
        QWidget *internalWidget = loader.load(&file, this);
        file.close();
        auto layout = new QVBoxLayout;
        layout->addWidget(internalWidget);
        setLayout(layout);
        setWindowTitle(internalWidget->windowTitle());
        createProperties(internalWidget);
    } else {
        spdlog::error("{}: Can't open {}", FUNCTION_NAME, fileName);
    }
}

void ScriptDialogItem::createProperties(QWidget *dialogWidget)
{
    const auto widgets = dialogWidget->findChildren<QWidget *>();
    for (auto widget : widgets) {
        if (auto dialogButtonBox = qobject_cast<QDialogButtonBox *>(widget)) {
            connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &ScriptDialogItem::accept);
            connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &ScriptDialogItem::reject);
        } else if (auto pushButton = qobject_cast<QPushButton *>(widget)) {
            connect(pushButton, &QPushButton::clicked, this, [this, pushButton]() {
                emit clicked(pushButton->objectName());
            });
        } else if (auto toolButton = qobject_cast<QToolButton *>(widget)) {
            connect(toolButton, &QToolButton::clicked, this, [this, toolButton]() {
                emit clicked(toolButton->objectName());
            });
        } else if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "QString", QMetaType::QString, lineEdit->text());
            connect(lineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), text);
            });
        } else if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "bool", QMetaType::Bool, checkBox->isChecked());
            connect(checkBox, &QCheckBox::toggled, this, [this](bool value) {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), value);
            });
        } else if (auto radioButton = qobject_cast<QRadioButton *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "bool", QMetaType::Bool, radioButton->isChecked());
            connect(radioButton, &QCheckBox::toggled, this, [this](bool value) {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), value);
            });
        } else if (auto spinBox = qobject_cast<QSpinBox *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "int", QMetaType::Int, spinBox->value());
            connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), value);
            });
        } else if (auto doubleSpinBox = qobject_cast<QDoubleSpinBox *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "double", QMetaType::Double,
                                doubleSpinBox->value());
            connect(doubleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value) {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), value);
            });
        } else if (auto comboBox = qobject_cast<QComboBox *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "QString", QMetaType::QString,
                                comboBox->currentText());
            connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, comboBox]() {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), comboBox->currentText());
            });
            auto model = comboBox->model();
            QStringList itemList;
            for (int i = 0; i < model->rowCount(); ++i)
                itemList.push_back(model->data(model->index(i, 0)).toString());
            m_data->addProperty((widget->objectName() + "Model").toLocal8Bit(), "QStringList", QMetaType::QStringList,
                                itemList);
            if (comboBox->isEditable()) {
                auto completer = new QCompleter(comboBox);
                completer->setFilterMode(Qt::MatchContains);
                completer->setModel(comboBox->model());
                comboBox->setCompleter(completer);
            }
        } else if (auto textEdit = qobject_cast<QTextEdit *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "QString", QMetaType::QString,
                                textEdit->toPlainText());
            connect(textEdit, &QTextEdit::textChanged, this, [this, textEdit]() {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), textEdit->toPlainText());
            });
            m_data->addProperty((widget->objectName() + "Syntax").toLocal8Bit(), "QString", QMetaType::QString,
                                QString());
        } else if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(widget)) {
            m_data->addProperty(widget->objectName().toLocal8Bit(), "QString", QMetaType::QString,
                                plainTextEdit->toPlainText());
            connect(plainTextEdit, &QPlainTextEdit::textChanged, this, [this, plainTextEdit]() {
                m_data->setProperty(sender()->objectName().toLocal8Bit(), plainTextEdit->toPlainText());
            });
            m_data->addProperty((widget->objectName() + "Syntax").toLocal8Bit(), "QString", QMetaType::QString,
                                QString());
        }
    }

    m_data->ready();
}

void ScriptDialogItem::changeValue(const QString &key, const QVariant &value)
{
    auto widget = findChild<QWidget *>(key);

    if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
        lineEdit->setText(value.toString());
        return;
    } else if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
        checkBox->setChecked(value.toBool());
        return;
    } else if (auto radioButton = qobject_cast<QRadioButton *>(widget)) {
        radioButton->setChecked(value.toBool());
        return;
    } else if (auto spinBox = qobject_cast<QSpinBox *>(widget)) {
        spinBox->setValue(value.toInt());
        return;
    } else if (auto doubleSpinBox = qobject_cast<QDoubleSpinBox *>(widget)) {
        doubleSpinBox->setValue(value.toDouble());
        return;
    } else if (auto comboBox = qobject_cast<QComboBox *>(widget)) {
        comboBox->setCurrentText(value.toString());
        return;
    } else if (auto textEdit = qobject_cast<QTextEdit *>(widget)) {
        QString valueString = value.toString();
        if (textEdit->toPlainText() != valueString)
            textEdit->setPlainText(value.toString());
        return;
    } else if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(widget)) {
        QString valueString = value.toString();
        if (plainTextEdit->toPlainText() != valueString)
            plainTextEdit->setPlainText(value.toString());
        return;
    }

    // It may be a combobox model
    if (key.endsWith("Model")) {
        if (auto comboBox = findChild<QComboBox *>(key.left(key.length() - 5))) {
            comboBox->clear();
            comboBox->addItems(value.toStringList());
            if (comboBox->count())
                comboBox->setCurrentIndex(0);
            return;
        }
    }

    // It may be a syntax rule
    if (key.endsWith("Syntax")) {
        if (auto textEdit = findChild<QTextEdit *>(key.left(key.length() - 6))) {
            applySyntaxHighlighting(textEdit->document(), value.toString());
            return;
        } else if (auto plainTextEdit = findChild<QPlainTextEdit *>(key.left(key.length() - 6))) {
            applySyntaxHighlighting(plainTextEdit->document(), value.toString());
            return;
        }
    }

    if (!widget) {
        spdlog::warn("{}: No widget found for the key '{}'.", FUNCTION_NAME, key.toStdString());
    } else {
        spdlog::warn("{}: Unsupported widget type '{}' for the key '{}'.", FUNCTION_NAME,
                     widget->metaObject()->className(), key.toStdString());
    }
}

void ScriptDialogItem::applySyntaxHighlighting(QTextDocument *document, const QString &syntax)
{
    if (!syntax.isEmpty()) {
        auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(document);
        static KSyntaxHighlighting::Repository repository;
        highlighter->setTheme(repository.themeForPalette(QApplication::palette()));
        const auto definition = repository.definitionForFileName("." + syntax);
        if (!definition.isValid()) {
            qWarning() << "No valid syntax definition for file extension" << syntax;
            delete highlighter;
            return;
        }
        highlighter->setDefinition(definition);
    }
}

void ScriptDialogItem::appendChild(QQmlListProperty<QObject> *list, QObject *obj)
{
    if (auto that = qobject_cast<ScriptDialogItem *>(list->object)) {
        obj->setParent(that);
        that->m_children.push_back(obj);
        emit that->childrenDataChanged();
    }
}

QObject *ScriptDialogItem::atChild(QQmlListProperty<QObject> *list, qsizetype index)
{
    if (auto that = qobject_cast<ScriptDialogItem *>(list->object))
        return that->m_children.at(index);
    return nullptr;
}

qsizetype ScriptDialogItem::countChildren(QQmlListProperty<QObject> *list)
{
    if (auto that = qobject_cast<ScriptDialogItem *>(list->object))
        return static_cast<int>(that->m_children.size());
    return 0;
}

void ScriptDialogItem::clearChildren(QQmlListProperty<QObject> *list)
{
    if (auto that = qobject_cast<ScriptDialogItem *>(list->object)) {
        that->m_children.clear();
        emit that->childrenDataChanged();
    }
}

} // namespace Core
