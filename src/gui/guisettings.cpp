#include "guisettings.h"

#include "knutstyle.h"

#ifdef USE_SYNTAX_HIGHLIGHTING
#include <definition.h>
#include <repository.h>
#include <syntaxhighlighter.h>
#include <theme.h>
#endif

#include <QAction>
#include <QApplication>
#include <QPainter>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QToolButton>
#include <QWidget>

#include <algorithm>
#include <functional>

namespace Gui {

constexpr const char StyleKey[] = "Interface/Style";
constexpr const char ThemeKey[] = "Interface/Theme";
constexpr const char FontFamilyKey[] = "TextEditor/FontFamily";
constexpr const char FontSizeKey[] = "TextEditor/FontSize";
constexpr const char WordWrapKey[] = "TextEditor/WordWrap";
constexpr const char IsDocument[] = "isDoc";

GuiSettings::GuiSettings()
{
    QSettings settings;
    m_style = static_cast<GuiSettings::Style>(settings.value(StyleKey, m_style).toInt());
    m_theme = settings.value(ThemeKey, m_theme).toString();
    m_fontFamily = settings.value(FontFamilyKey, m_fontFamily).toString();
    m_fontSize = settings.value(FontSizeKey, m_fontSize).toInt();
    m_wordWrap = settings.value(WordWrapKey, m_wordWrap).toBool();
    updateStyle();
}

GuiSettings *GuiSettings::instance()
{
    static GuiSettings settings;
    return &settings;
}

void GuiSettings::setStyle(Style style)
{
    if (m_style != style) {
        m_style = style;
        QSettings settings;
        settings.setValue(StyleKey, static_cast<int>(m_style));
        updateStyle();
    }
}

GuiSettings::Style GuiSettings::style() const
{
    return m_style;
}

void GuiSettings::setTheme(const QString &theme)
{
    if (m_theme != theme) {
        m_theme = theme;
        QSettings settings;
        settings.setValue(ThemeKey, m_theme);
        updateTheme();
    }
}

QString GuiSettings::theme() const
{
    return m_theme;
}

void GuiSettings::setFontFamily(const QString &fontFamily)
{
    if (m_fontFamily != fontFamily) {
        m_fontFamily = fontFamily;
        QSettings settings;
        settings.setValue(FontFamilyKey, m_fontFamily);
        updateTextEdits();
    }
}

QString GuiSettings::fontFamily() const
{
    return m_fontFamily;
}

void GuiSettings::setFontSize(int size)
{
    if (m_fontSize != size) {
        m_fontSize = size;
        QSettings settings;
        settings.setValue(FontSizeKey, m_fontSize);
        updateTextEdits();
    }
}

int GuiSettings::fontSize() const
{
    return m_fontSize;
}

void GuiSettings::setWordWrap(bool wordWrap)
{
    if (m_wordWrap != wordWrap) {
        m_wordWrap = wordWrap;
        QSettings settings;
        settings.setValue(FontSizeKey, m_wordWrap);
        updateTextEdits();
    }
}

bool GuiSettings::isWordWrap() const
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

void GuiSettings::setupDocumentTextEdit(QPlainTextEdit *textEdit, const QString &fileName)
{
    textEdit->setProperty(IsDocument, true);
    instance()->updateTextEdit(textEdit);

#ifdef USE_SYNTAX_HIGHLIGHTING
    auto highlighter = new KSyntaxHighlighting::SyntaxHighlighter(textEdit->document());
    setupHighlighter(highlighter, instance()->m_theme, fileName);
#endif
}

void GuiSettings::setupTextEdit(QPlainTextEdit *textEdit)
{
    instance()->updateTextEdit(textEdit);
}

void GuiSettings::setIcon(QObject *object, const QString &asset)
{
    GuiSettings::instance()->updateIcon(object, asset);
    GuiSettings::instance()->m_iconInfos.push_back({object, asset});
}

void GuiSettings::updateStyle() const
{
    // Store the name of the default style... else there's no way to get it back
    static QString defaultStyle = qApp->style()->objectName();

    switch (m_style) {
    case GuiSettings::DefaultStyle: {
        auto style = QStyleFactory::create(defaultStyle);
        qApp->setStyle(style);
        qApp->setPalette(style->standardPalette());
        break;
    }
    case GuiSettings::FusionLight: {
        auto style = new KnutStyle("Fusion");
        qApp->setStyle(style);
        qApp->setPalette(style->standardPalette());
        break;
    }
    case GuiSettings::FusionDark: {
        auto style = new KnutStyle("Fusion");
        qApp->setStyle(style);

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
        darkPalette.setColor(QPalette::Light, gray.lighter());
        darkPalette.setColor(QPalette::Midlight, gray);
        darkPalette.setColor(QPalette::Mid, darkGray.lighter());
        darkPalette.setColor(QPalette::Dark, darkGray);
        darkPalette.setColor(QPalette::ButtonText, text);
        darkPalette.setColor(QPalette::Link, blue);
        darkPalette.setColor(QPalette::Highlight, blue);
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
        darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
        darkPalette.setColor(QPalette::Disabled, QPalette::Midlight, darkGray);
        qApp->setPalette(darkPalette);
        break;
    }
    }

    // Update the theme only if it's the default one, as it's palette based
    if (m_theme.isEmpty())
        updateTheme();
    updateIcons();
}

void GuiSettings::updateTheme() const
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

void GuiSettings::updateTextEdits() const
{
    const auto topLevels = QApplication::topLevelWidgets();
    for (auto topLevel : topLevels) {
        const auto textEdits = topLevel->findChildren<QPlainTextEdit *>();
        for (auto *textEdit : textEdits)
            updateTextEdit(textEdit);
    }
}

void GuiSettings::updateTextEdit(QPlainTextEdit *textEdit) const
{
    auto f = textEdit->font();
    f.setFamily(m_fontFamily);
    f.setPointSize(m_fontSize);
    textEdit->setFont(f);
    QFontMetrics fm(f);

    // Only for document text edits
    if (textEdit->property(IsDocument).toBool()) {
        textEdit->setTabStopDistance(4 * fm.horizontalAdvance(' '));
        textEdit->setLineWrapMode(m_wordWrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    }
}

void GuiSettings::updateIcons() const
{
    for (const auto &info : m_iconInfos) {
        if (info.object)
            updateIcon(info.object, info.asset);
    }
}

void GuiSettings::updateIcon(QObject *object, const QString &asset) const
{
    if (!object)
        return;

    QColor iconColor = qApp->palette().windowText().color();

    QPixmap pix(asset);
    if (iconColor != Qt::black) {
        QPainter p;
        p.begin(&pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(0, 0, pix.width(), pix.height(), iconColor);
        p.end();
    }

    if (auto button = qobject_cast<QAbstractButton *>(object))
        button->setIcon(pix);
    else if (auto action = qobject_cast<QAction *>(object))
        action->setIcon(pix);
}

} // namespace Gui
