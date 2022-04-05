#pragma once

#include <QPlainTextEdit>

class QLabel;

namespace Gui {

class ScriptPanel : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ScriptPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

private:
    void openScript();
    void newScript();
    void saveScript();
    void playScript();

    QWidget *const m_toolBar = nullptr;
    QLabel *m_scriptName = nullptr;
    QString m_fileName;
};

} // namespace Gui
