#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <variant>

namespace Lsp {

using MessageId = std::variant<int, std::string>;

using IntegerOrNull = std::variant<int, std::nullptr_t>;
using BoolOrNull = std::variant<bool, std::nullptr_t>;

using DocumentUri = std::string;

using SymbolTag = int;

enum class TraceValue {
    Off = 1,
    Message = 2,
    Verbose = 3
};

enum class ResourceOperationKind  {
    Create = 1,
    Rename = 2,
    Delete = 3,
};

enum class FailureHandlingKind  {
    Abort,
    Transactional,
    Undo,
    TextOnlyTransactional
};

enum class SymbolKind {
    File = 1,
    Module = 2,
    Namespace = 3,
    Package = 4,
    Class = 5,
    Method = 6,
    Property = 7,
    Field = 8,
    Constructor = 9,
    Enum = 10,
    Interface = 11,
    Function = 12,
    Variable = 13,
    Constant = 14,
    String = 15,
    Number = 16,
    Boolean = 17,
    Array = 18,
    Object = 19,
    Key = 20,
    Null = 21,
    EnumMember = 22,
    Struct = 23,
    Event = 24,
    Operator = 25,
    TypeParameter = 26
};

enum class MarkupKind {
    PlainText,
    MarkDown
};

struct WorkspaceFolder {
    DocumentUri uri;
    std::string name;
};


struct WorkspaceEditClientCapabilities {
    struct ChangeAnnotationSupport {
        std::optional<bool> groupsOnLabel;
    };

    std::optional<bool> documentChanges;
    std::optional<std::vector<ResourceOperationKind>> resourceOperations;
    std::optional<FailureHandlingKind> failureHandling;
    std::optional<bool> normalizesLineEndings;

};

struct DidChangeConfigurationClientCapabilities {
    std::optional<bool> dynamicRegistration;
};


struct DidChangeWatchedFilesClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct WorkspaceSymbolClientCapabilities {
    struct SymbolKind {
        std::optional<std::vector<SymbolKind>> valueSet;
    };

    struct TagSupport {
        std::optional<std::vector<SymbolTag>> valueSet;
    };

    std::optional<bool> dynamicRegistration;
};

struct ExecuteCommandClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct SemanticTokensWorkspaceClientCapabilities {
    std::optional<bool> refreshSupport;
};

struct CodeLensWorkspaceClientCapabilities {
    std::optional<bool> refreshSupport;
};

struct ShowMessageRequestClientCapabilities {
    struct MessageActionItem {
        std::optional<bool> additionalPropertiesSupport;
    };
    std::optional<MessageActionItem> messageActionItem;
};

struct ShowDocumentClientCapabilities {
    std::optional<bool> support;
};

struct RegularExpressionsClientCapabilities {
    std::string engine;
    std::optional<std::string> version;
};

struct MarkdownClientCapabilities {
    std::string parser;
    std::optional<std::string> version;
};

struct TextDocumentSyncClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<bool> didSave;
};

struct CompletionClientCapabilities {
    struct CompletionItem {
        // TODO
    };

    struct CompletionItemKind {
        // TODO
    };


    std::optional<bool> dynamicRegistration;
    std::optional<CompletionItem> completionItem;
    std::optional<CompletionItemKind> completionItemKind;
    std::optional<bool> contextSupport;
};

struct HoverClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<std::vector<MarkupKind>> contentFormat;
};

struct SignatureHelpClientCapabilities {
    struct SignatueInformation {
        struct ParameterInformation {
            std::optional<bool> labelOffsetSupport;
        };

        std::optional<std::vector<MarkupKind>> documentationFormat;
        std::optional<bool> activeParameterSupport;

    };

    std::optional<bool> dynamicRegistration;
    std::optional<bool> contextSupport;
};

struct DeclarationClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct DefinitionClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct TypeDefinitionClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};

struct ReferenceClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct DocumentHighlightClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct DocumentSymbolClientCapabilities {
    struct SymbolKinds {
        std::optional<std::vector<SymbolKind>> valueSet;
    };

    struct TagSupport {
        std::optional<std::vector<SymbolTag>> valueSet;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<SymbolKinds> symbolKind;
    std::optional<bool> hierarchicalDocumentSymbolSupport;
    std::optional<TagSupport> tagSupport;
};

struct CodeActionClientCapabilities {
    struct ResolveSupport {
        std::vector<std::string> properties;
    };

    std::optional<bool> dynamicRegistration;
    // TODO std::optional<CodeActionLiteralSupport> codeActionLiteralSupport;
    std::optional<bool> isPreferredSupport;
    std::optional<bool> disabledSupport;
    std::optional<bool> dataSupport;
    std::optional<ResolveSupport> resolveSupport;
    std::optional<bool> honorsChangeAnnotations;
};

struct CodeLensClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct DocumentLinkClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> tooltipSupport;
};

struct DocumentColorClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct DocumentFormattingClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct DocumentRangeFormattingClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct DocumentOnTypeFormattingClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct RenameClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> prepareSupport;
    //TODO std::optional<PrepareSupportDefaultBehavior> prepareSupportDefaultBehavior;
    std::optional<bool> honorsChangeAnnotations;
};

struct PublishDiagnosticsClientCapabilities {
    std::optional<bool> relatedInformation;
    //TODO std::optional<TagSupport> tagSupport;
    std::optional<bool> versionSupport;
    std::optional<bool> codeDescriptionSupport;
    std::optional<bool> dataSupport;
};

struct FoldingRangeClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<unsigned int> rangeLimit;
    std::optional<bool> lineFoldingOnly;
};

struct SelectionRangeClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct LinkedEditingRangeClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct CallHierarchyClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct SemanticTokensClientCapabilities {
    struct Requests {
        struct Full {
            std::optional<bool> delta;
        };

        std::optional<BoolOrNull> range;
        std::optional<std::variant<bool, Full>> full;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<Requests> requests;
    std::vector<std::string> tokenTypes;
    std::vector<std::string> tokenModifiers;
    // TODO TokenFormat, it is always "relative" now std::vector<TokenFormat> formats;
    std::optional<bool> overlappingTokenSupport;
    std::optional<bool> multilineTokenSupport;
};

struct MonikerClientCapabilities {
    std::optional<bool> dynamicRegistration;
};

struct TextDocumentClientCapabilities {
    std::optional<TextDocumentSyncClientCapabilities> synchronization;
    std::optional<CompletionClientCapabilities> completion;
    std::optional<HoverClientCapabilities> hover;
    std::optional<SignatureHelpClientCapabilities> signatureHelp;
    std::optional<DeclarationClientCapabilities> declaration;
    std::optional<DefinitionClientCapabilities> definition;
    std::optional<TypeDefinitionClientCapabilities> typeDefinition;
    std::optional<ReferenceClientCapabilities> references;
    std::optional<DocumentHighlightClientCapabilities> documentHighlight;
    std::optional<DocumentSymbolClientCapabilities> documentSymbol;
    std::optional<CodeActionClientCapabilities> codeAction;
    std::optional<CodeLensClientCapabilities> codeLens;
    std::optional<DocumentLinkClientCapabilities> documentLink;
    std::optional<DocumentColorClientCapabilities> colorProvider;
    std::optional<DocumentFormattingClientCapabilities> formatting;
    std::optional<DocumentRangeFormattingClientCapabilities> rangeFormatting;
    std::optional<DocumentOnTypeFormattingClientCapabilities> onTypeFormatting;
    std::optional<RenameClientCapabilities> rename;
    std::optional<PublishDiagnosticsClientCapabilities> publishDiagnostics;
    std::optional<FoldingRangeClientCapabilities> foldingRange;
    std::optional<SelectionRangeClientCapabilities> selectionRange;
    std::optional<LinkedEditingRangeClientCapabilities> linkedEditingRange;
    std::optional<CallHierarchyClientCapabilities> callHierarchy;
    std::optional<SemanticTokensClientCapabilities> semanticTokens;
    std::optional<MonikerClientCapabilities> moniker;
};

struct ClientCapabilities {
    struct Workspace {
        struct FileOperations {
            std::optional<bool> dynamicRegistration;
            std::optional<bool> didCreate;
            std::optional<bool> willCreate;
            std::optional<bool> didRename;
            std::optional<bool> willRename;
            std::optional<bool> didDelete;
            std::optional<bool> willDelete;

        };

        std::optional<bool> applyEdit;
        std::optional<WorkspaceEditClientCapabilities> workspaceEdit;
        std::optional<DidChangeConfigurationClientCapabilities> didChangeConfiguration;
        std::optional<DidChangeWatchedFilesClientCapabilities> didChangeWatchedFiles;
        std::optional<WorkspaceSymbolClientCapabilities> symbol;
        std::optional<ExecuteCommandClientCapabilities> executeCommand;
        std::optional<bool> workspaceFolders;
        std::optional<bool> configuration;
        std::optional<SemanticTokensWorkspaceClientCapabilities> semanticTokens;
        std::optional<CodeLensWorkspaceClientCapabilities> codeLens;
        std::optional<FileOperations> fileOperations;

    };

    struct Window {
        std::optional<bool> workDoneProgress;
        std::optional<ShowMessageRequestClientCapabilities> showMessage;
        std::optional<ShowDocumentClientCapabilities> showDocument;

    };

    struct General {
        std::optional<RegularExpressionsClientCapabilities> regularExpressions;
        std::optional<MarkdownClientCapabilities> markdown;

    };


    std::optional<Workspace> workspace;
    std::optional<TextDocumentClientCapabilities> textDocument;
    std::optional<Window> window;
    std::optional<General> general;
    //TODO experimental?: any;
};
}

///////////////////////////////////////////////////////////////////////////////
// Serialization
///////////////////////////////////////////////////////////////////////////////
namespace nlohmann {
template <>
struct adl_serializer<Lsp::MessageId>
{
    static void to_json(nlohmann::json &j, const Lsp::MessageId &data)
    {
        if (std::holds_alternative<int>(data))
            j = std::get<int>(data);
        else
            j = std::get<std::string>(data);
    }

    static void from_json(const nlohmann::json &j, Lsp::MessageId &data)
    {
        if (j.is_number())
            data = j.get<int>();
        else
            data = j.get<std::string>();
    }
};

template <>
struct adl_serializer<Lsp::IntegerOrNull>
{
    static void to_json(nlohmann::json &j, const Lsp::IntegerOrNull &data)
    {
        if (std::holds_alternative<int>(data))
            j = std::get<int>(data);
        else
            j = nullptr;
    }

    static void from_json(const nlohmann::json &j, Lsp::IntegerOrNull &data)
    {
        if (j.is_number())
            data = j.get<int>();
        else
            data = nullptr;
    }
};

template <>
struct adl_serializer<Lsp::BoolOrNull>
{
    static void to_json(nlohmann::json &j, const Lsp::BoolOrNull &data)
    {
        if (std::holds_alternative<bool>(data))
            j = std::get<bool>(data);
        else
            j = nullptr;
    }

    static void from_json(const nlohmann::json &j, Lsp::BoolOrNull &data)
    {
        if (j.is_boolean())
            data = j.get<bool>();
        else
            data = nullptr;
    }
};

}
