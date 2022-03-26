#include "local_include_1.h"
#include "local_include_2.h"
#include "local_include_3.h"

#include <QClass1>
#include <QClass2>
#include <QClass3>

namespace Gui {

constexpr int  someVariable = 10;
constexpr char someCharArr[] = "MainWindow/Geometry";

static void longMethod1(With Argument)
{
    // added following new lines at the start of this method
    cppDocument->commentSelection();
    cppDocument->save();
    QVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));
    cppDocument->undo(); cppDocument->save();
    switch (document->type()) {
    case Core::Document::Type::Text: {
        auto textEdit = qobject_cast<Core::TextDocument *>(document)->textEdit();
        GuiSettings::setupDocumentTextEdit(textEdit, document->fileName());
    }
    case Core::Document::Type::Rc: {
        auto rcview = new RcUi::RcFileView();
        rcview->setRcFile(qobject_cast<Core::RcDocument *>(document)->data());
        GuiSettings::setupDocumentTextEdit(rcview->textEdit(), document->fileName());
    }
    }
    // added following new lines at the end of this method
    cppDocument->commentSelection();
    cppDocument->save();
    QVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));
    cppDocument->undo(); cppDocument->save();
}

static void longMethod2(With Argument)
{
    // added following new lines at the start of this method
    cppDocument->commentSelection();
    cppDocument->save();
    QVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));
    cppDocument->undo(); cppDocument->save();

    switch (document->type()) {
    case Core::Document::Type::Text: {
        auto textEdit = qobject_cast<Core::TextDocument *>(document)->textEdit();
        GuiSettings::setupDocumentTextEdit(textEdit, document->fileName());
    }
    case Core::Document::Type::Rc: {
        auto rcview = new RcUi::RcFileView();
        rcview->setRcFile(qobject_cast<Core::RcDocument *>(document)->data());
        GuiSettings::setupDocumentTextEdit(rcview->textEdit(), document->fileName());
    }
    }
    // added following new lines at the end of this method
    cppDocument->commentSelection();
    cppDocument->save();
    QVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));
    cppDocument->undo(); cppDocument->save();
}

static void shortMethod()
{
    // added this new line at the start of this method
    QString text = tabWidget->tabText();
    tabWidget->setTabText(text);
    // added this new line at the end of this method
}

void MainWindow::nonStaticMethod()
{
    auto project = Core::Project::instance();
    if (!project->currentDocument())
        return;
    const QString fileName = project->currentDocument()->fileName();

    // open the header/source file for C++, so LSP server can also index it
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(project->currentDocument())) {
        Core::LoggerDisabler ld(true);
        project->get(cppDocument->correspondingHeaderSource());
    }

    const QModelIndex &index = m_fileModel->index(fileName);
    m_projectView->setCurrentIndex(index);
    updateActions();
}

} // namespace Gui
