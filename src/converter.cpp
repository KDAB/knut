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

static void removeStyles(QJsonObject &widget, const QStringList &styles)
{
    auto s = getStyle(widget);
    std::for_each(styles.cbegin(), styles.cend(), [&](const QString &style){ s.removeOne(style); });
    widget[KeyStyle] = QJsonArray::fromStringList(s);
}

static bool takeStyle(QJsonObject &widget, const QString &style)
{
    auto s = getStyle(widget);

    if (!s.contains(style))
        return false;

    s.removeAll(style);

    widget[KeyStyle] = QJsonArray::fromStringList(s);

    return true;
}

// Extended and Window styles
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
static void convertGeneralStyle(QJsonObject &widget)
{
    if (takeStyle(widget, "WS_EX_CLIENTEDGE")) {
        widget["frameShape"] = "QFrame::Plain";
        widget["frameShadow"] = "QFrame::Sunken";
    }

    if (takeStyle(widget, "WS_BORDER"))
        widget["frameShape"] = "QFrame::Box";
    if (takeStyle(widget, "WS_DISABLED"))
        widget["enabled"] = "false";

    // WS_TABSTOP is handled by Qt widgets (focus navigation)
    takeStyle(widget, "WS_TABSTOP");

    // 0 means there's no style (used with the extended styles)
    takeStyle(widget, "0");
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

// COMBOBOX
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa380889(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb775796(v=vs.85).aspx
static void convertComboBox(QJsonObject &widget)
{
    widget["class"] = "QComboBox";
    convertGeneralStyle(widget);
    const auto styles = getStyle(widget);

    // Type CBS_DROPDOWNLIST is the default QComboBox
    if (styles.contains("CBS_SIMPLE")) {
        // It's more a QListWidget than a QComboBox
        widget["class"] = "QListWidget";
    } else if (styles.contains("CBS_DROPDOWN")) {
        widget["editable"] = "true";
        widget["insertPolicy"] = "QComboBox::NoInsert";
    }

    removeStyles(widget, {"CBS_SIMPLE", "CBS_DROPDOWN", "CBS_DROPDOWNLIST",
                          "WS_VSCROLL"});
}

// STATIC CONTROL
// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb760773(v=vs.85).aspx
static void convertStatic(QJsonObject &widget)
{
    widget["class"] = "QLabel";
    convertGeneralStyle(widget);

    // Alignement
    if (takeStyle(widget, "SS_RIGHT"))
        widget["alignment"] = "Qt::AlignRight";
    if (takeStyle(widget, "SS_CENTER"))
        widget["alignment"] = "Qt::AlignHCenter";
    if (takeStyle(widget, "SS_CENTERIMAGE"))
        widget["alignment"] = "Qt::AlignCenter";

    // Frame
    if (takeStyle(widget, "SS_SUNKEN")) {
        widget["frameShape"] = "QFrame::Plain";
        widget["frameShadow"] = "QFrame::Sunken";
    }

    if (takeStyle(widget, "SS_BLACKFRAME"))
        widget["frameShape"] = "QFrame::Box";

    if (takeStyle(widget, "SS_REALSIZECONTROL"))
        widget["scaledContents"] = "true";

    const auto type = widget.value(KeyType).toString();
    if (takeStyle(widget, "SS_BITMAP") || takeStyle(widget, "SS_ICON") || type == "ICON")
        widget["pixmap"] = idToPath(widget.take("text").toString().toInt());

    if (!takeStyle(widget, "SS_LEFTNOWORDWRAP"))
        widget["wordWrap"] = "true";

    takeStyle(widget, "SS_LEFT");
}

static void convertPushButton(QJsonObject &button)
{
    button["class"] = "QPushButton";

    convertGeneralStyle(button);

    if (takeStyle(button, "BS_AUTO3STATE") || takeStyle(button, "BS_3STATE"))
        button["checkable"] = "true";

    if (takeStyle(button, "BS_DEFPUSHBUTTON"))
        button["default"] = "true";

    const bool hasIcon = (takeStyle(button, "BS_BITMAP")
            || takeStyle(button, "BS_ICON"));

    if (hasIcon)
        button["icon"] = "FIXME";

    const auto type = button.value(KeyType).toString();
    if (takeStyle(button, "BS_FLAT") || type == "PUSHBOX")
        button["flat"] = "true";

    // Already taken care of with PUSHBOX type
    takeStyle(button, "BS_PUSHBOX");
}

static void convertCheckBox(QJsonObject &checkbox)
{
    if (takeStyle(checkbox, "BS_PUSHLIKE")) {
        // treat like a push button
        convertPushButton(checkbox);
        return;
    }

    checkbox["class"] = "QCheckBox";

    convertGeneralStyle(checkbox);

    const auto type = checkbox.value(KeyType).toString();
    if (takeStyle(checkbox, "BS_3STATE") || type == "STATE3")
        checkbox["tristate"] = "true";

    // Default behavior for a checkbox
    takeStyle(checkbox, "BS_AUTOCHECKBOX");
}

static void convertGroupBox(QJsonObject &groupBox)
{
    groupBox["class"] = "QGroupBox";
    groupBox["title"] = groupBox.take("text");

    convertGeneralStyle(groupBox);
}

static void convertRadioButton(QJsonObject &radioButton)
{
    radioButton["class"] = "QRadioButton";

    convertGeneralStyle(radioButton);
}

// EDITTEXT CONTROL
// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775464(v=vs.85).aspx
static void convertEditText(QJsonObject &editText)
{
    const auto controlClass = editText.value(KeyClass).toString();

    if (takeStyle(editText, "ES_MULTILINE") || controlClass == "RICHEDIT") {
        editText["class"] = "QTextEdit";

        removeStyles(editText,
                {"ES_CENTER", "ES_LEFT", "ES_RIGHT", "ES_PASSWORD"});
    } else {
        editText["class"] = "QLineEdit";

        QString align;

        if (takeStyle(editText, "ES_CENTER"))
            align = "Qt::AlignCenter";
        else if (takeStyle(editText, "ES_LEFT"))
            align = "Qt::AlignLeft";
        else if (takeStyle(editText, "ES_RIGHT"))
            align = "Qt::AlignRight";
        else
            align = "Qt::AlignLeft";

        align += "|Qt::AlignVCenter";

        editText["alignment"] = align;

        if (takeStyle(editText, "ES_PASSWORD"))
            editText["echoMode"] = "QLineEdit::Password";
    }

    if (takeStyle(editText, "ES_READONLY"))
        editText["readOnly"] = "true";
}

// BUTTON CONTROL
// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775951(v=vs.85).aspx
static void convertButton(QJsonObject &widget)
{
    const auto styles = widget.value(KeyStyle).toVariant().toStringList();

    if (styles.contains("BS_3STATE"))
        convertCheckBox(widget);
    else if (styles.contains("BS_AUTO3STATE"))
        convertCheckBox(widget);
    else if (styles.contains("BS_AUTOCHECKBOX"))
        convertCheckBox(widget);
    else if (styles.contains("BS_AUTORADIOBUTTON"))
        convertRadioButton(widget);
    else if (styles.contains("BS_CHECKBOX"))
        convertCheckBox(widget);
    else if (styles.contains("BS_GROUPBOX"))
        convertGroupBox(widget);
    else if (styles.contains("BS_DEFPUSHBUTTON"))
        convertPushButton(widget);
    else if (styles.contains("BS_PUSHBUTTON"))
        convertPushButton(widget);
    else if (styles.contains("BS_RADIOBUTTON"))
        convertRadioButton(widget);
}

// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb760147%28v=vs.85%29.aspx
static void convertSlider(QJsonObject &widget)
{
    widget["class"] = "QSlider";
    widget.take("text");
    convertGeneralStyle(widget);

    if (takeStyle(widget, "TBS_HORZ"))
        widget["orientation"] = "Qt::Horizontal";
    else
        widget["orientation"] = "Qt::Vertical";
    takeStyle(widget, "TBS_VERT");

    if (takeStyle(widget, "TBS_NOTICKS"))
        widget["tickPosition"] = "QSlider::NoTicks";
    if (takeStyle(widget, "TBS_BOTH"))
        widget["tickPosition"] = "QSlider::TicksBothSides";
    if (takeStyle(widget, "TBS_LEFT"))
        widget["tickPosition"] = "QSlider::TicksLeft";
    if (takeStyle(widget, "TBS_RIGHT"))
        widget["tickPosition"] = "QSlider::TicksRight";
    if (takeStyle(widget, "TBS_TOP"))
        widget["tickPosition"] = "QSlider::TicksAbove";
    if (takeStyle(widget, "TBS_BOTTOM"))
        widget["tickPosition"] = "QSlider::TicksBelow";
}

// LISTBOX or SysListView32 CONTROL
// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775149(v=vs.85).aspx#LBS_MULTIPLESEL
static void convertListWidget(QJsonObject &widget)
{
    widget["class"] = "QListWidget";
    widget.take("text");
    convertGeneralStyle(widget);

    // The control is an icon view
    const auto type = widget.value(KeyType).toString();
    if (type == "CONTROL")
        widget["viewMode"] = "QListView::IconMode";

    if (takeStyle(widget, "LBS_NOSEL"))
        widget["selectionMode"] = "QAbstractItemView::NoSelection";
    else if (takeStyle(widget, "LBS_MULTIPLESEL"))
        widget["selectionMode"] = "QAbstractItemView::MultiSelection";
    else if (takeStyle(widget, "LBS_EXTENDEDSEL"))
        widget["selectionMode"] = "QAbstractItemView::ExtendedSelection";
    else
        widget["selectionMode"] = "QAbstractItemView::SingleSelection";

    if (takeStyle(widget, "LBS_SORT"))
        widget["sortingEnabled"] = "true";
    if (takeStyle(widget, "LBS_STANDARD"))
        widget["sortingEnabled"] = "true";

    bool alwaysOn = takeStyle(widget, "LBS_DISABLENOSCROLL");
    if (takeStyle(widget, "WS_HSCROLL"))
        widget["horizontalScrollBarPolicy"] =
                alwaysOn ? "Qt::ScrollBarAlwaysOn" : "Qt::ScrollBarAsNeeded";
    else
        widget["horizontalScrollBarPolicy"] = "Qt::ScrollBarAlwaysOff";
    if (takeStyle(widget, "WS_VSCROLL"))
        widget["verticalScrollBarPolicy"] =
                alwaysOn ? "Qt::ScrollBarAlwaysOn" : "Qt::ScrollBarAsNeeded";
    else
        widget["verticalScrollBarPolicy"] = "Qt::ScrollBarAlwaysOff";
}

// SCROLLBAR
// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb787533%28v=vs.85%29.aspx
static void convertScrollBar(QJsonObject &widget)
{
    widget["class"] = "QScrollBar";
    convertGeneralStyle(widget);

    if (takeStyle(widget, "SBS_VERT"))
        widget["orientation"] = "Qt::Vertical";
    else
        widget["orientation"] = "Qt::Horizontal";

    takeStyle(widget, "SBS_HORZ");
}

// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb759885%28v=vs.85%29.aspx
static void convertSpinBox(QJsonObject &widget)
{
    widget["class"] = "QSpinBox";
    widget.take("text");
    convertGeneralStyle(widget);
}

// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb760820(v=vs.85).aspx
static void convertProgressBar(QJsonObject &widget)
{
    widget["class"] = "QProgressBar";
    widget.take("text");

    convertGeneralStyle(widget);

    // QProgressBar does not support these
    widget.take("frameShape");
    widget.take("frameShadow");
}

static void convertCalendarWidget(QJsonObject &widget)
{
    widget["class"] = "QCalendarWidget";
    widget.take("text");
    convertGeneralStyle(widget);
}

// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb761728%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
static void convertDateTime(QJsonObject &widget)
{
    widget["class"] = "QDateTimeEdit";
    widget.take("text");
    convertGeneralStyle(widget);

    if (takeStyle(widget, "DTS_LONGDATEFORMAT"))
        widget["displayFormat"] = "dddd, MMMM dd, yyyy";
    if (takeStyle(widget, "DTS_SHORTDATEFORMAT"))
        widget["displayFormat"] = "M/d/yy";
    if (takeStyle(widget, "DTS_SHORTDATECENTURYFORMAT"))
        widget["displayFormat"] = "M/d/yyyy";
    if (takeStyle(widget, "DTS_TIMEFORMAT"))
        widget["displayFormat"] = "hh:mm:ss";

    widget["calendarPopup"] = "true";
}

static void convertIpAddress(QJsonObject &widget)
{
    widget["class"] = "QLineEdit";
    convertGeneralStyle(widget);

    widget["inputMask"] = "000.000.000.000;_";
}

// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb760013%28v=vs.85%29.aspx
static void convertTreeWidget(QJsonObject &widget)
{
    widget["class"] = "QTreeWidget";
    widget.take("text");
    convertGeneralStyle(widget);
}

// Style: https://msdn.microsoft.com/en-us/library/windows/desktop/bb760549%28v=vs.85%29.aspx
static void convertTabWidget(QJsonObject &widget)
{
    widget["class"] = "QTabWidget";
    widget.take("text");
    convertGeneralStyle(widget);

    if (takeStyle(widget, "TCS_BOTTOM"))
        widget["tabPosition"] = "QTabWidget::South";
    if (takeStyle(widget, "TCS_VERTICAL"))
        widget["tabPosition"] = "QTabWidget::West";
    if (takeStyle(widget, "TCS_RIGHT"))
        widget["tabPosition"] = "QTabWidget::East";
}

static bool convertControl(QJsonObject &widget)
{
    const auto controlClass = widget.value(KeyClass).toString();

    if (controlClass == "Static")
        convertStatic(widget);
    else if (controlClass == "Button")
        convertButton(widget);
    else if (controlClass == "ComboBox")
        convertComboBox(widget);
    else if (controlClass == "ComboBoxEx32")
        convertComboBox(widget);
    else if (controlClass == "Edit")
        convertEditText(widget);
    else if (controlClass == "RichEdit20W")
        convertEditText(widget);
    else if (controlClass == "msctls_trackbar")
        convertSlider(widget);
    else if (controlClass == "msctls_trackbar32")
        convertSlider(widget);
    else if (controlClass == "msctls_updown")
        convertSpinBox(widget);
    else if (controlClass == "msctls_updown32")
        convertSpinBox(widget);
    else if (controlClass == "msctls_progress")
        convertProgressBar(widget);
    else if (controlClass == "msctls_progress32")
        convertProgressBar(widget);
    else if (controlClass == "ScrollBar")
        convertScrollBar(widget);
    else if (controlClass == "SysMonthCal32")
        convertCalendarWidget(widget);
    else if (controlClass == "SysDateTimePick32")
        convertDateTime(widget);
    else if (controlClass == "SysIPAddress32")
        convertIpAddress(widget);
    else if (controlClass == "SysListView")
        convertListWidget(widget);
    else if (controlClass == "SysListView32")
        convertListWidget(widget);
    else if (controlClass == "SysTreeView")
        convertTreeWidget(widget);
    else if (controlClass == "SysTreeView32")
        convertTreeWidget(widget);
    else if (controlClass == "SysTabControl")
        convertTabWidget(widget);
    else if (controlClass == "SysTabControl32")
        convertTabWidget(widget);
    else if (controlClass == "SysLink")
        convertLabel(widget);
    else
        return false;
    return true;
}

static QJsonObject convertWidget(QJsonObject widget, const QString &id)
{
    const auto type = widget.value(KeyType).toString();
    const auto wid = widget.value(KeyId).toString();

    if (type == "AUTO3STATE") {
        convertPushButton(widget);
    } else if (type == "AUTOCHECKBOX") {
        convertPushButton(widget);
    } else if (type == "AUTORADIOBUTTON") {
        convertRadioButton(widget);
    } else if (type == "CHECKBOX") {
        convertCheckBox(widget);
    } else if (type == "COMBOBOX") {
        convertComboBox(widget);
    } else if (type == "CONTROL") {
        if (!convertControl(widget)) {
            qCWarning(converter)
                << QObject::tr("%1: Unknow control %2 class %3")
                   .arg(id).arg(wid).arg(widget.value(KeyClass).toString());
            return widget;
        }
    } else if (type == "CTEXT") {
        convertLabel(widget);
    } else if (type == "DEFPUSHBUTTON") {
        convertPushButton(widget);
    } else if (type == "EDITTEXT") {
        convertEditText(widget);
    } else if (type == "GROUPBOX") {
        convertGroupBox(widget);
    } else if (type == "ICON") {
        convertStatic(widget); // the only possible style is SS_ICON
    } else if (type == "LISTBOX") {
        convertListWidget(widget);
    } else if (type == "LTEXT") {
        convertLabel(widget);
    } else if (type == "PUSHBOX") {
        convertPushButton(widget);
    } else if (type == "PUSHBUTTON") {
        convertPushButton(widget);
    } else if (type == "RADIOBUTTON") {
        convertRadioButton(widget);
    } else if (type == "RTEXT") {
        convertLabel(widget);
    } else if (type == "SCROLLBAR") {
        convertScrollBar(widget);
    } else if (type == "STATE3") {
        convertCheckBox(widget);
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
