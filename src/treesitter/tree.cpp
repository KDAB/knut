#include "tree.h"

#include <tree_sitter/api.h>
#include <utility>

namespace treesitter {

Tree::Tree(TSTree *tree)
    : m_tree(tree)
{
}

Tree::Tree(Tree &&other) noexcept
    : m_tree(other.m_tree)
{
    other.m_tree = nullptr;
}

Tree::~Tree()
{
    if (m_tree) {
        ts_tree_delete(m_tree);
    }
}

Tree &Tree::operator=(Tree &&other) noexcept
{
    Tree(std::move(other)).swap(*this);
    return *this;
}

void Tree::swap(Tree &other) noexcept
{
    std::swap(m_tree, other.m_tree);
}

Node Tree::rootNode() const
{
    return Node(ts_tree_root_node(m_tree));
}

}
