#include "apiselector.h"
#include "ui_apiselector.h"

#include "core/logger.h"
#include "core/project.h"
#include "guisettings.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QKeySequence>
#include <QLineEdit>
#include <QList>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextDocument>

#include "core/cppdocument.h"

namespace Gui {

APIExecutorWidget::APIExecutorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::APIExecutorWidget)
{

    ui->setupUi(this);
    setProperty("panelWidget", true);
    // register the API
    APIExecutorWidget::addApi("insertInclude", {{"include", "QString"}, {"newGroup", "bool"}},
                              Core::Document::Type::Cpp);
    APIExecutorWidget::addApi(
        "addMember",
        {{"method", "QString"}, {"className", "QString"}, {"specifier", "Core::CppDocument::AccessSpecifier"}},
        Core::Document::Type::Cpp);
    APIExecutorWidget::addApi(
        "addMethodDeclaration",
        {{"method", "QString"}, {"className", "QString"}, {"specifier", "Core::CppDocument::AccessSpecifier"}},
        Core::Document::Type::Cpp);
    APIExecutorWidget::addApi("addMethodDefinition", {{"method", "QString"}, {"className", "QString"}},
                              Core::Document::Type::Cpp);
    APIExecutorWidget::addApi("removeInclude", {{"include", "QString"}}, Core::Document::Type::Cpp);
    APIExecutorWidget::addApi("deleteMethod", {{"methodName", "QString"}, {"signature", "QString"}},
                              Core::Document::Type::Cpp);
    APIExecutorWidget::addApi("insertForwardDeclaration", {{"fwddecl", "QString"}}, Core::Document::Type::Cpp);

    GuiSettings::setIcon(ui->closeButton, ":/gui/close.png");
    connect(ui->closeButton, &QToolButton::clicked, this, &QWidget::hide);
    ui->label->setFixedWidth(50);

    auto project = Core::Project::instance();
    connect(project, &Core::Project::currentDocumentChanged, this, &APIExecutorWidget::populateApiList);
    connect(ui->apiComboBox, &QComboBox::currentIndexChanged, this, &APIExecutorWidget::populateArgumentList);

    connect(ui->executeButton, &QPushButton::clicked, this, &APIExecutorWidget::onExecuteButtonClicked);
}

APIExecutorWidget::~APIExecutorWidget() = default;

void APIExecutorWidget::addApi(const QString &name, const QVector<QPair<QString, QString>> &args,
                               Core::Document::Type type)
{
    ApiInfo info {name, args, type};
    m_apis.insert(name, info);
}

void APIExecutorWidget::populateApiList()
{
    ui->apiComboBox->clear();

    if (qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument())) {
        QList<ApiInfo> apis = m_apis.values();
        for (const auto &api : apis) {
            if (api.type == Core::Document::Type::Cpp) {
                ui->apiComboBox->addItem(api.name);
            }
        }
    }

    ui->apiComboBox->setCurrentIndex(-1);
}

void APIExecutorWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Reinitialize the UI
    populateArgumentList();
}

void APIExecutorWidget::populateArgumentList()
{

    const QString apiName = ui->apiComboBox->currentText();

    // Look up the API information
    const ApiInfo &apiInfo = m_apis.value(apiName);

    auto obj = Core::Project::instance()->currentDocument()->metaObject();
    QString signature = getSignature(apiName, apiInfo.args);

    int methodIndex = obj->indexOfMethod(QMetaObject::normalizedSignature(signature.toStdString().c_str()));

    qDeleteAll(m_argumentFields);
    m_argumentFields.clear();

    QLayoutItem *item;
    while ((item = ui->layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (methodIndex < 0) {
        qWarning() << "Initialize it correctly";
        return;
    }
    QMetaMethod api = obj->method(methodIndex);

    const int argCount = api.parameterCount();

    for (int i = 0; i < argCount; ++i) {
        const QString &name = QString::fromUtf8(api.parameterNames().at(i));
        int typeId = QMetaType::Type(api.parameterType(i));
        createArgumentField(ui->formLayout, name, typeId);
    }
}

void APIExecutorWidget::createArgumentField(QHBoxLayout *parent, const QString &name, int typeId)
{
    int id = qRegisterMetaType<Core::CppDocument::AccessSpecifier>();
    QWidget *widget = nullptr;
    if (typeId == id) {

        const QMetaObject *metaObj = QMetaType(id).metaObject();
        const int index = metaObj->indexOfEnumerator("AccessSpecifier");
        if (index >= 0) {
            const QMetaEnum metaEnum = metaObj->enumerator(index);
            QComboBox *comboBox = new QComboBox(parent->parentWidget());
            for (int i = 0; i < metaEnum.keyCount(); ++i) {
                comboBox->addItem(metaEnum.key(i), metaEnum.value(i));
            }
            widget = comboBox;
        }
    } else {
        switch (typeId) {
        case QMetaType::Int: {
            QSpinBox *spinBox = new QSpinBox(parent->parentWidget());
            widget = spinBox;
            break;
        }
        case QMetaType::Bool: {
            QComboBox *comboBox = new QComboBox(parent->parentWidget());
            comboBox->addItem("false");
            comboBox->addItem("true");
            widget = comboBox;
            break;
        }
        case QMetaType::QString: {
            QLineEdit *lineEdit = new QLineEdit(parent->parentWidget());
            widget = lineEdit;
            break;
        }

        default:
            qWarning() << "Unsupported argument type:" << name;
            break;
        }
    }

    if (widget) {
        const QString labelName = ":";

        QLabel *label = new QLabel(name + labelName);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setFixedWidth(100);
        ui->layout->addWidget(label);
        ui->layout->addWidget(widget);

        ArgumentWidget *argWidget = new Gui::ArgumentWidget();
        argWidget->label = label;
        argWidget->widget = widget;
        m_argumentFields.append(argWidget);
    }
}

void APIExecutorWidget::onExecuteButtonClicked()
{
    const QString apiName = ui->apiComboBox->currentText();
    auto obj = Core::Project::instance()->currentDocument()->metaObject();
    const ApiInfo &apiInfo = m_apis.value(apiName);
    const QString signature = getSignature(apiName, apiInfo.args);

    int methodIndex = obj->indexOfMethod(QMetaObject::normalizedSignature(signature.toStdString().c_str()));
    if (methodIndex < 0) {
        qWarning() << "Failed to find method" << apiName << "in meta-object";
        return;
    }
    QMetaMethod api = obj->method(methodIndex);

    QList<QGenericArgument> genericArgs;
    for (int i = 0; i < api.parameterCount(); i++) {
        QWidget *field = m_argumentFields.at(i)->widget;
        const auto type = apiInfo.args.at(i).second;
        QString value;
        if (QComboBox *comboBox = qobject_cast<QComboBox *>(field)) {
            value = comboBox->currentText();
        } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(field)) {
            value = lineEdit->text();
        } else {
            qWarning() << "Unsupported widget type";
            continue;
        }

        if (type == "int") {
            int intValue = value.toInt();
            genericArgs.append(QGenericArgument("int", &intValue));
        } else if (type == "bool") {
            bool boolValue = (value.toLower() == "true");
            genericArgs.append(QGenericArgument("bool", &boolValue));
        } else if (type == "QString") {
            QString *stringValue = new QString(value);
            genericArgs.append(QGenericArgument("QString", stringValue));
        } else if (type == "Core::CppDocument::AccessSpecifier") {
            Core::CppDocument::AccessSpecifier accessSpecifier;
            if (value == "Public") {
                accessSpecifier = Core::CppDocument::AccessSpecifier::Public;
            } else if (value == "Protected") {
                accessSpecifier = Core::CppDocument::AccessSpecifier::Protected;
            } else if (value == "Private") {
                accessSpecifier = Core::CppDocument::AccessSpecifier::Private;
            }
            genericArgs.append(QGenericArgument("Core::CppDocument::AccessSpecifier", &accessSpecifier));
        } else {
            qWarning() << "Unsupported argument type:" << type;
            genericArgs.append(QGenericArgument());
        }
    }

    executeAPI(Core::Project::instance()->currentDocument(), api, genericArgs);
    genericArgs.clear();
}

QString APIExecutorWidget::getSignature(const QString &apiName, const QVector<QPair<QString, QString>> &args)
{
    QString signature = apiName + "(";
    for (const auto &arg : args) {
        signature += arg.second + ", ";
    }
    signature.chop(2); // remove the last comma with space
    signature += ")";
    return signature;
}

void APIExecutorWidget::executeAPI(Core::Document *document, const QMetaMethod &api,
                                   const QList<QGenericArgument> &genericArgs)
{

    bool success = api.invoke(document, genericArgs.value(0), genericArgs.value(1), genericArgs.value(2));
    if (!success) {
        qWarning() << "Failed to execute API:" << api.name();
    }
}

void APIExecutorWidget::showAPIExecutorWidget()
{
    show();
    raise();
}

}
