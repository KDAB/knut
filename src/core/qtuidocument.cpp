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
 * \qmlmethod QtUiDocument::preview()
 * Open a dialog to preview the current ui file.
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

    return true;
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
/*!
 * \qmlproperty bool QtUiWidget::id
 * Read-only property returning the id of the widget (property idString).
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

QString QtUiWidget::id() const
{
    return QString::fromLatin1(m_widget.select_node("property[@name='idString']/string").node().text().as_string());
}

void QtUiWidget::setClassName(const QString &newClassName)
{
    LOG("QtUiWidget::setClassName", newClassName);

    m_widget.attribute("class").set_value(newClassName.toLatin1().constData());
    qobject_cast<QtUiDocument *>(parent())->setHasChanged(true);
    emit classNameChanged(newClassName);
}

} // namespace Core
