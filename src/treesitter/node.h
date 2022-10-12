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

    QString type() const;
    const char *rawType() const;

    uint32_t namedChildCount() const;
    Node namedChild(uint32_t index) const;
    QVector<Node> namedChildren() const;

    QString fieldNameForChild(const Node &child) const;

    QVector<Node> children() const;

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

    bool operator==(const Node &other) const;

private:
    Node(const TSNode &node);

    // TODO: make private again
public:
    TSNode m_node;

    friend class Tree;
    friend class QueryCursor;
    friend class QueryMatch;
};

}

template <>
struct std::hash<treesitter::Node>
{
    std::size_t operator()(const treesitter::Node &node) const
    {
        std::size_t result = 0;
        for (int i = 0; i < 4; i++) {
            result ^= std::hash<uint32_t> {}(node.m_node.context[i]);
        }
        return result ^ std::hash<const void *> {}(node.m_node.id) ^ std::hash<const TSTree *> {}(node.m_node.tree);
    }
};
