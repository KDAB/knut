#pragma once

#include "lsp_types.h"
#include "notificationmessage.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialized
namespace Lsp {

struct InitializeParams
{
    struct ClientInfo
    {
        std::string name;
        std::optional<std::string> version;
    };

    IntegerOrNull processId = nullptr;
    std::optional<ClientInfo> clientInfo;
    std::optional<std::string> locale;
    // TODO user provided initializationOptions?: any;
    ClientCapabilities capabilities;
    std::optional<TraceValue> trace;
    std::optional<std::vector<WorkspaceFolder>> workspaceFolders; // deprecated not implemented rootPath and rootUri
};

struct InitializeResult
{
    struct ServerInfo
    {
        std::string name;
        std::optional<std::string> version;
    };

    // TODO capabilities
    std::optional<ServerInfo> serverInfo;
};

struct InitializeError
{
    bool retry;
};

inline constexpr char initializeName[] = "initialize";
struct InitializeRequest : public RequestMessage<initializeName, InitializeParams, InitializeResult, InitializeError>
{
};

inline constexpr char initializedName[] = "initialized";
struct InitializedNotification : public NotificationMessage<initializedName, std::nullptr_t>
{
};

///////////////////////////////////////////////////////////////////////////////
// Serialization
///////////////////////////////////////////////////////////////////////////////

void to_json(nlohmann::json &j, const ClientCapabilities::Workspace::FileOperations &object);
void from_json(const nlohmann::json &j, ClientCapabilities::Workspace::FileOperations &object);
void to_json(nlohmann::json &j, const ClientCapabilities::Workspace &object);
void from_json(const nlohmann::json &j, ClientCapabilities::Workspace &object);

void to_json(nlohmann::json &j, const DidChangeConfigurationClientCapabilities &object);
void from_json(const nlohmann::json &j, DidChangeConfigurationClientCapabilities &object);

void to_json(nlohmann::json &j, const DidChangeWatchedFilesClientCapabilities &object);
void from_json(const nlohmann::json &j, DidChangeWatchedFilesClientCapabilities &object);

void to_json(nlohmann::json &j, const WorkspaceSymbolClientCapabilities::SymbolKind &object);
void from_json(const nlohmann::json &j, WorkspaceSymbolClientCapabilities::SymbolKind &object);
void to_json(nlohmann::json &j, const WorkspaceSymbolClientCapabilities::TagSupport &object);
void from_json(const nlohmann::json &j, WorkspaceSymbolClientCapabilities::TagSupport &object);
void to_json(nlohmann::json &j, const WorkspaceSymbolClientCapabilities &object);
void from_json(const nlohmann::json &j, WorkspaceSymbolClientCapabilities &object);

void to_json(nlohmann::json &j, const ExecuteCommandClientCapabilities &object);
void from_json(const nlohmann::json &j, ExecuteCommandClientCapabilities &object);

void to_json(nlohmann::json &j, const SemanticTokensWorkspaceClientCapabilities &object);
void from_json(const nlohmann::json &j, SemanticTokensWorkspaceClientCapabilities &object);

void to_json(nlohmann::json &j, const CodeLensWorkspaceClientCapabilities &object);
void from_json(const nlohmann::json &j, CodeLensWorkspaceClientCapabilities &object);

void to_json(nlohmann::json &j, const ShowMessageRequestClientCapabilities::MessageActionItem &object);
void from_json(const nlohmann::json &j, ShowMessageRequestClientCapabilities::MessageActionItem &object);
void to_json(nlohmann::json &j, const ShowMessageRequestClientCapabilities &object);
void from_json(const nlohmann::json &j, ShowMessageRequestClientCapabilities &object);

void to_json(nlohmann::json &j, const ShowDocumentClientCapabilities &object);
void from_json(const nlohmann::json &j, ShowDocumentClientCapabilities &object);

void to_json(nlohmann::json &j, const RegularExpressionsClientCapabilities &object);
void from_json(const nlohmann::json &j, RegularExpressionsClientCapabilities &object);

void to_json(nlohmann::json &j, const TextDocumentSyncClientCapabilities &object);
void from_json(const nlohmann::json &j, TextDocumentSyncClientCapabilities &object);

void to_json(nlohmann::json &j, const CompletionClientCapabilities &object);
void from_json(const nlohmann::json &j, CompletionClientCapabilities &object);

void to_json(nlohmann::json &j, const HoverClientCapabilities &object);
void from_json(const nlohmann::json &j, HoverClientCapabilities &object);

void to_json(nlohmann::json &j,
             const SignatureHelpClientCapabilities::SignatueInformation::ParameterInformation &object);
void from_json(const nlohmann::json &j,
               SignatureHelpClientCapabilities::SignatueInformation::ParameterInformation &object);
void to_json(nlohmann::json &j, const SignatureHelpClientCapabilities::SignatueInformation &object);
void from_json(const nlohmann::json &j, SignatureHelpClientCapabilities::SignatueInformation &object);
void to_json(nlohmann::json &j, const SignatureHelpClientCapabilities &object);
void from_json(const nlohmann::json &j, SignatureHelpClientCapabilities &object);

void to_json(nlohmann::json &j, const DeclarationClientCapabilities &object);
void from_json(const nlohmann::json &j, DeclarationClientCapabilities &object);

void to_json(nlohmann::json &j, const DefinitionClientCapabilities &object);
void from_json(const nlohmann::json &j, DefinitionClientCapabilities &object);

void to_json(nlohmann::json &j, const TypeDefinitionClientCapabilities &object);
void from_json(const nlohmann::json &j, TypeDefinitionClientCapabilities &object);

void to_json(nlohmann::json &j, const ReferenceClientCapabilities &object);
void from_json(const nlohmann::json &j, ReferenceClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentHighlightClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentHighlightClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentSymbolClientCapabilities::SymbolKinds &object);
void from_json(const nlohmann::json &j, DocumentSymbolClientCapabilities::SymbolKinds &object);
void to_json(nlohmann::json &j, const DocumentSymbolClientCapabilities::TagSupport &object);
void from_json(const nlohmann::json &j, DocumentSymbolClientCapabilities::TagSupport &object);
void to_json(nlohmann::json &j, const DocumentSymbolClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentSymbolClientCapabilities &object);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CodeActionClientCapabilities::ResolveSupport, properties);
void to_json(nlohmann::json &j, const CodeActionClientCapabilities &object);
void from_json(const nlohmann::json &j, CodeActionClientCapabilities &object);

void to_json(nlohmann::json &j, const CodeLensClientCapabilities &object);
void from_json(const nlohmann::json &j, CodeLensClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentLinkClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentLinkClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentColorClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentColorClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentFormattingClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentFormattingClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentRangeFormattingClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentRangeFormattingClientCapabilities &object);

void to_json(nlohmann::json &j, const DocumentOnTypeFormattingClientCapabilities &object);
void from_json(const nlohmann::json &j, DocumentOnTypeFormattingClientCapabilities &object);

void to_json(nlohmann::json &j, const RenameClientCapabilities &object);
void from_json(const nlohmann::json &j, RenameClientCapabilities &object);

void to_json(nlohmann::json &j, const PublishDiagnosticsClientCapabilities &object);
void from_json(const nlohmann::json &j, PublishDiagnosticsClientCapabilities &object);

void to_json(nlohmann::json &j, const FoldingRangeClientCapabilities &object);
void from_json(const nlohmann::json &j, FoldingRangeClientCapabilities &object);

void to_json(nlohmann::json &j, const SelectionRangeClientCapabilities &object);
void from_json(const nlohmann::json &j, SelectionRangeClientCapabilities &object);

void to_json(nlohmann::json &j, const LinkedEditingRangeClientCapabilities &object);
void from_json(const nlohmann::json &j, LinkedEditingRangeClientCapabilities &object);

void to_json(nlohmann::json &j, const CallHierarchyClientCapabilities &object);
void from_json(const nlohmann::json &j, CallHierarchyClientCapabilities &object);

void to_json(nlohmann::json &j, const SemanticTokensClientCapabilities::Requests::Full &object);
void from_json(const nlohmann::json &j, SemanticTokensClientCapabilities::Requests::Full &object);
void to_json(nlohmann::json &j, const SemanticTokensClientCapabilities::Requests &object);
void from_json(const nlohmann::json &j, SemanticTokensClientCapabilities::Requests &object);
void to_json(nlohmann::json &j, const SemanticTokensClientCapabilities &object);
void from_json(const nlohmann::json &j, SemanticTokensClientCapabilities &object);

void to_json(nlohmann::json &j, const MonikerClientCapabilities &object);
void from_json(const nlohmann::json &j, MonikerClientCapabilities &object);

void to_json(nlohmann::json &j, const TextDocumentClientCapabilities &object);
void from_json(const nlohmann::json &j, TextDocumentClientCapabilities &object);

void to_json(nlohmann::json &j, const MarkdownClientCapabilities &object);
void from_json(const nlohmann::json &j, MarkdownClientCapabilities &object);

void to_json(nlohmann::json &j, const WorkspaceEditClientCapabilities::ChangeAnnotationSupport &object);
void from_json(const nlohmann::json &j, WorkspaceEditClientCapabilities::ChangeAnnotationSupport &object);
void to_json(nlohmann::json &j, const WorkspaceEditClientCapabilities &object);
void from_json(const nlohmann::json &j, WorkspaceEditClientCapabilities &object);

void to_json(nlohmann::json &j, const ClientCapabilities::Window &object);
void from_json(const nlohmann::json &j, ClientCapabilities::Window &object);
void to_json(nlohmann::json &j, const ClientCapabilities::General &object);
void from_json(const nlohmann::json &j, ClientCapabilities::General &object);
void to_json(nlohmann::json &j, const ClientCapabilities &object);
void from_json(const nlohmann::json &j, ClientCapabilities &object);

void to_json(nlohmann::json &j, const InitializeParams::ClientInfo &clientInfo);
void from_json(const nlohmann::json &j, InitializeParams::ClientInfo &clientInfo);
void to_json(nlohmann::json &j, const InitializeParams &request);
void from_json(const nlohmann::json &j, InitializeParams &request);

void to_json(nlohmann::json &j, const InitializeResult::ServerInfo &serverInfo);
void from_json(const nlohmann::json &j, InitializeResult::ServerInfo &serverInfo);
void to_json(nlohmann::json &j, const InitializeResult &result);
void from_json(const nlohmann::json &j, InitializeResult &result);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitializeError, retry);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WorkspaceFolder, uri, name);
}
