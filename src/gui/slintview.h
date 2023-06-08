#pragma once

#include <QWidget>
#include <QPointer>

namespace Core {
class SlintDocument;
}

class QProcess;

namespace Gui {

class SlintView: public QWidget
{
    Q_OBJECT

public:
    explicit SlintView(QWidget *parent = nullptr);
    ~SlintView() override;

    void setSlintDocument(Core::SlintDocument *document);

private:
    void runSlint();

private:
    QPointer<Core::SlintDocument> m_document;
    QProcess *m_process = nullptr;
};

}
