#include "scriptdialogitem.h"

#include "scriptrunner.h"

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QQmlContext>
#include <QQmlPropertyMap>
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
 * \instantiates Core::ScriptItemDialog
 * \inqmlmodule Script
 * \since 4.0
 *
 * The `ScriptDialog` allows creating a scipt dialog based on a ui file. It requires creating a ui file with the same
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
 *     onClicked: {
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
    , m_data(new QQmlPropertyMap(this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    Q_ASSERT(!ScriptRunner::currentScriptPath.isEmpty());
    QFileInfo fi(ScriptRunner::currentScriptPath);
    setUiFile(fi.absolutePath() + '/' + fi.baseName() + ".ui");

    connect(m_data, &QQmlPropertyMap::valueChanged, this, &ScriptDialogItem::changeValue);
}

QQmlPropertyMap *ScriptDialogItem::data() const
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
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(internalWidget);
        setLayout(layout);
        setWindowTitle(internalWidget->windowTitle());

        auto widgets = internalWidget->findChildren<QWidget *>();
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
                m_data->insert(widget->objectName(), lineEdit->text());
                connect(lineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
                    (*m_data)[sender()->objectName()] = text;
                    emit dataChanged();
                });
            } else if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
                m_data->insert(widget->objectName(), checkBox->isChecked());
                connect(checkBox, &QCheckBox::toggled, this, [this](bool value) {
                    (*m_data)[sender()->objectName()] = value;
                    emit dataChanged();
                });
            } else if (auto radioButton = qobject_cast<QRadioButton *>(widget)) {
                m_data->insert(widget->objectName(), radioButton->isChecked());
                connect(radioButton, &QCheckBox::toggled, this, [this](bool value) {
                    (*m_data)[sender()->objectName()] = value;
                    emit dataChanged();
                });
            } else if (auto spinBox = qobject_cast<QSpinBox *>(widget)) {
                m_data->insert(widget->objectName(), spinBox->value());
                connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
                    (*m_data)[sender()->objectName()] = value;
                    emit dataChanged();
                });
            } else if (auto doubleSpinBox = qobject_cast<QDoubleSpinBox *>(widget)) {
                m_data->insert(widget->objectName(), doubleSpinBox->value());
                connect(doubleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value) {
                    (*m_data)[sender()->objectName()] = value;
                    emit dataChanged();
                });
            } else if (auto comboBox = qobject_cast<QComboBox *>(widget)) {
                m_data->insert(widget->objectName(), comboBox->currentText());
                connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, comboBox]() {
                    (*m_data)[sender()->objectName()] = comboBox->currentText();
                    emit dataChanged();
                });
                auto model = comboBox->model();
                QStringList itemList;
                for (int i = 0; i < model->rowCount(); ++i)
                    itemList.push_back(model->data(model->index(i, 0)).toString());
                m_data->insert(widget->objectName() + "Model", itemList);
                if (comboBox->isEditable()) {
                    auto completer = new QCompleter(comboBox);
                    completer->setFilterMode(Qt::MatchContains);
                    completer->setModel(comboBox->model());
                    comboBox->setCompleter(completer);
                }
            }
        }
    } else {
        spdlog::error("Can't open {}", fileName.toStdString());
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

QObject *ScriptDialogItem::atChild(QQmlListProperty<QObject> *list, Core::TypeSize index)
{
    if (auto that = qobject_cast<ScriptDialogItem *>(list->object))
        return that->m_children.at(index);
    return nullptr;
}

TypeSize ScriptDialogItem::countChildren(QQmlListProperty<QObject> *list)
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
