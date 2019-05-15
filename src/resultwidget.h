#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include "jsrunner.h"

#include <QTabWidget>

class ResultWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit ResultWidget(QWidget *parent = nullptr);

    void setResult(const QVector<JsResult::Document> &documents);
};

#endif // RESULTWIDGET_H
