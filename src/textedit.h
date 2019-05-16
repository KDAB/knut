#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QWidget>
class QTextEdit;
class QLineEdit;
class QPushButton;
class TextEdit : public QWidget
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);
    ~TextEdit();

    QTextEdit *textEdit() const;

private:
    void slotSearchLineEditChanged(const QString &str);
    void slotSearchPreviewText();
    void slotSearchNextText();
    void slotSearchText();
    void slotCloseSearchText();

    QTextEdit *m_textEdit = nullptr;
    QLineEdit *m_searchText = nullptr;
    QPushButton *m_searchPreviewButton = nullptr;
    QPushButton *m_searchNextButton = nullptr;
    QWidget *m_searchWidget = nullptr;
};

#endif // TEXTEDIT_H
