#include "qmlview.h"

#include "core/qmldocument.h"
#include "guisettings.h"
#include <QFile>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include <QAction>
#include <QQuickView>
#include <QToolBar>
namespace Gui {

QmlView::QmlView(QWidget *parent)
    : QWidget {parent}
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QmlView::~QmlView() = default;

void QmlView::setQmlDocument(Core::QmlDocument *document)
{
    m_qmlDocument = document;
    auto layout = new QVBoxLayout(this);
    auto action = new QAction(tr("Run"));
    GuiSettings::setIcon(action, ":/gui/eye.png");
    connect(action, &QAction::triggered, this, &QmlView::runQml);

    auto bar = new QToolBar(this);
    bar->addAction(action);
    layout->addWidget(bar);

    layout->setContentsMargins({});
    auto *textEdit = m_qmlDocument->textEdit();
    layout->addWidget(textEdit);
    textEdit->setVisible(true);
    textEdit->installEventFilter(this);
    setFocusProxy(textEdit);
    GuiSettings::setupDocumentTextEdit(textEdit, m_qmlDocument->fileName());
}

void QmlView::runQml()
{
    if (!m_qmlDocument) {
        return;
    }

    QString qmlFilePath = m_qmlDocument->fileName();

    auto qmlView = new QQuickView();
    qmlView->setSource(QUrl::fromLocalFile(qmlFilePath));
    qmlView->show();
}

} // namespace Gui
