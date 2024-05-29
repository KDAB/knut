#pragma once

#include "lsp/types.h"
#include "querymatch.h"
#include "textlocation.h"
#include "textrange.h"

#include <QVector>

namespace Core {

class CodeDocument;
class ClassSymbol;
class FunctionSymbol;

class Symbol : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(Kind kind READ kind CONSTANT)
    Q_PROPERTY(Core::TextRange range READ range CONSTANT)
    Q_PROPERTY(Core::TextRange selectionRange READ selectionRange CONSTANT)

public:
    enum Kind {
        File = static_cast<int>(Lsp::SymbolKind::File),
        Module = static_cast<int>(Lsp::SymbolKind::Module),
        Namespace = static_cast<int>(Lsp::SymbolKind::Namespace),
        Package = static_cast<int>(Lsp::SymbolKind::Package),
        Class = static_cast<int>(Lsp::SymbolKind::Class),
        Method = static_cast<int>(Lsp::SymbolKind::Method),
        Property = static_cast<int>(Lsp::SymbolKind::Property),
        Field = static_cast<int>(Lsp::SymbolKind::Field),
        Constructor = static_cast<int>(Lsp::SymbolKind::Constructor),
        Enum = static_cast<int>(Lsp::SymbolKind::Enum),
        Interface = static_cast<int>(Lsp::SymbolKind::Interface),
        Function = static_cast<int>(Lsp::SymbolKind::Function),
        Variable = static_cast<int>(Lsp::SymbolKind::Variable),
        Constant = static_cast<int>(Lsp::SymbolKind::Constant),
        String = static_cast<int>(Lsp::SymbolKind::String),
        Number = static_cast<int>(Lsp::SymbolKind::Number),
        Boolean = static_cast<int>(Lsp::SymbolKind::Boolean),
        Array = static_cast<int>(Lsp::SymbolKind::Array),
        Object = static_cast<int>(Lsp::SymbolKind::Object),
        Key = static_cast<int>(Lsp::SymbolKind::Key),
        Null = static_cast<int>(Lsp::SymbolKind::Null),
        EnumMember = static_cast<int>(Lsp::SymbolKind::EnumMember),
        Struct = static_cast<int>(Lsp::SymbolKind::Struct),
        Event = static_cast<int>(Lsp::SymbolKind::Event),
        Operator = static_cast<int>(Lsp::SymbolKind::Operator),
        TypeParameter = static_cast<int>(Lsp::SymbolKind::TypeParameter),
    };
    Q_ENUM(Kind)

protected:
    Symbol(QObject *parent, const QueryMatch &match, Kind kind);

    QString m_name;
    Kind m_kind;
    TextRange m_range;
    TextRange m_selectionRange;
    QueryMatch m_queryMatch;

    CodeDocument *document() const;

public:
    static Symbol *makeSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    Q_INVOKABLE bool isClass() const;
    Core::ClassSymbol *toClass();

    Q_INVOKABLE bool isFunction() const;
    Core::FunctionSymbol *toFunction();

    QString name() const;
    virtual QString description() const;
    Kind kind() const;
    Core::TextRange range() const;
    Core::TextRange selectionRange() const;

    // As per KNUT-163, these are no longer public API.
    // They are only used internally by the editor/GUI and not available from QML/JS.
    // As this relies on the clangd LSP, it is not reliable enough to use for scripting.
    QVector<Core::TextLocation> references() const;

    Q_INVOKABLE void select();

    bool operator==(const Symbol &) const;

private:
    void assignContext(const QVector<Symbol *> &contexts);

    friend class CodeDocument;
    friend class TreeSitterHelper;
};

using SymbolList = QList<Core::Symbol *>;

} // namespace Core

Q_DECLARE_METATYPE(Core::Symbol)
Q_DECLARE_METATYPE(QVector<Core::Symbol *>)
