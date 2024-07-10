#include "tree_cursor.h"

#include <tree_sitter/api.h>

#include <QString>

namespace treesitter {
TreeCursor::TreeCursor(Node node)
{
    m_cursor = ts_tree_cursor_new(node.m_node);
}

Node TreeCursor::currentNode() const
{
    return ts_tree_cursor_current_node(&m_cursor);
}
QString TreeCursor::currentFieldName() const
{
    auto fieldName = ts_tree_cursor_current_field_name(&m_cursor);

    return fieldName ? QString(fieldName) : QString();
}

bool TreeCursor::gotoFirstChild()
{
    return ts_tree_cursor_goto_first_child(&m_cursor);
}

bool TreeCursor::gotoNextSibling()
{
    return ts_tree_cursor_goto_next_sibling(&m_cursor);
}

bool TreeCursor::gotoParent()
{
    return ts_tree_cursor_goto_parent(&m_cursor);
}

}
