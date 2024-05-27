#include "treesittertreemodel.h"

#include <QBrush>
#include <QColor>

#include "utils/log.h"

namespace Gui {

TreeSitterTreeModel::TreeNode::TreeNode(const treesitter::Node &node, const TreeNode *parent, bool enableUnnamed)
    : m_parent(parent)
    , m_node(node)
    , m_enableUnnamed(enableUnnamed)
{
}

const std::vector<std::unique_ptr<TreeSitterTreeModel::TreeNode>> &TreeSitterTreeModel::TreeNode::children() const
{
    // This const_cast is safe, as "children" is the non-const version of "children" is a const function
    // apart from the non-const return value.
    return const_cast<TreeSitterTreeModel::TreeNode *>(this)->children();
}

std::vector<std::unique_ptr<TreeSitterTreeModel::TreeNode>> &TreeSitterTreeModel::TreeNode::children()
{

    if (m_children.empty() && childCount() > 0) {
        auto children = m_enableUnnamed ? m_node.children() : m_node.namedChildren();
        for (const auto &child : children) {
            m_children.emplace_back(new TreeNode(child, this, m_enableUnnamed));
        }
    }

    return m_children;
}

int TreeSitterTreeModel::TreeNode::childCount() const
{
    return static_cast<int>(m_enableUnnamed ? m_node.childCount() : m_node.namedChildCount());
}

const TreeSitterTreeModel::TreeNode *TreeSitterTreeModel::TreeNode::child(int row) const
{
    if (row < childCount()) {
        return children().at(row).get();
    }
    return nullptr;
}

QVariant TreeSitterTreeModel::TreeNode::data(int column) const
{
    auto fieldName = QString();
    if (parent()) {
        fieldName = parent()->m_node.fieldNameForChild(m_node);
    }
    switch (column) {
    case 0:
        if (fieldName.isEmpty()) {
            auto type = m_node.type();
            // Some anonymous nodes actually cover a newline, so we need to escape it.
            // This is usually for C preprocessor directives.
            type.replace("\n", "\\n");
            return type;
        } else {
            return QString("%1: %2").arg(fieldName, m_node.type());
        }
    case 1:
        return QString("[%1:%2] - [%3:%4]")
            .arg(m_node.startPoint().row)
            .arg(m_node.startPoint().column)
            .arg(m_node.endPoint().row)
            .arg(m_node.endPoint().column);
    default:
        break;
    }
    return {};
}

bool operator==(const std::unique_ptr<TreeSitterTreeModel::TreeNode> &unique, const TreeSitterTreeModel::TreeNode *ptr)
{
    return unique.get() == ptr;
}

int TreeSitterTreeModel::TreeNode::row() const
{
    if (m_parent) {
        const auto &children = m_parent->children();
        const auto it = std::find(children.cbegin(), children.cend(), this);

        return std::distance(children.cbegin(), it);
    }
    return 0;
}

const TreeSitterTreeModel::TreeNode *TreeSitterTreeModel::TreeNode::parent() const
{
    return m_parent;
}

bool TreeSitterTreeModel::TreeNode::includesPosition(int position) const
{
    return static_cast<int>(m_node.startPosition()) <= position && position <= static_cast<int>(m_node.endPosition());
}

treesitter::Node TreeSitterTreeModel::TreeNode::tsNode() const
{
    return m_node;
}

void TreeSitterTreeModel::TreeNode::traverse(const std::function<void(TreeNode *)> &fun,
                                             const std::function<bool(TreeNode *)> &filter)
{
    if (filter(this)) {
        fun(this);

        for (const auto &child : children()) {
            child->traverse(fun, filter);
        }
    }
}

// ***************** TreeModel ********************8

TreeSitterTreeModel::TreeSitterTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_cursorPosition(-1)
{
}

QModelIndex TreeSitterTreeModel::indexFor(const TreeNode &node, int column) const
{
    return createIndex(node.row(), column, &node);
}

QModelIndex TreeSitterTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_rootNode) {
        return {};
    }

    if (!parent.isValid() && row == 0) {
        return createIndex(row, column, m_rootNode.get());
    }

    const auto *parentNode = static_cast<TreeNode *>(parent.internalPointer());
    const auto child = parentNode->child(row);
    if (child) {
        return createIndex(row, column, child);
    } else {
        return {};
    }
}

int TreeSitterTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (m_rootNode) {
            return 1;
        }
        return 0;
    }

    const auto *node = static_cast<TreeNode *>(parent.internalPointer());
    return node->childCount();
}

int TreeSitterTreeModel::columnCount(const QModelIndex &) const
{
    if (m_rootNode) {
        return 3;
    }
    return 0;
}

QModelIndex TreeSitterTreeModel::parent(const QModelIndex &index) const
{
    const auto *node = static_cast<TreeNode *>(index.internalPointer());
    auto *parent = node->parent();

    if (parent) {
        return createIndex(parent->row(), 0, parent);
    }

    return {};
}

QVariant TreeSitterTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Node");
        case 1:
            return tr("Range");
        case 2:
            return tr("Query Captures");
        default:
            return {};
        }
    }

    return {};
}

QVariant TreeSitterTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto *node = static_cast<TreeNode *>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() < 2) {
            return node->data(index.column());
        } else {
            if (m_query.has_value()) {
                const auto it = m_query->captures.find(node->tsNode());
                if (it != m_query->captures.cend()) {
                    return it->second;
                }
            }
        }
        break;
    case Qt::ForegroundRole:
        return node->includesPosition(m_cursorPosition) ? QBrush(QColorConstants::Green) : QVariant();
    default:
        break;
    }

    return {};
}

Qt::ItemFlags TreeSitterTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

void TreeSitterTreeModel::setTree(treesitter::Tree &&tree, std::unique_ptr<treesitter::Predicates> &&predicates,
                                  bool enableUnnamed)
{
    beginResetModel();
    m_tree = std::move(tree);
    m_rootNode = std::make_unique<TreeNode>(m_tree->rootNode(), nullptr, enableUnnamed);
    executeQuery(std::move(predicates));
    endResetModel();
}

void TreeSitterTreeModel::clear()
{
    beginResetModel();
    m_tree = {};
    m_rootNode.reset();
    m_cursorPosition = -1;
    endResetModel();
}

void TreeSitterTreeModel::executeQuery(std::unique_ptr<treesitter::Predicates> &&predicates)
{
    treesitter::QueryCursor cursor;

    if (m_rootNode && m_query.has_value()) {
        m_query->captures = decltype(m_query->captures)();
        m_query->numCaptures = 0;
        m_query->numMatches = 0;

        cursor.execute(m_query->query, m_rootNode->tsNode(), std::move(predicates));

        while (const auto match = cursor.nextMatch()) {
            m_query->numMatches++;

            const auto captures = match->captures();
            for (const auto &capture : captures) {
                m_query->numCaptures++;
                m_query->captures[capture.node] += " @" + m_query->query->captureAt(capture.id).name;
            }
        }
    }
}

void TreeSitterTreeModel::capturesChanged(const std::unordered_map<treesitter::Node, QString> &oldCaptures)
{
    if (m_rootNode) {
        m_rootNode->traverse([&oldCaptures, this](const auto *node) {
            const bool isOldCapture = oldCaptures.contains(node->tsNode());
            const bool isNewCapture = m_query.has_value() && m_query->captures.contains(node->tsNode());
            if (isOldCapture || isNewCapture) {
                emit dataChanged(indexFor(*node, 2), indexFor(*node, 2));
            }
        });
    }
}

void TreeSitterTreeModel::setQuery(const std::shared_ptr<treesitter::Query> &query,
                                   std::unique_ptr<treesitter::Predicates> &&predicates)
{
    std::unordered_map<treesitter::Node, QString> oldCaptures =
        m_query.has_value() ? std::move(m_query->captures) : decltype(m_query->captures)();

    if (query != nullptr) {
        m_query =
            QueryData {.query = query, .captures = decltype(m_query->captures)(), .numMatches = 0, .numCaptures = 0};
    } else {
        m_query = {};
    }

    executeQuery(std::move(predicates));
    capturesChanged(oldCaptures);
}

void TreeSitterTreeModel::positionChanged(int position)
{
    if (m_rootNode) {
        m_rootNode->traverse(
            [this](const auto *node) {
                emit dataChanged(indexFor(*node, 0), indexFor(*node, columnCount() - 1));
            },
            [position](const auto *node) {
                return node->includesPosition(position);
            });
    }
}

void TreeSitterTreeModel::setCursorPosition(int position)
{
    int oldPosition = m_cursorPosition;
    m_cursorPosition = position;

    positionChanged(oldPosition);
    positionChanged(m_cursorPosition);
}

std::optional<treesitter::Node> TreeSitterTreeModel::tsNode(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }
    auto node = static_cast<TreeNode *>(index.internalPointer());
    return node->tsNode();
}

bool TreeSitterTreeModel::hasQuery() const
{
    return m_query.has_value();
}

int TreeSitterTreeModel::patternCount() const
{
    if (m_query.has_value()) {
        return m_query->query->patterns().size();
    }

    return 0;
}

int TreeSitterTreeModel::matchCount() const
{
    if (m_query.has_value()) {
        return m_query->numMatches;
    }
    return 0;
}

int TreeSitterTreeModel::captureCount() const
{
    if (m_query.has_value()) {
        return m_query->numCaptures;
    }
    return 0;
}

}
