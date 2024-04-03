#include "uiwriter.h"
#include "data.h"

#include <QRect>
#include <QVariant>

#include <kdalgorithms.h>

#include <spdlog/spdlog.h>

namespace RcCore {

UiWriter::UiWriter(QIODevice *device, QObject *parent)
    : QObject(parent)
    , m_writer(device)
{
    Q_ASSERT(device);

    m_writer.setAutoFormatting(true);
    m_writer.writeStartDocument();

    m_writer.writeStartElement("ui");
    m_writer.writeAttribute("version", "4.0");
}

UiWriter::~UiWriter()
{
    if (!m_customWidgets.isEmpty()) {
        m_writer.writeStartElement("customwidgets");
        for (const auto &widget : std::as_const(m_customWidgets)) {
            m_writer.writeStartElement("customwidget");
            m_writer.writeTextElement("class", widget.className);
            m_writer.writeTextElement("extends", widget.baseClassName);
            {
                m_writer.writeStartElement("header");
                if (widget.isGlobal)
                    m_writer.writeAttribute("location", "global");
                m_writer.writeCharacters(widget.header);
                m_writer.writeEndElement();
            }
            if (widget.isContainer)
                m_writer.writeTextElement("container", "1");
            m_writer.writeEndElement();
        }
        m_writer.writeEndElement();
    }

    m_writer.writeEmptyElement("resources");
    m_writer.writeEmptyElement("connections");

    m_writer.writeEndElement();
}

void UiWriter::setClassName(const QString &className)
{
    m_writer.writeTextElement("class", className);
}

void UiWriter::addCustomWidget(const QString &className, const QString &baseClassName, const QString &header,
                               bool isContainer)
{
    if (m_customWidgets.contains(className))
        return;

    if ((!header.startsWith('<') || !header.endsWith('>')) && (!header.startsWith('"') || !header.endsWith('"'))) {
        spdlog::error(R"(UiWriter::addCustomWidget - the include '{}' is malformed, should be '<foo.h>' or '"foo.h"')",
                      header.toStdString());
    }
    const bool isGlobal = header.startsWith('<');
    const auto &include = header.mid(1, header.length() - 2);
    m_customWidgets[className] = {className, baseClassName, include, isGlobal, isContainer};
}

void UiWriter::startWidget(const QString &className, const RcCore::Widget &widget)
{
    const auto &name = widget.id;
    const bool isQMainWindow = [className, this]() {
        if (className == "QMainWindow")
            return true;
        auto custom = m_customWidgets.value(className);
        return custom.baseClassName == "QMainWindow";
    }();
    m_isMainWindow.push(isQMainWindow);
    m_currentId = name;

    m_writer.writeStartElement("widget");
    m_writer.writeAttribute("class", className);

    // This is to avoid multiple widget having the same name, typically because of IDC_STATIC
    auto widgetName = name;
    const int count = m_widgetName.value(name, 0);
    if (count != 0)
        widgetName += QString::number(count + 1);
    m_writer.writeAttribute("name", widgetName);
    m_widgetName[name] = count + 1;

    // Geometry
    const auto &geometry = widget.geometry;
    m_writer.writeStartElement("property");
    m_writer.writeAttribute("name", "geometry");
    m_writer.writeStartElement("rect");
    m_writer.writeTextElement("x", QString::number(geometry.x()));
    m_writer.writeTextElement("y", QString::number(geometry.y()));
    m_writer.writeTextElement("width", QString::number(geometry.width()));
    m_writer.writeTextElement("height", QString::number(geometry.height()));
    m_writer.writeEndElement();
    m_writer.writeEndElement();

    if (m_isMainWindow.top()) {
        m_writer.writeStartElement("widget");
        m_writer.writeAttribute("class", "QWidget");
        m_writer.writeAttribute("name", "centralwidget");
    }
}

void UiWriter::endWidget()
{
    if (m_isMainWindow.pop())
        m_writer.writeEndElement();
    m_writer.writeEndElement();
}

void UiWriter::addProperty(const QString &name, const QVariant &value)
{
    switch (static_cast<QMetaType::Type>(value.typeId())) {
    case QMetaType::Bool:
        m_writer.writeStartElement("property");
        m_writer.writeAttribute("name", name);
        m_writer.writeTextElement("bool", value.toBool() ? "true" : "false");
        m_writer.writeEndElement();
        break;
    case QMetaType::Int:
        m_writer.writeStartElement("property");
        m_writer.writeAttribute("name", name);
        m_writer.writeTextElement("number", QString::number(value.toInt()));
        m_writer.writeEndElement();
        break;
    case QMetaType::QString: {
        m_writer.writeStartElement("property");
        m_writer.writeAttribute("name", name);
        const auto text = value.toString();
        if (name == "alignment") {
            m_writer.writeTextElement("set", text);
        } else if (name == "text") {
            m_writer.writeStartElement("string");
            m_writer.writeAttribute("comment", m_currentId);
            m_writer.writeCharacters(text);
            m_writer.writeEndElement();
        } else {
            // Good enough for now, may need update for corner cases
            if (text.contains("::") && !text.contains(' '))
                m_writer.writeTextElement("enum", text);
            else
                m_writer.writeTextElement("string", text);
        }
        m_writer.writeEndElement();
        break;
    }
    case QMetaType::QStringList: {
        const auto values = value.toStringList();
        for (const auto &text : values) {
            m_writer.writeStartElement("item");
            m_writer.writeStartElement("property");
            m_writer.writeAttribute("name", name);
            m_writer.writeTextElement("string", text);
            m_writer.writeEndElement();
            m_writer.writeEndElement();
        }
        break;
    }
    default:
        spdlog::error(R"(UiWriter::addProperty - unknown {} type)", value.typeName());
        break;
    }
}

} // namespace RcCore
