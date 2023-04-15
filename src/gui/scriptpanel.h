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

    void setNewScript(const QString &script);

    bool hasScript() const;
    void playScript();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void openScript();
    void newScript();
    void saveScript();

    QWidget *const m_toolBar = nullptr;
    QLabel *m_scriptName = nullptr;
    QString m_fileName;
};

} // namespace Gui
