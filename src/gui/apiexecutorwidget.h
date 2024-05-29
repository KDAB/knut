/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/document.h"

#include <QWidget>

class QLabel;

namespace Core {
class Document;
}

namespace Gui {

namespace Ui {
    class APIExecutorWidget;
}

class APIExecutorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit APIExecutorWidget(QWidget *parent = nullptr);
    ~APIExecutorWidget() override;

    void open();

    bool eventFilter(QObject *obj, QEvent *event) override;

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

    void populateApiList(Core::Document *document);
    void populateArgumentList();

    void executeAPI(Core::Document *document, const QByteArray &name, const QList<QGenericArgument> &genericArgs);
    void createArgumentField(const QByteArray &name, const QMetaType &type);

    std::unique_ptr<Ui::APIExecutorWidget> ui;
    QVector<ArgumentField> m_argumentFields;
    QMap<QString, ApiInfo> m_apis;
};

} // namespace Gui
