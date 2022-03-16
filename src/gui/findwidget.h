#pragma once

#include <QWidget>

namespace Gui {

namespace Ui {
    class FindWidget;
}

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();

    void findNext();
    void findPrevious();

    void open();

private:
    int findFlags() const;
    QString findString();

    void find(int options);
    void replaceOne();
    void replaceAll();
    void replace(bool onlyOne);

    Ui::FindWidget *ui;
    QAction *m_matchCase = nullptr;
    QAction *m_matchWord = nullptr;
    QAction *m_matchRegexp = nullptr;
    QAction *m_preserveCase = nullptr;
    QString m_defaultString;
    bool m_isDefaultSelection = false;
    bool m_firstTime = true;
};

} // namespace Gui
