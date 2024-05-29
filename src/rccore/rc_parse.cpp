#include "lexer.h"
#include "rcfile.h"
#include "stream.h"
#include "utils/log.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QKeySequence>
#include <QTextStream>
#include <kdalgorithms.h>

namespace RcCore {

struct Context
{
    RcFile &rcFile;
    Lexer &lexer;
    QString currentLanguage = {};

    std::string fileName() const { return lexer.fileName().toStdString(); }
    int line() const { return lexer.line(); }

    Data &currentData()
    {
        Q_ASSERT(rcFile.data.contains(currentLanguage));
        return rcFile.data[currentLanguage];
    }
    void setCurrentData(const QString &language)
    {
        rcFile.data[language].language = language;
        rcFile.data[language].fileName = rcFile.fileName;
        currentLanguage = language;
    }
};

#define LEXER_FROM_CONTEXT Lexer &lexer = context.lexer

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

static QString toId(const std::optional<Token> &token, const Context &context)
{
    if (token->type == Token::Integer) {
        const int value = token->toInt();
        if (value == 0)
            return {};
        return context.rcFile.resourceMap.value(value, QString::number(value));
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

static void readDialogStatements(Context &context, Data::Dialog &dialog)
{
    LEXER_FROM_CONTEXT;

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
            dialog.menu = toId(lexer.next(), context);
            break;
        case Keywords::EXSTYLE:
        case Keywords::STYLE:
            dialog.styles = readStyles(lexer);
            break;
        default:
            spdlog::error("{}({}): parser error on token {}", context.fileName(), context.line(), token->prettyPrint());
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

    const QKeySequence seq(key | modifiers);
    return seq.toString();
}

//=============================================================================
// Resource parser
//=============================================================================
static void readLanguage(Context &context)
{
    // LANGUAGE LANG, SUBLANG
    LEXER_FROM_CONTEXT;

    // All the strings and codes for the languages and sublanguages are defined in winnt.h
    // Just include the file to see them all.

    static QHash<int, QString> langNames = {
        {0x00, "LANG_NEUTRAL"},
        {0x7f, "LANG_INVARIANT"},
        {0x36, "LANG_AFRIKAANS"},
        {0x1c, "LANG_ALBANIAN"},
        {0x84, "LANG_ALSATIAN"},
        {0x5e, "LANG_AMHARIC"},
        {0x01, "LANG_ARABIC"},
        {0x2b, "LANG_ARMENIAN"},
        {0x4d, "LANG_ASSAMESE"},
        {0x2c, "LANG_AZERI"}, // for Azerbaijani, "LANG_AZERBAIJANI is preferred
        {0x2c, "LANG_AZERBAIJANI"},
        {0x45, "LANG_BANGLA"},
        {0x6d, "LANG_BASHKIR"},
        {0x2d, "LANG_BASQUE"},
        {0x23, "LANG_BELARUSIAN"},
        {0x45, "LANG_BENGALI"}, // Some prefer to use LANG_BANGLA
        {0x7e, "LANG_BRETON"},
        {0x1a, "LANG_BOSNIAN"}, // Use with SUBLANG_BOSNIAN_* Sublanguage IDs
        {0x781a, "LANG_BOSNIAN_NEUTRAL"}, // Use with the ConvertDefaultLocale function
        {0x02, "LANG_BULGARIAN"},
        {0x03, "LANG_CATALAN"},
        {0x92, "LANG_CENTRAL_KURDISH"},
        {0x5c, "LANG_CHEROKEE"},
        {0x04, "LANG_CHINESE"}, // Use with SUBLANG_CHINESE_* Sublanguage IDs
        {0x04, "LANG_CHINESE_SIMPLIFIED"}, // Use with the ConvertDefaultLocale function
        {0x7c04, "LANG_CHINESE_TRADITIONAL"}, // Use with the ConvertDefaultLocale function
        {0x83, "LANG_CORSICAN"},
        {0x1a, "LANG_CROATIAN"},
        {0x05, "LANG_CZECH"},
        {0x06, "LANG_DANISH"},
        {0x8c, "LANG_DARI"},
        {0x65, "LANG_DIVEHI"},
        {0x13, "LANG_DUTCH"},
        {0x09, "LANG_ENGLISH"},
        {0x25, "LANG_ESTONIAN"},
        {0x38, "LANG_FAEROESE"},
        {0x29, "LANG_FARSI"}, // Deprecated: use LANG_PERSIAN instead
        {0x64, "LANG_FILIPINO"},
        {0x0b, "LANG_FINNISH"},
        {0x0c, "LANG_FRENCH"},
        {0x62, "LANG_FRISIAN"},
        {0x67, "LANG_FULAH"},
        {0x56, "LANG_GALICIAN"},
        {0x37, "LANG_GEORGIAN"},
        {0x07, "LANG_GERMAN"},
        {0x08, "LANG_GREEK"},
        {0x6f, "LANG_GREENLANDIC"},
        {0x47, "LANG_GUJARATI"},
        {0x68, "LANG_HAUSA"},
        {0x75, "LANG_HAWAIIAN"},
        {0x0d, "LANG_HEBREW"},
        {0x39, "LANG_HINDI"},
        {0x0e, "LANG_HUNGARIAN"},
        {0x0f, "LANG_ICELANDIC"},
        {0x70, "LANG_IGBO"},
        {0x21, "LANG_INDONESIAN"},
        {0x5d, "LANG_INUKTITUT"},
        {0x3c, "LANG_IRISH"}, // Use with the SUBLANG_IRISH_IRELAND Sublanguage ID
        {0x10, "LANG_ITALIAN"},
        {0x11, "LANG_JAPANESE"},
        {0x4b, "LANG_KANNADA"},
        {0x60, "LANG_KASHMIRI"},
        {0x3f, "LANG_KAZAK"},
        {0x53, "LANG_KHMER"},
        {0x86, "LANG_KICHE"},
        {0x87, "LANG_KINYARWANDA"},
        {0x57, "LANG_KONKANI"},
        {0x12, "LANG_KOREAN"},
        {0x40, "LANG_KYRGYZ"},
        {0x54, "LANG_LAO"},
        {0x26, "LANG_LATVIAN"},
        {0x27, "LANG_LITHUANIAN"},
        {0x2e, "LANG_LOWER_SORBIAN"},
        {0x6e, "LANG_LUXEMBOURGISH"},
        {0x2f, "LANG_MACEDONIAN"}, // the Former Yugoslav Republic of Macedonia
        {0x3e, "LANG_MALAY"},
        {0x4c, "LANG_MALAYALAM"},
        {0x3a, "LANG_MALTESE"},
        {0x58, "LANG_MANIPURI"},
        {0x81, "LANG_MAORI"},
        {0x7a, "LANG_MAPUDUNGUN"},
        {0x4e, "LANG_MARATHI"},
        {0x7c, "LANG_MOHAWK"},
        {0x50, "LANG_MONGOLIAN"},
        {0x61, "LANG_NEPALI"},
        {0x14, "LANG_NORWEGIAN"},
        {0x82, "LANG_OCCITAN"},
        {0x48, "LANG_ODIA"},
        {0x48, "LANG_ORIYA"}, // Deprecated: use LANG_ODIA, instead.
        {0x63, "LANG_PASHTO"},
        {0x29, "LANG_PERSIAN"},
        {0x15, "LANG_POLISH"},
        {0x16, "LANG_PORTUGUESE"},
        {0x67, "LANG_PULAR"}, // Deprecated: use LANG_FULAH instead
        {0x46, "LANG_PUNJABI"},
        {0x6b, "LANG_QUECHUA"},
        {0x18, "LANG_ROMANIAN"},
        {0x17, "LANG_ROMANSH"},
        {0x19, "LANG_RUSSIAN"},
        {0x85, "LANG_SAKHA"},
        {0x3b, "LANG_SAMI"},
        {0x4f, "LANG_SANSKRIT"},
        {0x91, "LANG_SCOTTISH_GAELIC"},
        {0x1a, "LANG_SERBIAN"}, // Use with the SUBLANG_SERBIAN_* Sublanguage IDs
        {0x7c1a, "LANG_SERBIAN_NEUTRAL"}, // Use with the ConvertDefaultLocale function
        {0x59, "LANG_SINDHI"},
        {0x5b, "LANG_SINHALESE"},
        {0x1b, "LANG_SLOVAK"},
        {0x24, "LANG_SLOVENIAN"},
        {0x6c, "LANG_SOTHO"},
        {0x0a, "LANG_SPANISH"},
        {0x41, "LANG_SWAHILI"},
        {0x1d, "LANG_SWEDISH"},
        {0x5a, "LANG_SYRIAC"},
        {0x28, "LANG_TAJIK"},
        {0x5f, "LANG_TAMAZIGHT"},
        {0x49, "LANG_TAMIL"},
        {0x44, "LANG_TATAR"},
        {0x4a, "LANG_TELUGU"},
        {0x1e, "LANG_THAI"},
        {0x51, "LANG_TIBETAN"},
        {0x73, "LANG_TIGRIGNA"},
        {0x73, "LANG_TIGRINYA"}, // Preferred spelling in locale
        {0x32, "LANG_TSWANA"},
        {0x1f, "LANG_TURKISH"},
        {0x42, "LANG_TURKMEN"},
        {0x80, "LANG_UIGHUR"},
        {0x22, "LANG_UKRAINIAN"},
        {0x2e, "LANG_UPPER_SORBIAN"},
        {0x20, "LANG_URDU"},
        {0x43, "LANG_UZBEK"},
        {0x03, "LANG_VALENCIAN"},
        {0x2a, "LANG_VIETNAMESE"},
        {0x52, "LANG_WELSH"},
        {0x88, "LANG_WOLOF"},
        {0x34, "LANG_XHOSA"},
        {0x85, "LANG_YAKUT"}, // Deprecated: use LANG_SAKHA,instead
        {0x78, "LANG_YI"},
        {0x6a, "LANG_YORUBA"},
        {0x35, "LANG_ZULU"},
    };

    static QMultiHash<int, QString> sublangNames = {
        {0x00, "SUBLANG_NEUTRAL"}, // language neutral
        {0x01, "SUBLANG_DEFAULT"}, // user default
        {0x02, "SUBLANG_SYS_DEFAULT"}, // system default
        {0x03, "SUBLANG_CUSTOM_DEFAULT"}, // default custom language/locale
        {0x04, "SUBLANG_CUSTOM_UNSPECIFIED"}, // custom language/locale
        {0x05, "SUBLANG_UI_CUSTOM_DEFAULT"}, // Default custom MUI language/locale
        {0x01, "SUBLANG_AFRIKAANS_SOUTH_AFRICA"}, // Afrikaans (South Africa) 0x0436 af-ZA
        {0x01, "SUBLANG_ALBANIAN_ALBANIA"}, // Albanian (Albania) 0x041c sq-AL
        {0x01, "SUBLANG_ALSATIAN_FRANCE"}, // Alsatian (France) 0x0484
        {0x01, "SUBLANG_AMHARIC_ETHIOPIA"}, // Amharic (Ethiopia) 0x045e
        {0x01, "SUBLANG_ARABIC_SAUDI_ARABIA"}, // Arabic (Saudi Arabia)
        {0x02, "SUBLANG_ARABIC_IRAQ"}, // Arabic (Iraq)
        {0x03, "SUBLANG_ARABIC_EGYPT"}, // Arabic (Egypt)
        {0x04, "SUBLANG_ARABIC_LIBYA"}, // Arabic (Libya)
        {0x05, "SUBLANG_ARABIC_ALGERIA"}, // Arabic (Algeria)
        {0x06, "SUBLANG_ARABIC_MOROCCO"}, // Arabic (Morocco)
        {0x07, "SUBLANG_ARABIC_TUNISIA"}, // Arabic (Tunisia)
        {0x08, "SUBLANG_ARABIC_OMAN"}, // Arabic (Oman)
        {0x09, "SUBLANG_ARABIC_YEMEN"}, // Arabic (Yemen)
        {0x0a, "SUBLANG_ARABIC_SYRIA"}, // Arabic (Syria)
        {0x0b, "SUBLANG_ARABIC_JORDAN"}, // Arabic (Jordan)
        {0x0c, "SUBLANG_ARABIC_LEBANON"}, // Arabic (Lebanon)
        {0x0d, "SUBLANG_ARABIC_KUWAIT"}, // Arabic (Kuwait)
        {0x0e, "SUBLANG_ARABIC_UAE"}, // Arabic (U.A.E)
        {0x0f, "SUBLANG_ARABIC_BAHRAIN"}, // Arabic (Bahrain)
        {0x10, "SUBLANG_ARABIC_QATAR"}, // Arabic (Qatar)
        {0x01, "SUBLANG_ARMENIAN_ARMENIA"}, // Armenian (Armenia) 0x042b hy-AM
        {0x01, "SUBLANG_ASSAMESE_INDIA"}, // Assamese (India) 0x044d
        {0x01,
         "SUBLANG_AZERI_LATIN"}, // Azeri (Latin) - for Azerbaijani, "SUBLANG_AZERBAIJANI_AZERBAIJAN_LATIN preferred
        {0x02, "SUBLANG_AZERI_CYRILLIC"}, // Azeri (Cyrillic) - for Azerbaijani,
                                          // "SUBLANG_AZERBAIJANI_AZERBAIJAN_CYRILLIC preferred
        {0x01, "SUBLANG_AZERBAIJANI_AZERBAIJAN_LATIN"}, // Azerbaijani (Azerbaijan, Latin)
        {0x02, "SUBLANG_AZERBAIJANI_AZERBAIJAN_CYRILLIC"}, // Azerbaijani (Azerbaijan, Cyrillic)
        {0x01, "SUBLANG_BANGLA_INDIA"}, // Bangla (India)
        {0x02, "SUBLANG_BANGLA_BANGLADESH"}, // Bangla (Bangladesh)
        {0x01, "SUBLANG_BASHKIR_RUSSIA"}, // Bashkir (Russia) 0x046d ba-RU
        {0x01, "SUBLANG_BASQUE_BASQUE"}, // Basque (Basque) 0x042d eu-ES
        {0x01, "SUBLANG_BELARUSIAN_BELARUS"}, // Belarusian (Belarus) 0x0423 be-BY
        {0x01, "SUBLANG_BENGALI_INDIA"}, // Bengali (India) - Note some prefer SUBLANG_BANGLA_INDIA
        {0x02, "SUBLANG_BENGALI_BANGLADESH"}, // Bengali (Bangladesh) - Note some prefer SUBLANG_BANGLA_BANGLADESH
        {0x05,
         "SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_LATIN"}, // Bosnian (Bosnia and Herzegovina - Latin) 0x141a bs-BA-Latn
        {0x08, "SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_CYRILLIC"}, // Bosnian (Bosnia and Herzegovina - Cyrillic) 0x201a
                                                               // bs-BA-Cyrl
        {0x01, "SUBLANG_BRETON_FRANCE"}, // Breton (France) 0x047e
        {0x01, "SUBLANG_BULGARIAN_BULGARIA"}, // Bulgarian (Bulgaria) 0x0402
        {0x01, "SUBLANG_CATALAN_CATALAN"}, // Catalan (Catalan) 0x0403
        {0x01, "SUBLANG_CENTRAL_KURDISH_IRAQ"}, // Central Kurdish (Iraq) 0x0492 ku-Arab-IQ
        {0x01, "SUBLANG_CHEROKEE_CHEROKEE"}, // Cherokee (Cherokee) 0x045c chr-Cher-US
        {0x01, "SUBLANG_CHINESE_TRADITIONAL"}, // Chinese (Taiwan) 0x0404 zh-TW
        {0x02, "SUBLANG_CHINESE_SIMPLIFIED"}, // Chinese (PR China) 0x0804 zh-CN
        {0x03, "SUBLANG_CHINESE_HONGKONG"}, // Chinese (Hong Kong S.A.R., P.R.C.) 0x0c04 zh-HK
        {0x04, "SUBLANG_CHINESE_SINGAPORE"}, // Chinese (Singapore) 0x1004 zh-SG
        {0x05, "SUBLANG_CHINESE_MACAU"}, // Chinese (Macau S.A.R.) 0x1404 zh-MO
        {0x01, "SUBLANG_CORSICAN_FRANCE"}, // Corsican (France) 0x0483
        {0x01, "SUBLANG_CZECH_CZECH_REPUBLIC"}, // Czech (Czech Republic) 0x0405
        {0x01, "SUBLANG_CROATIAN_CROATIA"}, // Croatian (Croatia)
        {0x04, "SUBLANG_CROATIAN_BOSNIA_HERZEGOVINA_LATIN"}, // Croatian (Bosnia and Herzegovina - Latin) 0x101a hr-BA
        {0x01, "SUBLANG_DANISH_DENMARK"}, // Danish (Denmark) 0x0406
        {0x01, "SUBLANG_DARI_AFGHANISTAN"}, // Dari (Afghanistan)
        {0x01, "SUBLANG_DIVEHI_MALDIVES"}, // Divehi (Maldives) 0x0465 div-MV
        {0x01, "SUBLANG_DUTCH"}, // Dutch
        {0x02, "SUBLANG_DUTCH_BELGIAN"}, // Dutch (Belgian)
        {0x01, "SUBLANG_ENGLISH_US"}, // English (USA)
        {0x02, "SUBLANG_ENGLISH_UK"}, // English (UK)
        {0x03, "SUBLANG_ENGLISH_AUS"}, // English (Australian)
        {0x04, "SUBLANG_ENGLISH_CAN"}, // English (Canadian)
        {0x05, "SUBLANG_ENGLISH_NZ"}, // English (New Zealand)
        {0x06, "SUBLANG_ENGLISH_EIRE"}, // English (Irish)
        {0x07, "SUBLANG_ENGLISH_SOUTH_AFRICA"}, // English (South Africa)
        {0x08, "SUBLANG_ENGLISH_JAMAICA"}, // English (Jamaica)
        {0x09, "SUBLANG_ENGLISH_CARIBBEAN"}, // English (Caribbean)
        {0x0a, "SUBLANG_ENGLISH_BELIZE"}, // English (Belize)
        {0x0b, "SUBLANG_ENGLISH_TRINIDAD"}, // English (Trinidad)
        {0x0c, "SUBLANG_ENGLISH_ZIMBABWE"}, // English (Zimbabwe)
        {0x0d, "SUBLANG_ENGLISH_PHILIPPINES"}, // English (Philippines)
        {0x10, "SUBLANG_ENGLISH_INDIA"}, // English (India)
        {0x11, "SUBLANG_ENGLISH_MALAYSIA"}, // English (Malaysia)
        {0x12, "SUBLANG_ENGLISH_SINGAPORE"}, // English (Singapore)
        {0x01, "SUBLANG_ESTONIAN_ESTONIA"}, // Estonian (Estonia) 0x0425 et-EE
        {0x01, "SUBLANG_FAEROESE_FAROE_ISLANDS"}, // Faroese (Faroe Islands) 0x0438 fo-FO
        {0x01, "SUBLANG_FILIPINO_PHILIPPINES"}, // Filipino (Philippines) 0x0464 fil-PH
        {0x01, "SUBLANG_FINNISH_FINLAND"}, // Finnish (Finland) 0x040b
        {0x01, "SUBLANG_FRENCH"}, // French
        {0x02, "SUBLANG_FRENCH_BELGIAN"}, // French (Belgian)
        {0x03, "SUBLANG_FRENCH_CANADIAN"}, // French (Canadian)
        {0x04, "SUBLANG_FRENCH_SWISS"}, // French (Swiss)
        {0x05, "SUBLANG_FRENCH_LUXEMBOURG"}, // French (Luxembourg)
        {0x06, "SUBLANG_FRENCH_MONACO"}, // French (Monaco)
        {0x01, "SUBLANG_FRISIAN_NETHERLANDS"}, // Frisian (Netherlands) 0x0462 fy-NL
        {0x02, "SUBLANG_FULAH_SENEGAL"}, // Fulah (Senegal) 0x0867 ff-Latn-SN
        {0x01, "SUBLANG_GALICIAN_GALICIAN"}, // Galician (Galician) 0x0456 gl-ES
        {0x01, "SUBLANG_GEORGIAN_GEORGIA"}, // Georgian (Georgia) 0x0437 ka-GE
        {0x01, "SUBLANG_GERMAN"}, // German
        {0x02, "SUBLANG_GERMAN_SWISS"}, // German (Swiss)
        {0x03, "SUBLANG_GERMAN_AUSTRIAN"}, // German (Austrian)
        {0x04, "SUBLANG_GERMAN_LUXEMBOURG"}, // German (Luxembourg)
        {0x05, "SUBLANG_GERMAN_LIECHTENSTEIN"}, // German (Liechtenstein)
        {0x01, "SUBLANG_GREEK_GREECE"}, // Greek (Greece)
        {0x01, "SUBLANG_GREENLANDIC_GREENLAND"}, // Greenlandic (Greenland) 0x046f kl-GL
        {0x01, "SUBLANG_GUJARATI_INDIA"}, // Gujarati (India (Gujarati Script)) 0x0447 gu-IN
        {0x01, "SUBLANG_HAUSA_NIGERIA_LATIN"}, // Hausa (Latin, Nigeria) 0x0468 ha-NG-Latn
        {0x01, "SUBLANG_HAWAIIAN_US"}, // Hawiian (US) 0x0475 haw-US
        {0x01, "SUBLANG_HEBREW_ISRAEL"}, // Hebrew (Israel) 0x040d
        {0x01, "SUBLANG_HINDI_INDIA"}, // Hindi (India) 0x0439 hi-IN
        {0x01, "SUBLANG_HUNGARIAN_HUNGARY"}, // Hungarian (Hungary) 0x040e
        {0x01, "SUBLANG_ICELANDIC_ICELAND"}, // Icelandic (Iceland) 0x040f
        {0x01, "SUBLANG_IGBO_NIGERIA"}, // Igbo (Nigeria) 0x0470 ig-NG
        {0x01, "SUBLANG_INDONESIAN_INDONESIA"}, // Indonesian (Indonesia) 0x0421 id-ID
        {0x01, "SUBLANG_INUKTITUT_CANADA"}, // Inuktitut (Syllabics) (Canada) 0x045d iu-CA-Cans
        {0x02, "SUBLANG_INUKTITUT_CANADA_LATIN"}, // Inuktitut (Canada - Latin)
        {0x02, "SUBLANG_IRISH_IRELAND"}, // Irish (Ireland)
        {0x01, "SUBLANG_ITALIAN"}, // Italian
        {0x02, "SUBLANG_ITALIAN_SWISS"}, // Italian (Swiss)
        {0x01, "SUBLANG_JAPANESE_JAPAN"}, // Japanese (Japan) 0x0411
        {0x01, "SUBLANG_KANNADA_INDIA"}, // Kannada (India (Kannada Script)) 0x044b kn-IN
        {0x02, "SUBLANG_KASHMIRI_SASIA"}, // Kashmiri (South Asia)
        {0x02, "SUBLANG_KASHMIRI_INDIA"}, // For app compatibility only
        {0x01, "SUBLANG_KAZAK_KAZAKHSTAN"}, // Kazakh (Kazakhstan) 0x043f kk-KZ
        {0x01, "SUBLANG_KHMER_CAMBODIA"}, // Khmer (Cambodia) 0x0453 kh-KH
        {0x01, "SUBLANG_KICHE_GUATEMALA"}, // K'iche (Guatemala)
        {0x01, "SUBLANG_KINYARWANDA_RWANDA"}, // Kinyarwanda (Rwanda) 0x0487 rw-RW
        {0x01, "SUBLANG_KONKANI_INDIA"}, // Konkani (India) 0x0457 kok-IN
        {0x01, "SUBLANG_KOREAN"}, // Korean (Extended Wansung)
        {0x01, "SUBLANG_KYRGYZ_KYRGYZSTAN"}, // Kyrgyz (Kyrgyzstan) 0x0440 ky-KG
        {0x01, "SUBLANG_LAO_LAO"}, // Lao (Lao PDR) 0x0454 lo-LA
        {0x01, "SUBLANG_LATVIAN_LATVIA"}, // Latvian (Latvia) 0x0426 lv-LV
        {0x01, "SUBLANG_LITHUANIAN"}, // Lithuanian
        {0x02, "SUBLANG_LOWER_SORBIAN_GERMANY"}, // Lower Sorbian (Germany) 0x082e wee-DE
        {0x01, "SUBLANG_LUXEMBOURGISH_LUXEMBOURG"}, // Luxembourgish (Luxembourg) 0x046e lb-LU
        {0x01, "SUBLANG_MACEDONIAN_MACEDONIA"}, // Macedonian (Macedonia (FYROM)) 0x042f mk-MK
        {0x01, "SUBLANG_MALAY_MALAYSIA"}, // Malay (Malaysia)
        {0x02, "SUBLANG_MALAY_BRUNEI_DARUSSALAM"}, // Malay (Brunei Darussalam)
        {0x01, "SUBLANG_MALAYALAM_INDIA"}, // Malayalam (India (Malayalam Script) ) 0x044c ml-IN
        {0x01, "SUBLANG_MALTESE_MALTA"}, // Maltese (Malta) 0x043a mt-MT
        {0x01, "SUBLANG_MAORI_NEW_ZEALAND"}, // Maori (New Zealand) 0x0481 mi-NZ
        {0x01, "SUBLANG_MAPUDUNGUN_CHILE"}, // Mapudungun (Chile) 0x047a arn-CL
        {0x01, "SUBLANG_MARATHI_INDIA"}, // Marathi (India) 0x044e mr-IN
        {0x01, "SUBLANG_MOHAWK_MOHAWK"}, // Mohawk (Mohawk) 0x047c moh-CA
        {0x01, "SUBLANG_MONGOLIAN_CYRILLIC_MONGOLIA"}, // Mongolian (Cyrillic, Mongolia)
        {0x02, "SUBLANG_MONGOLIAN_PRC"}, // Mongolian (PRC)
        {0x02, "SUBLANG_NEPALI_INDIA"}, // Nepali (India)
        {0x01, "SUBLANG_NEPALI_NEPAL"}, // Nepali (Nepal) 0x0461 ne-NP
        {0x01, "SUBLANG_NORWEGIAN_BOKMAL"}, // Norwegian (Bokmal)
        {0x02, "SUBLANG_NORWEGIAN_NYNORSK"}, // Norwegian (Nynorsk)
        {0x01, "SUBLANG_OCCITAN_FRANCE"}, // Occitan (France) 0x0482 oc-FR
        {0x01, "SUBLANG_ODIA_INDIA"}, // Odia (India (Odia Script)) 0x0448 or-IN
        {0x01, "SUBLANG_ORIYA_INDIA"}, // Deprecated: use SUBLANG_ODIA_INDIA instead
        {0x01, "SUBLANG_PASHTO_AFGHANISTAN"}, // Pashto (Afghanistan)
        {0x01, "SUBLANG_PERSIAN_IRAN"}, // Persian (Iran) 0x0429 fa-IR
        {0x01, "SUBLANG_POLISH_POLAND"}, // Polish (Poland) 0x0415
        {0x02, "SUBLANG_PORTUGUESE"}, // Portuguese
        {0x01, "SUBLANG_PORTUGUESE_BRAZILIAN"}, // Portuguese (Brazil)
        {0x02, "SUBLANG_PULAR_SENEGAL"}, // Deprecated: Use SUBLANG_FULAH_SENEGAL instead
        {0x01, "SUBLANG_PUNJABI_INDIA"}, // Punjabi (India (Gurmukhi Script)) 0x0446 pa-IN
        {0x02, "SUBLANG_PUNJABI_PAKISTAN"}, // Punjabi (Pakistan (Arabic Script)) 0x0846 pa-Arab-PK
        {0x01, "SUBLANG_QUECHUA_BOLIVIA"}, // Quechua (Bolivia)
        {0x02, "SUBLANG_QUECHUA_ECUADOR"}, // Quechua (Ecuador)
        {0x03, "SUBLANG_QUECHUA_PERU"}, // Quechua (Peru)
        {0x01, "SUBLANG_ROMANIAN_ROMANIA"}, // Romanian (Romania) 0x0418
        {0x01, "SUBLANG_ROMANSH_SWITZERLAND"}, // Romansh (Switzerland) 0x0417 rm-CH
        {0x01, "SUBLANG_RUSSIAN_RUSSIA"}, // Russian (Russia) 0x0419
        {0x01, "SUBLANG_SAKHA_RUSSIA"}, // Sakha (Russia) 0x0485 sah-RU
        {0x01, "SUBLANG_SAMI_NORTHERN_NORWAY"}, // Northern Sami (Norway)
        {0x02, "SUBLANG_SAMI_NORTHERN_SWEDEN"}, // Northern Sami (Sweden)
        {0x03, "SUBLANG_SAMI_NORTHERN_FINLAND"}, // Northern Sami (Finland)
        {0x04, "SUBLANG_SAMI_LULE_NORWAY"}, // Lule Sami (Norway)
        {0x05, "SUBLANG_SAMI_LULE_SWEDEN"}, // Lule Sami (Sweden)
        {0x06, "SUBLANG_SAMI_SOUTHERN_NORWAY"}, // Southern Sami (Norway)
        {0x07, "SUBLANG_SAMI_SOUTHERN_SWEDEN"}, // Southern Sami (Sweden)
        {0x08, "SUBLANG_SAMI_SKOLT_FINLAND"}, // Skolt Sami (Finland)
        {0x09, "SUBLANG_SAMI_INARI_FINLAND"}, // Inari Sami (Finland)
        {0x01, "SUBLANG_SANSKRIT_INDIA"}, // Sanskrit (India) 0x044f sa-IN
        {0x01, "SUBLANG_SCOTTISH_GAELIC"}, // Scottish Gaelic (United Kingdom) 0x0491 gd-GB
        {0x06, "SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN"}, // Serbian (Bosnia and Herzegovina - Latin)
        {0x07, "SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC"}, // Serbian (Bosnia and Herzegovina - Cyrillic)
        {0x0b, "SUBLANG_SERBIAN_MONTENEGRO_LATIN"}, // Serbian (Montenegro - Latn)
        {0x0c, "SUBLANG_SERBIAN_MONTENEGRO_CYRILLIC"}, // Serbian (Montenegro - Cyrillic)
        {0x09, "SUBLANG_SERBIAN_SERBIA_LATIN"}, // Serbian (Serbia - Latin)
        {0x0a, "SUBLANG_SERBIAN_SERBIA_CYRILLIC"}, // Serbian (Serbia - Cyrillic)
        {0x01, "SUBLANG_SERBIAN_CROATIA"}, // Croatian (Croatia) 0x041a hr-HR
        {0x02, "SUBLANG_SERBIAN_LATIN"}, // Serbian (Latin)
        {0x03, "SUBLANG_SERBIAN_CYRILLIC"}, // Serbian (Cyrillic)
        {0x01, "SUBLANG_SINDHI_INDIA"}, // Sindhi (India) reserved 0x0459
        {0x02, "SUBLANG_SINDHI_PAKISTAN"}, // Sindhi (Pakistan) 0x0859 sd-Arab-PK
        {0x02, "SUBLANG_SINDHI_AFGHANISTAN"}, // For app compatibility only
        {0x01, "SUBLANG_SINHALESE_SRI_LANKA"}, // Sinhalese (Sri Lanka)
        {0x01, "SUBLANG_SOTHO_NORTHERN_SOUTH_AFRICA"}, // Northern Sotho (South Africa)
        {0x01, "SUBLANG_SLOVAK_SLOVAKIA"}, // Slovak (Slovakia) 0x041b sk-SK
        {0x01, "SUBLANG_SLOVENIAN_SLOVENIA"}, // Slovenian (Slovenia) 0x0424 sl-SI
        {0x01, "SUBLANG_SPANISH"}, // Spanish (Castilian)
        {0x02, "SUBLANG_SPANISH_MEXICAN"}, // Spanish (Mexico)
        {0x03, "SUBLANG_SPANISH_MODERN"}, // Spanish (Modern)
        {0x04, "SUBLANG_SPANISH_GUATEMALA"}, // Spanish (Guatemala)
        {0x05, "SUBLANG_SPANISH_COSTA_RICA"}, // Spanish (Costa Rica)
        {0x06, "SUBLANG_SPANISH_PANAMA"}, // Spanish (Panama)
        {0x07, "SUBLANG_SPANISH_DOMINICAN_REPUBLIC"}, // Spanish (Dominican Republic)
        {0x08, "SUBLANG_SPANISH_VENEZUELA"}, // Spanish (Venezuela)
        {0x09, "SUBLANG_SPANISH_COLOMBIA"}, // Spanish (Colombia)
        {0x0a, "SUBLANG_SPANISH_PERU"}, // Spanish (Peru)
        {0x0b, "SUBLANG_SPANISH_ARGENTINA"}, // Spanish (Argentina)
        {0x0c, "SUBLANG_SPANISH_ECUADOR"}, // Spanish (Ecuador)
        {0x0d, "SUBLANG_SPANISH_CHILE"}, // Spanish (Chile)
        {0x0e, "SUBLANG_SPANISH_URUGUAY"}, // Spanish (Uruguay)
        {0x0f, "SUBLANG_SPANISH_PARAGUAY"}, // Spanish (Paraguay)
        {0x10, "SUBLANG_SPANISH_BOLIVIA"}, // Spanish (Bolivia)
        {0x11, "SUBLANG_SPANISH_EL_SALVADOR"}, // Spanish (El Salvador)
        {0x12, "SUBLANG_SPANISH_HONDURAS"}, // Spanish (Honduras)
        {0x13, "SUBLANG_SPANISH_NICARAGUA"}, // Spanish (Nicaragua)
        {0x14, "SUBLANG_SPANISH_PUERTO_RICO"}, // Spanish (Puerto Rico)
        {0x15, "SUBLANG_SPANISH_US"}, // Spanish (United States)
        {0x01, "SUBLANG_SWAHILI_KENYA"}, // Swahili (Kenya) 0x0441 sw-KE
        {0x01, "SUBLANG_SWEDISH"}, // Swedish
        {0x02, "SUBLANG_SWEDISH_FINLAND"}, // Swedish (Finland)
        {0x01, "SUBLANG_SYRIAC_SYRIA"}, // Syriac (Syria) 0x045a syr-SY
        {0x01, "SUBLANG_TAJIK_TAJIKISTAN"}, // Tajik (Tajikistan) 0x0428 tg-TJ-Cyrl
        {0x02, "SUBLANG_TAMAZIGHT_ALGERIA_LATIN"}, // Tamazight (Latin, Algeria) 0x085f tzm-Latn-DZ
        {0x04, "SUBLANG_TAMAZIGHT_MOROCCO_TIFINAGH"}, // Tamazight (Tifinagh) 0x105f tzm-Tfng-MA
        {0x01, "SUBLANG_TAMIL_INDIA"}, // Tamil (India)
        {0x02, "SUBLANG_TAMIL_SRI_LANKA"}, // Tamil (Sri Lanka) 0x0849 ta-LK
        {0x01, "SUBLANG_TATAR_RUSSIA"}, // Tatar (Russia) 0x0444 tt-RU
        {0x01, "SUBLANG_TELUGU_INDIA"}, // Telugu (India (Telugu Script)) 0x044a te-IN
        {0x01, "SUBLANG_THAI_THAILAND"}, // Thai (Thailand) 0x041e th-TH
        {0x01, "SUBLANG_TIBETAN_PRC"}, // Tibetan (PRC)
        {0x02, "SUBLANG_TIGRIGNA_ERITREA"}, // Tigrigna (Eritrea)
        {0x02, "SUBLANG_TIGRINYA_ERITREA"}, // Tigrinya (Eritrea) 0x0873 ti-ER (preferred spelling)
        {0x01, "SUBLANG_TIGRINYA_ETHIOPIA"}, // Tigrinya (Ethiopia) 0x0473 ti-ET
        {0x02, "SUBLANG_TSWANA_BOTSWANA"}, // Setswana / Tswana (Botswana) 0x0832 tn-BW
        {0x01, "SUBLANG_TSWANA_SOUTH_AFRICA"}, // Setswana / Tswana (South Africa) 0x0432 tn-ZA
        {0x01, "SUBLANG_TURKISH_TURKEY"}, // Turkish (Turkey) 0x041f tr-TR
        {0x01, "SUBLANG_TURKMEN_TURKMENISTAN"}, // Turkmen (Turkmenistan) 0x0442 tk-TM
        {0x01, "SUBLANG_UIGHUR_PRC"}, // Uighur (PRC) 0x0480 ug-CN
        {0x01, "SUBLANG_UKRAINIAN_UKRAINE"}, // Ukrainian (Ukraine) 0x0422 uk-UA
        {0x01, "SUBLANG_UPPER_SORBIAN_GERMANY"}, // Upper Sorbian (Germany) 0x042e wen-DE
        {0x01, "SUBLANG_URDU_PAKISTAN"}, // Urdu (Pakistan)
        {0x02, "SUBLANG_URDU_INDIA"}, // Urdu (India)
        {0x01, "SUBLANG_UZBEK_LATIN"}, // Uzbek (Latin)
        {0x02, "SUBLANG_UZBEK_CYRILLIC"}, // Uzbek (Cyrillic)
        {0x02, "SUBLANG_VALENCIAN_VALENCIA"}, // Valencian (Valencia) 0x0803 ca-ES-Valencia
        {0x01, "SUBLANG_VIETNAMESE_VIETNAM"}, // Vietnamese (Vietnam) 0x042a vi-VN
        {0x01, "SUBLANG_WELSH_UNITED_KINGDOM"}, // Welsh (United Kingdom) 0x0452 cy-GB
        {0x01, "SUBLANG_WOLOF_SENEGAL"}, // Wolof (Senegal)
        {0x01, "SUBLANG_XHOSA_SOUTH_AFRICA"}, // isiXhosa / Xhosa (South Africa) 0x0434 xh-ZA
        {0x01, "SUBLANG_YAKUT_RUSSIA"}, // Deprecated: use SUBLANG_SAKHA_RUSSIA instead
        {0x01, "SUBLANG_YI_PRC"}, // Yi (PRC)) 0x0478
        {0x01, "SUBLANG_YORUBA_NIGERIA"}, // Yoruba (Nigeria) 046a yo-NG
        {0x01, "SUBLANG_ZULU_SOUTH_AFRICA"}, // isiZulu / Zulu (South Africa) 0x0435 zu-ZA
    };

    auto langToken = lexer.next();
    const QString lang =
        langToken->type == Token::Word ? langToken->toString() : langNames.value(langToken->toInt(), "UNKNOWN");

    lexer.skipComma();

    auto sublangToken = lexer.next();
    QString sublang;
    if (sublangToken->type == Token::Word) {
        sublang = sublangToken->toString();
    } else {
        auto values = sublangNames.values(sublangToken->toInt());
        auto sameLang = [langName = lang.sliced(5)](const QString &sub) {
            return sub.contains(langName);
        };
        kdalgorithms::filter(values, sameLang);
        sublang = values.isEmpty() ? "UNKNOWN" : values.first();
    }

    const QString language = lang + ";" + sublang;
    context.setCurrentData(language);
}

static void readAsset(Context &context, Keywords keyword, const QString &id)
{
    //    nameID KEYWORD filename
    LEXER_FROM_CONTEXT;

    Asset asset;
    asset.line = lexer.line();
    asset.id = id;

    const auto fileName = lexer.next()->toString().replace("\\\\", "/");
    if (const auto fullPath = computeFilePath(lexer.fileName(), fileName)) {
        asset.fileName = fullPath.value();
        asset.exist = true;
    } else {
        asset.fileName = fileName;
        asset.exist = false;
    }
    switch (keyword) {
    case Keywords::ICON:
        context.currentData().icons.push_back(asset);
        break;
    case Keywords::BITMAP:
    case Keywords::CURSOR:
    case Keywords::IMAGE:
    case Keywords::PNG:
        context.currentData().assets.push_back(asset);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

static void readStringTable(Context &context)
{
    //    STRINGTABLE
    //      [optional-statements]
    //    BEGIN
    //    stringID string
    //    . . .
    //    END
    LEXER_FROM_CONTEXT;

    lexer.skipToBegin();
    while (lexer.peek()->type != Token::Keyword) {
        const int line = lexer.line();
        const auto id = lexer.next()->toString();
        const auto string = lexer.next()->toString();
        context.currentData().strings[id] = {id, string, line};
    }
    lexer.next(); // END
}

static Data::Accelerator readAccelerator(Context &context)
{
    LEXER_FROM_CONTEXT;

    Data::Accelerator accelerator;
    accelerator.line = lexer.line();
    QString event;
    if (lexer.peek()->type == Token::Integer)
        event = QChar(lexer.next()->toInt());
    else
        event = lexer.next()->toString();
    lexer.skipComma();
    accelerator.id = toId(lexer.next(), context);

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
        spdlog::warn("{}({}): parser unknown accelerator {}", context.fileName(), context.line(), event);
    }
    return accelerator;
}

static void readAccelerators(Context &context, const QString &id)
{
    //    acctablename ACCELERATORS
    //    [optional-statements]
    //    BEGIN
    //        event, idvalue, [type] [options]
    //    END
    LEXER_FROM_CONTEXT;

    Data::AcceleratorTable table;
    table.line = lexer.line() - 1;
    table.id = id;
    lexer.skipToBegin();

    while (lexer.peek()->type != Token::Keyword)
        table.accelerators.push_back(readAccelerator(context));
    lexer.next(); // END

    context.currentData().acceleratorTables.push_back(table);
}

static void readDialogInit(Context &context, const QString &id)
{
    //    dialogId DLGINIT
    //      [optional-statements]
    //    BEGIN
    //    controlID, bytes, int, 0, bytes...
    //    . . .
    //    0
    //    END
    LEXER_FROM_CONTEXT;

    Data::DialogData dialogData;
    dialogData.line = lexer.line();
    dialogData.id = id;
    lexer.skipToBegin();

    while (lexer.peek()->type != Token::Keyword) {
        const auto controlId = toId(lexer.next(), context);
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
            lexer.skipComma();
            size -= 2;
        }
        if (text.endsWith("00"))
            text.chop(2);
        dialogData.values[controlId].push_back(QString::fromLocal8Bit(QByteArray::fromHex(text.toLocal8Bit())));
    }
    lexer.next(); // END

    context.currentData().dialogData.push_back(dialogData);
}

static MenuItem readMenuItem(Context &context)
{
    //    MENUITEM text, result, [optionlist]
    //    MENUITEM SEPARATOR
    LEXER_FROM_CONTEXT;

    MenuItem item;
    item.line = lexer.line();

    const auto token = lexer.next();
    if (token->type == Token::String) {
        const auto split = token->toString().split('\t');
        item.text = split.first();
        if (split.size() == 2)
            item.shortcut = split.last();
        lexer.skipComma();
        item.id = toId(lexer.next(), context);

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
            case Keywords::MFSENABLED:
            case Keywords::MFTSTRING:
            case Keywords::MFTRIGHTJUSTIFY:
                spdlog::info("{}({}): parser unused token {}", context.fileName(), context.line(),
                             flagToken->prettyPrint());
                break;
            default:
                spdlog::warn("{}({}): parser unhandled token {}", context.fileName(), context.line(),
                             flagToken->prettyPrint());
            }

            const auto type = lexer.peek()->type;
            readNext = (type == Token::Operator_Comma || type == Token::Operator_Or);
        }
    }
    return item;
}

static MenuItem readMenuPopup(Context &context);

static void readMenuChildren(Context &context, QVector<MenuItem> &children)
{
    LEXER_FROM_CONTEXT;

    lexer.skipToBegin();

    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        switch (token->toKeyword()) {
        case Keywords::POPUP:
            children.append(readMenuPopup(context));
            break;
        case Keywords::MENUITEM:
            children.append(readMenuItem(context));
            break;
        default:
            spdlog::warn("{}({}): parser unhandled token {}", context.fileName(), context.line(), token->prettyPrint());
        }
        token = lexer.next();
    }
}

static MenuItem readMenuPopup(Context &context)
{
    //    POPUP text, [optionlist]
    //    BEGIN
    //        item-definitions ...
    //    END
    LEXER_FROM_CONTEXT;

    MenuItem popup;
    popup.line = lexer.line();
    popup.text = lexer.next()->toString();

    readMenuChildren(context, popup.children);
    return popup;
}

static void readMenu(Context &context, const QString &id)
{
    //    menuID MENU
    //    [optional-statements]
    //    BEGIN
    //        item-definitions ...
    //    END
    LEXER_FROM_CONTEXT;

    Menu menu;
    menu.line = lexer.line() - 1;
    menu.id = id;

    readMenuChildren(context, menu.children);
    for (auto &child : menu.children)
        child.isTopLevel = true;
    context.currentData().menus.push_back(menu);
}

static void readToolBar(Context &context, const QString &id)
{
    //    toolbarID TOOLBAR width, height
    //    [optional-statements]
    //    BEGIN
    //        BUTTON  id
    //        SEPARATOR
    //        ...
    //    END
    LEXER_FROM_CONTEXT;

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
            buttonNode.id = toId(lexer.next(), context);
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
            spdlog::warn("{}({}): parser unhandled token {}", context.fileName(), context.line(), token->prettyPrint());
        }
        lexer.skipLine();
        token = lexer.next();
    }
    context.currentData().toolBars.push_back(toolBar);
}

static Data::Control readControl(Context &context, const std::optional<Token> &token)
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
    LEXER_FROM_CONTEXT;

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
        spdlog::warn("{}({}): parser unknown control {}", context.fileName(), context.line(), token->prettyPrint());
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
            control.text = toId(lexer.next(), context);
        else
            control.text = lexer.next()->toString();
        lexer.skipComma();
    }

    control.id = toId(lexer.next(), context);
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

static void readDialog(Context &context, const QString &id)
{
    //    nameID DIALOG x, y, width, height [, helpID]
    //    [optional-statements]
    //    BEGIN
    //        control-statements ...
    //    END
    LEXER_FROM_CONTEXT;

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

    readDialogStatements(context, dialog);

    lexer.next(); // BEGIN
    auto token = lexer.next();
    while (token->toKeyword() != Keywords::END) {
        dialog.controls.push_back(readControl(context, token));
        token = lexer.next();
    }

    context.currentData().dialogs.push_back(dialog);
}

static void skipResourceAttributes(Lexer &lexer)
{
    // Ignore any resource attributes, only used in 16-bits Windows
    const auto next = lexer.peek();
    if (next.has_value() && next->type == Token::Keyword && next->toKeyword() == Keywords::IGNORE_16BITS)
        lexer.next();
}

static void readResource(Context &context, const std::optional<Token> &token, const std::optional<Token> &id)
{
    LEXER_FROM_CONTEXT;
    skipResourceAttributes(lexer);

    const auto keyword = token->toKeyword();
    switch (keyword) {
    case Keywords::LANGUAGE:
        readLanguage(context);
        break;
    case Keywords::FONT:
    case Keywords::HTML:
    case Keywords::MESSAGETABLE:
    case Keywords::REGISTRY:
    case Keywords::RT_RIBBON_XML:
        spdlog::info("{}({}): parser unused token {}", context.fileName(), context.line(), token->prettyPrint());
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
        readAccelerators(context, id->toString());
        break;
    case Keywords::BITMAP:
    case Keywords::CURSOR:
    case Keywords::ICON:
    case Keywords::IMAGE:
    case Keywords::PNG:
        readAsset(context, keyword, id->toString());
        break;
    case Keywords::DIALOG:
    case Keywords::DIALOGEX:
        readDialog(context, id->toString());
        break;
    case Keywords::DLGINIT:
        readDialogInit(context, id->toString());
        break;
    case Keywords::MENU:
    case Keywords::MENUEX:
        readMenu(context, id->toString());
        break;
    case Keywords::STRINGTABLE:
        readStringTable(context);
        break;
    case Keywords::TOOLBAR:
        readToolBar(context, id->toString());
        break;

    case Keywords::BEGIN:
        spdlog::warn("{}({}): parser unhandled token {}", context.fileName(), context.line(), token->prettyPrint());
        lexer.skipScope();
        break;
    default:
        spdlog::warn("{}({}): parser unhandled token {}", context.fileName(), context.line(), token->prettyPrint());
        lexer.skipLine();
        break;
    }
}

static void readDirective(Context &context, const QString &directive)
{
    LEXER_FROM_CONTEXT;

    // Only the include directive is handled here
    if (directive == "include") {
        Data::Include include;
        include.line = context.line();

        const QString fileName = lexer.next()->toString().replace('\\', '/');
        if (const auto fullPath = computeFilePath(context.rcFile.fileName, fileName)) {
            include.fileName = fullPath.value();
            include.exist = true;
            if (fullPath.value().endsWith(".h")) {
                QHash<int, QString> resourceMap = loadResourceFile(include.fileName);
                if (resourceMap.isEmpty()) {
                    spdlog::warn("{}({}): parser can't load resource file {}", context.fileName(), context.line(),
                                 include.fileName);
                } else {
                    context.rcFile.resourceMap.insert(resourceMap);
                }
            }
        } else {
            include.fileName = fileName;
            include.exist = false;
        }

        context.rcFile.includes.push_back(include);
    }
    lexer.skipLine();
}

//=============================================================================
// RcFileUtils::parse
//=============================================================================
RcFile parse(const QString &fileName)
{
    QElapsedTimer time;
    time.start();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    RcFile rcFile;
    rcFile.fileName = fileName;

    Lexer lexer(Stream {&file});
    lexer.setFileName(fileName);
    rcFile.content = lexer.content();

    Context context = {.rcFile = rcFile, .lexer = lexer};

    try {
        std::optional<Token> previousToken;
        while (lexer.peek()) {
            const auto token = lexer.next();
            switch (token->type) {
            case Token::Operator_Comma:
            case Token::Operator_Or:
            case Token::String:
                spdlog::error("{}({}): parser error on token {}", context.fileName(), context.line(),
                              token->prettyPrint());
                break;
            case Token::Integer:
            case Token::Word:
                // We can only read one integer/id/word
                if (previousToken) {
                    spdlog::error("{}({}): parser error on token {}", context.fileName(), context.line(),
                                  token->prettyPrint());
                }
                previousToken = token;
                break;
            case Token::Directive:
                readDirective(context, token->toString());
                break;
            case Token::Keyword: {
                readResource(context, token, previousToken);
                previousToken.reset();
                break;
            }
            }
        }
    } catch (...) {
        spdlog::critical("{}({}): parser general error", context.fileName(), context.line());
        return {};
    }
    spdlog::trace("{} ms for parsing {}", static_cast<int>(time.elapsed()), context.fileName());
    rcFile.isValid = true;
    return rcFile;
}

} // namespace RcCore
