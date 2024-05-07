#include "scriptdialogitem.h"
#include "scriptdialogitem_p.h"

#include "scriptrunner.h"
#include "settings.h"

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QQmlContext>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>
#include <QUiLoader>
#include <QVBoxLayout>

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype ScriptDialog
 * \brief QML Item for writing visual scripts.
 * \inqmlmodule Script
 * \ingroup Items
 * \since 1.0
 *
 * The `ScriptDialog` allows creating a script dialog based on a ui file. It requires creating a ui file with the same
 * name as the qml script.
 *
 * Widget's main properties are mapped to a property inside the data property, using the same name as the `objectName`.
 * Buttons (`QPushButton` or `QToolButton`) `clicked` signal is mapped to the `clicked` signal of this class, with the
 * button `objectName` as parameter. `QDialogButtonBox` `accepted` or `rejected` signals are automatically connected.
 *
 * ```qml
 * import Script 1.0
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
 * }
 * ```
 */

/*!
 * \qmlproperty QQmlPropertyMap ScriptDialog::data
 * This read-only property contains all properties mapping the widgets.
 */

/*!
 * \qmlproperty bool ScriptDialog::showProgress
 *
 * If set to true, a progress dialog will be shown when the dialog is accepted.
 * This is useful for long-running scripts.
 */

/*!
 * \qmlproperty bool ScriptDialog::interactive
 *
 * If set to false, runSteps will not ask for user input, the entire script will be run at once.
 * This is especially useful for testing.
 */

/*!
 * \qmlsignal ScriptDialog::clicked(string name)
 * This handler is called when a button is cliked, the `name` is the name of the button.
 */

/*!
 * \qmlsignal ScriptDialog::accepted()
 * This handler is called when a button with an accept role from a `QDialogButtonBox` is pressed (usually the OK
 * button).
 */

/*!
 * \qmlsignal ScriptDialog::rejected()
 * This handler is called when a button with a reject role from a `QDialogButtonBox` is pressed (usually the Cancel
 * button).
 */

ScriptDialogItem::ScriptDialogItem(QWidget *parent)
    : QDialog(parent)
    , m_data(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose, false);

    // The ui File by default is determined from the script path that the ScriptRunner is currently executing.
    // However, if this ScriptDialog is created from another QML file, this will not be correct (i.e. during testing).
    // Therefore, we expose a uiFilePath property that can be set to the correct path.
    // This property is not yet set in the constructor, therefore we need to use a QueuedConnection to load the UI file
    // after the property has been set by QML.
    QMetaObject::invokeMethod(
        this,
        [this]() {
            this->initializeUiAndData();
        },
        Qt::QueuedConnection);
}

QString ScriptDialogItem::uiFilePath() const
{
    // if no other ui file path has been specified, assume it can be derived
    // from the script path that the ScriptRunner is currently executing.
    if (m_uiFilePath.isNull()) {
        Q_ASSERT(!ScriptRunner::currentScriptPath.isEmpty());
        QFileInfo fi(ScriptRunner::currentScriptPath);
        m_uiFilePath = fi.absolutePath() + '/' + fi.baseName() + ".ui";
        emit const_cast<ScriptDialogItem *>(this)->uiFilePathChanged(m_uiFilePath);
    }

    return m_uiFilePath;
}

void ScriptDialogItem::setUiFilePath(const QString &filePath)
{
    if (!m_uiFilePath.isNull()) {
        spdlog::error("ScriptDialog::uiFilePath should only be set once");
        return;
    }

    if (m_uiFilePath != filePath) {
        QFileInfo fi(filePath);
        // If the path is relative, make it relative to the currently executing QML file,
        // not relative to the `knut` binary.
        if (fi.isRelative()) {
            Q_ASSERT(!ScriptRunner::currentScriptPath.isEmpty());
            QFileInfo base(ScriptRunner::currentScriptPath);
            m_uiFilePath = base.absolutePath() + "/" + filePath;
        } else {
            m_uiFilePath = filePath;
        }
        emit uiFilePathChanged(m_uiFilePath);
    }
}

bool ScriptDialogItem::showProgress()
{
    return m_showProgress;
}

void ScriptDialogItem::done(int code)
{
    if (code == QDialog::Accepted && showProgress()) {
        startShowingProgress();
    }

    // When showing progress, the `onAccepted` handler will likely call `QCoreApplication::processEvents` multiple
    // times. This will cause other slots that are connected to `finished` to be evaluated as well. Therefore we use the
    // additional `conversionFinished` signal for cleanup. This signal is only emitted once all handlers of
    // `QDialog::done` have finished.
    QDialog::done(code);
    if (!m_interactiveConversion.has_value()) {
        emit conversionFinished();
    }
}

void ScriptDialogItem::setShowProgress(bool value)
{
    if (m_showProgress != value) {
        m_showProgress = value;
        emit showProgressChanged(m_showProgress);
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

/**
 * \qmlmethod ScriptDialog::startProgress(string firstStep, int numSteps)
 *
 * Start a progress bar with the given `firstStep` title and number of steps.
 *
 * The number of following `nextStep` calls (or yield calls if using runSteps) should be one less than the number of
 * steps set here.
 */
void ScriptDialogItem::startProgress(const QString &firstStep, int numSteps)
{
    setProgressSteps(numSteps);
    nextStep(firstStep);
}

void ScriptDialogItem::setProgressSteps(int numSteps)
{
    m_numProgressSteps = numSteps;
    if (m_progressDialog) {
        m_progressDialog->setMaximum(numSteps);
    }
}

/**
 * \qmlmethod ScriptDialog::nextStep(string title)
 *
 * Indicate a new progress step.
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
    m_currentStepTitle = title;

    if (m_currentProgressStep >= m_numProgressSteps) {
        setProgressSteps(m_numProgressSteps + 1);
    }

    if (m_progressDialog) {
        m_progressDialog->setLabelText(title);
        m_progressDialog->setValue(m_currentProgressStep);
    }
    ++m_currentProgressStep;
}

bool isGenerator(const QJSValue &generator)
{
    return generator.property("next").isCallable();
}

void ScriptDialogItem::interactiveStep()
{
    auto result = m_interactiveConversion->property("next").callWithInstance(m_interactiveConversion.value());
    auto done = result.property("done").toBool();
    auto nextStepTitle = result.property("value").toString();

    auto finish = [this]() {
        m_interactiveConversion.reset();
        emit conversionFinished();
    };

    if (done) {
        finish();
        return;
    }

    auto continueConversion = [this, nextStepTitle]() {
        nextStep(nextStepTitle);
        if (m_progressDialog && m_showProgress) {
            m_progressDialog->show();
        }
        interactiveStep();
    };

    if (!isInteractive()) {
        return continueConversion();
    }

    if (m_progressDialog) {
        m_progressDialog->hide();
    }

    auto title = m_numProgressSteps > 0
        ? QString("%1/%2 conversions done!").arg(m_currentProgressStep).arg(m_numProgressSteps)
        : QString("A conversion step finished!");

    auto message = m_currentStepTitle.isEmpty()
        ? QString("A conversion step finished!\nContinue with %1?").arg(nextStepTitle)
        : QString("Finished %1!\nContinue with %2?").arg(m_currentStepTitle, nextStepTitle);

    auto msgBox = new QMessageBox(QMessageBox::Question, title, message, QMessageBox::Ok | QMessageBox::Abort, this);
    msgBox->setModal(false);

    connect(msgBox, &QMessageBox::accepted, this, continueConversion);
    connect(msgBox, &QMessageBox::rejected, this, finish);
    msgBox->show();
}

/**
 * \qmlmethod ScriptDialog::runSteps(function generator)
 *
 * Run a script in multiple (interactive) steps.
 * The argument to this function must be a JavaScript generator object
 * ([See this documentation on JS
 * Generators](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator)).
 *
 * The generator should yield a string with the next step title,
 * whenever the user should be able to pause the script and inspect the changes.
 * This will behave the same as calling `nextStep`, but pauses the script, until the user continues or aborts the
 * script.
 * You can also mix and match between `yield` and `nextStep` calls.
 *
 * For the best experience, we recommend to use `startProgress` and `nextStep` to indicate the remaining progress.
 *
 * Example:
 * ```javascript
 * function *conversionSteps() {
 *    startProgress("Adding member", 2)
 *    document.addMember("test", "int", CppDocument.Public)
 *
 *    yield "Inserting include" // <--- The user can check that the member was inserted correctly
 *    document.insertInclude("<iostream>")
 * }
 *
 * function convert() {
 *    runSteps(conversionSteps())
 * }
 * ```
 */
void ScriptDialogItem::runSteps(QJSValue generator)
{
    if (!isGenerator(generator)) {
        spdlog::error("ScriptDialogItem::runSteps: Argument is not a generator!\n"
                      "Make sure to create a generator function using function* myGenerator() { ... } and call the "
                      "generator with e.g. runSteps(myGenerator())!\n"
                      "See also:"
                      "https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator");
        return;
    }

    m_interactiveConversion = generator;
    interactiveStep();
}

static bool isShowingProgress = false;

void ScriptDialogItem::startShowingProgress()
{
    if (!m_progressDialog) {
        m_progressDialog = new QProgressDialog();
        m_progressDialog->setModal(true);
        m_progressDialog->setWindowModality(Qt::ApplicationModal);
        m_progressDialog->setWindowTitle(windowTitle());
        m_progressDialog->setLabelText("Converting your code...");
        m_progressDialog->setCancelButton(nullptr);
        m_progressDialog->setMinimumDuration(0);
        m_progressDialog->setAutoClose(false);
        m_progressDialog->setAutoReset(false);
        // Using min,max,value of 0 causes an undetermined progress bar
        // As we don't know how long a script may take without the script telling us, this is the default.
        m_progressDialog->setMinimum(0);
        m_progressDialog->setMaximum(m_numProgressSteps);
        m_progressDialog->setValue(m_currentProgressStep);

        connect(this, &ScriptDialogItem::conversionFinished, m_progressDialog, [this]() {
            m_progressDialog->close();
            m_progressDialog->deleteLater();
            isShowingProgress = false;
        });
    }
    m_progressDialog->show();
    isShowingProgress = true;

    ScriptDialogItem::updateProgress();
}

void ScriptDialogItem::updateProgress()
{
    if (isShowingProgress) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void ScriptDialogItem::initializeUiAndData()
{
    if (m_data) {
        return;
    }

    m_data = new DynamicObject(this);

    setUiFile(uiFilePath());

    m_data->ready();

    m_data->registerDataChangedCallback([this](const QString &key, const QVariant &value) {
        changeValue(key, value);
    });
}

QObject *ScriptDialogItem::data()
{
    initializeUiAndData();
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
        spdlog::error("Can't open {}", fileName.toStdString());
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
            connect(pushButton, &QToolButton::clicked, this, [this, toolButton]() {
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
        }
    }
}

void ScriptDialogItem::changeValue(const QString &key, const QVariant &value)
{
    auto widget = findChild<QWidget *>(key);
    if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
        lineEdit->setText(value.toString());
    } else if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
        checkBox->setChecked(value.toBool());
    } else if (auto radioButton = qobject_cast<QRadioButton *>(widget)) {
        radioButton->setChecked(value.toBool());
    } else if (auto spinBox = qobject_cast<QSpinBox *>(widget)) {
        spinBox->setValue(value.toInt());
    } else if (auto doubleSpinBox = qobject_cast<QDoubleSpinBox *>(widget)) {
        doubleSpinBox->setValue(value.toDouble());
    } else if (auto comboBox = qobject_cast<QComboBox *>(widget)) {
        comboBox->setCurrentText(value.toString());
    }

    // It may be a combobox model
    if (key.endsWith("Model")) {
        if (auto comboBox = findChild<QComboBox *>(key.left(key.length() - 5))) {
            comboBox->clear();
            comboBox->addItems(value.toStringList());
            if (comboBox->count())
                comboBox->setCurrentIndex(0);
        }
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
