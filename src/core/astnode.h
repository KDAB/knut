#pragma once

#include "rangemark.h"

#include <QObject>

namespace treesitter {
class Node;
}

class TestLspDocument;

namespace Core {

class LspDocument;

class AstNode
{
    Q_GADGET

    Q_PROPERTY(QString type READ type CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(int startPos READ startPos CONSTANT)
    Q_PROPERTY(int endPos READ endPos CONSTANT)

public:
    Q_INVOKABLE Core::AstNode parentNode() const;
    Q_INVOKABLE QVector<Core::AstNode> childrenNodes() const;
    Q_INVOKABLE bool isValid() const;

    QString type() const;
    QString text() const;
    int startPos() const;
    int endPos() const;

    AstNode() = default;

private:
    explicit AstNode(const treesitter::Node &node, LspDocument *parent);

    std::optional<treesitter::Node> node() const;
    LspDocument *document() const;

private:
    RangeMark m_mark;
    QString m_type;

    friend class LspDocument;
};

}
Q_DECLARE_METATYPE(Core::AstNode)
