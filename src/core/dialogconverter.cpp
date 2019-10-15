#include "converter.h"

#include "data.h"
#include "lexer.h"
#include "logging.h"

#include <algorithm>

namespace Converter {
constexpr double ScaleX = 1.5;
constexpr double ScaleY = 1.65;

// It's not good practice to create static QString, but it makes the code way more easy to read
// MFC Styles
const auto BS_3STATE = QStringLiteral("BS_3STATE");
const auto BS_AUTO3STATE = QStringLiteral("BS_AUTO3STATE");
const auto BS_AUTOCHECKBOX = QStringLiteral("BS_AUTOCHECKBOX");
const auto BS_AUTORADIOBUTTON = QStringLiteral("BS_AUTORADIOBUTTON");
const auto BS_CHECKBOX = QStringLiteral("BS_CHECKBOX");
const auto BS_GROUPBOX = QStringLiteral("BS_GROUPBOX");
const auto BS_DEFPUSHBUTTON = QStringLiteral("BS_DEFPUSHBUTTON");
const auto BS_PUSHBUTTON = QStringLiteral("BS_PUSHBUTTON");
const auto BS_RADIOBUTTON = QStringLiteral("BS_RADIOBUTTON");
const auto BS_BITMAP = QStringLiteral("BS_BITMAP");
const auto BS_ICON = QStringLiteral("BS_ICON");
const auto BS_FLAT = QStringLiteral("BS_FLAT");
const auto BS_PUSHLIKE = QStringLiteral("BS_PUSHLIKE");

const auto CBS_DROPDOWN = QStringLiteral("CBS_DROPDOWN");
const auto CBS_DROPDOWNLIST = QStringLiteral("CBS_DROPDOWNLIST");
const auto CBS_SIMPLE = QStringLiteral("CBS_SIMPLE");

const auto SS_RIGHT = QStringLiteral("SS_RIGHT");
const auto SS_CENTER = QStringLiteral("SS_CENTER");
const auto SS_CENTERIMAGE = QStringLiteral("SS_CENTERIMAGE");
const auto SS_SUNKEN = QStringLiteral("SS_SUNKEN");
const auto SS_BLACKFRAME = QStringLiteral("SS_BLACKFRAME");
const auto SS_REALSIZECONTROL = QStringLiteral("SS_REALSIZECONTROL");
const auto SS_BITMAP = QStringLiteral("SS_BITMAP");
const auto SS_ICON = QStringLiteral("SS_ICON");
const auto SS_LEFTNOWORDWRAP = QStringLiteral("SS_LEFTNOWORDWRAP");
const auto SS_LEFT = QStringLiteral("SS_LEFT");

const auto ES_MULTILINE = QStringLiteral("ES_MULTILINE");
const auto ES_CENTER = QStringLiteral("ES_CENTER");
const auto ES_LEFT = QStringLiteral("ES_LEFT");
const auto ES_RIGHT = QStringLiteral("ES_RIGHT");
const auto ES_PASSWORD = QStringLiteral("ES_PASSWORD");
const auto ES_READONLY = QStringLiteral("ES_READONLY");

const auto LBS_NOSEL = QStringLiteral("LBS_NOSEL");
const auto LBS_MULTIPLESEL = QStringLiteral("LBS_MULTIPLESEL");
const auto LBS_EXTENDEDSEL = QStringLiteral("LBS_EXTENDEDSEL");
const auto LBS_SORT = QStringLiteral("LBS_SORT");
const auto LBS_STANDARD = QStringLiteral("LBS_STANDARD");
const auto LBS_DISABLENOSCROLL = QStringLiteral("LBS_DISABLENOSCROLL");

const auto SBS_VERT = QStringLiteral("SBS_VERT");
const auto SBS_HORZ = QStringLiteral("SBS_HORZ");

const auto TBS_VERT = QStringLiteral("TBS_VERT");
const auto TBS_HORZ = QStringLiteral("TBS_HORZ");
const auto TBS_NOTICKS = QStringLiteral("TBS_NOTICKS");
const auto TBS_BOTH = QStringLiteral("TBS_BOTH");
const auto TBS_LEFT = QStringLiteral("TBS_LEFT");
const auto TBS_RIGHT = QStringLiteral("TBS_RIGHT");
const auto TBS_TOP = QStringLiteral("TBS_TOP");
const auto TBS_BOTTOM = QStringLiteral("TBS_BOTTOM");

const auto TCS_BOTTOM = QStringLiteral("TCS_BOTTOM");
const auto TCS_VERTICAL = QStringLiteral("TCS_VERTICAL");
const auto TCS_RIGHT = QStringLiteral("TCS_RIGHT");

const auto DTS_LONGDATEFORMAT = QStringLiteral("DTS_LONGDATEFORMAT");
const auto DTS_SHORTDATEFORMAT = QStringLiteral("DTS_SHORTDATEFORMAT");
const auto DTS_SHORTDATECENTURYFORMAT = QStringLiteral("DTS_SHORTDATECENTURYFORMAT");
const auto DTS_TIMEFORMAT = QStringLiteral("DTS_TIMEFORMAT");

const auto WS_CAPTION = QStringLiteral("WS_CAPTION");
const auto WS_BORDER = QStringLiteral("WS_BORDER");
const auto WS_DISABLED = QStringLiteral("WS_DISABLED");
const auto WS_TABSTOP = QStringLiteral("WS_TABSTOP");
const auto WS_HSCROLL = QStringLiteral("WS_HSCROLL");
const auto WS_VSCROLL = QStringLiteral("WS_VSCROLL");
const auto WS_EX_CLIENTEDGE = QStringLiteral("WS_EX_CLIENTEDGE");
const auto WS_EX_STATICEDGE = QStringLiteral("WS_EX_STATICEDGE");
const auto WS_EX_DLGMODALFRAME = QStringLiteral("WS_EX_DLGMODALFRAME");

// Qt properties
const auto WindowTitle = QStringLiteral("windowTitle");
const auto Text = QStringLiteral("text");
const auto Checkable = QStringLiteral("checkable");
const auto Default = QStringLiteral("default");
const auto Flat = QStringLiteral("flat");
const auto FrameShape = QStringLiteral("frameShape");
const auto FrameShadow = QStringLiteral("frameShadow");
const auto Enabled = QStringLiteral("enabled");
const auto Tristate = QStringLiteral("tristate");
const auto Editable = QStringLiteral("editable");
const auto InsertPolicy = QStringLiteral("insertPolicy");
const auto Title = QStringLiteral("title");
const auto Alignment = QStringLiteral("alignment");
const auto ScaledContents = QStringLiteral("scaledContents");
const auto Pixmap = QStringLiteral("pixmap");
const auto WordWrap = QStringLiteral("wordWrap");
const auto EchoMode = QStringLiteral("echoMode");
const auto ReadOnly = QStringLiteral("readOnly");
const auto ViewMode = QStringLiteral("viewMode");
const auto SelectionMode = QStringLiteral("selectionMode");
const auto SortingEnabled = QStringLiteral("sortingEnabled");
const auto HorizontalScrollBarPolicy = QStringLiteral("horizontalScrollBarPolicy");
const auto VerticalScrollBarPolicy = QStringLiteral("verticalScrollBarPolicy");
const auto Orientation = QStringLiteral("orientation");
const auto TickPosition = QStringLiteral("tickPosition");
const auto InputMask = QStringLiteral("inputMask");
const auto TabPosition = QStringLiteral("tabPosition");
const auto DisplayFormat = QStringLiteral("displayFormat");
const auto CalendarPopup = QStringLiteral("calendarPopup");

static QRect updateGeometry(const QRect &geometry)
{
    QRect result(static_cast<int>(ScaleX * geometry.x()), static_cast<int>(ScaleY * geometry.y()),
                 static_cast<int>(std::ceil(ScaleX * geometry.width())),
                 static_cast<int>(std::ceil(ScaleY * geometry.height())));
    return result;
}

static void convertStyles(Widget &widget, Data::Control &control, bool isFrame = false)
{
    if (isFrame) {
        QString shadow = QStringLiteral("QFrame::Plain");
        QString shape = QStringLiteral("QFrame::StyledPanel");
        if (control.styles.removeOne(WS_EX_CLIENTEDGE)) {
            widget.properties[FrameShape] = QStringLiteral("QFrame::Panel");
            widget.properties[FrameShadow] = QStringLiteral("QFrame::Sunken");
        }
        if (control.styles.removeOne(WS_EX_STATICEDGE)) {
            widget.properties[FrameShape] = QStringLiteral("QFrame::StyledPanel");
            widget.properties[FrameShadow] = QStringLiteral("QFrame::Sunken");
        }
        if (control.styles.removeOne(WS_EX_DLGMODALFRAME)) {
            widget.properties[FrameShape] = QStringLiteral("QFrame::Panel");
            widget.properties[FrameShadow] = QStringLiteral("QFrame::Raised");
        }
        if (control.styles.removeOne(WS_BORDER))
            widget.properties[FrameShape] = QStringLiteral("QFrame::Box");
    }

    if (control.styles.removeOne(WS_DISABLED))
        widget.properties[Enabled] = false;

    // WS_TABSTOP is handled by Qt widgets (focus navigation)
    control.styles.removeOne(WS_TABSTOP);

    if (!control.styles.isEmpty())
        qCInfo(CONVERTER) << control.id << "unused styles:" << control.styles;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/defpushbutton-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/pushbox-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/pushbutton-control
// https://docs.microsoft.com/en-us/windows/desktop/controls/button-styles
static Widget convertPushButton(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QPushButton");
    widget.properties[Text] = control.text;

    auto styles = control.styles;
    if (control.styles.removeOne(BS_AUTO3STATE) || control.styles.removeOne(BS_3STATE)
        || control.styles.removeOne(BS_CHECKBOX) || control.styles.removeOne(BS_RADIOBUTTON)
        || control.styles.removeOne(BS_AUTOCHECKBOX)
        || control.styles.removeOne(BS_AUTORADIOBUTTON))
        widget.properties[Checkable] = true;

    if (control.styles.removeOne(BS_DEFPUSHBUTTON)
        || control.type == static_cast<int>(Parser::Keywords::DEFPUSHBUTTON))
        widget.properties[Default] = true;

    // The button has an image, but it's handled with the message BM_SETIMAGE, in code
    // No need to port here
    control.styles.removeOne(BS_BITMAP);
    control.styles.removeOne(BS_ICON);

    if (control.styles.removeOne(BS_FLAT)
        || control.type == static_cast<int>(Parser::Keywords::PUSHBOX))
        widget.properties[Flat] = true;

    convertStyles(widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/radiobutton-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/autoradiobutton-control
static Widget convertRadioButton(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QRadioButton");
    widget.properties[Text] = control.text;

    control.styles.removeOne(BS_RADIOBUTTON);
    control.styles.removeOne(BS_AUTORADIOBUTTON);
    convertStyles(widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/auto3state-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/autocheckbox-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/checkbox-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/state3-control
static Widget convertCheckBox(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QCheckBox");
    widget.properties[Text] = control.text;

    if (control.styles.removeOne(BS_AUTO3STATE) || control.styles.removeOne(BS_3STATE)
        || control.type == static_cast<int>(Parser::Keywords::STATE3)
        || control.type == static_cast<int>(Parser::Keywords::AUTO3STATE))
        widget.properties[Tristate] = true;

    control.styles.removeOne(BS_CHECKBOX);
    control.styles.removeOne(BS_AUTOCHECKBOX);

    convertStyles(widget, control);
    return widget;
}

// TODO
static Widget convertComboBox(Data *data, const QString &dialogId, Data::Control &control)
{
    Q_UNUSED(data)
    Widget widget;
    widget.className = QStringLiteral("QComboBox");

    if (control.styles.removeOne(CBS_SIMPLE)) {
        widget.className = QStringLiteral("QListWidget");
    } else {
        // In MFC, the height is not the height of the combobox
        // So we take the default height of a combobox
        control.geometry.setHeight(22 / ScaleY);

        if (control.styles.removeOne(CBS_DROPDOWN)) {
            widget.properties[Editable] = true;
            widget.properties[InsertPolicy] = QLatin1String("QComboBox::NoInsert");
        }
    }

    // Initialize the values if they exists
    const auto it =
        std::find_if(data->dialogData.cbegin(), data->dialogData.cend(),
                     [dialogId](const auto &dialogData) { return dialogData.id == dialogId; });
    if (it != data->dialogData.cend()) {
        const auto &values = it->values.value(control.id);
        if (!values.isEmpty())
            widget.properties[Text] = values;
    }

    control.styles.removeOne(CBS_DROPDOWNLIST);
    control.styles.removeOne(WS_VSCROLL);
    convertStyles(widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/ltext-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/rtext-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/ctext-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/icon-control
static Widget convertLabel(Data *data, Data::Control &control)
{
    Q_UNUSED(data)
    Widget widget;
    widget.className = QStringLiteral("QLabel");

    if (control.styles.removeOne(SS_RIGHT)
        || control.type == static_cast<int>(Parser::Keywords::RTEXT))
        widget.properties[Alignment] = QStringLiteral("Qt::AlignRight");
    if (control.styles.removeOne(SS_CENTER) || control.styles.removeOne(SS_CENTERIMAGE)
        || control.type == static_cast<int>(Parser::Keywords::CTEXT))
        widget.properties[Alignment] = QStringLiteral("Qt::AlignHCenter");

    if (control.styles.removeOne(SS_SUNKEN)) {
        widget.properties[FrameShape] = QStringLiteral("QFrame::Plain");
        widget.properties[FrameShadow] = QStringLiteral("QFrame::Sunken");
    }

    if (control.styles.removeOne(SS_BLACKFRAME))
        widget.properties[FrameShape] = QStringLiteral("QFrame::Box");

    if (control.styles.removeOne(SS_REALSIZECONTROL))
        widget.properties[ScaledContents] = true;

    // TODO: get the real path
    if (control.styles.removeOne(SS_BITMAP) || control.styles.removeOne(SS_ICON)
        || control.type == static_cast<int>(Parser::Keywords::ICON))
        widget.properties[Pixmap] = control.text;
    else
        widget.properties[Text] = control.text;

    if (control.styles.removeOne(SS_LEFTNOWORDWRAP))
        widget.properties[WordWrap] = true;

    control.styles.removeOne(SS_LEFT);
    convertStyles(widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/edittext-control
static Widget convertEditText(Data::Control &control)
{
    Widget widget;
    bool hasFrame = false;

    // TODO what about RichEdit20W
    if (control.styles.removeOne(ES_MULTILINE) || control.className == QStringLiteral("RICHEDIT")) {
        widget.className = QStringLiteral("QTextEdit");
        hasFrame = true;
    } else {
        widget.className = QStringLiteral("QLineEdit");
        if (control.styles.removeOne(ES_CENTER))
            widget.properties[Alignment] = QStringLiteral("Qt::AlignCenter|Qt::AlignVCenter");
        else if (control.styles.removeOne(ES_RIGHT))
            widget.properties[Alignment] = QStringLiteral("Qt::AlignRight|Qt::AlignVCenter");
        else if (control.styles.removeOne(ES_LEFT)
                 || true) // this is the default, but I want to remove the style too
            widget.properties[Alignment] = QStringLiteral("Qt::AlignLeft|Qt::AlignVCenter");

        if (control.styles.removeOne(ES_PASSWORD))
            widget.properties[EchoMode] = QStringLiteral("QLineEdit::Password");
    }

    if (control.styles.removeOne(ES_READONLY))
        widget.properties[ReadOnly] = true;

    convertStyles(widget, control, hasFrame);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/groupbox-control
static Widget convertGroupBox(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QGroupBox");
    widget.properties[Title] = control.text;
    convertStyles(widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/listbox-control
static Widget convertListWidget(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QListWidget");

    // The control is an icon view
    if (control.type == static_cast<int>(Parser::Keywords::CONTROL)
        && control.className == QStringLiteral("SysListView32"))
        widget.properties[ViewMode] = QStringLiteral("QListView::IconMode");

    if (control.styles.removeOne(LBS_NOSEL))
        widget.properties[SelectionMode] = QStringLiteral("QAbstractItemView::NoSelection");
    else if (control.styles.removeOne(LBS_MULTIPLESEL))
        widget.properties[SelectionMode] = QStringLiteral("QAbstractItemView::MultiSelection");
    else if (control.styles.removeOne(LBS_EXTENDEDSEL))
        widget.properties[SelectionMode] = QStringLiteral("QAbstractItemView::ExtendedSelection");
    else
        widget.properties[SelectionMode] = QStringLiteral("QAbstractItemView::SingleSelection");

    if (control.styles.removeOne(LBS_SORT) || control.styles.removeOne(LBS_STANDARD))
        widget.properties[SortingEnabled] = true;
    else if (control.styles.removeOne(LBS_MULTIPLESEL))
        widget.properties[SelectionMode] = QStringLiteral("QAbstractItemView::MultiSelection");

    bool alwaysOn = control.styles.removeOne(LBS_DISABLENOSCROLL);
    if (control.styles.removeOne(WS_HSCROLL))
        widget.properties[HorizontalScrollBarPolicy] = alwaysOn
            ? QStringLiteral("Qt::ScrollBarAlwaysOn")
            : QStringLiteral("Qt::ScrollBarAsNeeded");
    else
        widget.properties[HorizontalScrollBarPolicy] = QStringLiteral("ScrollBarAlwaysOff");
    if (control.styles.removeOne(WS_VSCROLL))
        widget.properties[VerticalScrollBarPolicy] = alwaysOn
            ? QStringLiteral("Qt::ScrollBarAlwaysOn")
            : QStringLiteral("Qt::ScrollBarAsNeeded");
    else
        widget.properties[VerticalScrollBarPolicy] = QStringLiteral("ScrollBarAlwaysOff");

    convertStyles(widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/scrollbar-control
static Widget convertScrollBar(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QScrollBar");

    if (control.styles.removeOne(SBS_VERT))
        widget.properties[Orientation] = QStringLiteral("Qt::Vertical");
    else if (control.styles.removeOne(SBS_HORZ) || true) // We want to remove the style if it exits
        widget.properties[Orientation] = QStringLiteral("Qt::Horizontal");

    convertStyles(widget, control);
    return widget;
}

static Widget convertButton(Data::Control &control)
{
    if (control.styles.contains(BS_PUSHLIKE))
        return convertPushButton(control);
    if (control.styles.contains(BS_3STATE))
        return convertCheckBox(control);
    if (control.styles.contains(BS_AUTO3STATE))
        return convertCheckBox(control);
    if (control.styles.contains(BS_AUTOCHECKBOX))
        return convertCheckBox(control);
    if (control.styles.contains(BS_AUTORADIOBUTTON))
        return convertRadioButton(control);
    if (control.styles.contains(BS_CHECKBOX))
        return convertCheckBox(control);
    if (control.styles.contains(BS_GROUPBOX))
        return convertGroupBox(control);
    if (control.styles.contains(BS_DEFPUSHBUTTON))
        return convertPushButton(control);
    if (control.styles.contains(BS_PUSHBUTTON))
        return convertPushButton(control);
    if (control.styles.contains(BS_RADIOBUTTON))
        return convertRadioButton(control);
    return convertPushButton(control);
}

static Widget convertSlider(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QSlider");

    if (control.styles.removeOne(TBS_VERT))
        widget.properties[Orientation] = QStringLiteral("Qt::Vertical");
    else if (control.styles.removeOne(TBS_HORZ) || true) // We want to remove the style if it exits
        widget.properties[Orientation] = QStringLiteral("Qt::Horizontal");

    if (control.styles.removeOne(TBS_NOTICKS))
        widget.properties[TickPosition] = QStringLiteral("QSlider::NoTicks");
    if (control.styles.removeOne(TBS_BOTH))
        widget.properties[TickPosition] = QStringLiteral("QSlider::TicksBothSides");
    if (control.styles.removeOne(TBS_LEFT))
        widget.properties[TickPosition] = QStringLiteral("QSlider::TicksLeft");
    if (control.styles.removeOne(TBS_RIGHT))
        widget.properties[TickPosition] = QStringLiteral("QSlider::TicksRight");
    if (control.styles.removeOne(TBS_TOP))
        widget.properties[TickPosition] = QStringLiteral("QSlider::TicksAbove");
    if (control.styles.removeOne(TBS_BOTTOM))
        widget.properties[TickPosition] = QStringLiteral("QSlider::TicksBelow");

    convertStyles(widget, control);
    return widget;
}

static Widget convertSpinBox(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QSpinBox");
    convertStyles(widget, control, true);
    return widget;
}

static Widget convertProgressBar(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QProgressBar");

    if (control.styles.removeOne(TBS_VERT))
        widget.properties[Orientation] = QStringLiteral("Qt::Vertical");
    else if (control.styles.removeOne(TBS_HORZ) || true) // We want to remove the style if it exits
        widget.properties[Orientation] = QStringLiteral("Qt::Horizontal");

    convertStyles(widget, control);
    return widget;
}

static Widget convertCalendarWidget(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QCalendarWidget");
    convertStyles(widget, control, true);
    return widget;
}

static Widget convertDateTime(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QDateTimeEdit");

    if (control.styles.removeOne(DTS_LONGDATEFORMAT))
        widget.properties[DisplayFormat] = QStringLiteral("dddd, MMMM dd, yyyy");
    if (control.styles.removeOne(DTS_SHORTDATEFORMAT))
        widget.properties[DisplayFormat] = QStringLiteral("M/d/yy");
    if (control.styles.removeOne(DTS_SHORTDATECENTURYFORMAT))
        widget.properties[DisplayFormat] = QStringLiteral("M/d/yyyy");
    if (control.styles.removeOne(DTS_TIMEFORMAT))
        widget.properties[DisplayFormat] = QStringLiteral("hh:mm:ss");

    widget.properties[CalendarPopup] = true;

    convertStyles(widget, control, true);
    return widget;
}

static Widget convertIpAddress(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QLineEdit");

    widget.properties[InputMask] = QStringLiteral("000.000.000.000;_");

    convertStyles(widget, control, true);
    return widget;
}

static Widget convertTreeWidget(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QTreeWidget");
    convertStyles(widget, control, true);
    return widget;
}

static Widget convertTabWidget(Data::Control &control)
{
    Widget widget;
    widget.className = QStringLiteral("QTabWidget");

    if (control.styles.removeOne(TCS_BOTTOM))
        widget.properties[TabPosition] = QStringLiteral("QTabWidget::South");
    if (control.styles.removeOne(TCS_VERTICAL))
        widget.properties[TabPosition] = QStringLiteral("QTabWidget::West");
    if (control.styles.removeOne(TCS_RIGHT))
        widget.properties[TabPosition] = QStringLiteral("QTabWidget::East");

    convertStyles(widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/control-control
static Widget convertControl(Data *data, const QString &dialogId, Data::Control &control)
{
    if (control.className == QLatin1String("Static"))
        return convertLabel(data, control);
    if (control.className == QLatin1String("Button"))
        return convertButton(control);
    if (control.className == QLatin1String("ComboBox"))
        return convertComboBox(data, dialogId, control);
    if (control.className == QLatin1String("ComboBoxEx32"))
        return convertComboBox(data, dialogId, control);
    if (control.className == QLatin1String("Edit"))
        return convertEditText(control);
    if (control.className == QLatin1String("RICHEDIT"))
        return convertEditText(control);
    if (control.className == QLatin1String("RichEdit20W"))
        return convertEditText(control);
    if (control.className == QLatin1String("RichEdit20A"))
        return convertEditText(control);
    if (control.className == QLatin1String("msctls_trackbar"))
        return convertSlider(control);
    if (control.className == QLatin1String("msctls_trackbar32"))
        return convertSlider(control);
    if (control.className == QLatin1String("msctls_updown"))
        return convertSpinBox(control);
    if (control.className == QLatin1String("msctls_updown32"))
        return convertSpinBox(control);
    if (control.className == QLatin1String("msctls_progress"))
        return convertProgressBar(control);
    if (control.className == QLatin1String("msctls_progress32"))
        return convertProgressBar(control);
    if (control.className == QLatin1String("ScrollBar"))
        return convertScrollBar(control);
    if (control.className == QLatin1String("SysMonthCal32"))
        return convertCalendarWidget(control);
    if (control.className == QLatin1String("SysDateTimePick32"))
        return convertDateTime(control);
    if (control.className == QLatin1String("SysIPAddress32"))
        return convertIpAddress(control);
    if (control.className == QLatin1String("SysListView"))
        return convertListWidget(control);
    if (control.className == QLatin1String("SysListView32"))
        return convertListWidget(control);
    if (control.className == QLatin1String("SysTreeView"))
        return convertTreeWidget(control);
    if (control.className == QLatin1String("SysTreeView32"))
        return convertTreeWidget(control);
    if (control.className == QLatin1String("SysTabControl"))
        return convertTabWidget(control);
    if (control.className == QLatin1String("SysTabControl32"))
        return convertTabWidget(control);
    if (control.className == QLatin1String("SysLink"))
        return convertLabel(data, control);
    if (control.className == QLatin1String("MfcPropertyGrid"))
        return convertTreeWidget(control);

    qCCritical(CONVERTER) << control.id << "- Unknow CONTROL:" << control.className;

    Widget widget;
    widget.className = QStringLiteral("QWidget");
    return widget;
}

static Widget convertChildWidget(Data *data, const QString &dialogId, Data::Control control)
{
    Widget widget;

    Parser::Keywords type = static_cast<Parser::Keywords>(control.type);
    switch (type) {
    case Parser::Keywords::DEFPUSHBUTTON:
    case Parser::Keywords::PUSHBOX:
    case Parser::Keywords::PUSHBUTTON:
        widget = convertPushButton(control);
        break;
    case Parser::Keywords::AUTORADIOBUTTON:
    case Parser::Keywords::RADIOBUTTON:
        widget = convertRadioButton(control);
        break;
    case Parser::Keywords::AUTO3STATE:
    case Parser::Keywords::AUTOCHECKBOX:
    case Parser::Keywords::CHECKBOX:
    case Parser::Keywords::STATE3:
        widget = convertCheckBox(control);
        break;
    case Parser::Keywords::COMBOBOX:
        widget = convertComboBox(data, dialogId, control);
        break;
    case Parser::Keywords::CTEXT:
    case Parser::Keywords::LTEXT:
    case Parser::Keywords::RTEXT:
    case Parser::Keywords::ICON:
        widget = convertLabel(data, control);
        break;
    case Parser::Keywords::EDITTEXT:
        widget = convertEditText(control);
        break;
    case Parser::Keywords::GROUPBOX:
        widget = convertGroupBox(control);
        break;
    case Parser::Keywords::LISTBOX:
        widget = convertListWidget(control);
        break;
    case Parser::Keywords::SCROLLBAR:
        widget = convertScrollBar(control);
        break;
    case Parser::Keywords::CONTROL:
        widget = convertControl(data, dialogId, control);
        break;
    default:
        qCCritical(CONVERTER) << "Unknown control type:"
                              << Parser::Token {Parser::Token::Keyword, type}.toString();
    }

    widget.id = control.id;
    widget.geometry = updateGeometry(control.geometry);

    return widget;
}

static QVariantList adjustHierarchy(QVector<Widget> widgets)
{
    if (widgets.isEmpty())
        return {};

    auto sortByArea = [](const auto &w1, const auto &w2) {
        const int area1 = w1.geometry.width() * w1.geometry.height();
        const int area2 = w2.geometry.width() * w2.geometry.height();
        return area1 < area2;
    };
    std::sort(widgets.begin(), widgets.end(), sortByArea);

    QList<int> toRemove;
    for (int i = 0; i < widgets.size(); ++i) {
        for (int j = i + 1; j < widgets.size(); ++j) {
            QRect geomi = widgets.at(i).geometry;
            const QRect geomj = widgets.at(j).geometry;
            if (geomj.contains(geomi)) {
                auto &iWidget = widgets[i];
                geomi.moveLeft(geomi.x() - geomj.x());
                geomi.moveTop(geomi.y() - geomj.y());
                iWidget.geometry = geomi;

                widgets[j].children.push_back(QVariant::fromValue(iWidget));
                toRemove.push_front(i);
                break;
            }
        }
    }

    for (const int i : toRemove)
        widgets.removeAt(i);

    QVariantList children;
    std::transform(widgets.cbegin(), widgets.cend(), std::back_inserter(children),
                   [](const Widget &w) { return QVariant::fromValue(w); });
    return children;
}

Widget convertDialog(Data *data, int index)
{
    Data::Dialog item = data->dialogs.value(index);
    Widget widget;
    widget.id = item.id;
    widget.geometry = updateGeometry(item.geometry);

    if (item.menu.isEmpty()) {
        // If the dialog has a caption, it's a true Qt dialog, otherwise it's a widget
        if (item.styles.removeOne(WS_CAPTION)) {
            widget.className = QStringLiteral("QDialog");
        } else {
            widget.className = QStringLiteral("QWidget");
        }
    } else {
        widget.className = QStringLiteral("QMainWindow");
    }
    if (!item.caption.isEmpty())
        widget.properties[WindowTitle] = item.caption;

    if (!item.styles.isEmpty())
        qCInfo(CONVERTER) << item.id << "unused styles:" << item.styles;

    QVector<Widget> widgets;
    for (const auto &control : item.controls)
        widgets.push_back(convertChildWidget(data, item.id, control));

    widget.children = adjustHierarchy(widgets);

    return widget;
}

QVariantList convertDialogs(Data *data, const Knut::DataCollection &collection)
{
    QVariantList result;
    for (const auto &item : collection)
        result.push_back(QVariant::fromValue(convertDialog(data, item.second)));
    return result;
}
}
