/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rangemark.h"

#include <QObject>

namespace treesitter {
class QueryMatch;
}

namespace Core {

class TextDocument;

class QueryCapture
{
    Q_GADGET

    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(Core::RangeMark range MEMBER range CONSTANT)

public:
    Q_INVOKABLE QString toString() const;

    QString name;
    Core::RangeMark range;
};

class QueryMatch
{
    Q_GADGET

    Q_PROPERTY(QVector<QueryCapture> captures READ captures CONSTANT FINAL)
    Q_PROPERTY(bool isEmpty READ isEmpty CONSTANT FINAL)

public:
    // Default constructor is required for Q_DECLARE_METATYPE
    QueryMatch() = default;
    QueryMatch(TextDocument &document, const treesitter::QueryMatch &match);

    const QVector<QueryCapture> &captures() const;
    bool isEmpty() const;

    // Access to captures
    Q_INVOKABLE Core::RangeMark get(const QString &name) const;
    Q_INVOKABLE Core::RangeMark getInRange(const QString &name, const Core::RangeMark &range) const;
    Q_INVOKABLE Core::RangeMarkList getAll(const QString &name) const;
    Q_INVOKABLE Core::RangeMarkList getAllInRange(const QString &name, const Core::RangeMark &range) const;
    Q_INVOKABLE Core::RangeMark getAllJoined(const QString &name) const;

    // Sub-query in capture
    // This API is exposed publicly here, instead of on RangeMark, as we could in future
    // add the treesitter nodes to the `QueryCapture` without changing public API.
    // This would allow us to do sub-querying without having to re-parse & filter the document.
    // Howevever, then we would have to update the nodes, should the text in the document change,
    // which will be difficult to do inside Q_GADGET.
    //
    // It also results in a quite nice JS API:
    // ```
    // let [function] = document.query(...);
    // let matches = function.queryIn("body", ...);
    // ```
    Q_INVOKABLE QList<Core::QueryMatch> queryIn(const QString &capture, const QString &query) const;

    Q_INVOKABLE QString toString() const;

private:
    QVector<QueryCapture> m_captures;
};

using QueryMatchList = QList<Core::QueryMatch>;

} // namespace Core

Q_DECLARE_METATYPE(Core::QueryCapture)
Q_DECLARE_METATYPE(Core::QueryMatch)
