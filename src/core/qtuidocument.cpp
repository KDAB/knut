/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qtuidocument.h"
#include "logger.h"
#include "utils/log.h"

#include <QFile>
#include <QUiLoader>
#include <QWidget>
#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype QtUiDocument
 * \brief Provides access to the content of a Ui file (Qt designer file).
 * \inqmlmodule Script
 * \ingroup QtUiDocument/@first
 * \since 1.0
 */

/*!
 * \qmlproperty array<QtUiWidget> QtUiDocument::widgets
 * List of all widgets in the ui file.
 */

QtUiDocument::QtUiDocument(QObject *parent)
    : Document(Type::QtUi, parent)
{
}

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

    initializeXml();

    auto uiNode = m_document.child("ui");
    pugi::xml_node widget =
        parent ? parent->xmlNode().append_child("widget") : uiNode.insert_child_after("widget", uiNode.child("class"));
    widget.append_attribute("class").set_value(className.toLatin1().constData());
    widget.append_attribute("name").set_value(name.toLatin1().constData());

    if (parent == nullptr) {
        // Also change the <class> tag
        auto node = widget.previous_sibling();
        node.text().set(name.toLatin1().constData());
    }

    if (className == "QMainWindow") {
        auto centralWidget = widget.append_child("widget");
        centralWidget.append_attribute("class").set_value("QWidget");
        centralWidget.append_attribute("name").set_value("centralwidget");
    }

    QtUiWidget *newWidget = new QtUiWidget(widget, parent == nullptr, this);
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

    const auto customPath = "customwidget[@class='" % className % "']";
    auto customNode = m_document.select_node(customPath.toLatin1().constData()).node();
    if (!customNode.empty()) {
        spdlog::info(R"(QtUiDocument::addCustomWidget - the custom widget '{}' already exists)", className);
        return;
    }

    if ((!header.startsWith('<') || !header.endsWith('>')) && (!header.startsWith('"') || !header.endsWith('"'))) {
        spdlog::error(
            R"(QtUiDocument::addCustomWidget - the include '{}' is malformed, should be '<foo.h>' or '"foo.h"')",
            header);
        return;
    }
    const bool isGlobal = header.startsWith('<');
    const auto &include = header.mid(1, header.length() - 2);

    auto customWidgets = m_document.select_node("ui/customwidgets").node();
    if (customWidgets.empty())
        customWidgets = m_document.child("ui").append_child("customwidgets");

    customNode = customWidgets.append_child("customwidget");
    customNode.append_child("class").text().set(className.toLatin1().constData());
    customNode.append_child("extends").text().set(baseClassName.toLatin1().constData());
    auto headerNode = customNode.append_child("header");
    headerNode.text().set(include.toLatin1().constData());
    if (isGlobal)
        headerNode.append_attribute("location").set_value("global");
    if (isContainer)
        customNode.append_child("container").text().set("1");
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

void QtUiDocument::initializeXml()
{
    auto ui = m_document.select_node("ui");
    if (ui.node().empty()) {
        pugi::xml_node uiNode = m_document.append_child("ui");
        uiNode.append_attribute("version").set_value("4.0");
        uiNode.append_child("class");
        uiNode.append_child("resources");
        uiNode.append_child("connections");
    }
}

/*!
 * \qmltype QtUiWidget
 * \brief Provides access to widget attributes in the ui files.
 * \inqmlmodule Script
 * \ingroup QtUiDocument
 * \since 1.0
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

    m_widget.attribute("name").set_value(newName.toLatin1().constData());
    if (m_isRoot) {
        // Also change the <class> tag
        auto node = m_widget.previous_sibling();
        node.text().set(newName.toLatin1().constData());
    }
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

    m_widget.attribute("class").set_value(newClassName.toLatin1().constData());
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

    // Special case for stringlist
    if (static_cast<QMetaType::Type>(value.typeId()) == QMetaType::QStringList) {
        const auto values = value.toStringList();
        for (const auto &text : values) {
            auto valueNode = m_widget.append_child("item").append_child("property");
            valueNode.append_attribute("name").set_value(name.toLatin1().constData());
            valueNode.append_child("string").text().set(text.toLatin1().constData());
        }
        return;
    }

    auto propertyNode = m_widget.append_child("property");
    propertyNode.append_attribute("name").set_value(name.toLatin1().constData());

    pugi::xml_node dataNode;
    switch (static_cast<QMetaType::Type>(value.typeId())) {
    case QMetaType::QRect:
        dataNode = propertyNode.append_child("rect");
        dataNode.append_child("x").text().set(value.toRect().x());
        dataNode.append_child("y").text().set(value.toRect().y());
        dataNode.append_child("width").text().set(value.toRect().width());
        dataNode.append_child("height").text().set(value.toRect().height());
        break;
    case QMetaType::Bool:
        dataNode = propertyNode.append_child("bool");
        dataNode.text().set(value.toBool());
        break;
    case QMetaType::Int:
        dataNode = propertyNode.append_child("number");
        dataNode.text().set(value.toInt());
        break;
    case QMetaType::QString: {
        const auto text = value.toString();
        if (name.compare("alignment") == 0) {
            dataNode = propertyNode.append_child("set");
        } else {
            // Good enough for now, may need update for corner cases
            if (text.contains("::") && !text.contains(' '))
                dataNode = propertyNode.append_child("enum");
            else
                dataNode = propertyNode.append_child("string");
        }
        dataNode.text().set(text.toLatin1().constData());
        break;
    }
    case QMetaType::QStringList: {
        break;
    }
    default:
        spdlog::error(R"(QtUiWidget::addProperty - unknown {} type)", value.typeName());
        return;
    }

    for (const auto &attribute : attributes.asKeyValueRange()) {
        dataNode.append_attribute(attribute.first.toLatin1().constData())
            .set_value(attribute.second.toLatin1().constData());
    }

    // Document has changed
    qobject_cast<QtUiDocument *>(parent())->setHasChanged(true);
}

pugi::xml_node QtUiWidget::xmlNode() const
{
    if (className() == "QMainWindow")
        return m_widget.child("widget");
    return m_widget;
}

} // namespace Core
