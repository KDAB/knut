#pragma once

#include "utils/json_utils.h"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <variant>

namespace Lsp {

using DocumentUri = std::string;

using SymbolTag = int;

enum class TraceValue {
    Off = 1,
    Message = 2,
    Verbose = 3,
};

enum class ResourceOperationKind {
    Create = 1,
    Rename = 2,
    Delete = 3,
};

enum class FailureHandlingKind {
    Abort,
    Transactional,
    Undo,
    TextOnlyTransactional,
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

enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25
};

enum class MarkupKind {
    PlainText,
    MarkDown,
};

enum class TokenFormat {
    Relative = 1,
};

struct WorkspaceFolder
{
    DocumentUri uri;
    std::string name;
};
JSONIFY(WorkspaceFolder, uri, name);

struct WorkspaceEditClientCapabilities
{
    struct ChangeAnnotationSupport
    {
        std::optional<bool> groupsOnLabel;
    };

    std::optional<bool> documentChanges;
    std::optional<std::vector<ResourceOperationKind>> resourceOperations;
    std::optional<FailureHandlingKind> failureHandling;
    std::optional<bool> normalizesLineEndings;
    std::optional<ChangeAnnotationSupport> changeAnnotationSupport;
};
JSONIFY(WorkspaceEditClientCapabilities::ChangeAnnotationSupport, groupsOnLabel);
JSONIFY(WorkspaceEditClientCapabilities, documentChanges, resourceOperations, failureHandling, normalizesLineEndings,
        changeAnnotationSupport);

struct DidChangeConfigurationClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DidChangeConfigurationClientCapabilities, dynamicRegistration);

struct DidChangeWatchedFilesClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DidChangeWatchedFilesClientCapabilities, dynamicRegistration);

struct WorkspaceSymbolClientCapabilities
{
    struct SymbolKinds
    {
        std::optional<std::vector<SymbolKind>> valueSet;
    };

    struct TagSupport
    {
        std::optional<std::vector<SymbolTag>> valueSet;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<SymbolKinds> symbolKind;
    std::optional<TagSupport> tagSupport;
};
JSONIFY(WorkspaceSymbolClientCapabilities::SymbolKinds, valueSet);
JSONIFY(WorkspaceSymbolClientCapabilities::TagSupport, valueSet);
JSONIFY(WorkspaceSymbolClientCapabilities, dynamicRegistration, symbolKind, tagSupport);

struct ExecuteCommandClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(ExecuteCommandClientCapabilities, dynamicRegistration);

struct SemanticTokensWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};
JSONIFY(SemanticTokensWorkspaceClientCapabilities, refreshSupport);

struct CodeLensWorkspaceClientCapabilities
{
    std::optional<bool> refreshSupport;
};
JSONIFY(CodeLensWorkspaceClientCapabilities, refreshSupport);

struct ShowMessageRequestClientCapabilities
{
    struct MessageActionItem
    {
        std::optional<bool> additionalPropertiesSupport;
    };
    std::optional<MessageActionItem> messageActionItem;
};
JSONIFY(ShowMessageRequestClientCapabilities::MessageActionItem, additionalPropertiesSupport);
JSONIFY(ShowMessageRequestClientCapabilities, messageActionItem);

struct ShowDocumentClientCapabilities
{
    bool support;
};
JSONIFY(ShowDocumentClientCapabilities, support);

struct RegularExpressionsClientCapabilities
{
    std::string engine;
    std::optional<std::string> version;
};
JSONIFY(RegularExpressionsClientCapabilities, engine, version);

struct MarkdownClientCapabilities
{
    std::string parser;
    std::optional<std::string> version;
};
JSONIFY(MarkdownClientCapabilities, parser, version);

struct TextDocumentSyncClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<bool> didSave;
};
JSONIFY(TextDocumentSyncClientCapabilities, dynamicRegistration, willSave, willSaveWaitUntil, didSave);

struct CompletionClientCapabilities
{
    struct CompletionItem
    {
        std::optional<bool> snippetSupport;
        std::optional<bool> commitCharactersSupport;
        std::optional<std::vector<MarkupKind>> documentationFormat;
        std::optional<bool> deprecatedSupport;
        std::optional<bool> preselectSupport;
        std::optional<bool> insertReplaceSupport;
        // TODO complete
    };

    struct CompletionItemKinds
    {
        std::optional<std::vector<CompletionItemKind>> valueSet;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<CompletionItem> completionItem;
    std::optional<CompletionItemKinds> completionItemKind;
    std::optional<bool> contextSupport;
};
JSONIFY(CompletionClientCapabilities::CompletionItem, snippetSupport, commitCharactersSupport, documentationFormat,
        deprecatedSupport, preselectSupport, insertReplaceSupport);
JSONIFY(CompletionClientCapabilities::CompletionItemKinds, valueSet);
JSONIFY(CompletionClientCapabilities, dynamicRegistration, completionItem, completionItemKind, contextSupport);

struct HoverClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<std::vector<MarkupKind>> contentFormat;
};
JSONIFY(HoverClientCapabilities, dynamicRegistration, contentFormat);

struct SignatureHelpClientCapabilities
{
    struct SignatueInformation
    {
        struct ParameterInformation
        {
            std::optional<bool> labelOffsetSupport;
        };

        std::optional<std::vector<MarkupKind>> documentationFormat;
        std::optional<ParameterInformation> parameterInformation;
        std::optional<bool> activeParameterSupport;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<SignatueInformation> signatureInformation;
    std::optional<bool> contextSupport;
};
JSONIFY(SignatureHelpClientCapabilities::SignatueInformation::ParameterInformation, labelOffsetSupport);
JSONIFY(SignatureHelpClientCapabilities::SignatueInformation, documentationFormat, parameterInformation,
        activeParameterSupport);
JSONIFY(SignatureHelpClientCapabilities, dynamicRegistration, signatureInformation, contextSupport);

struct DeclarationClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};
JSONIFY(DeclarationClientCapabilities, dynamicRegistration, linkSupport);

struct DefinitionClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};
JSONIFY(DefinitionClientCapabilities, dynamicRegistration, linkSupport);

struct TypeDefinitionClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};
JSONIFY(TypeDefinitionClientCapabilities, dynamicRegistration, linkSupport);

struct ImplementationClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> linkSupport;
};
JSONIFY(ImplementationClientCapabilities, dynamicRegistration, linkSupport);

struct ReferenceClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(ReferenceClientCapabilities, dynamicRegistration);

struct DocumentHighlightClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DocumentHighlightClientCapabilities, dynamicRegistration);

struct DocumentSymbolClientCapabilities
{
    struct SymbolKinds
    {
        std::optional<std::vector<SymbolKind>> valueSet;
    };

    struct TagSupport
    {
        std::vector<SymbolTag> valueSet;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<SymbolKinds> symbolKind;
    std::optional<bool> hierarchicalDocumentSymbolSupport;
    std::optional<TagSupport> tagSupport;
    std::optional<bool> labelSupport;
};
JSONIFY(DocumentSymbolClientCapabilities::SymbolKinds, valueSet);
JSONIFY(DocumentSymbolClientCapabilities::TagSupport, valueSet);
JSONIFY(DocumentSymbolClientCapabilities, dynamicRegistration, symbolKind, hierarchicalDocumentSymbolSupport,
        tagSupport, labelSupport);

struct CodeActionClientCapabilities
{
    struct ResolveSupport
    {
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
JSONIFY(CodeActionClientCapabilities::ResolveSupport, properties);
JSONIFY(CodeActionClientCapabilities, dynamicRegistration, isPreferredSupport, disabledSupport, dataSupport,
        resolveSupport, honorsChangeAnnotations);

struct CodeLensClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(CodeLensClientCapabilities, dynamicRegistration);

struct DocumentLinkClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> tooltipSupport;
};
JSONIFY(DocumentLinkClientCapabilities, dynamicRegistration, tooltipSupport);

struct DocumentColorClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DocumentColorClientCapabilities, dynamicRegistration);

struct DocumentFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DocumentFormattingClientCapabilities, dynamicRegistration);

struct DocumentRangeFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DocumentRangeFormattingClientCapabilities, dynamicRegistration);

struct DocumentOnTypeFormattingClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(DocumentOnTypeFormattingClientCapabilities, dynamicRegistration);

struct RenameClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<bool> prepareSupport;
    // TODO std::optional<PrepareSupportDefaultBehavior> prepareSupportDefaultBehavior;
    std::optional<bool> honorsChangeAnnotations;
};
JSONIFY(RenameClientCapabilities, dynamicRegistration, prepareSupport, honorsChangeAnnotations);

struct PublishDiagnosticsClientCapabilities
{
    std::optional<bool> relatedInformation;
    // TODO std::optional<TagSupport> tagSupport;
    std::optional<bool> versionSupport;
    std::optional<bool> codeDescriptionSupport;
    std::optional<bool> dataSupport;
};
JSONIFY(PublishDiagnosticsClientCapabilities, relatedInformation, versionSupport, codeDescriptionSupport, dataSupport);

struct FoldingRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration;
    std::optional<unsigned int> rangeLimit;
    std::optional<bool> lineFoldingOnly;
};
JSONIFY(FoldingRangeClientCapabilities, dynamicRegistration, rangeLimit, lineFoldingOnly);

struct SelectionRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(SelectionRangeClientCapabilities, dynamicRegistration);

struct LinkedEditingRangeClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(LinkedEditingRangeClientCapabilities, dynamicRegistration);

struct CallHierarchyClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(CallHierarchyClientCapabilities, dynamicRegistration);

struct SemanticTokensClientCapabilities
{
    struct Requests
    {
        struct Full
        {
            std::optional<bool> delta;
        };

        std::optional<std::variant<bool, std::nullptr_t>> range;
        std::optional<std::variant<bool, Full>> full;
    };

    std::optional<bool> dynamicRegistration;
    std::optional<Requests> requests;
    std::vector<std::string> tokenTypes;
    std::vector<std::string> tokenModifiers;
    std::vector<TokenFormat> formats;
    std::optional<bool> overlappingTokenSupport;
    std::optional<bool> multilineTokenSupport;
};
// TODO JSONIFY(SemanticTokensClientCapabilities::Requests::Full, delta);
JSONIFY(SemanticTokensClientCapabilities::Requests, range);
JSONIFY(SemanticTokensClientCapabilities, dynamicRegistration, requests, tokenTypes, tokenModifiers,
        overlappingTokenSupport, multilineTokenSupport);

struct MonikerClientCapabilities
{
    std::optional<bool> dynamicRegistration;
};
JSONIFY(MonikerClientCapabilities, dynamicRegistration);

struct TextDocumentClientCapabilities
{
    std::optional<TextDocumentSyncClientCapabilities> synchronization;
    std::optional<CompletionClientCapabilities> completion;
    std::optional<HoverClientCapabilities> hover;
    std::optional<SignatureHelpClientCapabilities> signatureHelp;
    std::optional<DeclarationClientCapabilities> declaration;
    std::optional<DefinitionClientCapabilities> definition;
    std::optional<TypeDefinitionClientCapabilities> typeDefinition;
    std::optional<ImplementationClientCapabilities> implementation;
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
JSONIFY(TextDocumentClientCapabilities, synchronization, completion, hover, signatureHelp, declaration, definition,
        typeDefinition, references, documentHighlight, documentSymbol, codeAction, codeLens, documentLink,
        colorProvider, formatting, rangeFormatting, onTypeFormatting, rename, publishDiagnostics, foldingRange,
        selectionRange, linkedEditingRange, callHierarchy, semanticTokens, moniker);

struct ClientCapabilities
{
    struct Workspace
    {
        struct FileOperations
        {
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

    struct Window
    {
        std::optional<bool> workDoneProgress;
        std::optional<ShowMessageRequestClientCapabilities> showMessage;
        std::optional<ShowDocumentClientCapabilities> showDocument;
    };

    struct General
    {
        std::optional<RegularExpressionsClientCapabilities> regularExpressions;
        std::optional<MarkdownClientCapabilities> markdown;
    };

    std::optional<Workspace> workspace;
    std::optional<TextDocumentClientCapabilities> textDocument;
    std::optional<Window> window;
    std::optional<General> general;
    // TODO experimental?: any;
};
JSONIFY(ClientCapabilities::Workspace::FileOperations, dynamicRegistration, didCreate, willCreate, didRename,
        willRename, didDelete, willDelete);
JSONIFY(ClientCapabilities::Workspace, applyEdit, workspaceEdit, didChangeConfiguration, didChangeWatchedFiles, symbol,
        executeCommand, workspaceFolders, configuration, semanticTokens, codeLens, fileOperations);
JSONIFY(ClientCapabilities::Window, workDoneProgress, showMessage, showDocument);
JSONIFY(ClientCapabilities::General, regularExpressions, markdown);
JSONIFY(ClientCapabilities, workspace, textDocument, window, general);
}
