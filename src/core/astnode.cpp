#include "astnode.h"

#include "lspdocument.h"
#include "lspdocument_p.h"
#include <treesitter/node.h>

#include <QDebug>
#include <QPlainTextEdit>

namespace Core {

AstNode::AstNode(const treesitter::Node &node, LspDocument *parent)
    : m_mark(parent, node.startPosition(), node.endPosition())
    , m_type(node.type())
{
}

AstNode AstNode::parentNode() const
{
    if (auto n = node())
        return AstNode(n->parent(), document());
    return {};
}

Core::AstNodeList AstNode::childrenNodes() const
{
    QVector<AstNode> children;
    if (auto n = node()) {
        for (const auto &node : n->children()) {
            children.append(AstNode(node, document()));
        }
    }
    return children;
}

bool AstNode::isValid() const
{
    return m_mark.isValid();
}

std::optional<treesitter::Node> AstNode::node() const
{
    if (!isValid()) {
        qWarning() << "AstNode is invalid";
        return std::nullopt;
    }

    if (auto doc = document()) {
        return doc->m_treeSitterHelper->syntaxTree()->rootNode().descendantForRange(startPos(), endPos());
    }
    return std::nullopt;
}

LspDocument *AstNode::document() const
{
    return qobject_cast<LspDocument *>(m_mark.document());
}

QString AstNode::type() const
{
    return m_type;
}

QString AstNode::text() const
{
    return m_mark.text();
}

int AstNode::startPos() const
{
    return m_mark.start();
}

int AstNode::endPos() const
{
    return m_mark.end();
}

}
