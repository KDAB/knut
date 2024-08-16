/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "node.h"
#include "utils/log.h"

#include <kdalgorithms.h>

namespace treesitter {

Node::Node(const TSNode &node)
    : m_node(node)
{
}

const char *Node::rawType() const
{
    return ts_node_type(m_node);
}

QString Node::type() const
{
    return QString(rawType());
}

uint32_t Node::namedChildCount() const
{
    return ts_node_named_child_count(m_node);
}

Node Node::namedChild(uint32_t index) const
{
    return Node(ts_node_named_child(m_node, index));
}

QString Node::fieldNameForChild(const Node &child) const
{
    // Implementation note:
    //
    // I would expect the code below to work:
    //
    // const auto index = children().indexOf(child);
    // QString result;
    // if (index != -1) {
    //     auto name = ts_node_field_name_for_child(m_node, static_cast<uint32_t>(index));
    //     if (name) {
    //         result = name;
    //     }
    // } else {
    //     spdlog::warn("{}: given node is not a child!", FUNCTION_NAME);
    // }
    //
    // However, the code produces incorrect field names in a few cases.
    // Most notably a parameter declaration like this:
    //
    // const Thing &test
    //
    // returns `type` as the field name of the `const` token, instead of `Thing`, which is incorrect.
    // When querying for the `type: (_) @type`, we do actually find `Thing`, so the field name returned here
    // wouldn't match what the query actually finds.
    //
    // When looking at the Tree-Sitter playground [^1], this works correctly.
    // Investigating the playground [^2] of this shows that the playground uses
    // the TreeCursor to get the field name, which seems to work correctly.
    // Therefore the code below fixes the issue by using a TreeCursor as well.
    //
    // This issue has already been reported as:
    // https://github.com/tree-sitter/tree-sitter/issues/1642
    // and fixed in:
    // https://github.com/tree-sitter/tree-sitter/pull/2104
    //
    // However, this has been merged a few hours **after** the release 0.20.8.
    // So it should be fixed in 0.20.9, but for now, we need to use the TreeCursor API
    // until a new tree-sitter release is available.
    //
    // [^1]: https://tree-sitter.github.io/tree-sitter/playground
    // [^2]:
    // https://github.com/tree-sitter/tree-sitter/blob/20924fa4cdeb10d82ac308481e39bf8519334e55/docs/assets/js/playground.js#L178C35-L178C35

    QString result;
    bool found = false;

    TSTreeCursor cursor = ts_tree_cursor_new(m_node);

    if (ts_tree_cursor_goto_first_child(&cursor)) {
        do {
            if (Node(ts_tree_cursor_current_node(&cursor)) == child) {
                found = true;

                if (const auto *name = ts_tree_cursor_current_field_name(&cursor)) {
                    result = name;
                }
                break;
            }
        } while (ts_tree_cursor_goto_next_sibling(&cursor));
    }

    ts_tree_cursor_delete(&cursor);

    if (!found) {
        spdlog::warn("{}: given node is not a child!", FUNCTION_NAME);
    }

    return result;
}

uint32_t Node::childCount() const
{
    return ts_node_child_count(m_node);
}

QList<Node> Node::children() const
{
    const auto count = childCount();
    QList<Node> result;
    result.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        result.emplace_back(Node(ts_node_child(m_node, i)));
    }

    return result;
}

QList<Node> Node::namedChildren() const
{
    const auto count = namedChildCount();
    QList<Node> result;
    result.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        result.emplace_back(Node(ts_node_named_child(m_node, i)));
    }

    return result;
}

Node Node::nextSibling() const
{
    return Node(ts_node_next_sibling(m_node));
}

Node Node::previousSibling() const
{
    return Node(ts_node_prev_sibling(m_node));
}

Node Node::nextNamedSibling() const
{
    return Node(ts_node_next_named_sibling(m_node));
}

Node Node::previousNamedSibling() const
{
    return Node(ts_node_prev_named_sibling(m_node));
}

uint32_t Node::startPosition() const
{
    return ts_node_start_byte(m_node) / sizeof(QChar);
}

uint32_t Node::endPosition() const
{
    return ts_node_end_byte(m_node) / sizeof(QChar);
}

Point Node::startPoint() const
{
    return ts_node_start_point(m_node);
}

Point Node::endPoint() const
{
    return ts_node_end_point(m_node);
}

bool Node::isExtra() const
{
    return ts_node_is_extra(m_node);
}

bool Node::isMissing() const
{
    return ts_node_is_missing(m_node);
}

bool Node::isNamed() const
{
    return ts_node_is_named(m_node);
}

bool Node::isNull() const
{
    return ts_node_is_null(m_node);
}

bool Node::hasError() const
{
    return ts_node_has_error(m_node);
}

QString Node::textIn(const QString &source) const
{
    const auto start = this->startPosition();
    const auto end = this->endPosition();

    return source.sliced(start, end - start);
}

QString Node::textExcept(const QString &source, const QList<QString> &nodeTypes) const
{
    auto text = textIn(source);

    auto children = allChildrenOfType(nodeTypes);
    // make sure to sort the children back-to-front, so that removing them in order
    // doesn't mess up the ranges of the remaining children.
    kdalgorithms::sort(children, [](const auto &left, const auto &right) {
        return left.startPosition() > right.startPosition();
    });

    for (const auto &child : std::as_const(children)) {
        const auto start = child.startPosition();
        const auto end = child.endPosition();
        text.remove(start - this->startPosition(), end - start);
    }

    return text;
}

QList<Node> Node::allChildrenOfType(const QList<QString> &nodeTypes) const
{
    auto result = QList<Node>();

    const auto allChildren = children();
    for (const auto &child : allChildren) {
        // break the recursion at the first node that is of the given type
        // That way we don't get overlapping child nodes.
        if (kdalgorithms::contains(nodeTypes, child.type())) {
            result.push_back(child);
        } else {
            result.append(child.allChildrenOfType(nodeTypes));
        }
    }

    return result;
}

bool Node::operator==(const Node &other) const
{
    return ts_node_eq(m_node, other.m_node);
}

Node Node::descendantForRange(uint32_t left, uint32_t right) const
{
    return Node(ts_node_descendant_for_byte_range(m_node, left * sizeof(QChar), right * sizeof(QChar)));
}

Node Node::parent() const
{
    return Node(ts_node_parent(m_node));
}

}
