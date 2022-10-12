#include "node.h"

#include <spdlog/spdlog.h>

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
    const auto index = children().indexOf(child);
    QString result;
    if (index != -1) {
        auto name = ts_node_field_name_for_child(m_node, static_cast<uint32_t>(index));
        if (name) {
            result = name;
        }
    } else {
        spdlog::warn("Node::fieldNameForChild - given node is not a child!");
    }

    return result;
}

QVector<Node> Node::children() const
{
    const auto count = ts_node_child_count(m_node);
    QVector<Node> result;
    result.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        result.emplace_back(Node(ts_node_child(m_node, i)));
    }

    return result;
}

QVector<Node> Node::namedChildren() const
{
    const auto count = namedChildCount();
    QVector<Node> result;
    result.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        result.emplace_back(Node(ts_node_named_child(m_node, i)));
    }

    return result;
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

bool Node::operator==(const Node &other) const
{
    return ts_node_eq(m_node, other.m_node);
}

}
