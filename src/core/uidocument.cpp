#include "uidocument.h"

#include <QFile>
#include <QUiLoader>
#include <QWidget>

#include <spdlog/spdlog.h>

#include <algorithm>

namespace Core {

/*!
 * \qmltype UiDocument
 * \brief Provides access to the content of a Ui file (Qt designer file).
 * \instantiates Core::UiDocument
 * \inqmlmodule Script
 * \since 4.0
 */

/*!
 * \qmlproperty std::vector<UiWidget> widgets
 * List of all widgets in the ui file.
 */

UiDocument::UiDocument(QObject *parent)
    : Document(Type::Ui, parent)
{
}

QWidget *UiDocument::widget() const
{
    return nullptr;
}

/*!
 * \qmlmethod UiWidget UiDocument::findWidget( string name)
 * Returns the widget for the given `name`.
 */
UiWidget *UiDocument::findWidget(const QString &name) const
{
    auto it = std::find_if(m_widgets.cbegin(), m_widgets.cend(), [name](UiWidget *widget) {
        return widget->name() == name;
    });
    if (it == m_widgets.cend())
        return nullptr;
    return *it;
}

/*!
 * \qmlmethod UiDocument::preview()
 * Open a dilaog to preview the current ui file.
 */
void UiDocument::preview()
{
    QUiLoader loader;

    QFile file(fileName());
    if (file.open(QIODevice::ReadOnly)) {
        QWidget *widget = loader.load(&file);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    }
}

bool UiDocument::doSave(const QString &fileName)
{
    pugi::xml_node decl = m_document.prepend_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";

    return m_document.save_file(fileName.toLatin1().constData(), "    ");
}

bool UiDocument::doLoad(const QString &fileName)
{
    pugi::xml_parse_result result = m_document.load_file(fileName.toLatin1().constData());

    if (!result) {
        spdlog::critical("{}({}): {}", fileName.toStdString(), result.offset, result.description());
        return false;
    }

    auto widgets = m_document.select_nodes("//widget");

    bool isRoot = true;
    for (auto it = widgets.begin(); it != widgets.end(); ++it) {
        pugi::xpath_node node = *it;
        Q_ASSERT(!node.node().empty());
        m_widgets.push_back(new UiWidget(node.node(), isRoot, this));
        isRoot = false;
    }

    return true;
}

/*!
 * \qmltype UiWidget
 * \brief Provides access to widget attributes in the ui files.
 * \instantiates Core::UiWidget
 * \inqmlmodule Script
 * \since 4.0
 * \sa UiDocument
 */

/*!
 * \qmlproperty string name
 * Name of the widget.
 */
/*!
 * \qmlproperty string className
 * Name of the widget's class.
 */
/*!
 * \qmlproperty bool isRoot
 * Read-only property returning `true` if the widget is the root widget.
 */

UiWidget::UiWidget(pugi::xml_node widget, bool isRoot, QObject *parent)
    : QObject(parent)
    , m_widget(widget)
    , m_isRoot(isRoot)
{
}

QString UiWidget::name() const
{
    return QString::fromLatin1(m_widget.attribute("name").value());
}

void UiWidget::setName(const QString &newName)
{
    m_nameChanged = true;
    m_widget.attribute("name").set_value(newName.toLatin1().constData());
    if (m_isRoot) {
        // Also change the <class> tag
        auto node = m_widget.previous_sibling();
        node.text().set(newName.toLatin1().constData());
    }
}

QString UiWidget::className() const
{
    return QString::fromLatin1(m_widget.attribute("class").value());
}

void UiWidget::setClassName(const QString &newClassName)
{
    m_widget.attribute("class").set_value(newClassName.toLatin1().constData());
}

} // namespace Core
