/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qtuidocument.h"
#include "logger.h"
#include "utils/log.h"
#include "utils/qtuiwriter.h"

#include <QFile>
#include <QUiLoader>
#include <QWidget>
#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype QtUiDocument
 * \brief Provides access to the content of a Ui file (Qt designer file).
 * \inqmlmodule Knut
 * \ingroup QtUiDocument/@first
 */

/*!
 * \qmlproperty array<QtUiWidget> QtUiDocument::widgets
 * List of all widgets in the ui file.
 */

QtUiDocument::QtUiDocument(QObject *parent)
    : Document(Type::QtUi, parent)
{
}

QtUiDocument::~QtUiDocument() = default;

/*!
 * \qmlmethod QtUiWidget QtUiDocument::findWidget(string name)
 * Returns the widget for the given `name`.
 */
QtUiWidget *QtUiDocument::findWidget(const QString &name) const
{
    LOG("QtUiDocument::findWidget", name);

    auto result = kdalgorithms::find_if(m_widgets, [name](QtUiWidget *widget) {
        return widget->name() == name;
    });
    if (!result)
        return nullptr;
    return *result;
}

/*!
 * \qmlmethod QtUiWidget QtUiDocument::addWidget(string className, string name, QtUiWidget parent)
 * Creates a new widget with the given `className` and `name`, and adds it to the `parent` widget.
 *
 * If `parent` is null, it creates the top level widget (there can only be one).
 */
Core::QtUiWidget *QtUiDocument::addWidget(const QString &className, const QString &name, Core::QtUiWidget *parent)
{
    LOG("QtUiDocument::addWidget", className, name);

    if (m_widgets.empty() && parent) {
        spdlog::error("QtUiDocument::addWidget - adding a widget to a non-root widget is not supported yet.");
        return nullptr;
    }

    const auto node = uiWriter()->addWidget(className, name, parent ? parent->xmlNode() : pugi::xml_node {});

    QtUiWidget *newWidget = new QtUiWidget(node, parent == nullptr, this);
    m_widgets.push_back(newWidget);
    setHasChanged(true);
    emit widgetsChanged();
    return newWidget;
}

// clang-format off
/*!
 * \qmlmethod QtUiWidget QtUiDocument::addCustomWidget(string className, string baseClassName, string header, bool isContainer)
 * Adds a new custom widget to the ui file.
 *
 * The `className` is the name of the custom widget, `baseClassName` is the name of the base class, `header` is the
 * include. If `isContainer` is true, the widget is a container.
 *
 * The `header` should be in the form of `<foo.h>` or `"foo.h"`.
 */
// clang-format on
void QtUiDocument::addCustomWidget(const QString &className, const QString &baseClassName, const QString &header,
                                   bool isContainer)
{
    LOG("QtUiDocument::addCustomWidget", className, baseClassName, header, isContainer);

    const auto result = uiWriter()->addCustomWidget(className, baseClassName, header, isContainer);

    switch (result) {
    case Utils::QtUiWriter::Success:
        setHasChanged(true);
        return;
    case Utils::QtUiWriter::AlreadyExists:
        spdlog::info(R"(QtUiDocument::addCustomWidget - the custom widget '{}' already exists)", className);
        return;
    case Utils::QtUiWriter::InvalidHeader:
        spdlog::error(
            R"(QtUiDocument::addCustomWidget - the include '{}' is malformed, should be '<foo.h>' or '"foo.h"')",
            header);
        return;
    case Utils::QtUiWriter::InvalidProperty:
        Q_UNREACHABLE();
    }
}

/*!
 * \qmlmethod QtUiDocument::preview()
 * Opens a dialog to preview the current ui file.
 */
void QtUiDocument::preview() const
{
    LOG("QtUiDocument::preview");

    QUiLoader loader;

    QFile file(fileName());
    if (file.open(QIODevice::ReadOnly)) {
        QWidget *widget = loader.load(&file);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    }
}

bool QtUiDocument::doSave(const QString &fileName)
{
    return m_document.save_file(fileName.toLatin1().constData(), "    ");
}

bool QtUiDocument::doLoad(const QString &fileName)
{
    m_widgets.clear();
    pugi::xml_parse_result result =
        m_document.load_file(fileName.toLatin1().constData(), pugi::parse_default | pugi::parse_declaration);

    if (!result) {
        spdlog::critical("{}({}): {}", fileName, result.offset, result.description());
        return false;
    }

    const auto widgets = m_document.select_nodes("//widget");

    bool isRoot = true;
    for (const auto &node : widgets) {
        Q_ASSERT(!node.node().empty());
        m_widgets.push_back(new QtUiWidget(node.node(), isRoot, this));
        isRoot = false;
    }
    emit widgetsChanged();

    return true;
}

Utils::QtUiWriter *QtUiDocument::uiWriter()
{
    if (!m_writer)
        m_writer = std::make_unique<Utils::QtUiWriter>(m_document);
    return m_writer.get();
}

/*!
 * \qmltype QtUiWidget
 * \brief Provides access to widget attributes in the ui files.
 * \inqmlmodule Knut
 * \ingroup QtUiDocument
 * \sa QtUiDocument
 */

/*!
 * \qmlproperty string QtUiWidget::name
 * Name of the widget.
 */
/*!
 * \qmlproperty string QtUiWidget::className
 * Name of the widget's class.
 */
/*!
 * \qmlproperty bool QtUiWidget::isRoot
 * Read-only property returning `true` if the widget is the root widget.
 */

QtUiWidget::QtUiWidget(pugi::xml_node widget, bool isRoot, QObject *parent)
    : QObject(parent)
    , m_widget(widget)
    , m_isRoot(isRoot)
{
}

QString QtUiWidget::name() const
{
    return QString::fromLatin1(m_widget.attribute("name").value());
}

void QtUiWidget::setName(const QString &newName)
{
    LOG("QtUiWidget::setName", newName);

    if (newName == name())
        return;

    qobject_cast<QtUiDocument *>(parent())->uiWriter()->setWidgetName(m_widget, newName, m_isRoot);
    qobject_cast<QtUiDocument *>(parent())->setHasChanged(true);
    emit nameChanged(newName);
}

QString QtUiWidget::className() const
{
    return QString::fromLatin1(m_widget.attribute("class").value());
}

void QtUiWidget::setClassName(const QString &newClassName)
{
    LOG("QtUiWidget::setClassName", newClassName);

    if (newClassName == className())
        return;

    qobject_cast<QtUiDocument *>(parent())->uiWriter()->setWidgetClassName(m_widget, newClassName);
    qobject_cast<QtUiDocument *>(parent())->setHasChanged(true);
    emit classNameChanged(newClassName);
}

/*!
 * \qmlmethod var QtUiWidget::getProperty(string name)
 * Returns the value of the property `name`.
 */
QVariant QtUiWidget::getProperty(const QString &name) const
{
    LOG("QtUiWidget::getProperty", name);

    const QString propertyPath = "property[@name='" % name % "']/*[1]";
    const auto dataNode = m_widget.select_node(propertyPath.toLatin1().constData()).node();
    if (dataNode.empty())
        return {};

    if (QLatin1String("rect") == dataNode.name()) {
        const auto x = dataNode.child("x").text().as_int();
        const auto y = dataNode.child("y").text().as_int();
        const auto width = dataNode.child("width").text().as_int();
        const auto height = dataNode.child("height").text().as_int();
        return QRect(x, y, width, height);
    } else if (QLatin1String("bool") == dataNode.name()) {
        return dataNode.text().as_bool();
    } else if (QLatin1String("number") == dataNode.name()) {
        return dataNode.text().as_int();
    } else if (QLatin1String("string") == dataNode.name() || QLatin1String("set") == dataNode.name()
               || QLatin1String("enum") == dataNode.name()) {
        return dataNode.text().as_string();
    }

    spdlog::error(R"(QtUiWidget::property - unknown {} property)", name);
    return {};
}

/*!
 * \qmlmethod QtUiWidget::addProperty(string name, var value, object attributes = {})
 * Adds a new property with the given `name`, `value` and `attributes`.
 *
 * Attributes is a has<string, string> object, where the key is the attribute name and the value is the attribute value.
 * For example:
 *
 * ```
 * widget.setProperty("text", "My text", { "comment": "some comment for translation" });
 * ```
 */
void QtUiWidget::addProperty(const QString &name, const QVariant &value, const QHash<QString, QString> &attributes)
{
    LOG("QtUiWidget::addProperty", name, value);

    const auto result =
        qobject_cast<QtUiDocument *>(parent())->uiWriter()->addWidgetProperty(m_widget, name, value, attributes);

    switch (result) {
    case Utils::QtUiWriter::Success:
        qobject_cast<QtUiDocument *>(parent())->setHasChanged(true);
        return;
    case Utils::QtUiWriter::InvalidProperty:
        spdlog::error(R"(QtUiWidget::addProperty - unknown {} type)", value.typeName());
        return;
    case Utils::QtUiWriter::AlreadyExists:
    case Utils::QtUiWriter::InvalidHeader:
        Q_UNREACHABLE();
    }
}

pugi::xml_node QtUiWidget::xmlNode() const
{
    return m_widget;
}

} // namespace Core
