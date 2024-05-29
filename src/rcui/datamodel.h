/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>

namespace RcCore {
struct RcFile;
}

namespace RcUi {

class DataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        TypeRole = Qt::UserRole,
        IndexRole,
        EmptyRole,
    };

public:
    explicit DataModel(const RcCore::RcFile &rcFile, QString language, QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setLanguage(const QString &language);

private:
    const RcCore::RcFile &m_rcFile;
    QString m_language;
};

} // namespace RcUi
