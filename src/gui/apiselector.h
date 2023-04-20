#pragma once

#include "core/document.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

namespace Gui {

namespace Ui {
    class APIExecutorWidget;
}

struct ApiInfo
{
    QString name;
    QVector<QPair<QString, QString>> args; // vectors of pair of type and name
    Core::Document::Type type;
};

struct ArgumentWidget
{
    QLabel *label;
    QWidget *widget;
};

class APIExecutorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit APIExecutorWidget(QWidget *parent = nullptr);
    ~APIExecutorWidget();

    void showAPIExecutorWidget();

protected:
    void showEvent(QShowEvent *event) override;

public slots:
    void onExecuteButtonClicked();
    void populateArgumentList();
    void populateApiList();

private:
    void addApi(const QString &name, const QVector<QPair<QString, QString>> &args, Core::Document::Type type);
    void executeAPI(Core::Document *document, const QMetaMethod &api, const QList<QGenericArgument> &genericArgs);
    QString getSignature(const QString &apiName, const QVector<QPair<QString, QString>> &args);
    void createArgumentField(QHBoxLayout *parent, const QString &name, int typeId);

    std::unique_ptr<Ui::APIExecutorWidget> ui;
    QVector<ArgumentWidget *> m_argumentFields;
    QMap<QString, ApiInfo> m_apis;
};

}
