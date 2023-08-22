#include "rcfile.h"

#include "lexer.h"

#include <QDir>
#include <QHash>
#include <QImage>

#include <spdlog/spdlog.h>

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
        spdlog::warn("{}({}): asset and toolbar widths don't match for {}", data.fileName.toStdString(), asset.line,
                     asset.id.toStdString());
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

static void convertStyles(const Data &data, Widget &widget, Data::Control &control, bool isFrame = false)
{
    if (isFrame) {
        if (control.styles.removeOne("WS_EX_CLIENTEDGE")) {
            widget.properties["frame"] = "ClientEdge";
        }
        if (control.styles.removeOne("WS_EX_STATICEDGE")) {
            widget.properties["frame"] = "StaticEdge";
        }
        if (control.styles.removeOne("WS_EX_DLGMODALFRAME")) {
            widget.properties["frame"] = "ModalFrame";
        }
        if (control.styles.removeOne("WS_BORDER")) {
            widget.properties["frame"] = "Border";
        }
    }

    if (control.styles.removeOne("WS_DISABLED")) {
        widget.properties["enabled"] = false;
    }

    // WS_TABSTOP is handled by Qt widgets (focus navigation)
    control.styles.removeOne("WS_TABSTOP");

    if (!control.styles.isEmpty()) {
        spdlog::info("{}({}): {} has unused styles {}", data.fileName.toStdString(), control.line,
                     control.id.toStdString(), control.styles.join(", ").toStdString());
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
    widget.properties["text"] = control.text;

    if (control.styles.removeOne("BS_AUTO3STATE") || control.styles.removeOne("BS_3STATE")
        || control.styles.removeOne("BS_CHECKBOX") || control.styles.removeOne("BS_RADIOBUTTON")
        || control.styles.removeOne("BS_AUTOCHECKBOX") || control.styles.removeOne("BS_AUTORADIOBUTTON"))
        widget.properties["checkable"] = true;

    if (control.styles.removeOne("BS_DEFPUSHBUTTON") || control.type == static_cast<int>(Keywords::DEFPUSHBUTTON))
        widget.properties["default"] = true;

    // The button has an image, but it's handled with the message BM_SETIMAGE, in code
    // No need to port here
    control.styles.removeOne("BS_BITMAP");
    control.styles.removeOne("BS_ICON");

    if (control.styles.removeOne("BS_FLAT") || control.type == static_cast<int>(Keywords::PUSHBOX))
        widget.properties["flat"] = true;

    convertStyles(data, widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/radiobutton-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/autoradiobutton-control
static Widget convertRadioButton(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QRadioButton";
    widget.properties["text"] = control.text;

    control.styles.removeOne("BS_RADIOBUTTON");
    control.styles.removeOne("BS_AUTORADIOBUTTON");
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
    widget.properties["text"] = control.text;

    if (control.styles.removeOne("BS_AUTO3STATE") || control.styles.removeOne("BS_3STATE")
        || control.type == static_cast<int>(Keywords::STATE3) || control.type == static_cast<int>(Keywords::AUTO3STATE))
        widget.properties["tristate"] = true;

    control.styles.removeOne("BS_CHECKBOX");
    control.styles.removeOne("BS_AUTOCHECKBOX");

    convertStyles(data, widget, control);
    return widget;
}

// TODO
static Widget convertComboBox(const Data &data, const QString &dialogId, Data::Control &control)
{
    Widget widget;
    widget.className = "QComboBox";

    if (control.styles.removeOne("CBS_SIMPLE")) {
        widget.className = "QListWidget";
    } else {
        // In MFC, the height is not the height of the combobox
        // So we take the "default" height of a combobox
        control.geometry.setHeight(22);

        if (control.styles.removeOne("CBS_DROPDOWN")) {
            widget.properties["editable"] = true;
            widget.properties["insertPolicy"] = "QComboBox::NoInsert";
        }
    }

    // Initialize the values if they exists
    const auto it = std::find_if(data.dialogData.cbegin(), data.dialogData.cend(), [dialogId](const auto &dialogData) {
        return dialogData.id == dialogId;
    });
    if (it != data.dialogData.cend()) {
        const auto &values = it->values.value(control.id);
        if (!values.isEmpty())
            widget.properties["text"] = values;
    }

    control.styles.removeOne("CBS_DROPDOWNLIST");
    control.styles.removeOne("WS_VSCROLL");
    convertStyles(data, widget, control);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/l"text"-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/r"text"-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/c"text"-control
// https://docs.microsoft.com/en-us/windows/desktop/menurc/icon-control
static Widget convertLabel(const Data &data, Data::Control &control, bool useIdForPixmap)
{
    Widget widget;
    widget.className = "QLabel";

    if (control.styles.removeOne("SS_RIGHT") || control.type == static_cast<int>(Keywords::RTEXT))
        widget.properties["alignment"] = "Qt::AlignRight";
    if (control.styles.removeOne("SS_CENTER") || control.styles.removeOne("SS_CENTERIMAGE")
        || control.type == static_cast<int>(Keywords::CTEXT))
        widget.properties["alignment"] = "Qt::AlignHCenter";

    if (control.styles.removeOne("SS_SUNKEN")) {
        widget.properties["frame"] = "Sunken";
    }

    if (control.styles.removeOne("SS_BLACKFRAME"))
        widget.properties["frame"] = "BlackFrame";

    if (control.styles.removeOne("SS_REALSIZECONTROL"))
        widget.properties["scaledContents"] = true;

    if (control.styles.removeOne("SS_BITMAP") || control.styles.removeOne("SS_ICON")
        || control.type == static_cast<int>(Keywords::ICON)) {
        if (useIdForPixmap) {
            widget.properties["pixmap"] = QStringLiteral(":/%1").arg(control.text);
        } else {
            auto asset = data.asset(control.text);
            if (asset)
                widget.properties["pixmap"] = asset->fileName;
            else
                widget.properties["pixmap"] = QStringLiteral(":/%1").arg(control.text);
        }
    } else {
        widget.properties["text"] = control.text;
    }

    if (control.styles.removeOne("SS_LEFTNOWORDWRAP"))
        widget.properties["wordWrap"] = true;

    control.styles.removeOne("SS_LEFT");
    convertStyles(data, widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/edit"text"-control
static Widget convertEditText(const Data &data, Data::Control &control)
{
    Widget widget;
    bool hasFrame = false;

    // TODO what about RichEdit20W
    if (control.styles.removeOne("ES_MULTILINE") || control.className == "RICHEDIT") {
        widget.className = "QTextEdit";
        hasFrame = true;
    } else {
        widget.className = "QLineEdit";
        if (control.styles.removeOne("ES_CENTER"))
            widget.properties["alignment"] = "Qt::AlignCenter|Qt::AlignVCenter";
        else if (control.styles.removeOne("ES_RIGHT"))
            widget.properties["alignment"] = "Qt::AlignRight|Qt::AlignVCenter";
        else if (control.styles.removeOne("ES_LEFT")
                 || true) // this is the "default", but I want to remove the style too
            widget.properties["alignment"] = "Qt::AlignLeft|Qt::AlignVCenter";

        if (control.styles.removeOne("ES_PASSWORD"))
            widget.properties["echoMode"] = "QLineEdit::Password";
    }

    if (control.styles.removeOne("ES_READONLY"))
        widget.properties["readOnly"] = true;

    convertStyles(data, widget, control, hasFrame);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/groupbox-control
static Widget convertGroupBox(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QGroupBox";
    widget.properties["title"] = control.text;
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
        widget.properties["viewMode"] = "QListView::IconMode";

    if (control.styles.removeOne("LBS_NOSEL"))
        widget.properties["selectionMode"] = "QAbstractItemView::NoSelection";
    else if (control.styles.removeOne("LBS_MULTIPLESEL"))
        widget.properties["selectionMode"] = "QAbstractItemView::MultiSelection";
    else if (control.styles.removeOne("LBS_EXTENDEDSEL"))
        widget.properties["selectionMode"] = "QAbstractItemView::ExtendedSelection";
    else
        widget.properties["selectionMode"] = "QAbstractItemView::SingleSelection";

    if (control.styles.removeOne("LBS_SORT") || control.styles.removeOne("LBS_STANDARD"))
        widget.properties["SortingEnabled"] = true;
    else if (control.styles.removeOne("LBS_MULTIPLESEL"))
        widget.properties["selectionMode"] = "QAbstractItemView::MultiSelection";

    bool alwaysOn = control.styles.removeOne("LBS_DISABLENOSCROLL");
    if (control.styles.removeOne("WS_HSCROLL"))
        widget.properties["horizontalScrollBarPolicy"] = alwaysOn ? "Qt::ScrollBarAlwaysOn" : "Qt::ScrollBarAsNeeded";
    else
        widget.properties["horizontalScrollBarPolicy"] = "Qt::ScrollBarAlwaysOff";
    if (control.styles.removeOne("WS_VSCROLL"))
        widget.properties["verticalScrollBarPolicy"] = alwaysOn ? "Qt::ScrollBarAlwaysOn" : "Qt::ScrollBarAsNeeded";
    else
        widget.properties["verticalScrollBarPolicy"] = "Qt::ScrollBarAlwaysOff";

    convertStyles(data, widget, control, true);
    return widget;
}

// https://docs.microsoft.com/en-us/windows/desktop/menurc/scrollbar-control
static Widget convertScrollBar(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QScrollBar";

    if (control.styles.removeOne("SBS_VERT"))
        widget.properties["orientation"] = "Qt::Vertical";
    else if (control.styles.removeOne("SBS_HORZ") || true) // We want to remove the style if it exits
        widget.properties["orientation"] = "Qt::Horizontal";

    convertStyles(data, widget, control);
    return widget;
}

static Widget convertButton(const Data &data, Data::Control &control)
{
    if (control.styles.contains("BS_PUSHLIKE"))
        return convertPushButton(data, control);
    if (control.styles.contains("BS_3STATE"))
        return convertCheckBox(data, control);
    if (control.styles.contains("BS_AUTO3STATE"))
        return convertCheckBox(data, control);
    if (control.styles.contains("BS_AUTOCHECKBOX"))
        return convertCheckBox(data, control);
    if (control.styles.contains("BS_AUTORADIOBUTTON"))
        return convertRadioButton(data, control);
    if (control.styles.contains("BS_CHECKBOX"))
        return convertCheckBox(data, control);
    if (control.styles.contains("BS_GROUPBOX"))
        return convertGroupBox(data, control);
    if (control.styles.contains("BS_DEFPUSHBUTTON"))
        return convertPushButton(data, control);
    if (control.styles.contains("BS_PUSHBUTTON"))
        return convertPushButton(data, control);
    if (control.styles.contains("BS_RADIOBUTTON"))
        return convertRadioButton(data, control);
    return convertPushButton(data, control);
}

static Widget convertSlider(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QSlider";

    if (control.styles.removeOne("TBS_VERT"))
        widget.properties["orientation"] = "Qt::Vertical";
    else if (control.styles.removeOne("TBS_HORZ") || true) // We want to remove the style if it exits
        widget.properties["orientation"] = "Qt::Horizontal";

    if (control.styles.removeOne("TBS_NOTICKS"))
        widget.properties["tickPosition"] = "QSlider::NoTicks";
    if (control.styles.removeOne("TBS_BOTH"))
        widget.properties["tickPosition"] = "QSlider::TicksBothSides";
    if (control.styles.removeOne("TBS_LEFT"))
        widget.properties["tickPosition"] = "QSlider::TicksLeft";
    if (control.styles.removeOne("TBS_RIGHT"))
        widget.properties["tickPosition"] = "QSlider::TicksRight";
    if (control.styles.removeOne("TBS_TOP"))
        widget.properties["tickPosition"] = "QSlider::TicksAbove";
    if (control.styles.removeOne("TBS_BOTTOM"))
        widget.properties["tickPosition"] = "QSlider::TicksBelow";

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

    if (control.styles.removeOne("TBS_VERT"))
        widget.properties["orientation"] = "Qt::Vertical";
    else if (control.styles.removeOne("TBS_HORZ") || true) // We want to remove the style if it exits
        widget.properties["orientation"] = "Qt::Horizontal";

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

    if (control.styles.removeOne("DTS_LONGDATEFORMAT"))
        widget.properties["displayFormat"] = "dddd, MMMM dd, yyyy";
    if (control.styles.removeOne("DTS_SHORTDATEFORMAT"))
        widget.properties["displayFormat"] = "M/d/yy";
    if (control.styles.removeOne("DTS_SHORTDATECENTURYFORMAT"))
        widget.properties["displayFormat"] = "M/d/yyyy";
    if (control.styles.removeOne("DTS_TIMEFORMAT"))
        widget.properties["displayFormat"] = "hh:mm:ss";

    widget.properties["calendarPopup"] = true;

    convertStyles(data, widget, control, true);
    return widget;
}

static Widget convertIpAddress(const Data &data, Data::Control &control)
{
    Widget widget;
    widget.className = "QLineEdit";

    widget.properties["inputMask"] = "000.000.000.000;_";

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

    if (control.styles.removeOne("TCS_BOTTOM"))
        widget.properties["tabPosition"] = "QTabWidget::South";
    if (control.styles.removeOne("TCS_VERTICAL"))
        widget.properties["tabPosition"] = "QTabWidget::West";
    if (control.styles.removeOne("TCS_RIGHT"))
        widget.properties["tabPosition"] = "QTabWidget::East";

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
    if (control.className == "MfcButton")
        return convertButton(data, control);

    spdlog::warn("{}({}): unknown CONTROL {} / {}", data.fileName.toStdString(), control.line, control.id.toStdString(),
                 control.className.toStdString());

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
        spdlog::error("{}({}): unknown control type {}", data.fileName.toStdString(), control.line,
                      Token {Token::Keyword, type}.toString().toStdString());
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
        if (dialog.styles.removeOne("WS_CAPTION")) {
            widget.className = "QDialog";
        } else {
            widget.className = "QWidget";
        }
    } else {
        widget.className = "QMainWindow";
    }

    if (!dialog.caption.isEmpty()) {
        // Set the window title property
        widget.properties["windowTitle"] = dialog.caption;
    }

    if (!dialog.styles.isEmpty()) {
        spdlog::info("{}({}): {} has unused styles {}", data.fileName.toStdString(), dialog.line,
                     dialog.id.toStdString(), dialog.styles.join(", ").toStdString());
    }

    for (const auto &control : std::as_const(dialog.controls)) {
        widget.children.push_back(convertChildWidget(data, dialog.id, control, flags & Widget::UseIdForPixmap));
    }

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
            spdlog::info("{}({}): duplicate action in menu {}", data.fileName.toStdString(), item.line,
                         item.id.toStdString());
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
        spdlog::warn("{}({}): unknown shortcut {} / {}", data.fileName.toStdString(), accelerator.line,
                     accelerator.id.toStdString(), accelerator.shortcut.toStdString());
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
            if (actions[index].iconId.isEmpty()) {
                actions[index].iconId = assets.value(iconIndex).id;
                actions[index].iconPath = assets.value(iconIndex).fileName;
            }
        }
        ++iconIndex;
    }
}

QVector<Action> convertActions(const Data &data, Asset::ConversionFlags flags)
{
    QVector<Action> actions;
    // Map the id of an action, already created, to the index in the vector
    // This allows fast lookup, and avoid creating multiple times the same action
    // Also note that the same action could be used multiple times in a menu or a toolbar
    QHash<QString, int> actionIdMap;

    for (const auto &menu : data.menus) {
        for (const auto &child : menu.children)
            createActionForMenu(data, actions, actionIdMap, child);
    }
    for (const auto &table : data.acceleratorTables)
        createActionForAccelerator(data, actions, actionIdMap, table);
    for (const auto &toolBar : data.toolBars)
        createActionForToolBar(data, actions, actionIdMap, toolBar, flags);

    return actions;
}

} // namespace RcCore
