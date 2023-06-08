#pragma once

#include <QPointer>
#include <QWidget>
namespace Core {
class QmlDocument;
}

namespace Gui {

class QmlView : public QWidget
{
    Q_OBJECT
public:
    explicit QmlView(QWidget *parent = nullptr);
    ~QmlView() override;

    void setQmlDocument(Core::QmlDocument *document);

private:
    void runQml();

    QPointer<Core::QmlDocument> m_qmlDocument;
};

} // namespace Gui
