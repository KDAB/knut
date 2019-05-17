#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QWidget>

class QTextEdit;
class QLineEdit;
class QToolButton;
class QTextDocument;
class QTextCursor;

class TextEdit : public QWidget
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);

    bool isReadOnly() const;
    void setReadOnly(bool value);

    QTextDocument *document() const;
    void setDocument(QTextDocument *document);

    QTextCursor textCursor() const;
    void setTextCursor(const QTextCursor &cursor);

    QString text() const;

public Q_SLOTS:
    void clear();
    void setText(const QString &text);

private:
    void slotSearchLineEditChanged(const QString &str);
    void slotSearchPreviewText();
    void slotSearchNextText();
    void slotSearchText();
    void slotCloseSearchText();

    QTextEdit *m_textEdit = nullptr;
    QLineEdit *m_searchText = nullptr;
    QToolButton *m_searchPreviewButton = nullptr;
    QToolButton *m_searchNextButton = nullptr;
    QWidget *m_searchWidget = nullptr;
};

#endif // TEXTEDIT_H
