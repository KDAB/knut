#pragma once

#include <QString>

class QPlainTextEdit;

namespace Gui {

class GuiSettings
{
public:
    enum Style {
        DefaultStyle = 0,
        FusionLight,
        FusionDark,
    };

    GuiSettings();

    void setStyle(Style style);
    Style style() const;

    void setTheme(const QString &theme);
    QString theme() const;

    void setFontFamily(const QString &fontFamily);
    QString fontFamily() const;

    void setFontSize(int size);
    int fontSize() const;

    void setWordWrap(bool wordWrap);
    bool isWordWrap() const;

    void setupTextEdit(QPlainTextEdit *textEdit, const QString &fileName) const;

private:
    void updateStyle() const;
    void updateTheme() const;
    void updateTextEdits() const;
    void updateTextEdit(QPlainTextEdit *textEdit) const;

    Style m_style = DefaultStyle;
    QString m_theme;
    QString m_fontFamily = "Courier New";
    int m_fontSize = 10;
    bool m_wordWrap = false;
};

} // namespace Gui
