#include "writer.h"

#include "logging.h"

#include <QIODevice>
#include <QMetaType>
#include <QXmlStreamWriter>

namespace Writer {

void writePropery(QXmlStreamWriter &w, const QString &name, const QVariant &value)
{
    switch (static_cast<QMetaType::Type>(value.type())) {
    case QMetaType::Bool:
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), name);
        w.writeTextElement(QLatin1String("bool"),
                           value.toBool() ? QLatin1String("true") : QLatin1String("false"));
        w.writeEndElement();
        break;
    case QMetaType::Int:
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), name);
        w.writeTextElement(QLatin1String("number"), QString::number(value.toInt()));
        w.writeEndElement();
        break;
    case QMetaType::QString: {
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), name);
        const auto text = value.toString();
        if (name == QLatin1String("alignment")) {
            w.writeTextElement(QLatin1String("set"), text);
        } else {
            if (text.contains(QLatin1String("::")) && !text.contains(QLatin1Char(' ')))
                w.writeTextElement(QLatin1String("enum"), text);
            else
                w.writeTextElement(QLatin1String("string"), text);
        }
        w.writeEndElement();
        break;
    }
    case QMetaType::QStringList: {
        const auto values = value.toStringList();
        for (const auto &text : values) {
            w.writeStartElement(QLatin1String("item"));
            w.writeStartElement(QLatin1String("property"));
            w.writeAttribute(QLatin1String("name"), name);
            w.writeTextElement(QLatin1String("string"), text);
            w.writeEndElement();
            w.writeEndElement();
        }
        break;
    }
    default:
        qCCritical(CONVERTER) << "Unknow property type:" << name << value.toString();
    }
}

void writeWidget(QXmlStreamWriter &w, const Converter::Widget &widget)
{
    w.writeStartElement(QLatin1String("widget"));
    w.writeAttribute(QLatin1String("class"), widget.className);
    w.writeAttribute(QLatin1String("name"), widget.id);

    {
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), QLatin1String("geometry"));
        w.writeStartElement(QLatin1String("rect"));
        w.writeTextElement(QLatin1String("x"), QString::number(widget.geometry.x()));
        w.writeTextElement(QLatin1String("y"), QString::number(widget.geometry.y()));
        w.writeTextElement(QLatin1String("width"), QString::number(widget.geometry.width()));
        w.writeTextElement(QLatin1String("height"), QString::number(widget.geometry.height()));

        w.writeEndElement();
        w.writeEndElement();
    }

    auto it = widget.properties.constBegin();
    for (; it != widget.properties.constEnd(); ++it) {
        writePropery(w, it.key(), it.value());
    }

    for (const auto &child : widget.children) {
        const auto childWidget = child.value<Converter::Widget>();
        writeWidget(w, childWidget);
    }

    w.writeEndElement();
}

void writeUi(QIODevice *device, const Converter::Widget &widget)
{
    QXmlStreamWriter w(device);

    w.setAutoFormatting(true);
    w.writeStartDocument();

    w.writeStartElement(QLatin1String("ui"));
    w.writeAttribute(QLatin1String("version"), QLatin1String("4.0"));

    w.writeTextElement(QLatin1String("class"), widget.id);
    writeWidget(w, widget);
    w.writeEmptyElement(QLatin1String("resources"));
    w.writeEmptyElement(QLatin1String("connections"));

    w.writeEndElement();
}

void writeQrc(QIODevice *device, const QVector<Converter::Asset> &assets, bool useAlias)
{
    QXmlStreamWriter w(device);

    w.setAutoFormatting(true);

    w.writeStartElement(QLatin1String("RCC"));
    w.writeStartElement(QLatin1String("qresource"));
    w.writeAttribute(QLatin1String("prefix"), QLatin1String("/"));

    if (useAlias) {
        for (const auto &asset : assets) {
            w.writeStartElement(QLatin1String("file"));
            w.writeAttribute(QLatin1String("alias"), asset.id);
            w.writeCharacters(asset.fileName);
            w.writeEndElement();
        }
    } else {
        for (const auto &asset : assets)
            w.writeTextElement(QLatin1String("file"), asset.fileName);
    }

    w.writeEndElement();
    w.writeEndElement();
}
}
