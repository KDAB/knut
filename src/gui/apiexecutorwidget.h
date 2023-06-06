#pragma once

#include "core/document.h"

#include <QWidget>

class QLabel;

namespace Gui {

namespace Ui {
    class APIExecutorWidget;
}

class APIExecutorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit APIExecutorWidget(QWidget *parent = nullptr);
    ~APIExecutorWidget();

    void open();

    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void onExecuteButtonClicked();

private:
    struct ApiInfo
    {
        Core::Document::Type type;
        QMetaMethod method;
    };

    struct ArgumentField
    {
        QLabel *label = nullptr;
        QWidget *widget = nullptr;
    };

    void initializeApi();
    void addApi(ApiInfo &&apiInfo);

    void populateApiList();
    void populateArgumentList();

    void executeAPI(Core::Document *document, const QByteArray &name, const QList<QGenericArgument> &genericArgs);
    void createArgumentField(const QByteArray &name, const QMetaType &type);

    std::unique_ptr<Ui::APIExecutorWidget> ui;
    QVector<ArgumentField> m_argumentFields;
    QMap<QString, ApiInfo> m_apis;
};

} // namespace Gui
