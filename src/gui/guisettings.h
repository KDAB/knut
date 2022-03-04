#pragma once

#include <QPixmap>
#include <QPointer>
#include <QString>

#include <vector>

class QPlainTextEdit;
class QObject;

namespace Gui {

class GuiSettings
{
public:
    enum Style {
        DefaultStyle = 0,
        FusionLight,
        FusionDark,
    };

    static GuiSettings *instance();

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

    static void setupDocumentTextEdit(QPlainTextEdit *textEdit, const QString &fileName);
    static void setupTextEdit(QPlainTextEdit *textEdit);

    static void setIcon(QObject *object, const QString &asset);

private:
    GuiSettings();

    void updateStyle() const;
    void updateTheme() const;
    void updateTextEdits() const;
    void updateTextEdit(QPlainTextEdit *textEdit) const;
    void updateIcons() const;
    void updateIcon(QObject *object, const QString &asset) const;

    Style m_style = DefaultStyle;
    QString m_theme;
    QString m_fontFamily = "Courier New";
    int m_fontSize = 10;
    bool m_wordWrap = false;

    struct IconInfo
    {
        QPointer<QObject> object;
        QString asset;
    };
    std::vector<IconInfo> m_iconInfos;
};

} // namespace Gui
