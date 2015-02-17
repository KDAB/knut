#include "converter.h"

#include "document.h"

#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>
#include <QFont>

#include <functional>

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
static const auto KeyItalic = QStringLiteral("italic");
static const auto KeyCharset = QStringLiteral("charset");
static const auto KeyTypeface = QStringLiteral("typeface");
static const auto KeyPointsize = QStringLiteral("pointsize");
static const auto KeyClass = QStringLiteral("class");

std::function<QString(int)> idToPath = [](int id) { return QString::number(id); };
}

static QStringList getStyle(const QJsonObject &widget)
{
    return widget.value(KeyStyle).toVariant().toStringList();
}

static void removeStyle(QJsonObject &widget, const QString &style)
{
    auto s = getStyle(widget);
    s.removeOne(style);
    widget[KeyStyle] = QJsonArray::fromStringList(s);
}

static void removeStyles(QJsonObject &widget, const QStringList &styles)
{
    auto s = getStyle(widget);
    std::for_each(styles.cbegin(), styles.cend(), [&](const QString &style){ s.removeOne(style); });
    widget[KeyStyle] = QJsonArray::fromStringList(s);
}

// Extended and Window styles
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
static void convertGeneralStyle(QJsonObject &widget)
{

    const auto styles = getStyle(widget);

    if (styles.contains("WS_EX_CLIENTEDGE")) {
        widget["frameshape"] = "QFrame::Plain";
        widget["frameShadow"] = "QFrame::Sunken";
    }
    if (styles.contains("WS_BORDER"))
        widget["frameshape"] = "QFrame::Box";

    removeStyles(widget, {"WS_EX_CLIENTEDGE", "WS_BORDER"});
}

// LTEXT, CTEXT and RTEXT
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa381021%28v=vs.85%29.aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa380915(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa381044(v=vs.85).aspx
static void convertLabel(QJsonObject &widget)
{
    widget["class"] = "QLabel";
    convertGeneralStyle(widget);

    // Alignment
    const auto type = widget.value(KeyType).toString();
    QString align;
    if (getStyle(widget).contains("SS_CENTERIMAGE"))
        align = "Qt::AlignVCenter";
    else
        align = "Qt::AlignTop";
    if (type == "LTEXT")
        align += "|Qt::AlignLeft";
    if (type == "RTEXT")
        align += "|Qt::AlignRight";
    else if (type == "CTEXT")
        align += "|Qt::AlignHCenter";

    widget["alignment"] = align;

    // Those are known by the type of the label, just remove them
    removeStyles(widget, {"SS_CENTERIMAGE", "SS_LEFT", "SS_CENTER", "SS_RIGHT"});
}

// STATIC CONTROL
// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb760773(v=vs.85).aspx
static void convertStatic(QJsonObject &widget)
{
    widget["class"] = "QLabel";
    convertGeneralStyle(widget);
    const auto styles = getStyle(widget);

    // Alignement
    if (styles.contains("SS_RIGHT"))
        widget["alignment"] = "Qt::AlignRight";
    if (styles.contains("SS_CENTER"))
        widget["alignment"] = "Qt::AlignHCenter";
    if (styles.contains("SS_CENTERIMAGE"))
        widget["alignment"] = "Qt::AlignCenter";

    // Frame
    if (styles.contains("SS_SUNKEN"))
        widget["frameshape"] = "QFrame::Plain";
        widget["frameShadow"] = "QFrame::Sunken";
    if (styles.contains("SS_BLACKFRAME"))
        widget["frameshape"] = "QFrame::Box";

    if (styles.contains("SS_REALSIZECONTROL"))
        widget["scaledContents"] = true;

    if (styles.contains("SS_BITMAP") || styles.contains("SS_ICON"))
        widget["pixmap"] = idToPath(widget.take("text").toString().toInt());

    if (!styles.contains("SS_LEFTNOWORDWRAP"))
        widget["wordWrap"] = true;

    removeStyles(widget, {"SS_LEFT", "SS_RIGHT", "SS_CENTER", "SS_CENTERIMAGE",
                 "SS_SUNKEN", "SS_BLACKFRAME", "SS_REALSIZECONTROL",
                 "SS_BITMAP", "SS_ICON", "SS_LEFTNOWORDWRAP"});
}


static bool convertControl(QJsonObject &widget)
{
    const auto controlClass = widget.value(KeyClass).toString();

    if (controlClass == "Static")
        convertStatic(widget);
    else
        return false;
    return true;
}

static QJsonObject convertWidget(QJsonObject widget, const QString &id)
{
    const auto type = widget.value(KeyType).toString();
    const auto wid = widget.take(KeyId).toString();

    if (type == "LTEXT" || type == "CTEXT" || type == "RTEXT") {
        convertLabel(widget);
    } else if (type == "CONTROL") {
        if (!convertControl(widget)) {
            qCWarning(converter)
                << QObject::tr("%1: Unknow control %2 class %3")
                   .arg(id).arg(wid).arg(widget.value(KeyClass).toString());
            return widget;
        }
    } else {
        qCWarning(converter)
            << QObject::tr("%1: Unknow widget %2 type %3").arg(id).arg(wid).arg(type);
        return widget;
    }

    // Remove used keys
    widget.remove(KeyType);
    const auto styles = widget.take(KeyStyle).toVariant().toStringList();
    if (!styles.isEmpty()) {
        qCDebug(converter)
            << QObject::tr("%1: Unused styles for widget %2: %3")
                .arg(id).arg(wid).arg(styles.join(" | "));
    }

    // Set the object name
    widget["objectName"] = wid;

    return widget;
}

static bool isDefaultFont(QJsonObject font)
{
    auto typeface = font.value(KeyTypeface).toString();
    if (typeface == "MS Sans Serif" ||
            typeface == "MS Shell Dlg" ||
            typeface == "Microsoft Sans Serif") {
        auto weight = font.value(KeyWeight).toInt();
        return font.value(KeyPointsize).toInt() == 8 &&
                font.value(KeyItalic).toInt() == 0 &&
                (weight == 0 || weight == 400);
    }
    return false;
}

// FONT
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa381013(v=vs.85).aspx
static QJsonObject convertFont(QJsonObject font, const QString &id)
{

    // Remove charset - Unused for now
    if (font.contains(KeyCharset)) {
        auto charset = font.take(KeyCharset).toInt();
        qCDebug(converter)
            << QObject::tr("%1: Unused charset %2").arg(id).arg(charset);
    }

    // Transform typeface into family
    font[QStringLiteral("family")] = font.take(KeyTypeface).toString();

    // Remove italic if unused
    if (!font.value(KeyItalic).toInt(0)) {
        font.take(KeyItalic);
    }

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
        if (isDefaultFont(font))
            dialog.take(KeyFont);
        else
            dialog[KeyFont] = convertFont(font, id);
    }

    // Widget conversion
    auto children = dialog.value(KeyChildren).toArray();
    for (int i = 0; i < children.count(); ++i) {
        const auto widget = children.at(i).toObject();
        children[i] = convertWidget(widget, id);
    }
    dialog[KeyChildren] = children;

    // Remove Style - Unused for now
    if (dialog.contains(KeyStyle)) {
        // Remove known styles
        removeStyles(dialog, {"WS_CAPTION", "DS_SETFONT", "DS_SHELLFONT"});

        const auto styles = dialog.take(KeyStyle).toVariant().toStringList();
        if (!styles.isEmpty()) {
            qCDebug(converter)
                << QObject::tr("%1: Unused styles: %2")
                    .arg(id).arg(styles.join(" | "));
        }
    }

    return dialog;
}

QJsonObject convertDialogs(const QJsonObject &root)
{
    QJsonObject result;

    // Set the local function idToPath
    idToPath = [&root](int id) { return documentAsset(root, id);};

    auto dialogs = documentDialogs(root);
    foreach (const QString &key, dialogs.keys()) {
        result[key] = convertDialog(dialogs.value(key).toObject());
    }

    return result;
}
