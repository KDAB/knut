/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "astnode.h"
#include "codedocument.h"
#include "codedocument_p.h"
#include "treesitter/node.h"

#include <QPlainTextEdit>

namespace Core {

AstNode::AstNode(const treesitter::Node &node, CodeDocument *parent)
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
        const auto childNodes = n->children();
        for (const auto &node : childNodes) {
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
        spdlog::warn("AstNode is invalid");
        return std::nullopt;
    }

    if (auto doc = document()) {
        return doc->m_treeSitterHelper->syntaxTree()->rootNode().descendantForRange(startPos(), endPos());
    }
    return std::nullopt;
}

CodeDocument *AstNode::document() const
{
    return qobject_cast<CodeDocument *>(m_mark.document());
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
