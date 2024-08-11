/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <tree_sitter/api.h>

#include <QString>
#include <QVector>

namespace treesitter {

using Point = TSPoint;

class Node
{
public:
    Node(const Node &) = default;
    Node(Node &&) = default;

    Node &operator=(Node &&) = default;
    Node &operator=(const Node &) = default;

    QString type() const;
    const char *rawType() const;

    uint32_t namedChildCount() const;
    Node namedChild(uint32_t index) const;
    QVector<Node> namedChildren() const;

    QString fieldNameForChild(const Node &child) const;

    uint32_t childCount() const;
    QVector<Node> children() const;

    Node nextSibling() const;
    Node previousSibling() const;
    Node nextNamedSibling() const;
    Node previousNamedSibling() const;

    uint32_t startPosition() const;
    uint32_t endPosition() const;

    Point startPoint() const;
    Point endPoint() const;

    bool isExtra() const;
    bool isMissing() const;
    bool isNamed() const;
    bool isNull() const;

    bool hasError() const;

    QString textIn(const QString &source) const;
    QString textExcept(const QString &source, const QVector<QString> &nodeTypes) const;

    Node descendantForRange(uint32_t left, uint32_t right) const;
    Node parent() const;

    bool operator==(const Node &other) const;

private:
    Node(const TSNode &node);

    QVector<Node> allChildrenOfType(const QVector<QString> &nodeTypes) const;

    // TODO: make private again
public:
    TSNode m_node;

    friend class Tree;
    friend class TreeCursor;
    friend class QueryCursor;
    friend class QueryMatch;
};

}

template <>
struct std::hash<treesitter::Node>
{
    std::size_t operator()(const treesitter::Node &node) const noexcept
    {
        std::size_t result = 0;
        for (unsigned int i : node.m_node.context) {
            result ^= std::hash<uint32_t> {}(i);
        }
        return result ^ std::hash<const void *> {}(node.m_node.id) ^ std::hash<const TSTree *> {}(node.m_node.tree);
    }
};
