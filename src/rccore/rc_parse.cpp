#include "rcfile.h"

#include "lexer.h"
#include "rc_utils.h"
#include "stream.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QKeySequence>
#include <QTextStream>

namespace RcCore {

//=============================================================================
// Utility methods
//=============================================================================
static std::optional<QString> computeFilePath(const QString &absolutePath, const QString &path)
{
    QFileInfo fi(path);
    if (!fi.isAbsolute()) {
        fi.setFile(absolutePath);
        fi.setFile(fi.absolutePath() + '/' + path);
    }
    if (fi.exists())
        return fi.absoluteFilePath();
    return {};
}

static QHash<int, QString> loadResourceFile(const QString &resourceFile)
{
    QFile file(resourceFile);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QTextStream stream(&file);
    QHash<int, QString> resourceMap;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (!line.startsWith("#define"))
            continue;

        QStringList fields = line.split(' ', Qt::SkipEmptyParts);
        if (fields.size() < 3)
            continue;
        const auto &value = fields.at(1);
        bool ok;
        const int key = fields.at(2).toInt(&ok);
        if (!ok)
            continue;

        resourceMap[key] = value;
    }
    return resourceMap;
}

static QString toId(const std::optional<Token> &token, const Data &data)
{
    if (token->type == Token::Integer) {
        const int value = token->toInt();
        if (value == 0)
            return {};
        return data.resourceMap.value(value, QString::number(value));
    }
    return token->toString();
}

static QStringList readStyles(Lexer &lexer)
{
    QStringList result;
    bool hasNot = false;
    bool readNext = true;
    while (readNext) {
        const auto token = lexer.next();
        if (token->type == Token::Word) {
            const QString item = (hasNot ? "!" : QString()) + token->toString();
            result.push_back(item);
            hasNot = false;
        } else if (token->type == Token::Keyword && token->toKeyword() == Keywords::NOT) {
            hasNot = true;
            continue;
        }
        readNext = (lexer.peek()->type == Token::Operator_Or);
        if (readNext)
            lexer.next();
    }
    return result;
}

static void readDialogStatements(Lexer &lexer, Data::Dialog &dialog, const Data &data)
{
    while (const auto peekToken = lexer.peek()) {
        // There could be some #if/#else/#end here
        // Just skip them, as they don't add much values
        if (peekToken->type == Token::Directive) {
            lexer.skipLine();
            continue;
        }
        if (peekToken->toKeyword() == Keywords::BEGIN)
            break;
        const auto token = lexer.next();
        switch (token->toKeyword()) {
        case Keywords::CHARACTERISTICS:
        case Keywords::CLASS:
        case Keywords::FONT:
        case Keywords::LANGUAGE:
        case Keywords::VERSION:
            lexer.skipLine();
            break;
        case Keywords::CAPTION:
            dialog.caption = lexer.next()->toString();
            break;
        case Keywords::MENU:
            dialog.menu = toId(lexer.next(), data);
            break;
        case Keywords::EXSTYLE:
        case Keywords::STYLE:
            dialog.styles = readStyles(lexer);
            break;
        default:
            logger()->critical("{}({}): file parser error on token {}",
                               QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                               token->prettyPrint().toStdString());
            break;
        }
    }
}

static QString toShortcut(QString event, bool isAscii, Qt::KeyboardModifiers modifiers)
{
    static const QHash<QString, Qt::Key> keyMap = {
        {"VK_CANCEL", Qt::Key_Cancel},
        {"VK_BACK", Qt::Key_Backspace},
        {"VK_TAB", Qt::Key_Tab},
        {"VK_CLEAR", Qt::Key_Clear},
        {"VK_RETURN", Qt::Key_Return},
        {"VK_SHIFT", Qt::Key_Shift},
        {"VK_CONTROL", Qt::Key_Control},
        {"VK_MENU", Qt::Key_Alt},
        {"VK_PAUSE", Qt::Key_Pause},
        {"VK_CAPITAL", Qt::Key_CapsLock},
        {"VK_ESCAPE", Qt::Key_Escape},
        {"VK_MODECHANGE", Qt::Key_Mode_switch},
        {"VK_SPACE", Qt::Key_Space},
        {"VK_PRIOR", Qt::Key_PageUp},
        {"VK_NEXT", Qt::Key_PageDown},
        {"VK_END", Qt::Key_End},
        {"VK_HOME", Qt::Key_Home},
        {"VK_LEFT", Qt::Key_Left},
        {"VK_UP", Qt::Key_Up},
        {"VK_RIGHT", Qt::Key_Right},
        {"VK_DOWN", Qt::Key_Down},
        {"VK_SELECT", Qt::Key_Select},
        {"VK_PRINT", Qt::Key_Printer},
        {"VK_EXECUTE", Qt::Key_Execute},
        {"VK_SNAPSHOT", Qt::Key_Print},
        {"VK_INSERT", Qt::Key_Insert},
        {"VK_DELETE", Qt::Key_Delete},
        {"VK_HELP", Qt::Key_Help},
        {"VK_0", Qt::Key_0},
        {"VK_1", Qt::Key_1},
        {"VK_2", Qt::Key_2},
        {"VK_3", Qt::Key_3},
        {"VK_4", Qt::Key_4},
        {"VK_5", Qt::Key_5},
        {"VK_6", Qt::Key_6},
        {"VK_7", Qt::Key_7},
        {"VK_8", Qt::Key_8},
        {"VK_9", Qt::Key_9},
        {"VK_A", Qt::Key_A},
        {"VK_B", Qt::Key_B},
        {"VK_C", Qt::Key_C},
        {"VK_D", Qt::Key_D},
        {"VK_E", Qt::Key_E},
        {"VK_F", Qt::Key_F},
        {"VK_G", Qt::Key_G},
        {"VK_H", Qt::Key_H},
        {"VK_I", Qt::Key_I},
        {"VK_J", Qt::Key_J},
        {"VK_K", Qt::Key_K},
        {"VK_L", Qt::Key_L},
        {"VK_M", Qt::Key_M},
        {"VK_N", Qt::Key_N},
        {"VK_O", Qt::Key_O},
        {"VK_P", Qt::Key_P},
        {"VK_Q", Qt::Key_Q},
        {"VK_R", Qt::Key_R},
        {"VK_S", Qt::Key_S},
        {"VK_T", Qt::Key_T},
        {"VK_U", Qt::Key_U},
        {"VK_V", Qt::Key_V},
        {"VK_W", Qt::Key_W},
        {"VK_X", Qt::Key_X},
        {"VK_Y", Qt::Key_Y},
        {"VK_Z", Qt::Key_Z},
        {"VK_LWIN", Qt::Key_Meta},
        {"VK_RWIN", Qt::Key_Meta},
        {"VK_APPS", Qt::Key_Menu},
        {"VK_SLEEP", Qt::Key_Sleep},
        {"VK_NUMPAD0", Qt::Key_0},
        {"VK_NUMPAD1", Qt::Key_1},
        {"VK_NUMPAD2", Qt::Key_2},
        {"VK_NUMPAD3", Qt::Key_3},
        {"VK_NUMPAD4", Qt::Key_4},
        {"VK_NUMPAD5", Qt::Key_5},
        {"VK_NUMPAD6", Qt::Key_6},
        {"VK_NUMPAD7", Qt::Key_7},
        {"VK_NUMPAD8", Qt::Key_8},
        {"VK_NUMPAD9", Qt::Key_9},
        {"VK_MULTIPLY", Qt::Key_Asterisk},
        {"VK_ADD", Qt::Key_Plus},
        {"VK_SUBTRACT", Qt::Key_Minus},
        {"VK_DIVIDE", Qt::Key_Slash},
        {"VK_F1", Qt::Key_F1},
        {"VK_F2", Qt::Key_F2},
        {"VK_F3", Qt::Key_F3},
        {"VK_F4", Qt::Key_F4},
        {"VK_F5", Qt::Key_F5},
        {"VK_F6", Qt::Key_F6},
        {"VK_F7", Qt::Key_F7},
        {"VK_F8", Qt::Key_F8},
        {"VK_F9", Qt::Key_F9},
        {"VK_F10", Qt::Key_F10},
        {"VK_F11", Qt::Key_F11},
        {"VK_F12", Qt::Key_F12},
        {"VK_F13", Qt::Key_F13},
        {"VK_F14", Qt::Key_F14},
        {"VK_F15", Qt::Key_F15},
        {"VK_F16", Qt::Key_F16},
        {"VK_F17", Qt::Key_F17},
        {"VK_F18", Qt::Key_F18},
        {"VK_F19", Qt::Key_F19},
        {"VK_F20", Qt::Key_F20},
        {"VK_F21", Qt::Key_F21},
        {"VK_F22", Qt::Key_F22},
        {"VK_F23", Qt::Key_F23},
        {"VK_F24", Qt::Key_F24},
        {"VK_NUMLOCK", Qt::Key_NumLock},
        {"VK_SCROLL", Qt::Key_ScrollLock},
        {"VK_LSHIFT", Qt::Key_Shift},
        {"VK_RSHIFT", Qt::Key_Shift},
        {"VK_LCONTROL", Qt::Key_Control},
        {"VK_RCONTROL", Qt::Key_Control},
        {"VK_LMENU", Qt::Key_Alt},
        {"VK_RMENU", Qt::Key_Alt},
        {"VK_BROWSER_BACK", Qt::Key_Back},
        {"VK_BROWSER_FORWARD", Qt::Key_Forward},
        {"VK_BROWSER_REFRESH", Qt::Key_Refresh},
        {"VK_BROWSER_STOP", Qt::Key_Stop},
        {"VK_BROWSER_SEARCH", Qt::Key_Search},
        {"VK_BROWSER_FAVORITES", Qt::Key_Favorites},
        {"VK_BROWSER_HOME", Qt::Key_HomePage},
        {"VK_VOLUME_MUTE", Qt::Key_VolumeMute},
        {"VK_VOLUME_DOWN", Qt::Key_VolumeDown},
        {"VK_VOLUME_UP", Qt::Key_VolumeUp},
        {"VK_MEDIA_NEXT_TRACK", Qt::Key_MediaNext},
        {"VK_MEDIA_PREV_TRACK", Qt::Key_MediaPrevious},
        {"VK_MEDIA_STOP", Qt::Key_MediaStop},
        {"VK_MEDIA_PLAY_PAUSE", Qt::Key_MediaPlay},
        {"VK_LAUNCH_MAIL", Qt::Key_LaunchMail},
        {"VK_LAUNCH_MEDIA_SELECT", Qt::Key_LaunchMedia},
        {"VK_LAUNCH_APP1", Qt::Key_Launch0},
        {"VK_LAUNCH_APP2", Qt::Key_Launch1},
        {"VK_PLAY", Qt::Key_Play},
        {"VK_ZOOM", Qt::Key_Zoom},
        {"VK_OEM_CLEAR", Qt::Key_Clear},
        {"VK_OEM_1", Qt::Key_unknown},
        {"VK_OEM_PLUS", Qt::Key_unknown},
        {"VK_OEM_COMMA", Qt::Key_unknown},
        {"VK_OEM_MINUS", Qt::Key_unknown},
        {"VK_OEM_PERIOD", Qt::Key_unknown},
        {"VK_OEM_2", Qt::Key_unknown},
        {"VK_OEM_3", Qt::Key_unknown},
        {"VK_OEM_4", Qt::Key_unknown},
        {"VK_OEM_5", Qt::Key_unknown},
        {"VK_OEM_6", Qt::Key_unknown},
        {"VK_OEM_7", Qt::Key_unknown},
        {"VK_OEM_8", Qt::Key_unknown},
        {"VK_OEM_AX", Qt::Key_unknown},
        {"VK_OEM_102", Qt::Key_unknown},
        {"VK_ICO_HELP", Qt::Key_unknown},
        {"VK_ICO_00", Qt::Key_unknown},
        {"VK_PROCESSKEY", Qt::Key_unknown},
        {"VK_ICO_CLEAR", Qt::Key_unknown},
        {"VK_PACKET", Qt::Key_unknown},
        {"VK_OEM_RESET", Qt::Key_unknown},
        {"VK_OEM_JUMP", Qt::Key_unknown},
        {"VK_OEM_PA1", Qt::Key_unknown},
        {"VK_OEM_PA2", Qt::Key_unknown},
        {"VK_OEM_PA3", Qt::Key_unknown},
        {"VK_OEM_WSCTRL", Qt::Key_unknown},
        {"VK_OEM_CUSEL", Qt::Key_unknown},
        {"VK_OEM_ATTN", Qt::Key_unknown},
        {"VK_OEM_FINISH", Qt::Key_unknown},
        {"VK_OEM_COPY", Qt::Key_unknown},
        {"VK_OEM_AUTO", Qt::Key_unknown},
        {"VK_OEM_ENLW", Qt::Key_unknown},
        {"VK_OEM_BACKTAB", Qt::Key_unknown},
        {"VK_ATTN", Qt::Key_unknown},
        {"VK_CRSEL", Qt::Key_unknown},
        {"VK_EXSEL", Qt::Key_unknown},
        {"VK_EREOF", Qt::Key_unknown},
        {"VK_NONAME", Qt::Key_unknown},
        {"VK_PA1", Qt::Key_unknown},
    };

    int key;
    if (keyMap.contains(event)) {
        key = keyMap.value(event);
        if (key == Qt::Key_unknown) {
            // We keep a text of the shortcut to follow on that in the code
            if (modifiers & Qt::AltModifier)
                event.prepend("Alt+");
            if (modifiers & Qt::ControlModifier)
                event.prepend("Ctrl+");
            if (modifiers & Qt::ShiftModifier)
                event.prepend("Shift+");
            return event;
        }
    } else {
        if (event.startsWith('^') && event.size() > 1) {
            modifiers |= Qt::ControlModifier;
            isAscii = false;
            event = event.mid(1);
        }

        if (isAscii && event.isUpper())
            modifiers |= Qt::ShiftModifier;
        key = event.toUpper().at(0).toLatin1();
    }

    QKeySequence seq(key | modifiers);
    return seq.toString();
}

//=============================================================================
// Resource parser
//=============================================================================
static void readAsset(Lexer &lexer, Data &data, Keywords keyword, const QString &id)
{
    //    nameID KEYWORD filename

    Asset asset;
    asset.line = lexer.line();
    asset.id = id;

    const auto fileName = lexer.next()->toString().replace("\\\\", "/");
    if (const auto fullPath = computeFilePath(data.fileName, fileName)) {
        asset.fileName = fullPath.value();
        asset.exist = true;
    } else {
        asset.fileName = fileName;
        asset.exist = false;
    }
    switch (keyword) {
    case Keywords::ICON:
        data.icons.push_back(asset);
        break;
    case Keywords::BITMAP:
    case Keywords::CURSOR:
    case Keywords::PNG:
        data.assets.push_back(asset);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

static void readStringTable(Lexer &lexer, Data &data)
{
    //    STRINGTABLE
    //      [optional-statements]
    //    BEGIN
    //    stringID string
    //    . . .
    //    END

    lexer.skipToBegin();
    while (lexer.peek()->type != Token::Keyword) {
        const int line = lexer.line();
        const auto id = lexer.next()->toString();
        const auto string = lexer.next()->toString();
        data.strings[id] = {line, id, string};
    }
    lexer.next(); // END
}

static Data::Accelerator readAccelerator(Lexer &lexer)
{
    Data::Accelerator accelerator;
    accelerator.line = lexer.line();
    QString event;
    if (lexer.peek()->type == Token::Integer)
        event = QChar(lexer.next()->toInt());
    else
        event = lexer.next()->toString();
    lexer.skipComma();
    accelerator.id = lexer.next()->toString();

    bool isAscii = true;
    Qt::KeyboardModifiers modifiers;
    while (lexer.peek()->type == Token::Operator_Comma) {
        lexer.skipComma();
        switch (lexer.next()->toKeyword()) {
        case Keywords::ASCII:
            isAscii = true;
            break;
        case Keywords::VIRTKEY:
            isAscii = false;
            break;
        case Keywords::CONTROL:
            modifiers |= Qt::ControlModifier;
            break;
        case Keywords::SHIFT:
            modifiers |= Qt::ShiftModifier;
            break;
        case Keywords::ALT:
            modifiers |= Qt::AltModifier;
            break;
        default:
            break;
        }
    }
    accelerator.shortcut = toShortcut(event, isAscii, modifiers);
    if (accelerator.shortcut.isEmpty()) {
        logger()->warn("{}({}): file parser unknown accelerator {}",
                       QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(), event.toStdString());
    }
    return accelerator;
}

static void readAccelerators(Lexer &lexer, Data &data, const QString &id)
{
    //    acctablename ACCELERATORS
    //    [optional-statements]
    //    BEGIN
    //        event, idvalue, [type] [options]
    //    END

    Data::AcceleratorTable table;
    table.line = lexer.line() - 1;
    table.id = id;
    lexer.skipToBegin();

    while (lexer.peek()->type != Token::Keyword)
        table.accelerators.push_back(readAccelerator(lexer));
    lexer.next(); // END

    data.acceleratorTables.push_back(table);
}

static void readDialogInit(Lexer &lexer, Data &data, const QString &id)
{
    //    dialogId DLGINIT
    //      [optional-statements]
    //    BEGIN
    //    controlID, bytes, int, 0, bytes...
    //    . . .
    //    0
    //    END
    Data::DialogData dialogData;
    dialogData.line = lexer.line();
    dialogData.id = id;
    lexer.skipToBegin();

    while (lexer.peek()->type != Token::Keyword) {
        const auto controlId = toId(lexer.next(), data);
        if (controlId.isNull()) // end of the DLGINIT
            continue;

        lexer.skipComma();
        lexer.next(); // 0x403 or 0x1234
        lexer.skipComma();
        int size = lexer.next()->toInt();
        lexer.skipComma();
        lexer.next(); // '0'
        lexer.skipComma();

        QString text;
        while (size > 0) {
            const auto bytes = lexer.next()->toString();
            if (bytes.startsWith("0x"))
                text += QString(bytes.at(4)) + bytes.at(5) + bytes.at(2) + bytes.at(3);
            else
                text += "00";
            lexer.skipComma();
            size -= 2;
        }
        dialogData.values[controlId].push_back(QString::fromLocal8Bit(QByteArray::fromHex(text.toLocal8Bit())));
    }
    lexer.next(); // END

    data.dialogData.push_back(dialogData);
}

static MenuItem readMenuItem(Lexer &lexer, Data &data)
{
    //    MENUITEM text, result, [optionlist]
    //    MENUITEM SEPARATOR

    MenuItem item;
    item.line = lexer.line();

    const auto token = lexer.next();
    if (token->type == Token::String) {
        const auto split = token->toString().split('\t');
        item.text = split.first();
        if (split.size() == 2)
            item.shortcut = split.last();
        lexer.skipComma();
        item.id = toId(lexer.next(), data);

        bool readNext = lexer.peek()->type == Token::Operator_Comma;

        while (readNext) {
            lexer.next();
            const auto flagToken = lexer.next();
            switch (flagToken->toKeyword()) {
            case Keywords::CHECKED:
                item.flags |= MenuItem::Checked;
                break;
            case Keywords::GRAYED:
                item.flags |= MenuItem::Grayed;
                break;
            case Keywords::HELP:
                item.flags |= MenuItem::Help;
                break;
            case Keywords::INACTIVE:
                item.flags |= MenuItem::Inactive;
                break;
            case Keywords::MENUBARBREAK:
                item.flags |= MenuItem::MenuBarBreak;
                break;
            case Keywords::MENUBREAK:
                item.flags |= MenuItem::MenuBreak;
                break;
            default:
                logger()->warn("{}({}): file parser unhandled token {}",
                               QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                               token->prettyPrint().toStdString());
            }

            const auto type = lexer.peek()->type;
            readNext = (type == Token::Operator_Comma || type == Token::Operator_Or);
        }
    }
    return item;
}

static MenuItem readMenuPopup(Lexer &lexer, Data &data);

static void readMenuChildren(Lexer &lexer, Data &data, QVector<MenuItem> &children)
{
    lexer.skipToBegin();

    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        switch (token->toKeyword()) {
        case Keywords::POPUP:
            children.append(readMenuPopup(lexer, data));
            break;
        case Keywords::MENUITEM:
            children.append(readMenuItem(lexer, data));
            break;
        default:
            logger()->warn("{}({}): file parser unhandled token {}",
                           QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                           token->prettyPrint().toStdString());
        }
        token = lexer.next();
    }
}

static MenuItem readMenuPopup(Lexer &lexer, Data &data)
{
    //    POPUP text, [optionlist]
    //    BEGIN
    //        item-definitions ...
    //    END

    MenuItem popup;
    popup.line = lexer.line();
    popup.text = lexer.next()->toString();

    readMenuChildren(lexer, data, popup.children);
    return popup;
}

static void readMenu(Lexer &lexer, Data &data, const QString &id)
{
    //    menuID MENU
    //    [optional-statements]
    //    BEGIN
    //        item-definitions ...
    //    END

    Menu menu;
    menu.line = lexer.line() - 1;
    menu.id = id;

    readMenuChildren(lexer, data, menu.children);
    for (auto &child : menu.children)
        child.isTopLevel = true;
    data.menus.push_back(menu);
}

static void readToolBar(Lexer &lexer, Data &data, const QString &id)
{
    //    toolbarID TOOLBAR width, height
    //    [optional-statements]
    //    BEGIN
    //        BUTTON  id
    //        SEPARATOR
    //        ...
    //    END

    ToolBar toolBar;
    toolBar.line = lexer.line();
    toolBar.id = id;
    toolBar.iconSize.setWidth(lexer.next()->toInt());
    lexer.skipComma();
    toolBar.iconSize.setHeight(lexer.next()->toInt());

    lexer.skipToBegin();
    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        switch (token->toKeyword()) {
        case Keywords::BUTTON: {
            ToolBarItem buttonNode;
            buttonNode.line = lexer.line();
            buttonNode.id = toId(lexer.next(), data);
            toolBar.children.push_back(buttonNode);
            break;
        }
        case Keywords::SEPARATOR: {
            ToolBarItem separatorNode;
            separatorNode.line = lexer.line();
            toolBar.children.push_back(separatorNode);
            break;
        }
        default:
            logger()->warn("{}({}): file parser unhandled token {}",
                           QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                           token->prettyPrint().toStdString());
        }
        lexer.skipLine();
        token = lexer.next();
    }
    data.toolBars.push_back(toolBar);
}

static Data::Control readControl(Lexer &lexer, Data &data, const std::optional<Token> &token)
{
    //  AUTO3STATE text, id, x, y, width, height [, style [, extended-style]]
    //	AUTOCHECKBOX text, id, x, y, width, height [, style [, extended-style]]
    //	AUTORADIOBUTTON text, id, x, y, width, height [, style [, extended-style]]
    //	CHECKBOX text, id, x, y, width, height [, style [, extended-style]]
    //	COMBOBOX id, x, y, width, height [, style [, extended-style]]
    //	CONTROL text, id, class, style, x, y, width, height [, extended-style]
    //	CTEXT text, id, x, y, width, height [, style [, extended-style]]
    //	DEFPUSHBUTTON text, id, x, y, width, height [, style [, extended-style]]
    //	EDITTEXT id, x, y, width, height [, style [, extended-style]]
    //	GROUPBOX text, id, x, y, width, height [, style [, extended-style]]
    //	ICON text, id, x, y [, width, height, style [, extended-style]]
    //	LISTBOX id, x, y, width, height [, style [, extended-style]]
    //	LTEXT text, id, x, y, width, height [, style [, extended-style]]
    //	PUSHBOX text, id, x, y, width, height [, style [, extended-style]]
    //	PUSHBUTTON text, id, x, y, width, height [, style [, extended-style]]
    //	RADIOBUTTON text, id, x, y, width, height [, style [, extended-style]]
    //	SCROLLBAR id, x, y, width, height [, style [, extended-style]]
    //	STATE3 text, id, x, y, width, height [, style [, extended-style]]

    static QVector<Keywords> knownControls = {
        Keywords::AUTO3STATE,  Keywords::AUTOCHECKBOX,  Keywords::AUTORADIOBUTTON,
        Keywords::CHECKBOX,    Keywords::COMBOBOX,      Keywords::CONTROL,
        Keywords::CTEXT,       Keywords::DEFPUSHBUTTON, Keywords::EDITTEXT,
        Keywords::GROUPBOX,    Keywords::ICON,          Keywords::LISTBOX,
        Keywords::LTEXT,       Keywords::PUSHBOX,       Keywords::PUSHBUTTON,
        Keywords::RADIOBUTTON, Keywords::RTEXT,         Keywords::SCROLLBAR,
        Keywords::STATE3,
    };

    const auto controlType = token->toKeyword();
    Data::Control control;
    control.line = lexer.line();
    control.type = static_cast<int>(controlType);

    if (!knownControls.contains(controlType)) {
        logger()->warn("{}({}): file parser unknown control {}",
                       QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                       token->prettyPrint().toStdString());
        return control;
    }

    // Only a limited number of controls have a text
    static QVector<Keywords> textControl = {Keywords::AUTO3STATE, Keywords::AUTOCHECKBOX, Keywords::AUTORADIOBUTTON,
                                            Keywords::CHECKBOX,   Keywords::CTEXT,        Keywords::DEFPUSHBUTTON,
                                            Keywords::GROUPBOX,   Keywords::ICON,         Keywords::LTEXT,
                                            Keywords::PUSHBOX,    Keywords::PUSHBUTTON,   Keywords::RADIOBUTTON,
                                            Keywords::RTEXT,      Keywords::STATE3,       Keywords::CONTROL};

    if (textControl.contains(controlType)) {
        if (controlType == Keywords::ICON || controlType == Keywords::CONTROL)
            control.text = toId(lexer.next(), data);
        else
            control.text = lexer.next()->toString();
        lexer.skipComma();
    }

    control.id = toId(lexer.next(), data);
    lexer.skipComma();

    // CONTROL has styles before geometry
    if (controlType == Keywords::CONTROL) {
        control.className = lexer.next()->toString();
        lexer.skipComma();
        control.styles = readStyles(lexer);
        lexer.skipComma();
    }

    QRect geometry;
    geometry.setX(lexer.next()->toInt());
    lexer.skipComma();
    geometry.setY(lexer.next()->toInt());
    lexer.skipComma();
    geometry.setWidth(lexer.next()->toInt());
    lexer.skipComma();
    geometry.setHeight(lexer.next()->toInt());
    control.geometry = geometry;

    if (lexer.peek()->type == Token::Operator_Comma) {
        lexer.skipComma();
        // Sometimes, there's a comma but the control is done
        if (lexer.peek()->type != Token::Keyword || lexer.peek()->toKeyword() == Keywords::NOT)
            control.styles += readStyles(lexer);
    }

    // CONTROL has no extended styles here
    if (controlType != Keywords::CONTROL && lexer.peek()->type == Token::Operator_Comma) {
        lexer.skipComma();
        // Sometimes, there's a comma but the control is done
        if (lexer.peek()->type != Token::Keyword || lexer.peek()->toKeyword() == Keywords::NOT)
            control.styles += readStyles(lexer);
    }

    if (lexer.peek()->type == Token::Operator_Comma) {
        lexer.skipComma();
        lexer.skipLine();
    }
    return control;
}

static void readDialog(Lexer &lexer, Data &data, const QString &id)
{
    //    nameID DIALOG x, y, width, height [, helpID]
    //    [optional-statements]
    //    BEGIN
    //        control-statements ...
    //    END

    Data::Dialog dialog;
    dialog.line = lexer.line();
    dialog.id = id;

    QRect geometry;
    geometry.setX(lexer.next()->toInt());
    lexer.skipComma();
    geometry.setY(lexer.next()->toInt());
    lexer.skipComma();
    geometry.setWidth(lexer.next()->toInt());
    lexer.skipComma();
    geometry.setHeight(lexer.next()->toInt());
    dialog.geometry = geometry;

    readDialogStatements(lexer, dialog, data);

    lexer.next(); // BEGIN
    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        dialog.controls.push_back(readControl(lexer, data, token));
        token = lexer.next();
    }

    data.dialogs.push_back(dialog);
}

static void skipResourceAttributes(Lexer &lexer)
{
    // Ignore any resource attributes, only used in 16-bits Windows
    const auto next = lexer.peek();
    if (next.has_value() && next->type == Token::Keyword && next->toKeyword() == Keywords::IGNORE_16BITS)
        lexer.next();
}

static void readResource(Lexer &lexer, Data &data, const std::optional<Token> &token, const std::optional<Token> &id)
{
    skipResourceAttributes(lexer);

    const auto keyword = token->toKeyword();
    switch (keyword) {
    case Keywords::LANGUAGE:
    case Keywords::FONT:
    case Keywords::HTML:
    case Keywords::MESSAGETABLE:
        lexer.skipLine();
        break;
    case Keywords::AFX_DIALOG_LAYOUT:
    case Keywords::DESIGNINFO:
    case Keywords::TEXTINCLUDE:
    case Keywords::RCDATA:
    case Keywords::VERSIONINFO:
        lexer.skipScope();
        break;

    case Keywords::ACCELERATORS:
        readAccelerators(lexer, data, id->toString());
        break;
    case Keywords::BITMAP:
    case Keywords::CURSOR:
    case Keywords::ICON:
    case Keywords::PNG:
        readAsset(lexer, data, keyword, id->toString());
        break;
    case Keywords::DIALOG:
    case Keywords::DIALOGEX:
        readDialog(lexer, data, id->toString());
        break;
    case Keywords::DLGINIT:
        readDialogInit(lexer, data, id->toString());
        break;
    case Keywords::MENU:
    case Keywords::MENUEX:
        readMenu(lexer, data, id->toString());
        break;
    case Keywords::STRINGTABLE:
        readStringTable(lexer, data);
        break;
    case Keywords::TOOLBAR:
        readToolBar(lexer, data, id->toString());
        break;

    case Keywords::BEGIN:
        logger()->warn("{}({}): file parser unhandled token {}",
                       QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                       token->prettyPrint().toStdString());
        lexer.skipScope();
        break;
    default:
        logger()->warn("{}({}): file parser unhandled token {}",
                       QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                       token->prettyPrint().toStdString());
        lexer.skipLine();
        break;
    }
}

static void readDirective(Lexer &lexer, Data &data, const QString &directive)
{
    // Only the include directive is handled here
    if (directive == "include") {
        Data::Include include;
        include.line = lexer.line();

        const QString fileName = lexer.next()->toString().replace('\\', '/');
        if (const auto fullPath = computeFilePath(data.fileName, fileName)) {
            include.fileName = fullPath.value();
            include.exist = true;
            if (fullPath.value().endsWith(".h")) {
                QHash<int, QString> resourceMap = loadResourceFile(include.fileName);
                if (resourceMap.isEmpty()) {
                    logger()->warn("{}({}): file parser can't load resource file {}",
                                   QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                                   include.fileName.toStdString());
                } else {
                    data.resourceMap.insert(resourceMap);
                }
            }
        } else {
            include.fileName = fileName;
            include.exist = false;
        }

        data.includes.push_back(include);
    }
    lexer.skipLine();
}

//=============================================================================
// RcFileUtils::parse
//=============================================================================
Data parse(const QString &fileName)
{
    QElapsedTimer time;
    time.start();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    Data data;
    data.fileName = fileName;

    Lexer lexer(Stream {&file});
    lexer.setFileName(fileName);
    data.content = lexer.content();

    try {
        std::optional<Token> previousToken;
        while (lexer.peek()) {
            const auto token = lexer.next();
            switch (token->type) {
            case Token::Operator_Comma:
            case Token::Operator_Or:
            case Token::String:
                logger()->critical("{}({}): file parser error on token {}",
                                   QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                                   token->prettyPrint().toStdString());
                break;
            case Token::Integer:
            case Token::Word:
                // We can only read one integer/id/word
                if (previousToken) {
                    logger()->critical("{}({}): file parser error on token {}",
                                       QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line(),
                                       token->prettyPrint().toStdString());
                }
                previousToken = token;
                break;
            case Token::Directive:
                readDirective(lexer, data, token->toString());
                break;
            case Token::Keyword: {
                readResource(lexer, data, token, previousToken);
                previousToken.reset();
                break;
            }
            }
        }
    } catch (...) {
        logger()->critical("{}({}): file parser general error",
                           QDir::toNativeSeparators(lexer.fileName()).toStdString(), lexer.line());
        return {};
    }
    logger()->info("Rc file parsing done in: {} ms", static_cast<int>(time.elapsed()));
    data.isValid = true;
    return data;
}

} // namespace RcCore
