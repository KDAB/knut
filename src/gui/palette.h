/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QAbstractItemModel>
#include <QFrame>
#include <functional>
#include <memory>
#include <vector>

class QMainWindow;
class QSortFilterProxyModel;

namespace Gui {

namespace Ui {
    class Palette;
}

class Palette : public QFrame
{
    Q_OBJECT

public:
    explicit Palette(QMainWindow *parent);
    ~Palette() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    void showPalette(const QString &prefix = {});

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void changeText(const QString &text);
    void clickItem(const QModelIndex &index);
    void setSourceModel(QAbstractItemModel *model);
    void updateListHeight();

    void addFileSelector();
    void addLineSelector();
    void addScriptSelector();
    void addSymbolSelector();
    void addActionSelector();

    struct Selector
    {
        QString prefix;
        std::unique_ptr<QAbstractItemModel> model;
        std::function<bool(const QVariant &)> selectionFunc;
        std::function<void()> resetFunc = {};

        Selector(QString prefix, std::unique_ptr<QAbstractItemModel> model,
                 std::function<bool(const QVariant &)> selectionFunc, std::function<void()> resetFunc = {})
            : prefix(std::move(prefix))
            , model(std::move(model))
            , selectionFunc(std::move(selectionFunc))
            , resetFunc(std::move(resetFunc))
        {
        }
    };

    std::unique_ptr<Ui::Palette> ui;
    std::vector<Selector> m_selectors;
    int m_currentSelector = 0;
    QSortFilterProxyModel *const m_proxyModel = nullptr;
};

} // namespace Gui
