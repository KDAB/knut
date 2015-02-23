#include "xmlwriter.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QXmlStreamWriter>

#include <QDebug>

static inline void beginUi(QXmlStreamWriter &w)
{
    w.writeStartElement("ui");
    w.writeAttribute("version", "4.0");
}

static inline void endUi(QXmlStreamWriter &w)
{
    w.writeEndElement();
}

static inline void writeStringAttribute(QXmlStreamWriter &w,
        const QString &name, const QJsonValue &value)
{
    w.writeAttribute(name, value.toString());
}

static inline void writeIntText(QXmlStreamWriter &w,
        const QString &name, const QJsonValue &value)
{
    w.writeTextElement(name, QString::number(value.toInt()));
}

static inline void beginProperty(QXmlStreamWriter &w, const QString &name)
{
    w.writeStartElement("property");
    w.writeAttribute("name", name);
}

static inline void endProperty(QXmlStreamWriter &w)
{
    w.writeEndElement();
}

static void writeSimpleProperty(QXmlStreamWriter &w, const QString &name,
        const QString &value, const QString &type)
{
    beginProperty(w, name);
    w.writeTextElement(type, value);
    endProperty(w);
}

static void writeSimpleProperty(QXmlStreamWriter &w, const QJsonObject &widget,
        const QString &name, const QString &type)
{
    if (!widget.contains(name))
        return;

    const auto value = widget.value(name).toString();

    writeSimpleProperty(w, name, value, type);
}

static inline void writeBoolProperty(QXmlStreamWriter &w, const QString &name,
        const QJsonObject &widget)
{
    writeSimpleProperty(w, widget, name, "bool");
}

static inline void writeEnumProperty(QXmlStreamWriter &w, const QString &name,
        const QJsonObject &widget)
{
    writeSimpleProperty(w, widget, name, "enum");
}

static inline void writeStringProperty(QXmlStreamWriter &w, const QString &name,
        const QJsonObject &widget)
{
    writeSimpleProperty(w, widget, name, "string");
}

static inline void writeSetProperty(QXmlStreamWriter &w, const QString &name,
        const QJsonObject &widget)
{
    writeSimpleProperty(w, widget, name, "set");
}

static inline void writePixmapProperty(QXmlStreamWriter &w, const QString &name,
        const QJsonObject &widget)
{
    writeSimpleProperty(w, widget, name, "pixmap");
}

static void writeIconProperty(QXmlStreamWriter &w, const QString &icon)
{
    beginProperty(w, "icon");
    // FIXME - write iconset
    endProperty(w);
}

static void writeIconProperty(QXmlStreamWriter &w, const QJsonObject &widget)
{
    if (!widget.contains("icon"))
        return;

    writeIconProperty(w, widget.value("icon").toString());
}

static void writeGeometry(QXmlStreamWriter &w, const QJsonValue &value)
{
    QJsonObject geometry = value.toObject();

    beginProperty(w, "geometry");

    w.writeStartElement("rect");

    writeIntText(w, "x", geometry.value("x"));
    writeIntText(w, "y", geometry.value("y"));
    writeIntText(w, "width", geometry.value("width"));
    writeIntText(w, "height", geometry.value("height"));

    w.writeEndElement(); // rect

    endProperty(w);
}

static void writeQPushButton(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "checkable", widget);
    writeBoolProperty(w, "default", widget);
    writeBoolProperty(w, "flat", widget);
    writeIconProperty(w, widget);
}

static void writeQLabel(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "scaledContents", widget);
    writeBoolProperty(w, "wordWrap", widget);
    writeSetProperty(w, "alignment", widget);
    writePixmapProperty(w, "pixmap", widget);
    writeStringProperty(w, "inputMask", widget);
}

static void writeQComboBox(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "editable", widget);
    writeEnumProperty(w, "insertPolicy", widget);
}

static void writeQRadioButton(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "editable", widget);
    writeEnumProperty(w, "insertPolicy", widget);
}

static void writeQGroupBox(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeStringProperty(w, "title", widget);
}

static void writeQLineEdit(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "readOnly", widget);
    writeSetProperty(w, "alignment", widget);
    writeEnumProperty(w, "echoMode", widget);
}

static void writeQCheckBox(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "tristate", widget);
}

static void writeQSlider(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeEnumProperty(w, "orientation", widget);
    writeEnumProperty(w, "tickPosition", widget);
}

static void writeQScrollBar(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeEnumProperty(w, "orientation", widget);
}


static void writeQDateTimeEdit(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeBoolProperty(w, "calendarPopup", widget);
    writeStringProperty(w, "displayFormat", widget);
}

static void writeQTabWidget(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    writeEnumProperty(w, "tabPosition", widget);
}

static void writeClassSpecicStuff(QXmlStreamWriter &w,
        const QJsonObject &widget)
{
    const auto widgetClass = widget.value("class").toString();

    if (widgetClass == "QCheckBox")
        writeQCheckBox(w, widget);
    else if (widgetClass == "QCalendarWidget")
        /* empty */;
    else if (widgetClass == "QComboBox")
        writeQComboBox(w, widget);
    else if (widgetClass == "QDateTimeEdit")
        writeQDateTimeEdit(w, widget);
    else if (widgetClass == "QGroupBox")
        writeQGroupBox(w, widget);
    else if (widgetClass == "QLabel")
        writeQLabel(w, widget);
    else if (widgetClass == "QLineEdit")
        writeQLineEdit(w, widget);
    else if (widgetClass == "QListWidget")
        /* empty */;
    else if (widgetClass == "QProgressBar")
        /* empty */;
    else if (widgetClass == "QPushButton")
        writeQPushButton(w, widget);
    else if (widgetClass == "QRadioButton")
        writeQRadioButton(w, widget);
    else if (widgetClass == "QScrollBar")
        writeQScrollBar(w, widget);
    else if (widgetClass == "QSpinBox")
        /* empty */;
    else if (widgetClass == "QSlider")
        writeQSlider(w, widget);
    else if (widgetClass == "QTabWidget")
        writeQTabWidget(w, widget);
    else if (widgetClass == "QTextEdit")
        writeQLineEdit(w, widget);
    else if (widgetClass == "QTreeWidget")
        /* empty */;
}

static void writeWidget(QXmlStreamWriter &w, const QJsonObject &widget)
{
    w.writeStartElement("widget");

    writeStringAttribute(w, "class", widget.value("class"));
    writeStringAttribute(w, "name", widget.value("id"));
    writeGeometry(w, widget.value("geometry"));

    writeEnumProperty(w, "frameShape", widget);
    writeEnumProperty(w, "frameShadow", widget);

    writeBoolProperty(w, "enabled", widget);

    writeStringProperty(w, "text", widget);

    if (widget.contains("children")) {
        QJsonArray children = widget.value("children").toArray();

        for (const auto &value : children) {
            QJsonObject widget = value.toObject();
            writeWidget(w, widget);
        }
    }

    writeClassSpecicStuff(w, widget);

    w.writeEndElement(); // widget
}

static void writeDialogs(QXmlStreamWriter &w, const QJsonObject &dialogs)
{
    for (const auto &value : dialogs) {
        QJsonObject dialog = value.toObject();
        writeWidget(w, dialog);
    }
}

QByteArray dialogToUi(const QJsonObject &dialog)
{
    QByteArray buffer;

    QXmlStreamWriter w(&buffer);

    w.setAutoFormatting(true);
    w.writeStartDocument();

    beginUi(w);

    writeWidget(w, dialog);

    endUi(w);

    return buffer;
}
