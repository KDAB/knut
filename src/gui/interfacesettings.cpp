#include "interfacesettings.h"

#ifdef USE_SYNTAX_HIGHLIGHTING
#include <definition.h>
#include <repository.h>
#include <syntaxhighlighter.h>
#include <theme.h>
#endif

#include <QApplication>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

namespace Gui {

constexpr const char StyleKey[] = "Style";
constexpr const char ThemeKey[] = "Theme";
constexpr const char FontFamilyKey[] = "FontFamily";
constexpr const char FontSizeKey[] = "FontSize";
constexpr const char WordWrapKey[] = "WordWrap";

InterfaceSettings::InterfaceSettings()
{
    QSettings settings;
    m_style = static_cast<InterfaceSettings::Style>(settings.value(StyleKey, m_style).toInt());
    m_theme = settings.value(ThemeKey, m_theme).toString();
    m_fontFamily = settings.value(FontFamilyKey, m_fontFamily).toString();
    m_fontSize = settings.value(FontSizeKey, m_fontSize).toInt();
    m_wordWrap = settings.value(WordWrapKey, m_wordWrap).toBool();
    updateStyle();
}

void InterfaceSettings::setStyle(Style style)
{
    if (m_style != style) {
        m_style = style;
        QSettings settings;
        settings.setValue(StyleKey, static_cast<int>(m_style));
        updateStyle();
    }
}

InterfaceSettings::Style InterfaceSettings::style() const
{
    return m_style;
}

void InterfaceSettings::setTheme(const QString &theme)
{
    if (m_theme != theme) {
        m_theme = theme;
        QSettings settings;
        settings.setValue(ThemeKey, m_theme);
        updateTheme();
    }
}

QString InterfaceSettings::theme() const
{
    return m_theme;
}

void InterfaceSettings::setFontFamily(const QString &fontFamily)
{
    if (m_fontFamily != fontFamily) {
        m_fontFamily = fontFamily;
        QSettings settings;
        settings.setValue(FontFamilyKey, m_fontFamily);
        updateTextEdits();
    }
}

QString InterfaceSettings::fontFamily() const
{
    return m_fontFamily;
}

void InterfaceSettings::setFontSize(int size)
{
    if (m_fontSize != size) {
        m_fontSize = size;
        QSettings settings;
        settings.setValue(FontSizeKey, m_fontSize);
        updateTextEdits();
    }
}

int InterfaceSettings::fontSize() const
{
    return m_fontSize;
}

void InterfaceSettings::setWordWrap(bool wordWrap)
{
    if (m_wordWrap != wordWrap) {
        m_wordWrap = wordWrap;
        QSettings settings;
        settings.setValue(FontSizeKey, m_wordWrap);
        updateTextEdits();
    }
}

bool InterfaceSettings::isWordWrap() const
{
    return m_wordWrap;
}

#ifdef USE_SYNTAX_HIGHLIGHTING
static void setupHighlighter(KSyntaxHighlighting::SyntaxHighlighter *highlighter, const QString &theme,
                             const QString &fileName = {})
{
    static KSyntaxHighlighting::Repository repository;
    if (theme.isEmpty())
        highlighter->setTheme(repository.themeForPalette(QApplication::palette()));
    else
        highlighter->setTheme(repository.theme(theme));
    if (!fileName.isEmpty()) {
        const auto def = repository.definitionForFileName(fileName);
        highlighter->setDefinition(def);
    }
}
#endif

void InterfaceSettings::setupTextEdit(QPlainTextEdit *textEdit, const QString &fileName) const
{
    updateTextEdit(textEdit);

#ifdef USE_SYNTAX_HIGHLIGHTING
    auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(textEdit->document());
    setupHighlighter(highlighter, m_theme, fileName);
#endif
}

void InterfaceSettings::updateStyle() const
{
    // Store the name of the default style... else there's no way to get it back
    static QString defaultStyle = qApp->style()->objectName();

    switch (m_style) {
    case InterfaceSettings::DefaultStyle: {
        auto style = QStyleFactory::create(defaultStyle);
        qApp->setStyle(style);
        qApp->setPalette(style->standardPalette());
        break;
    }
    case InterfaceSettings::FusionLight: {
        auto style = QStyleFactory::create("Fusion");
        qApp->setStyle(style);
        qApp->setPalette(style->standardPalette());
        break;
    }
    case InterfaceSettings::FusionDark: {
        qApp->setStyle(QStyleFactory::create("Fusion"));

        const QColor darkGray(64, 66, 65);
        const QColor gray(128, 128, 128);
        const QColor black(46, 47, 48);
        const QColor blue(42, 130, 218);
        const QColor text(190, 192, 193);

        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, darkGray);
        darkPalette.setColor(QPalette::WindowText, text);
        darkPalette.setColor(QPalette::Base, black);
        darkPalette.setColor(QPalette::AlternateBase, darkGray);
        darkPalette.setColor(QPalette::ToolTipBase, blue);
        darkPalette.setColor(QPalette::ToolTipText, text);
        darkPalette.setColor(QPalette::Text, text);
        darkPalette.setColor(QPalette::Button, darkGray);
        darkPalette.setColor(QPalette::ButtonText, text);
        darkPalette.setColor(QPalette::Link, blue);
        darkPalette.setColor(QPalette::Highlight, blue);
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
        darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
        qApp->setPalette(darkPalette);
        break;
    }
    }

    // Update the theme only if it's the default one, as it's palette based
    if (m_theme.isEmpty())
        updateTheme();
}

void InterfaceSettings::updateTheme() const
{
#ifdef USE_SYNTAX_HIGHLIGHTING
    const auto topLevels = QApplication::topLevelWidgets();
    for (auto topLevel : topLevels) {
        const auto highlighters = topLevel->findChildren<KSyntaxHighlighting::SyntaxHighlighter *>();
        for (auto *highlighter : highlighters) {
            setupHighlighter(highlighter, m_theme);
            highlighter->rehighlight();
        }
    }
#endif
}

void InterfaceSettings::updateTextEdits() const
{
    const auto topLevels = QApplication::topLevelWidgets();
    for (auto topLevel : topLevels) {
        const auto textEdits = topLevel->findChildren<QPlainTextEdit *>();
        for (auto *textEdit : textEdits)
            updateTextEdit(textEdit);
    }
}

void InterfaceSettings::updateTextEdit(QPlainTextEdit *textEdit) const
{
    auto f = textEdit->font();
    f.setFamily(m_fontFamily);
    f.setPointSize(m_fontSize);
    textEdit->setFont(f);
    QFontMetrics fm(f);
    textEdit->setTabStopDistance(4 * fm.horizontalAdvance(' '));
    textEdit->setLineWrapMode(m_wordWrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

} // namespace Gui
