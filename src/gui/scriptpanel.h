#pragma once

#include <QPlainTextEdit>

class QLabel;
class QToolButton;

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
    void newScriptDialog();
    void saveScript();
    void editDialog();
    void checkEditDialogButton();
    QString createDialogFile();

    void setupNewFile(const QString &scriptText, int cursorLeftMove);

    QWidget *const m_toolBar = nullptr;
    QLabel *m_scriptName = nullptr;
    QString m_fileName;
    QToolButton *m_editDialogButton = nullptr;
};

} // namespace Gui
