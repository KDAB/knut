#include "rcfile.h"

#include "lexer.h"
#include "rc_utils.h"

#include <QDir>
#include <QHash>
#include <QImage>

#include <algorithm>
#include <cmath>

namespace RcCore {

static QVector<Asset> splitToolBar(const Data &data, const ToolBar &toolBar, const Asset &asset)
{
    const int iconCount =
        std::count_if(toolBar.children.cbegin(), toolBar.children.cend(), [](const auto &toolBarItem) {
            return !toolBarItem.id.isEmpty();
        });
    const int zeroCount = std::log10(iconCount) + 1;

    QImage image(asset.originalFileName);
    const int width = toolBar.iconSize.width();

    if (iconCount * width != image.width()) {
        logger()->warn("{}({}): asset and toolbar widths don't match for {}",
                       QDir::toNativeSeparators(data.fileName).toStdString(), asset.line, asset.id.toStdString());
    }

    QVector<Asset> assets;
    for (int i = 0; i < iconCount; ++i) {
        Asset iconAsset = asset;
        const int dotPos = iconAsset.fileName.lastIndexOf('.');
        const QString suffix = QStringLiteral("_%1").arg(QString::number(i), zeroCount, '0');
        iconAsset.fileName.insert(dotPos, suffix);
        iconAsset.id += suffix;
        iconAsset.iconRect = QRect(i * width, 0, width, toolBar.iconSize.height());
        assets.push_back(iconAsset);
    }
    return assets;
}

QVector<Asset> convertAsset(const Data &data, Asset asset, Asset::ConversionFlags flags)
{
    if ((flags & Asset::RemoveUnknown) && !asset.exist)
        return {};

    if (flags & Asset::ConvertToPng) {
        if (asset.fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
            asset.originalFileName = asset.fileName;
            asset.fileName = asset.fileName.chopped(3) + "png";
        }
    }

    if (flags & Asset::SplitToolBar) {
        if (auto toolBar = data.toolBar(asset.id)) {
            if (asset.originalFileName.isEmpty())
                asset.originalFileName = asset.fileName;
            return splitToolBar(data, *toolBar, asset);
        }
    }

    return {asset};
}

/**
 * @brief Convert the assets of a rc file, based on the flags passed in
 * Depending on the flags, the conversion will:
 *  - transform BMP images into PNGs
 *  - split toolbars into individual icons
 *  - filter out non-existant assets
 * The conversion will not write anything - see writeAssetsToImage method.
 * @return New list of assets
 */
QVector<Asset> convertAssets(const Data &data, Asset::ConversionFlags flags)
{
    if (flags == Asset::NoFlags)
        return data.assets;

    QVector<Asset> assets;
    for (const auto &asset : data.assets)
        assets.append(convertAsset(data, asset, flags));

    return assets;
}

//=============================================================================
// Dialog conversion
//=============================================================================

// It's not good practice to create static QString, but it makes the code way more easy to read
// MFC Styles
const auto BS3STATE = "BS_3STATE";
const auto BSAUTO3STATE = "BS_AUTO3STATE";
const auto BSAUTOCHECKBOX = "BS_AUTOCHECKBOX";
const auto BSAUTORADIOBUTTON = "BS_AUTORADIOBUTTON";
const auto BSCHECKBOX = "BS_CHECKBOX";
const auto BSGROUPBOX = "BS_GROUPBOX";
const auto BSDEFPUSHBUTTON = "BS_DEFPUSHBUTTON";
const auto BSPUSHBUTTON = "BS_PUSHBUTTON";
const auto BSRADIOBUTTON = "BS_RADIOBUTTON";
const auto BSBITMAP = "BS_BITMAP";
const auto BSICON = "BS_ICON";
const auto BSFLAT = "BS_FLAT";
const auto BSPUSHLIKE = "BS_PUSHLIKE";

const auto CBSDROPDOWN = "CBS_DROPDOWN";
const auto CBSDROPDOWNLIST = "CBS_DROPDOWNLIST";
const auto CBSSIMPLE = "CBS_SIMPLE";

const auto SSRIGHT = "SS_RIGHT";
const auto SSCENTER = "SS_CENTER";
const auto SSCENTERIMAGE = "SS_CENTERIMAGE";
const auto SSSUNKEN = "SS_SUNKEN";
const auto SSBLACKFRAME = "SS_BLACKFRAME";
const auto SSREALSIZECONTROL = "SS_REALSIZECONTROL";
const auto SSBITMAP = "SS_BITMAP";
const auto SSICON = "SS_ICON";
const auto SSLEFTNOWORDWRAP = "SS_LEFTNOWORDWRAP";
const auto SSLEFT = "SS_LEFT";

const auto ESMULTILINE = "ES_MULTILINE";
const auto ESCENTER = "ES_CENTER";
const auto ESLEFT = "ES_LEFT";
const auto ESRIGHT = "ES_RIGHT";
const auto ESPASSWORD = "ES_PASSWORD";
const auto ESREADONLY = "ES_READONLY";

const auto LBSNOSEL = "LBS_NOSEL";
const auto LBSMULTIPLESEL = "LBS_MULTIPLESEL";
const auto LBSEXTENDEDSEL = "LBS_EXTENDEDSEL";
const auto LBSSORT = "LBS_SORT";
const auto LBSSTANDARD = "LBS_STANDARD";
const auto LBSDISABLENOSCROLL = "LBS_DISABLENOSCROLL";

const auto SBSVERT = "SBS_VERT";
const auto SBSHORZ = "SBS_HORZ";

const auto TBS_VERT = "TBS_VERT";
const auto TBS_HORZ = "TBS_HORZ";
const auto TBS_NOTICKS = "TBS_NOTICKS";
const auto TBS_BOTH = "TBS_BOTH";
const auto TBS_LEFT = "TBS_LEFT";
const auto TBS_RIGHT = "TBS_RIGHT";
const auto TBS_TOP = "TBS_TOP";
const auto TBS_BOTTOM = "TBS_BOTTOM";

const auto TCS_BOTTOM = "TCS_BOTTOM";
const auto TCS_VERTICAL = "TCS_VERTICAL";
const auto TCS_RIGHT = "TCS_RIGHT";

const auto DTS_LONGDATEFORMAT = "DTS_LONGDATEFORMAT";
const auto DTS_SHORTDATEFORMAT = "DTS_SHORTDATEFORMAT";
const auto DTS_SHORTDATECENTURYFORMAT = "DTS_SHORTDATECENTURYFORMAT";
const auto DTS_TIMEFORMAT = "DTS_TIMEFORMAT";

const auto WSCAPTION = "WS_CAPTION";
const auto WSBORDER = "WS_BORDER";
const auto WSDISABLED = "WS_DISABLED";
const auto WSTABSTOP = "WS_TABSTOP";
const auto WSHSCROLL = "WS_HSCROLL";
const auto WSVSCROLL = "WS_VSCROLL";
const auto WSEX_CLIENTEDGE = "WS_EX_CLIENTEDGE";
const auto WSEX_STATICEDGE = "WS_EX_STATICEDGE";
const auto WSEX_DLGMODALFRAME = "WS_EX_DLGMODALFRAME";

// Qt properties
const auto WindowTitle = "windowTitle";
const auto Text = "text";
const auto Checkable = "checkable";
const auto Default = "default";
const auto Flat = "flat";
const auto FrameShape = "frameShape";
const auto FrameShadow = "frameShadow";
const auto Enabled = "enabled";
const auto Tristate = "tristate";
const auto Editable = "editable";
const auto InsertPolicy = "insertPolicy";
const auto Title = "title";
const auto Alignment = "alignment";
const auto ScaledContents = "scaledContents";
const auto Pixmap = "pixmap";
const auto WordWrap = "wordWrap";
const auto EchoMode = "echoMode";
const auto ReadOnly = "readOnly";
const auto ViewMode = "viewMode";
const auto SelectionMode = "selectionMode";
const auto SortingEnabled = "sortingEnabled";
const auto HorizontalScrollBarPolicy = "horizontalScrollBarPolicy";
const auto VerticalScrollBarPolicy = "verticalScrollBarPolicy";
const auto Orientation = "orientation";
const auto TickPosition = "tickPosition";
const auto InputMask = "inputMask";
const auto TabPosition = "tabPosition";
const auto DisplayFormat = "displayFormat";
const auto CalendarPopup = "calendarPopup";

static void convertStyles(const Data &data, Widget &widget, Data::Control &control, bool isFrame = false)
{
    if (isFrame) {
        if (control.styles.removeOne(WSEX_CLIENTEDGE)) {
            widget.properties[FrameShape] = "QFrame::Panel";
            widget.properties[FrameShadow] = "QFrame::Sunken";
        }
        if (control.styles.removeOne(WSEX_STATICEDGE)) {
            widget.properties[FrameShape] = "QFrame::StyledPanel";
            widget.properties[FrameShadow] = "QFrame::Sunken";
        }
        if (control.styles.removeOne(WSEX_DLGMODALFRAME)) {
            widget.properties[FrameShape] = "QFrame::Panel";
            widget.properties[FrameShadow] = "QFrame::Raised";
        }
        if (control.styles.removeOne(WSBORDER))
            widget.properties[FrameShape] = "QFrame::Box";
    }

    if (control.styles.removeOne(WSDISABLED))
        widget.properties[Enabled] = false;

    // WS_TABSTOP is handled by Qt widgets (focus navigation)
    control.styles.removeOne(WSTABSTOP);

    if (!control.styles.isEmpty()) {
        logger()->debug("{}({}): {} has unused styles {}", QDir::toNativeSeparators(data.fileName).toStdString(),
                        control.line, control.id.toStdString(), control.styles.join(", ").toStdString());
    }
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/defpushbutton-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/pushbox-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/pushbutton-control
// https://docs.microsoft.com/en-us/windows/desktop/controls/button-styles
static Widget convertPushButton(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QPushButton";
    widget.properties[Text] = control.text;

    auto styles = control.styles;
    if (control.styles.removeOne(BSAUTO3STATE) || control.styles.removeOne(BS3STATE)
        || control.styles.removeOne(BSCHECKBOX) || control.styles.removeOne(BSRADIOBUTTON)
        || control.styles.removeOne(BSAUTOCHECKBOX) || control.styles.removeOne(BSAUTORADIOBUTTON))
        widget.properties[Checkable] = true;

    if (control.styles.removeOne(BSDEFPUSHBUTTON) || control.type == static_cast<int>(Keywords::DEFPUSHBUTTON))
        widget.properties[Default] = true;

    // The button has an image, but it's handled with the message BM_SETIMAGE, in code
    // No need to port here
    control.styles.removeOne(BSBITMAP);
    control.styles.removeOne(BSICON);

    if (control.styles.removeOne(BSFLAT) || control.type == static_cast<int>(Keywords::PUSHBOX))
        widget.properties[Flat] = true;

    convertStyles(data, widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/radiobutton-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/autoradiobutton-control
static Widget convertRadioButton(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QRadioButton";
    widget.properties[Text] = control.text;

    control.styles.removeOne(BSRADIOBUTTON);
    control.styles.removeOne(BSAUTORADIOBUTTON);
    convertStyles(data, widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/auto3state-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/autocheckbox-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/checkbox-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/state3-control
static Widget convertCheckBox(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QCheckBox";
    widget.properties[Text] = control.text;

    if (control.styles.removeOne(BSAUTO3STATE) || control.styles.removeOne(BS3STATE)
        || control.type == static_cast<int>(Keywords::STATE3) || control.type == static_cast<int>(Keywords::AUTO3STATE))
        widget.properties[Tristate] = true;

    control.styles.removeOne(BSCHECKBOX);
    control.styles.removeOne(BSAUTOCHECKBOX);

    convertStyles(data, widget, control);
    return widget;
}

// TODO
static Widget convertComboBox(const Data &data, const QString &dialogId, Data::Control &control)
{
    Widget widget;
    widget.className = "QComboBox";

    if (control.styles.removeOne(CBSSIMPLE)) {
        widget.className = "QListWidget";
    } else {
        // In MFC, the height is not the height of the combobox
        // So we take the default height of a combobox
        control.geometry.setHeight(22);

        if (control.styles.removeOne(CBSDROPDOWN)) {
            widget.properties[Editable] = true;
            widget.properties[InsertPolicy] = "QComboBox::NoInsert";
        }
    }

    // Initialize the values if they exists
    const auto it = std::find_if(data.dialogData.cbegin(), data.dialogData.cend(), [dialogId](const auto &dialogData) {
        return dialogData.id == dialogId;
    });
    if (it != data.dialogData.cend()) {
        const auto &values = it->values.value(control.id);
        if (!values.isEmpty())
            widget.properties[Text] = values;
    }

    control.styles.removeOne(CBSDROPDOWNLIST);
    control.styles.removeOne(WSVSCROLL);
    convertStyles(data, widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/ltext-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/rtext-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/ctext-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/icon-control
static Widget convertLabel(const Data &data, Data::Control &control, bool useIdForPixmap)
{
    Widget widget;
    widget.className = "QLabel";

    if (control.styles.removeOne(SSRIGHT) || control.type == static_cast<int>(Keywords::RTEXT))
        widget.properties[Alignment] = "Qt::AlignRight";
    if (control.styles.removeOne(SSCENTER) || control.styles.removeOne(SSCENTERIMAGE)
        || control.type == static_cast<int>(Keywords::CTEXT))
        widget.properties[Alignment] = "Qt::AlignHCenter";

    if (control.styles.removeOne(SSSUNKEN)) {
        widget.properties[FrameShape] = "QFrame::Plain";
        widget.properties[FrameShadow] = "QFrame::Sunken";
    }

    if (control.styles.removeOne(SSBLACKFRAME))
        widget.properties[FrameShape] = "QFrame::Box";

    if (control.styles.removeOne(SSREALSIZECONTROL))
        widget.properties[ScaledContents] = true;

    if (control.styles.removeOne(SSBITMAP) || control.styles.removeOne(SSICON)
        || control.type == static_cast<int>(Keywords::ICON)) {
        if (useIdForPixmap) {
            widget.properties[Pixmap] = QStringLiteral(":/%1").arg(control.text);
        } else {
            auto asset = data.asset(control.text);
            if (asset)
                widget.properties[Pixmap] = asset->fileName;
            else
                widget.properties[Pixmap] = QStringLiteral(":/%1").arg(control.text);
        }
    } else {
        widget.properties[Text] = control.text;
    }

    if (control.styles.removeOne(SSLEFTNOWORDWRAP))
        widget.properties[WordWrap] = true;

    control.styles.removeOne(SSLEFT);
    convertStyles(data, widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/edittext-control
static Widget convertEditText(const Data &data, Data::Control &control)
{
    Widget widget;
    bool hasFrame = false;

    // TODO what about RichEdit20W
    if (control.styles.removeOne(ESMULTILINE) || control.className == "RICHEDIT") {
        widget.className = "QTextEdit";
        hasFrame = true;
    } else {
        widget.className = "QLineEdit";
        if (control.styles.removeOne(ESCENTER))
            widget.properties[Alignment] = "Qt::AlignCenter|Qt::AlignVCenter";
        else if (control.styles.removeOne(ESRIGHT))
            widget.properties[Alignment] = "Qt::AlignRight|Qt::AlignVCenter";
        else if (control.styles.removeOne(ESLEFT) || true) // this is the default, but I want to remove the style too
            widget.properties[Alignment] = "Qt::AlignLeft|Qt::AlignVCenter";

        if (control.styles.removeOne(ESPASSWORD))
            widget.properties[EchoMode] = "QLineEdit::Password";
    }

    if (control.styles.removeOne(ESREADONLY))
        widget.properties[ReadOnly] = true;

    convertStyles(data, widget, control, hasFrame);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/groupbox-control
static Widget convertGroupBox(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QGroupBox";
    widget.properties[Title] = control.text;
    convertStyles(data, widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/listbox-control
static Widget convertListWidget(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QListWidget";

    // The control is an icon view
    if (control.type == static_cast<int>(Keywords::CONTROL) && control.className == "SysListView32")
        widget.properties[ViewMode] = "QListView::IconMode";

    if (control.styles.removeOne(LBSNOSEL))
        widget.properties[SelectionMode] = "QAbstractItemView::NoSelection";
    else if (control.styles.removeOne(LBSMULTIPLESEL))
        widget.properties[SelectionMode] = "QAbstractItemView::MultiSelection";
    else if (control.styles.removeOne(LBSEXTENDEDSEL))
        widget.properties[SelectionMode] = "QAbstractItemView::ExtendedSelection";
    else
        widget.properties[SelectionMode] = "QAbstractItemView::SingleSelection";

    if (control.styles.removeOne(LBSSORT) || control.styles.removeOne(LBSSTANDARD))
        widget.properties[SortingEnabled] = true;
    else if (control.styles.removeOne(LBSMULTIPLESEL))
        widget.properties[SelectionMode] = "QAbstractItemView::MultiSelection";

    bool alwaysOn = control.styles.removeOne(LBSDISABLENOSCROLL);
    if (control.styles.removeOne(WSHSCROLL))
        widget.properties[HorizontalScrollBarPolicy] = alwaysOn ? "Qt::ScrollBarAlwaysOn" : "Qt::ScrollBarAsNeeded";
    else
        widget.properties[HorizontalScrollBarPolicy] = "ScrollBarAlwaysOff";
    if (control.styles.removeOne(WSVSCROLL))
        widget.properties[VerticalScrollBarPolicy] = alwaysOn ? "Qt::ScrollBarAlwaysOn" : "Qt::ScrollBarAsNeeded";
    else
        widget.properties[VerticalScrollBarPolicy] = "ScrollBarAlwaysOff";

    convertStyles(data, widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/scrollbar-control
static Widget convertScrollBar(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QScrollBar";

    if (control.styles.removeOne(SBSVERT))
        widget.properties[Orientation] = "Qt::Vertical";
    else if (control.styles.removeOne(SBSHORZ) || true) // We want to remove the style if it exits
        widget.properties[Orientation] = "Qt::Horizontal";

    convertStyles(data, widget, control);
    return widget;
}

static Widget convertButton(const Data &data, Data::Control &control)
{
    if (control.styles.contains(BSPUSHLIKE))
        return convertPushButton(data, control);
    if (control.styles.contains(BS3STATE))
        return convertCheckBox(data, control);
    if (control.styles.contains(BSAUTO3STATE))
        return convertCheckBox(data, control);
    if (control.styles.contains(BSAUTOCHECKBOX))
        return convertCheckBox(data, control);
    if (control.styles.contains(BSAUTORADIOBUTTON))
        return convertRadioButton(data, control);
    if (control.styles.contains(BSCHECKBOX))
        return convertCheckBox(data, control);
    if (control.styles.contains(BSGROUPBOX))
        return convertGroupBox(data, control);
    if (control.styles.contains(BSDEFPUSHBUTTON))
        return convertPushButton(data, control);
    if (control.styles.contains(BSPUSHBUTTON))
        return convertPushButton(data, control);
    if (control.styles.contains(BSRADIOBUTTON))
        return convertRadioButton(data, control);
    return convertPushButton(data, control);
}

static Widget convertSlider(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QSlider";

    if (control.styles.removeOne(TBS_VERT))
        widget.properties[Orientation] = "Qt::Vertical";
    else if (control.styles.removeOne(TBS_HORZ) || true) // We want to remove the style if it exits
        widget.properties[Orientation] = "Qt::Horizontal";

    if (control.styles.removeOne(TBS_NOTICKS))
        widget.properties[TickPosition] = "QSlider::NoTicks";
    if (control.styles.removeOne(TBS_BOTH))
        widget.properties[TickPosition] = "QSlider::TicksBothSides";
    if (control.styles.removeOne(TBS_LEFT))
        widget.properties[TickPosition] = "QSlider::TicksLeft";
    if (control.styles.removeOne(TBS_RIGHT))
        widget.properties[TickPosition] = "QSlider::TicksRight";
    if (control.styles.removeOne(TBS_TOP))
        widget.properties[TickPosition] = "QSlider::TicksAbove";
    if (control.styles.removeOne(TBS_BOTTOM))
        widget.properties[TickPosition] = "QSlider::TicksBelow";

    convertStyles(data, widget, control);
    return widget;
}

static Widget convertSpinBox(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QSpinBox";
    convertStyles(data, widget, control, true);
    return widget;
}

static Widget convertProgressBar(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QProgressBar";

    if (control.styles.removeOne(TBS_VERT))
        widget.properties[Orientation] = "Qt::Vertical";
    else if (control.styles.removeOne(TBS_HORZ) || true) // We want to remove the style if it exits
        widget.properties[Orientation] = "Qt::Horizontal";

    convertStyles(data, widget, control);
    return widget;
}

static Widget convertCalendarWidget(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QCalendarWidget";
    convertStyles(data, widget, control, true);
    return widget;
}

static Widget convertDateTime(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QDateTimeEdit";

    if (control.styles.removeOne(DTS_LONGDATEFORMAT))
        widget.properties[DisplayFormat] = "dddd, MMMM dd, yyyy";
    if (control.styles.removeOne(DTS_SHORTDATEFORMAT))
        widget.properties[DisplayFormat] = "M/d/yy";
    if (control.styles.removeOne(DTS_SHORTDATECENTURYFORMAT))
        widget.properties[DisplayFormat] = "M/d/yyyy";
    if (control.styles.removeOne(DTS_TIMEFORMAT))
        widget.properties[DisplayFormat] = "hh:mm:ss";

    widget.properties[CalendarPopup] = true;

    convertStyles(data, widget, control, true);
    return widget;
}

static Widget convertIpAddress(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QLineEdit";

    widget.properties[InputMask] = "000.000.000.000;_";

    convertStyles(data, widget, control, true);
    return widget;
}

static Widget convertTreeWidget(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QTreeWidget";
    convertStyles(data, widget, control, true);
    return widget;
}

static Widget convertTabWidget(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QTabWidget";

    if (control.styles.removeOne(TCS_BOTTOM))
        widget.properties[TabPosition] = "QTabWidget::South";
    if (control.styles.removeOne(TCS_VERTICAL))
        widget.properties[TabPosition] = "QTabWidget::West";
    if (control.styles.removeOne(TCS_RIGHT))
        widget.properties[TabPosition] = "QTabWidget::East";

    convertStyles(data, widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/control-control
static Widget convertControl(const Data &data, const QString &dialogId, Data::Control &control, bool useIdForPixmap)
{
    if (control.className == "Static")
        return convertLabel(data, control, useIdForPixmap);
    if (control.className == "Button")
        return convertButton(data, control);
    if (control.className == "ComboBox")
        return convertComboBox(data, dialogId, control);
    if (control.className == "ComboBoxEx32")
        return convertComboBox(data, dialogId, control);
    if (control.className == "Edit")
        return convertEditText(data, control);
    if (control.className == "RICHEDIT")
        return convertEditText(data, control);
    if (control.className == "RichEdit20W")
        return convertEditText(data, control);
    if (control.className == "RichEdit20A")
        return convertEditText(data, control);
    if (control.className == "msctls_trackbar")
        return convertSlider(data, control);
    if (control.className == "msctls_trackbar32")
        return convertSlider(data, control);
    if (control.className == "msctls_updown")
        return convertSpinBox(data, control);
    if (control.className == "msctls_updown32")
        return convertSpinBox(data, control);
    if (control.className == "msctls_progress")
        return convertProgressBar(data, control);
    if (control.className == "msctls_progress32")
        return convertProgressBar(data, control);
    if (control.className == "ScrollBar")
        return convertScrollBar(data, control);
    if (control.className == "SysMonthCal32")
        return convertCalendarWidget(data, control);
    if (control.className == "SysDateTimePick32")
        return convertDateTime(data, control);
    if (control.className == "SysIPAddress32")
        return convertIpAddress(data, control);
    if (control.className == "SysListView")
        return convertListWidget(data, control);
    if (control.className == "SysListView32")
        return convertListWidget(data, control);
    if (control.className == "SysTreeView")
        return convertTreeWidget(data, control);
    if (control.className == "SysTreeView32")
        return convertTreeWidget(data, control);
    if (control.className == "SysTabControl")
        return convertTabWidget(data, control);
    if (control.className == "SysTabControl32")
        return convertTabWidget(data, control);
    if (control.className == "SysLink")
        return convertLabel(data, control, useIdForPixmap);
    if (control.className == "MfcPropertyGrid")
        return convertTreeWidget(data, control);

    logger()->critical("{}({}): unknown CONTROL {} / {}", QDir::toNativeSeparators(data.fileName).toStdString(),
                       control.line, control.id.toStdString(), control.className.toStdString());

    Widget widget;
    widget.className = "QWidget";
    return widget;
}

static Widget convertChildWidget(const Data &data, const QString &dialogId, Data::Control control, bool useIdForPixmap)
{
    Widget widget;

    Keywords type = static_cast<Keywords>(control.type);
    switch (type) {
    case Keywords::DEFPUSHBUTTON:
    case Keywords::PUSHBOX:
    case Keywords::PUSHBUTTON:
        widget = convertPushButton(data, control);
        break;
    case Keywords::AUTORADIOBUTTON:
    case Keywords::RADIOBUTTON:
        widget = convertRadioButton(data, control);
        break;
    case Keywords::AUTO3STATE:
    case Keywords::AUTOCHECKBOX:
    case Keywords::CHECKBOX:
    case Keywords::STATE3:
        widget = convertCheckBox(data, control);
        break;
    case Keywords::COMBOBOX:
        widget = convertComboBox(data, dialogId, control);
        break;
    case Keywords::CTEXT:
    case Keywords::LTEXT:
    case Keywords::RTEXT:
    case Keywords::ICON:
        widget = convertLabel(data, control, useIdForPixmap);
        break;
    case Keywords::EDITTEXT:
        widget = convertEditText(data, control);
        break;
    case Keywords::GROUPBOX:
        widget = convertGroupBox(data, control);
        break;
    case Keywords::LISTBOX:
        widget = convertListWidget(data, control);
        break;
    case Keywords::SCROLLBAR:
        widget = convertScrollBar(data, control);
        break;
    case Keywords::CONTROL:
        widget = convertControl(data, dialogId, control, useIdForPixmap);
        break;
    default:
        logger()->critical("{}({}): unknown control type {}", QDir::toNativeSeparators(data.fileName).toStdString(),
                           control.line, Token {Token::Keyword, type}.toString().toStdString());
    }

    widget.id = control.id;
    widget.geometry = control.geometry;

    return widget;
}

static QVector<Widget> adjustHierarchy(QVector<Widget> widgets)
{
    if (widgets.isEmpty())
        return {};

    auto sortByArea = [](const auto &w1, const auto &w2) {
        const int area1 = w1.geometry.width() * w1.geometry.height();
        const int area2 = w2.geometry.width() * w2.geometry.height();
        return area1 < area2;
    };
    std::stable_sort(widgets.begin(), widgets.end(), sortByArea);

    QVector<Widget> result;
    for (int i = 0; i < widgets.size(); ++i) {
        bool isChildren = false;
        Widget iWidget = std::move(widgets[i]);
        QRect geomi = widgets.at(i).geometry;

        // Check if the widget is inside another one
        for (int j = i + 1; j < widgets.size(); ++j) {
            const QRect geomj = widgets.at(j).geometry;
            if (geomj.contains(geomi)) {
                geomi.moveLeft(geomi.x() - geomj.x());
                geomi.moveTop(geomi.y() - geomj.y());
                iWidget.geometry = geomi;
                widgets[j].children.push_back(std::move(iWidget));
                isChildren = true;
                break;
            }
        }
        if (!isChildren)
            result.push_back(std::move(iWidget));
    }

    return result;
}

void adjustGeometry(Widget &widget, double scaleX, double scaleY)
{
    QRect geometry = widget.geometry;
    const bool isComboBox = (widget.className == "QComboBox");

    geometry = QRect(static_cast<int>(scaleX * geometry.x()), static_cast<int>(scaleY * geometry.y()),
                     static_cast<int>(std::ceil(scaleX * geometry.width())),
                     static_cast<int>(isComboBox ? geometry.height() : std::ceil(scaleY * geometry.height())));
    widget.geometry = geometry;

    for (auto &child : widget.children)
        adjustGeometry(child, scaleX, scaleY);
}

Widget convertDialog(const Data &data, const Data::Dialog &d, Widget::ConversionFlags flags, double scaleX,
                     double scaleY)
{
    Data::Dialog dialog = d;

    Widget widget;
    widget.id = dialog.id;
    widget.geometry = dialog.geometry;

    if (dialog.menu.isEmpty()) {
        // If the dialog has a caption, it's a true Qt dialog, otherwise it's a widget
        if (dialog.styles.removeOne(WSCAPTION)) {
            widget.className = "QDialog";
        } else {
            widget.className = "QWidget";
        }
    } else {
        widget.className = "QMainWindow";
    }
    if (!dialog.caption.isEmpty())
        widget.properties[WindowTitle] = dialog.caption;

    if (!dialog.styles.isEmpty()) {
        logger()->debug("{}({}): {} has unused styles {}", QDir::toNativeSeparators(data.fileName).toStdString(),
                        dialog.line, dialog.id.toStdString(), dialog.styles.join(", ").toStdString());
    }

    for (const auto &control : std::as_const(dialog.controls))
        widget.children.push_back(convertChildWidget(data, dialog.id, control, flags & Widget::UseIdForPixmap));

    if (flags & Widget::UpdateGeometry)
        adjustGeometry(widget, scaleX, scaleY);

    // The hierarchy update needs to be done after the geometry update
    // to ensure that combobox are well placed.
    if (flags & Widget::UpdateHierarchy)
        widget.children = adjustHierarchy(widget.children);

    return widget;
}

//=============================================================================
// Action conversion
//=============================================================================
static void fillTips(const Data &data, const QString &id, Action &action)
{
    const auto &text = data.strings.value(id).text;
    if (!text.isEmpty()) {
        const auto tips = text.split('\n');
        action.statusTip = tips.first();
        if (tips.size() > 1)
            action.toolTip = tips.value(1);
    }
}

// actionIdMap allows to find the index of the action in the action vector in case it already exists
static void createActionForMenu(const Data &data, QVector<Action> &actions, QHash<QString, int> &actionIdMap,
                                const MenuItem &item)
{
    if (!item.children.empty()) {
        for (const auto &child : item.children)
            createActionForMenu(data, actions, actionIdMap, child);
    } else if (item.isAction()) {
        // We stop here in case of duplication in the menu
        if (actionIdMap.contains(item.id)) {
            logger()->info("{}({}): duplicate action in menu {}", QDir::toNativeSeparators(data.fileName).toStdString(),
                           item.line, item.id.toStdString());
            return;
        }

        Action action;
        action.id = item.id;
        action.title = item.text;
        action.checked = item.flags & MenuItem::Checked;
        if (!item.shortcut.isEmpty())
            action.shortcuts.push_back({item.shortcut});
        fillTips(data, item.id, action);

        actions.push_back(action);
        // The action is now at the last position of the vector, ie size-1
        actionIdMap[action.id] = actions.size() - 1;
    }
}

static Shortcut createShortcut(const Data &data, const Data::Accelerator &accelerator)
{
    if (accelerator.isUnknown()) {
        logger()->warn("{}({}): unknown shortcut {} / {}", QDir::toNativeSeparators(data.fileName).toStdString(),
                       accelerator.line, accelerator.id.toStdString(), accelerator.shortcut.toStdString());
        return {accelerator.shortcut, true};
    }
    return {accelerator.shortcut};
}

// actionIdMap allows to find the index of the action in the action vector in case it already exists
static void createActionForAccelerator(const Data &data, QVector<Action> &actions, QHash<QString, int> &actionIdMap,
                                       const Data::AcceleratorTable &table)
{
    for (const auto &accelerator : table.accelerators) {
        int index = actionIdMap.value(accelerator.id, -1);
        if (index == -1) {
            Action action;
            action.id = accelerator.id;
            fillTips(data, accelerator.id, action);

            actions.push_back(action);
            // The action is now at the last position of the vector, ie size-1
            index = actions.size() - 1;
            actionIdMap[action.id] = index;
        }
        auto &shortcuts = actions[index].shortcuts;
        auto shortcut = createShortcut(data, accelerator);
        auto it = std::find(shortcuts.begin(), shortcuts.end(), shortcut);
        if (it == shortcuts.end())
            actions[index].shortcuts.push_back(shortcut);
    }
}

// actionIdMap allows to find the index of the action in the action vector in case it already exists
// the convesion flags are used to compute the name of the icon
static void createActionForToolBar(const Data &data, QVector<Action> &actions, QHash<QString, int> &actionIdMap,
                                   const ToolBar &toolBar, Asset::ConversionFlags flags)
{
    QVector<Asset> assets;
    if (auto asset = data.asset(toolBar.id))
        assets = convertAsset(data, *asset, flags);

    int iconIndex = 0;
    for (const auto &item : toolBar.children) {
        if (item.isSeparator())
            continue;

        int index = actionIdMap.value(item.id, -1);
        if (index == -1) {
            Action action;
            action.id = item.id;
            fillTips(data, item.id, action);

            actions.push_back(action);
            // The action is now at the last position of the vector, ie size-1
            index = actions.size() - 1;
            actionIdMap[action.id] = index;
        }
        if (iconIndex < assets.size()) {
            actions[index].iconPath = assets.value(iconIndex).fileName;
            actions[index].iconId = assets.value(iconIndex).id;
        }
        ++iconIndex;
    }
}

QVector<Action> convertActions(const Data &data, const QStringList &menus, const QStringList &accelerators,
                               const QStringList &toolBars, Asset::ConversionFlags flags)
{
    QVector<Action> actions;
    // Map the id of an action, already created, to the index in the vector
    // This allows fast lookup, and avoid creating multiple times the same action
    // Also note that the same action could be used multiple times in a menu or a toolbar
    QHash<QString, int> actionIdMap;

    for (const auto &id : menus) {
        if (const Menu *menu = data.menu(id)) {
            for (const auto &child : menu->children)
                createActionForMenu(data, actions, actionIdMap, child);
        }
    }
    for (const auto &id : accelerators) {
        if (const Data::AcceleratorTable *table = data.acceleratorTable(id))
            createActionForAccelerator(data, actions, actionIdMap, *table);
    }
    for (const auto &id : toolBars) {
        if (const ToolBar *toolBar = data.toolBar(id))
            createActionForToolBar(data, actions, actionIdMap, *toolBar, flags);
    }

    return actions;
}

} // namespace RcCore
