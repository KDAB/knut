#include "parser.h"

#include "lexer.h"
#include "logging.h"
#include "stream.h"

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QKeySequence>
#include <QTime>

namespace Parser {

//=============================================================================
// Utility methods
//=============================================================================
static std::optional<QString> computeFilePath(const QString &absolutePath, const QString &path)
{
    QFileInfo fi(path);
    if (!fi.isAbsolute()) {
        fi.setFile(absolutePath);
        fi.setFile(fi.absolutePath() + QLatin1Char('/') + path);
    }
    if (fi.exists())
        return fi.absoluteFilePath();
    return {};
}

static QHash<int, QString> loadResourceFile(const QString &resourceFile)
{
    QFile file(resourceFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(PARSER) << "Can't load resource file:" << resourceFile;
        return {};
    }

    QTextStream stream(&file);
    QHash<int, QString> resourceMap;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (!line.startsWith(QStringLiteral("#define")))
            continue;

        QStringList fields = line.split(QLatin1Char(' '), QString::SkipEmptyParts);
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
            const QString item = (hasNot ? QStringLiteral("!") : QString()) + token->toString();
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
    while (lexer.peek()->toKeyword() != Keywords::BEGIN) {
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
            dialog.styles += readStyles(lexer);
            break;
        default:
            qCCritical(PARSER) << "Parser error line:" << lexer.line()
                               << "- token: " << token->prettyPrint();
            break;
        }
    }
}

static QString toShortcut(QString event, bool isAscii, Qt::KeyboardModifiers modifiers)
{
    static const QHash<QString, Qt::Key> keyMap = {
        {QStringLiteral("VK_CANCEL"), Qt::Key_Cancel},
        {QStringLiteral("VK_BACK"), Qt::Key_Backspace},
        {QStringLiteral("VK_TAB"), Qt::Key_Tab},
        {QStringLiteral("VK_CLEAR"), Qt::Key_Clear},
        {QStringLiteral("VK_RETURN"), Qt::Key_Return},
        {QStringLiteral("VK_SHIFT"), Qt::Key_Shift},
        {QStringLiteral("VK_CONTROL"), Qt::Key_Control},
        {QStringLiteral("VK_MENU"), Qt::Key_Alt},
        {QStringLiteral("VK_PAUSE"), Qt::Key_Pause},
        {QStringLiteral("VK_CAPITAL"), Qt::Key_CapsLock},
        {QStringLiteral("VK_ESCAPE"), Qt::Key_Escape},
        {QStringLiteral("VK_MODECHANGE"), Qt::Key_Mode_switch},
        {QStringLiteral("VK_SPACE"), Qt::Key_Space},
        {QStringLiteral("VK_PRIOR"), Qt::Key_PageUp},
        {QStringLiteral("VK_NEXT"), Qt::Key_PageDown},
        {QStringLiteral("VK_END"), Qt::Key_End},
        {QStringLiteral("VK_HOME"), Qt::Key_Home},
        {QStringLiteral("VK_LEFT"), Qt::Key_Left},
        {QStringLiteral("VK_UP"), Qt::Key_Up},
        {QStringLiteral("VK_RIGHT"), Qt::Key_Right},
        {QStringLiteral("VK_DOWN"), Qt::Key_Down},
        {QStringLiteral("VK_SELECT"), Qt::Key_Select},
        {QStringLiteral("VK_PRINT"), Qt::Key_Printer},
        {QStringLiteral("VK_EXECUTE"), Qt::Key_Execute},
        {QStringLiteral("VK_SNAPSHOT"), Qt::Key_Print},
        {QStringLiteral("VK_INSERT"), Qt::Key_Insert},
        {QStringLiteral("VK_DELETE"), Qt::Key_Delete},
        {QStringLiteral("VK_HELP"), Qt::Key_Help},
        {QStringLiteral("VK_0"), Qt::Key_0},
        {QStringLiteral("VK_1"), Qt::Key_1},
        {QStringLiteral("VK_2"), Qt::Key_2},
        {QStringLiteral("VK_3"), Qt::Key_3},
        {QStringLiteral("VK_4"), Qt::Key_4},
        {QStringLiteral("VK_5"), Qt::Key_5},
        {QStringLiteral("VK_6"), Qt::Key_6},
        {QStringLiteral("VK_7"), Qt::Key_7},
        {QStringLiteral("VK_8"), Qt::Key_8},
        {QStringLiteral("VK_9"), Qt::Key_9},
        {QStringLiteral("VK_A"), Qt::Key_A},
        {QStringLiteral("VK_B"), Qt::Key_B},
        {QStringLiteral("VK_C"), Qt::Key_C},
        {QStringLiteral("VK_D"), Qt::Key_D},
        {QStringLiteral("VK_E"), Qt::Key_E},
        {QStringLiteral("VK_F"), Qt::Key_F},
        {QStringLiteral("VK_G"), Qt::Key_G},
        {QStringLiteral("VK_H"), Qt::Key_H},
        {QStringLiteral("VK_I"), Qt::Key_I},
        {QStringLiteral("VK_J"), Qt::Key_J},
        {QStringLiteral("VK_K"), Qt::Key_K},
        {QStringLiteral("VK_L"), Qt::Key_L},
        {QStringLiteral("VK_M"), Qt::Key_M},
        {QStringLiteral("VK_N"), Qt::Key_N},
        {QStringLiteral("VK_O"), Qt::Key_O},
        {QStringLiteral("VK_P"), Qt::Key_P},
        {QStringLiteral("VK_Q"), Qt::Key_Q},
        {QStringLiteral("VK_R"), Qt::Key_R},
        {QStringLiteral("VK_S"), Qt::Key_S},
        {QStringLiteral("VK_T"), Qt::Key_T},
        {QStringLiteral("VK_U"), Qt::Key_U},
        {QStringLiteral("VK_V"), Qt::Key_V},
        {QStringLiteral("VK_W"), Qt::Key_W},
        {QStringLiteral("VK_X"), Qt::Key_X},
        {QStringLiteral("VK_Y"), Qt::Key_Y},
        {QStringLiteral("VK_Z"), Qt::Key_Z},
        {QStringLiteral("VK_LWIN"), Qt::Key_Meta},
        {QStringLiteral("VK_RWIN"), Qt::Key_Meta},
        {QStringLiteral("VK_APPS"), Qt::Key_Menu},
        {QStringLiteral("VK_SLEEP"), Qt::Key_Sleep},
        {QStringLiteral("VK_NUMPAD0"), Qt::Key_0},
        {QStringLiteral("VK_NUMPAD1"), Qt::Key_1},
        {QStringLiteral("VK_NUMPAD2"), Qt::Key_2},
        {QStringLiteral("VK_NUMPAD3"), Qt::Key_3},
        {QStringLiteral("VK_NUMPAD4"), Qt::Key_4},
        {QStringLiteral("VK_NUMPAD5"), Qt::Key_5},
        {QStringLiteral("VK_NUMPAD6"), Qt::Key_6},
        {QStringLiteral("VK_NUMPAD7"), Qt::Key_7},
        {QStringLiteral("VK_NUMPAD8"), Qt::Key_8},
        {QStringLiteral("VK_NUMPAD9"), Qt::Key_9},
        {QStringLiteral("VK_MULTIPLY"), Qt::Key_Asterisk},
        {QStringLiteral("VK_ADD"), Qt::Key_Plus},
        {QStringLiteral("VK_SUBTRACT"), Qt::Key_Minus},
        {QStringLiteral("VK_DIVIDE"), Qt::Key_Slash},
        {QStringLiteral("VK_F1"), Qt::Key_F1},
        {QStringLiteral("VK_F2"), Qt::Key_F2},
        {QStringLiteral("VK_F3"), Qt::Key_F3},
        {QStringLiteral("VK_F4"), Qt::Key_F4},
        {QStringLiteral("VK_F5"), Qt::Key_F5},
        {QStringLiteral("VK_F6"), Qt::Key_F6},
        {QStringLiteral("VK_F7"), Qt::Key_F7},
        {QStringLiteral("VK_F8"), Qt::Key_F8},
        {QStringLiteral("VK_F9"), Qt::Key_F9},
        {QStringLiteral("VK_F10"), Qt::Key_F10},
        {QStringLiteral("VK_F11"), Qt::Key_F11},
        {QStringLiteral("VK_F12"), Qt::Key_F12},
        {QStringLiteral("VK_F13"), Qt::Key_F13},
        {QStringLiteral("VK_F14"), Qt::Key_F14},
        {QStringLiteral("VK_F15"), Qt::Key_F15},
        {QStringLiteral("VK_F16"), Qt::Key_F16},
        {QStringLiteral("VK_F17"), Qt::Key_F17},
        {QStringLiteral("VK_F18"), Qt::Key_F18},
        {QStringLiteral("VK_F19"), Qt::Key_F19},
        {QStringLiteral("VK_F20"), Qt::Key_F20},
        {QStringLiteral("VK_F21"), Qt::Key_F21},
        {QStringLiteral("VK_F22"), Qt::Key_F22},
        {QStringLiteral("VK_F23"), Qt::Key_F23},
        {QStringLiteral("VK_F24"), Qt::Key_F24},
        {QStringLiteral("VK_NUMLOCK"), Qt::Key_NumLock},
        {QStringLiteral("VK_SCROLL"), Qt::Key_ScrollLock},
        {QStringLiteral("VK_LSHIFT"), Qt::Key_Shift},
        {QStringLiteral("VK_RSHIFT"), Qt::Key_Shift},
        {QStringLiteral("VK_LCONTROL"), Qt::Key_Control},
        {QStringLiteral("VK_RCONTROL"), Qt::Key_Control},
        {QStringLiteral("VK_LMENU"), Qt::Key_Alt},
        {QStringLiteral("VK_RMENU"), Qt::Key_Alt},
        {QStringLiteral("VK_BROWSER_BACK"), Qt::Key_Back},
        {QStringLiteral("VK_BROWSER_FORWARD"), Qt::Key_Forward},
        {QStringLiteral("VK_BROWSER_REFRESH"), Qt::Key_Refresh},
        {QStringLiteral("VK_BROWSER_STOP"), Qt::Key_Stop},
        {QStringLiteral("VK_BROWSER_SEARCH"), Qt::Key_Search},
        {QStringLiteral("VK_BROWSER_FAVORITES"), Qt::Key_Favorites},
        {QStringLiteral("VK_BROWSER_HOME"), Qt::Key_HomePage},
        {QStringLiteral("VK_VOLUME_MUTE"), Qt::Key_VolumeMute},
        {QStringLiteral("VK_VOLUME_DOWN"), Qt::Key_VolumeDown},
        {QStringLiteral("VK_VOLUME_UP"), Qt::Key_VolumeUp},
        {QStringLiteral("VK_MEDIA_NEXT_TRACK"), Qt::Key_MediaNext},
        {QStringLiteral("VK_MEDIA_PREV_TRACK"), Qt::Key_MediaPrevious},
        {QStringLiteral("VK_MEDIA_STOP"), Qt::Key_MediaStop},
        {QStringLiteral("VK_MEDIA_PLAY_PAUSE"), Qt::Key_MediaPlay},
        {QStringLiteral("VK_LAUNCH_MAIL"), Qt::Key_LaunchMail},
        {QStringLiteral("VK_LAUNCH_MEDIA_SELECT"), Qt::Key_LaunchMedia},
        {QStringLiteral("VK_LAUNCH_APP1"), Qt::Key_Launch0},
        {QStringLiteral("VK_LAUNCH_APP2"), Qt::Key_Launch1},
        {QStringLiteral("VK_PLAY"), Qt::Key_Play},
        {QStringLiteral("VK_ZOOM"), Qt::Key_Zoom},
        {QStringLiteral("VK_OEM_CLEAR"), Qt::Key_Clear},
        {QStringLiteral("VK_OEM_1"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_PLUS"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_COMMA"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_MINUS"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_PERIOD"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_2"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_3"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_4"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_5"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_6"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_7"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_8"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_AX"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_102"), Qt::Key_unknown},
        {QStringLiteral("VK_ICO_HELP"), Qt::Key_unknown},
        {QStringLiteral("VK_ICO_00"), Qt::Key_unknown},
        {QStringLiteral("VK_PROCESSKEY"), Qt::Key_unknown},
        {QStringLiteral("VK_ICO_CLEAR"), Qt::Key_unknown},
        {QStringLiteral("VK_PACKET"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_RESET"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_JUMP"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_PA1"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_PA2"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_PA3"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_WSCTRL"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_CUSEL"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_ATTN"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_FINISH"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_COPY"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_AUTO"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_ENLW"), Qt::Key_unknown},
        {QStringLiteral("VK_OEM_BACKTAB"), Qt::Key_unknown},
        {QStringLiteral("VK_ATTN"), Qt::Key_unknown},
        {QStringLiteral("VK_CRSEL"), Qt::Key_unknown},
        {QStringLiteral("VK_EXSEL"), Qt::Key_unknown},
        {QStringLiteral("VK_EREOF"), Qt::Key_unknown},
        {QStringLiteral("VK_NONAME"), Qt::Key_unknown},
        {QStringLiteral("VK_PA1"), Qt::Key_unknown},
    };

    int key;
    if (keyMap.contains(event)) {
        key = keyMap.value(event);
        if (key == Qt::Key_unknown) {
            // We keep a text of the shortcut to follow on that in the code
            if (modifiers & Qt::AltModifier)
                event.prepend(QStringLiteral("Alt+"));
            if (modifiers & Qt::ControlModifier)
                event.prepend(QStringLiteral("Ctrl+"));
            if (modifiers & Qt::ShiftModifier)
                event.prepend(QStringLiteral("Shift+"));
            return event;
        }
    } else {
        if (event.startsWith(QLatin1Char('^')) && event.size() > 1) {
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

    Data::Asset asset;
    asset.line = lexer.line();
    asset.id = id;

    const auto fileName = lexer.next()->toString();
    if (const auto fullPath = computeFilePath(data.fileName, fileName)) {
        asset.fileName = fullPath.value();
        asset.exist = true;
    } else {
        asset.fileName = fileName;
        asset.exist = false;
    }
    switch (keyword) {
    case Keywords::ICON:
        data.icons[id] = asset;
        break;
    case Keywords::BITMAP:
    case Keywords::CURSOR:
    case Keywords::PNG:
        data.assets[id] = asset;
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
    if (accelerator.shortcut.isEmpty())
        qCWarning(PARSER) << "Unknown accelerator:" << lexer.line() << "- event: " << event;
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
    table.line = lexer.line();
    table.id = id;
    lexer.skipToBegin();

    while (lexer.peek()->type != Token::Keyword)
        table.accelerators.push_back(readAccelerator(lexer));
    lexer.next(); // END

    data.acceleratorTables.push_back(table);
    qCInfo(PARSER) << table.line << "- Accelerators:" << id;
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
            if (bytes.startsWith(QLatin1String("0x")))
                text += QString(bytes.at(4)) + bytes.at(5) + bytes.at(2) + bytes.at(3);
            else
                text += QLatin1String("00");
            lexer.skipComma();
            size -= 2;
        }
        dialogData.values[controlId].push_back(
            QString::fromLocal8Bit(QByteArray::fromHex(text.toLocal8Bit())));
    }
    lexer.next(); // END

    data.dialogData.push_back(dialogData);
    qCInfo(PARSER) << dialogData.line << "- Combobox data for dialog:" << id;
}

static Data::MenuItem readMenuItem(Lexer &lexer, Data &data)
{
    //    MENUITEM text, result, [optionlist]
    //    MENUITEM SEPARATOR

    Data::MenuItem item;
    item.line = lexer.line();

    const auto token = lexer.next();
    if (token->type == Token::String) {
        const auto split = token->toString().split(QLatin1Char('\t'));
        item.text = split.first();
        if (split.size() == 2)
            item.shortcut = split.last();
        lexer.skipComma();
        item.id = toId(lexer.next(), data);
        while (lexer.peek()->type == Token::Operator_Comma) {
            lexer.skipComma();
            const auto flagToken = lexer.next();
            switch (flagToken->toKeyword()) {
            case Keywords::CHECKED:
                item.flags |= Data::MenuItem::Checked;
                break;
            case Keywords::GRAYED:
                item.flags |= Data::MenuItem::Grayed;
                break;
            case Keywords::HELP:
                item.flags |= Data::MenuItem::Help;
                break;
            case Keywords::INACTIVE:
                item.flags |= Data::MenuItem::Inactive;
                break;
            case Keywords::MENUBARBREAK:
                item.flags |= Data::MenuItem::MenuBarBreak;
                break;
            case Keywords::MENUBREAK:
                item.flags |= Data::MenuItem::MenuBreak;
                break;
            default:
                qCWarning(PARSER) << "Parser unhandled:" << lexer.line()
                                  << "- token: " << token->prettyPrint();
            }
        }
    }
    return item;
}

static Data::MenuItem readMenuPopup(Lexer &lexer, Data &data, bool readTitle = true)
{
    //    POPUP text, [optionlist]
    //    BEGIN
    //        item-definitions ...
    //    END

    Data::MenuItem popup;
    popup.line = lexer.line();
    if (readTitle)
        popup.text = lexer.next()->toString();
    lexer.skipToBegin();

    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        switch (token->toKeyword()) {
        case Keywords::POPUP:
            popup.children.append(readMenuPopup(lexer, data));
            break;
        case Keywords::MENUITEM:
            popup.children.append(readMenuItem(lexer, data));
            break;
        default:
            qCWarning(PARSER) << "Parser unhandled:" << lexer.line()
                              << "- token: " << token->prettyPrint();
        }
        token = lexer.next();
    }
    return popup;
}

static void readMenu(Lexer &lexer, Data &data, const QString &id)
{
    //    menuID MENU
    //    [optional-statements]
    //    BEGIN
    //        item-definitions ...
    //    END

    auto menu = readMenuPopup(lexer, data, false);
    menu.id = id;
    data.menus.push_back(menu);
    qCInfo(PARSER) << menu.line << "- Menu:" << id;
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

    Data::ToolBar toolBar;
    toolBar.line = lexer.line();
    toolBar.id = id;
    toolBar.width = lexer.next()->toInt();
    lexer.skipComma();
    toolBar.height = lexer.next()->toInt();

    lexer.skipToBegin();
    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        switch (token->toKeyword()) {
        case Keywords::BUTTON: {
            Data::ToolBarItem buttonNode;
            buttonNode.line = lexer.line();
            buttonNode.id = toId(lexer.next(), data);
            toolBar.children.push_back(buttonNode);
            break;
        }
        case Keywords::SEPARATOR: {
            Data::ToolBarItem separatorNode;
            separatorNode.line = lexer.line();
            toolBar.children.push_back(separatorNode);
            break;
        }
        default:
            qCWarning(PARSER) << "Parser unhandled:" << lexer.line()
                              << "- token: " << token->prettyPrint();
        }
        lexer.skipLine();
        token = lexer.next();
    }
    data.toolBars.push_back(toolBar);
    qCInfo(PARSER) << toolBar.line << "- ToolBar:" << id;
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
    control.type = token->toString();

    if (!knownControls.contains(controlType)) {
        qCWarning(PARSER) << "Control unknown:" << lexer.line()
                          << "- token:" << token->prettyPrint();
        return control;
    }

    // Only a limited number of controls have a text
    static QVector<Keywords> textControl = {
        Keywords::AUTO3STATE, Keywords::AUTOCHECKBOX, Keywords::AUTORADIOBUTTON,
        Keywords::CHECKBOX,   Keywords::CTEXT,        Keywords::DEFPUSHBUTTON,
        Keywords::GROUPBOX,   Keywords::ICON,         Keywords::LTEXT,
        Keywords::PUSHBOX,    Keywords::PUSHBUTTON,   Keywords::RADIOBUTTON,
        Keywords::RTEXT,      Keywords::STATE3,       Keywords::CONTROL};

    if (textControl.contains(controlType)) {
        if (controlType == Keywords::ICON)
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
    qCInfo(PARSER) << dialog.line << "- Dialog:" << id;
}

static void readKeyWord(Lexer &lexer, Data &data, const std::optional<Token> &token,
                        const std::optional<Token> &id)
{
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
        qCWarning(PARSER) << "Parser unhandled:" << lexer.line() << "- token: BEGIN";
        lexer.skipScope();
        break;
    default:
        qCWarning(PARSER) << "Parser unhandled:" << lexer.line()
                          << "- token: " << token->prettyPrint();
        lexer.skipLine();
        break;
    }
}

static void readDirective(Lexer &lexer, Data &data, const QString &directive)
{
    // Only the include directive is handled here
    if (directive == QLatin1String("include")) {
        Data::Include include;
        include.line = lexer.line();

        const QString fileName = lexer.next()->toString();
        if (const auto fullPath = computeFilePath(data.fileName, fileName)) {
            include.fileName = fullPath.value();
            include.exist = true;
            if (fullPath.value().endsWith(QLatin1String(".h")))
                data.resourceMap.unite(loadResourceFile(include.fileName));
        } else {
            include.fileName = fileName;
            include.exist = false;
        }

        data.includes.push_back(include);
        qCInfo(PARSER) << lexer.line() << "- Include:" << fileName;
    }
    lexer.skipLine();
}

//=============================================================================
// Parser::parse
//=============================================================================
Data parse(const QString &fileName)
{
    QTime time;
    time.start();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    Data data;
    data.fileName = fileName;

    Lexer lexer(Stream {&file});
    data.content = lexer.content();

    try {
        std::optional<Token> previousToken;
        while (lexer.peek()) {
            const auto token = lexer.next();
            switch (token->type) {
            case Token::Operator_Comma:
            case Token::Operator_Or:
            case Token::String:
                qCCritical(PARSER)
                    << "Parser error line:" << lexer.line() << "- token: " << token->prettyPrint();
                break;
            case Token::Integer:
            case Token::Word:
                // We can only read one integer/id/word
                if (previousToken)
                    qCCritical(PARSER) << "Parser error line:" << lexer.line()
                                       << "- token: " << token->prettyPrint();
                previousToken = token;
                break;
            case Token::Directive:
                readDirective(lexer, data, token->toString());
                break;
            case Token::Keyword: {
                readKeyWord(lexer, data, token, previousToken);
                previousToken.reset();
                break;
            }
            }
        }
    } catch (...) {
        qCWarning(PARSER) << "Parser error line:" << lexer.line();
        return {};
    }
    qCInfo(PARSER) << QStringLiteral("Parsing done in: %1 ms").arg(time.elapsed());
    data.isValid = true;
    return data;
}

}
