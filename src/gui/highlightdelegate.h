/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QItemDelegate>

namespace Gui {

class HighlightDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit HighlightDelegate(QObject *parent = nullptr);

    void setHighlightedText(const QString &searchText, int options);

protected:
    QString transform(QString text, const QString &textColor, const QString &backgroundColor) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString m_highlightedText;
    int m_options;
};

} // namespace Gui
