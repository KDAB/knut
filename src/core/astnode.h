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
class Node;
}

class TestCodeDocument;

namespace Core {

class CodeDocument;

class AstNode
{
    Q_GADGET

    Q_PROPERTY(QString type READ type CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(int startPos READ startPos CONSTANT)
    Q_PROPERTY(int endPos READ endPos CONSTANT)

public:
    Q_INVOKABLE Core::AstNode parentNode() const;
    Q_INVOKABLE QList<Core::AstNode> childrenNodes() const;
    Q_INVOKABLE bool isValid() const;

    QString type() const;
    QString text() const;
    int startPos() const;
    int endPos() const;

    AstNode() = default;

private:
    explicit AstNode(const treesitter::Node &node, CodeDocument *parent);

    std::optional<treesitter::Node> node() const;
    CodeDocument *document() const;

private:
    RangeMark m_mark;
    QString m_type;

    friend class CodeDocument;
};

using AstNodeList = QList<Core::AstNode>;

}
Q_DECLARE_METATYPE(Core::AstNode)
