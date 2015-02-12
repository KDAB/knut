#include "converter.h"

#include "document.h"

#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>
#include <QFont>

Q_LOGGING_CATEGORY(converter, "converter")

namespace {
static const auto KeyCaption = QStringLiteral("caption");
static const auto KeyChildren = QStringLiteral("children");
static const auto KeyFont = QStringLiteral("font");
static const auto KeyGeometry = QStringLiteral("geometry");
static const auto KeyId = QStringLiteral("id");
static const auto KeyStyle = QStringLiteral("style");
static const auto KeyType = QStringLiteral("type");
static const auto KeyWeight = QStringLiteral("weight");
static const auto KeyCharset = QStringLiteral("charset");
static const auto KeyClass = QStringLiteral("class");
}

static QString labelAlignment(const QString &type, const QStringList &styles)
{
    QStringList alignments;

    if (type == "LTEXT")
        alignments << "Qt::AlignLeft";
    else if (type == "RTEXT")
        alignments << "Qt::AlignRight";
    else if (type == "CTEXT")
        alignments << "Qt::AlignHCenter";

    if (styles.contains("SS_LEFT"))
        alignments << "Qt::AlignLeft";

    if (styles.contains("SS_LEFT"))
        alignments << "Qt::AlignLeft";
    else if (styles.contains("SS_RIGHT"))
         alignments << "Qt::AlignRight";
    else if (styles.contains("SS_CENTER"))
        alignments << "Qt::AlignHCenter" << "Qt::AlignVCenter";

    if (styles.contains("SS_CENTERIMAGE"))
        alignments << "Qt::AlignVCenter";
    else
        alignments << "Qt::AlignTop";

    alignments.removeDuplicates();

    return alignments.join(" | ");
}

static QJsonObject convertLabel(const QJsonObject &widget)
{
    QJsonObject label = widget;

    label["class"] = "QLabel";

    const auto id = widget.value(KeyId).toString();

    label["objectName"] = id;
    label.remove(KeyId);

    const auto styles = label.value(KeyStyle).toVariant().toStringList();
    const auto type = widget.value(KeyType).toString();

    label.remove(KeyType);
    label.remove(KeyStyle);

    label["alignment"] = labelAlignment(type, styles);
    label["wordWrap"] = styles.contains("SS_LEFTNOWORDWRAP") ? "false" : "true";

    return label;
}

static QJsonObject convertStatic(const QJsonObject &widget)
{
    //FIXME switch on the control style (not ex style)
    return widget;
}

static QJsonObject convertControl(const QJsonObject &widget)
{
    const auto controlClass = widget.value(KeyClass).toVariant().toString();

    if (controlClass == "Static")
        return convertStatic(widget);

    return widget;
}

static QJsonObject convertWidget(const QJsonObject &widget)
{
    const auto type = widget.value(KeyType).toString();

    if (type == "LTEXT")
        return convertLabel(widget);
    else if (type == "CTEXT")
        return convertLabel(widget);
    else if (type == "RTEXT")
        return convertLabel(widget);
    else if (type == "CONTROL")
        return convertControl(widget);

    return widget;
}

static QJsonObject convertFont(QJsonObject font, const QString &id)
{
    // Remove charset - Unused for now
    if (font.contains(KeyCharset)) {
        auto charset = font.take(KeyCharset).toString();
        qCDebug(converter)
            << QObject::tr("%1: Unused charset %2").arg(id).arg(charset);
    }

    // Transform typeface into family
    font[QStringLiteral("family")] = font.take("typeface").toString();

    // Transform weight
    if (font.contains(KeyWeight)) {
        switch (font.value(KeyWeight).toInt()) {
        case 100:
        case 200:
        case 300:
            font[KeyWeight] = QFont::Light; break;
        case 500:
        case 600:
            font[KeyWeight] = QFont::DemiBold; break;
        case 700:
            font[KeyWeight] = QFont::Bold; break;
        case 800:
            font[KeyWeight] = QFont::Black; break;
        case 0:
        case 400:
        default:
            // Normal weight - we can remove
            font.remove(KeyWeight);
        }
    }

    return font;
}

QJsonObject convertDialog(const QJsonObject &d)
{
    QJsonObject dialog = d;

    const auto id = dialog.value(KeyId).toString();

    qCDebug(converter) << QObject::tr("=== %1 ===").arg(id);

    // Sanity check
    const auto type = dialog.value(KeyType).toString();
    if (type != "DIALOG" && type != "DIALOGEX") {
        qCWarning(converter)
            << QObject::tr("%1: Don't know type %2").arg(id).arg(type);
    }

    dialog["class"] = "QDialog";
    dialog.remove(KeyType);

    // Caption conversion
    if (dialog.contains(KeyCaption))
        dialog["windowTitle"] = dialog.take(KeyCaption).toString();

    // Font conversion
    if (dialog.contains(KeyFont)) {
        const auto font = dialog.value(KeyFont).toObject();
        dialog[KeyFont] = convertFont(font, id);
    }

    // Widget conversion
    auto children = dialog.value(KeyChildren).toArray();
    for (int i = 0; i < children.count(); ++i) {
        const auto widget = children.at(i).toObject();
        children[i] = convertWidget(widget);
    }
    dialog[KeyChildren] = children;

    // Remove Style - Unused for now
    if (dialog.contains(KeyStyle)) {
        // Remove known styles
        auto styles = dialog.value(KeyStyle).toVariant().toStringList();
        styles.removeOne("WS_CAPTION");
        styles.removeOne("DS_SETFONT");
        styles.removeOne("DS_SHELLFONT");

        if (!styles.isEmpty()) {
            qCDebug(converter)
                << QObject::tr("%1: Unused styles: %2")
                    .arg(id).arg(styles.join(" | "));
        }
    }

    return dialog;
}

QJsonObject convertDialogs(const QJsonObject &dialogs)
{
    QJsonObject result;

    foreach (const QString &key, dialogs.keys()) {
        result[key] = convertDialog(dialogs.value(key).toObject());
    }

    return result;
}
