/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "apiexecutorwidget.h"
#include "core/cppdocument.h"
#include "core/imagedocument.h"
#include "core/logger.h"
#include "core/project.h"
#include "core/qmldocument.h"
#include "core/rcdocument.h"
#include "core/slintdocument.h"
#include "core/uidocument.h"
#include "guisettings.h"
#include "ui_apiexecutorwidget.h"
#include "utils/log.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSpinBox>
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

static const QMetaObject *metaObjectFromType(Core::Document::Type type)
{
    switch (type) {
    case Core::Document::Type::Cpp:
        return &Core::CppDocument::staticMetaObject;
    case Core::Document::Type::Image:
        return &Core::ImageDocument::staticMetaObject;
    case Core::Document::Type::Rc:
        return &Core::RcDocument::staticMetaObject;
    case Core::Document::Type::Text:
        return &Core::TextDocument::staticMetaObject;
    case Core::Document::Type::Ui:
        return &Core::UiDocument::staticMetaObject;
    case Core::Document::Type::Slint:
        return &Core::SlintDocument::staticMetaObject;
    case Core::Document::Type::Qml:
        return &Core::QmlDocument::staticMetaObject;
    }
    Q_UNREACHABLE();
}

void APIExecutorWidget::initializeApi()
{
    qRegisterMetaType<Core::CppDocument::AccessSpecifier>();

    const QMetaEnum enumMeta = QMetaEnum::fromType<Core::Document::Type>();
    for (int enumIndex = 0; enumIndex < enumMeta.keyCount(); ++enumIndex) {
        auto type = static_cast<Core::Document::Type>(enumMeta.value(enumIndex));
        auto metaObject = metaObjectFromType(type);
        if (!metaObject) {
            continue;
        }

        for (int i = 0; i < metaObject->methodCount(); ++i) {
            auto method = metaObject->method(i);
            if (method.methodType() == QMetaMethod::Slot && method.tag() == QStringLiteral("API_EXECUTOR")) {
                m_apis.insert(method.name(), {type, method});
            }
        }
    }
}

void APIExecutorWidget::populateApiList(Core::Document *document)
{
    ui->apiComboBox->clear();

    QStringList availableAPIs;
    const auto currentType = document->type();
    for (const auto &api : std::as_const(m_apis)) {
        if (api.type == currentType)
            availableAPIs.push_back(api.method.name());
    }

    std::ranges::sort(availableAPIs);
    ui->apiComboBox->addItems(availableAPIs);
    if (availableAPIs.isEmpty())
        hide();
}

void APIExecutorWidget::populateArgumentList()
{
    // Remove all widgets
    for (const auto &field : std::as_const(m_argumentFields)) {
        delete field.label;
        delete field.widget;
    }
    m_argumentFields.clear();

    // Look up the API information
    const QString apiName = ui->apiComboBox->currentText();
    const ApiInfo &apiInfo = m_apis.value(apiName);

    for (int i = 0; i < apiInfo.method.parameterCount(); ++i) {
        const auto &parameterNames = apiInfo.method.parameterNames();
        createArgumentField(parameterNames[i], apiInfo.method.parameterMetaType(i));
    }

    // Change focus chain
    QWidget *current = ui->apiComboBox;
    QWidget *next = nullptr;
    for (const auto &field : std::as_const(m_argumentFields)) {
        next = field.widget;
        QWidget::setTabOrder(current, next);
        current = next;
    }
    QWidget::setTabOrder(current, ui->executeButton);
    QWidget::setTabOrder(ui->executeButton, ui->closeButton);
}

std::optional<QMetaEnum> metaEnumFromName(QMetaType metaType)
{
    Q_ASSERT(metaType.isValid());
    if (!metaType.flags().testFlag(QMetaType::IsEnumeration))
        return {};

    // We can't access the QMetaEnum directly, we need to go through the object containing it
    const QString typeName = metaType.name();
    auto surroundingClass = metaType.metaObject();
    const auto enumName = typeName.sliced(typeName.lastIndexOf(':') + 1);
    const int enumIndex = surroundingClass->indexOfEnumerator(qPrintable(enumName));
    Q_ASSERT(enumIndex != -1);
    return surroundingClass->enumerator(enumIndex);
}

void APIExecutorWidget::createArgumentField(const QByteArray &name, const QMetaType &type)
{
    QWidget *widget = nullptr;
    auto metaEnum = metaEnumFromName(type);
    if (metaEnum) {
        auto comboBox = new QComboBox(this);
        for (int i = 0; i < metaEnum->keyCount(); ++i)
            comboBox->addItem(metaEnum->key(i), metaEnum->value(i));
        widget = comboBox;

    } else {
        const int typeId = type.id();
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
            auto comboBox = new QComboBox(this);
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
        widget->installEventFilter(this);
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
    genericArgs.reserve(apiInfo.method.parameterCount());

    std::vector<std::unique_ptr<int>> intPointers;
    std::vector<std::unique_ptr<double>> doublePointers;
    std::vector<std::unique_ptr<bool>> boolPointers;
    std::vector<std::unique_ptr<QString>> stringPointers;

    for (int i = 0; i < apiInfo.method.parameterCount(); i++) {
        QWidget *field = m_argumentFields.at(i).widget;
        QVariant value;
        if (auto comboBox = qobject_cast<QComboBox *>(field)) {
            value = comboBox->currentData();
        } else if (auto lineEdit = qobject_cast<QLineEdit *>(field)) {
            value = lineEdit->text();
        } else if (auto spinBox = qobject_cast<QSpinBox *>(field)) {
            value = spinBox->value();
        } else if (auto doubleSpinBox = qobject_cast<QDoubleSpinBox *>(field)) {
            value = doubleSpinBox->value();
        } else {
            Q_UNREACHABLE();
        }

        const auto type = apiInfo.method.parameterMetaType(i);
        auto metaEnum = metaEnumFromName(type);
        if (metaEnum) {
            intPointers.push_back(std::make_unique<int>(value.toInt()));
            genericArgs.append(QGenericArgument(type.name(), intPointers.back().get()));
        } else {
            const int typeId = type.id();
            switch (typeId) {
            case QMetaType::Int: {
                intPointers.push_back(std::make_unique<int>(value.toInt()));
                genericArgs.append(QGenericArgument(type.name(), intPointers.back().get()));
                break;
            }
            case QMetaType::Double: {
                doublePointers.push_back(std::make_unique<double>(value.toDouble()));
                genericArgs.append(QGenericArgument(type.name(), doublePointers.back().get()));
                break;
            }
            case QMetaType::Bool: {
                boolPointers.push_back(std::make_unique<bool>(value.toBool()));
                genericArgs.append(QGenericArgument(type.name(), boolPointers.back().get()));
                break;
            }
            case QMetaType::QString: {
                stringPointers.push_back(std::make_unique<QString>(value.toString()));
                genericArgs.append(QGenericArgument(type.name(), stringPointers.back().get()));
                break;
            }
            default:
                Q_UNREACHABLE();
                break;
            }
        }
    }

    executeAPI(Core::Project::instance()->currentDocument(), apiInfo.method.name(), genericArgs);
    genericArgs.clear();
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
        ERROR("Error trying to execute the function {}", name);
}

void APIExecutorWidget::open()
{
    populateArgumentList();

    show();
    ui->apiComboBox->setFocus(Qt::OtherFocusReason);
}

bool APIExecutorWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
            onExecuteButtonClicked();
    }
    return QWidget::eventFilter(obj, event);
}

} // namespace Gui
