#include "apiselector.h"
#include "ui_apiselector.h"

#include "guisettings.h"

#include "core/cppdocument.h"
#include "core/logger.h"
#include "core/project.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>

#include <spdlog/spdlog.h>

#include <memory>

namespace Gui {

APIExecutorWidget::APIExecutorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::APIExecutorWidget)
{

    ui->setupUi(this);
    setProperty("panelWidget", true);

    initializeApi();

    GuiSettings::setIcon(ui->closeButton, ":/gui/close.png");
    connect(ui->closeButton, &QToolButton::clicked, this, &QWidget::hide);

    auto project = Core::Project::instance();
    connect(project, &Core::Project::currentDocumentChanged, this, &APIExecutorWidget::populateApiList);
    connect(ui->apiComboBox, &QComboBox::currentIndexChanged, this, &APIExecutorWidget::populateArgumentList);

    connect(ui->executeButton, &QToolButton::clicked, this, &APIExecutorWidget::onExecuteButtonClicked);
}

APIExecutorWidget::~APIExecutorWidget() = default;

void APIExecutorWidget::initializeApi()
{
    qRegisterMetaType<Core::CppDocument::AccessSpecifier>();

    addApi({Core::Document::Type::Cpp,
            "addMember",
            {{"member", "QString"}, {"className", "QString"}, {"scope", "Core::CppDocument::AccessSpecifier"}}});
    addApi({Core::Document::Type::Cpp,
            "addMethodDeclaration",
            {{"method", "QString"}, {"className", "QString"}, {"scope", "Core::CppDocument::AccessSpecifier"}}});
    addApi({Core::Document::Type::Cpp, "addMethodDefinition", {{"method", "QString"}, {"className", "QString"}}});
    addApi({Core::Document::Type::Cpp, "insertInclude", {{"include", "QString"}, {"newGroup", "bool"}}});
    addApi({Core::Document::Type::Cpp, "insertForwardDeclaration", {{"forwardDeclaration", "QString"}}});
    addApi({Core::Document::Type::Cpp, "removeInclude", {{"include", "QString"}}});
    addApi({Core::Document::Type::Cpp, "deleteMethod", {{"method", "QString"}, {"signature", "QString"}}});
}

static const QMetaObject *metaObjectFromType(Core::Document::Type type)
{
    switch (type) {
    case Core::Document::Type::Cpp:
        return &Core::CppDocument::staticMetaObject;
    case Core::Document::Type::Image:
    case Core::Document::Type::Rc:
    case Core::Document::Type::Text:
    case Core::Document::Type::Ui:
        Q_UNREACHABLE();
    }
    Q_UNREACHABLE();
    return nullptr;
}

void APIExecutorWidget::addApi(ApiInfo &&apiInfo)
{
    const auto signature = signatureForApi(apiInfo);
    auto metaObject = metaObjectFromType(apiInfo.type);
    int methodIndex = metaObject->indexOfMethod(QMetaObject::normalizedSignature(signature));
    Q_ASSERT(methodIndex != -1);

    m_apis.insert(apiInfo.name, std::move(apiInfo));
}

void APIExecutorWidget::populateApiList()
{
    ui->apiComboBox->clear();

    QStringList availableAPIs;
    const auto currentType = Core::Project::instance()->currentDocument()->type();
    for (const auto &api : std::as_const(m_apis)) {
        if (api.type == currentType)
            availableAPIs.push_back(api.name);
    }

    std::ranges::sort(availableAPIs);
    ui->apiComboBox->addItems(availableAPIs);
    if (availableAPIs.isEmpty())
        hide();
}

void APIExecutorWidget::populateArgumentList()
{
    // Remove all widgets
    for (const auto &field : m_argumentFields) {
        delete field.label;
        delete field.widget;
    }
    m_argumentFields.clear();

    // Look up the API information
    const QString apiName = ui->apiComboBox->currentText();
    const ApiInfo &apiInfo = m_apis.value(apiName);

    for (const auto &arg : apiInfo.args)
        createArgumentField(arg.first, arg.second);

    // Change focus chain
    QWidget *current = ui->apiComboBox;
    QWidget *next = nullptr;
    for (auto field : m_argumentFields) {
        next = field.widget;
        QWidget::setTabOrder(current, next);
        current = next;
    }
    QWidget::setTabOrder(current, ui->executeButton);
    QWidget::setTabOrder(ui->executeButton, ui->closeButton);
}

std::optional<QMetaEnum> metaEnumFromName(QByteArrayView typeName)
{
    auto metaType = QMetaType::fromName(typeName);
    Q_ASSERT(metaType.isValid());
    if (!metaType.flags().testFlag(QMetaType::IsEnumeration))
        return {};

    // We can't access the QMetaEnum directly, we need to go through the object containing it
    auto surroundingClass = metaType.metaObject();
    const auto enumName = typeName.sliced(typeName.lastIndexOf(':') + 1).constData();
    const int enumIndex = surroundingClass->indexOfEnumerator(enumName);
    Q_ASSERT(enumIndex != -1);
    return surroundingClass->enumerator(enumIndex);
}

void APIExecutorWidget::createArgumentField(const QByteArray &name, const QByteArray &typeName)
{
    QWidget *widget = nullptr;
    auto metaEnum = metaEnumFromName(typeName);
    if (metaEnum) {
        QComboBox *comboBox = new QComboBox(this);
        for (int i = 0; i < metaEnum->keyCount(); ++i)
            comboBox->addItem(metaEnum->key(i), metaEnum->value(i));
        widget = comboBox;

    } else {
        const int typeId = QMetaType::fromName(typeName).id();
        switch (typeId) {
        case QMetaType::Int: {
            widget = new QSpinBox(this);
            break;
        }
        case QMetaType::Double: {
            widget = new QDoubleSpinBox(this);
            break;
        }
        case QMetaType::Bool: {
            QComboBox *comboBox = new QComboBox(this);
            comboBox->addItem("false", false);
            comboBox->addItem("true", true);
            widget = comboBox;
            break;
        }
        case QMetaType::QString: {
            widget = new QLineEdit(this);
            break;
        }
        default:
            Q_UNREACHABLE();
            break;
        }
    }

    if (widget) {
        auto label = new QLabel(name + ':');
        ui->layout->addWidget(label);
        ui->layout->addWidget(widget);
        m_argumentFields.append({label, widget});
    } else {
        Q_UNREACHABLE();
    }
}

void APIExecutorWidget::onExecuteButtonClicked()
{
    const QString apiName = ui->apiComboBox->currentText();
    const ApiInfo &apiInfo = m_apis.value(apiName);

    QList<QGenericArgument> genericArgs;
    genericArgs.reserve(apiInfo.args.count());

    std::vector<std::unique_ptr<int>> intPointers;
    std::vector<std::unique_ptr<double>> doublePointers;
    std::vector<std::unique_ptr<bool>> boolPointers;
    std::vector<std::unique_ptr<QString>> stringPointers;

    for (int i = 0; i < apiInfo.args.count(); i++) {
        QWidget *field = m_argumentFields.at(i).widget;
        QVariant value;
        if (QComboBox *comboBox = qobject_cast<QComboBox *>(field)) {
            value = comboBox->currentData();
        } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(field)) {
            value = lineEdit->text();
        } else if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(field)) {
            value = spinBox->value();
        } else if (QDoubleSpinBox *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(field)) {
            value = doubleSpinBox->value();
        } else {
            Q_UNREACHABLE();
        }

        const auto typeName = apiInfo.args.at(i).second;
        auto metaEnum = metaEnumFromName(typeName);
        if (metaEnum) {
            intPointers.push_back(std::make_unique<int>(value.toInt()));
            genericArgs.append(QGenericArgument(typeName, intPointers.back().get()));
        } else {
            const int typeId = QMetaType::fromName(typeName).id();
            switch (typeId) {
            case QMetaType::Int: {
                intPointers.push_back(std::make_unique<int>(value.toInt()));
                genericArgs.append(QGenericArgument(typeName, intPointers.back().get()));
                break;
            }
            case QMetaType::Double: {
                doublePointers.push_back(std::make_unique<double>(value.toDouble()));
                genericArgs.append(QGenericArgument(typeName, doublePointers.back().get()));
                break;
            }
            case QMetaType::Bool: {
                boolPointers.push_back(std::make_unique<bool>(value.toBool()));
                genericArgs.append(QGenericArgument(typeName, boolPointers.back().get()));
                break;
            }
            case QMetaType::QString: {
                stringPointers.push_back(std::make_unique<QString>(value.toString()));
                genericArgs.append(QGenericArgument(typeName, stringPointers.back().get()));
                break;
            }
            default:
                Q_UNREACHABLE();
                break;
            }
        }
    }

    executeAPI(Core::Project::instance()->currentDocument(), apiInfo.name, genericArgs);
    genericArgs.clear();
}

QByteArray APIExecutorWidget::signatureForApi(const ApiInfo &apiInfo)
{
    QList<QByteArray> args;
    for (const auto &arg : apiInfo.args) {
        args += arg.second;
    }
    return apiInfo.name + '(' + args.join(',') + ')';
}

void APIExecutorWidget::executeAPI(Core::Document *document, const QByteArray &name,
                                   const QList<QGenericArgument> &genericArgs)
{
    Q_ASSERT(genericArgs.size() <= 10);
    bool success = QMetaObject::invokeMethod(document, name, genericArgs.value(0), genericArgs.value(1),
                                             genericArgs.value(2), genericArgs.value(3), genericArgs.value(4),
                                             genericArgs.value(5), genericArgs.value(6), genericArgs.value(7),
                                             genericArgs.value(8), genericArgs.value(9));
    if (!success)
        spdlog::error("Error trying to execute the function {}", name.toStdString());
}

void APIExecutorWidget::open()
{
    populateArgumentList();

    show();
    ui->apiComboBox->setFocus(Qt::OtherFocusReason);
}

} // namespace Gui
