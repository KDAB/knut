#pragma once

#include "requestmessage.h"
#include "types.h"

namespace Lsp {

inline constexpr char initializeName[] = "initialize";
struct InitializeRequest : public RequestMessage<initializeName, InitializeParams, InitializeResult, InitializeError>
{
};

inline constexpr char shutdownName[] = "shutdown";
struct ShutdownRequest : public RequestMessage<shutdownName, std::nullptr_t, std::nullptr_t, std::nullptr_t>
{
};

inline constexpr char showMessageRequestName[] = "window/showMessageRequest";
struct ShowMessageRequestRequest
    : public RequestMessage<showMessageRequestName, ShowMessageRequestParams,
                            std::variant<MessageActionItem, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char showDocumentName[] = "window/showDocument";
struct ShowDocumentRequest
    : public RequestMessage<showDocumentName, ShowDocumentParams, ShowDocumentResult, std::nullptr_t>
{
};

inline constexpr char workDoneProgressCreateName[] = "window/workDoneProgress/create";
struct WorkDoneProgressCreateRequest
    : public RequestMessage<workDoneProgressCreateName, WorkDoneProgressCreateParams, std::nullptr_t, std::nullptr_t>
{
};

inline constexpr char registerCapabilityName[] = "client/registerCapability";
struct RegisterCapabilityRequest
    : public RequestMessage<registerCapabilityName, RegistrationParams, std::nullptr_t, std::nullptr_t>
{
};

inline constexpr char unregisterCapabilityName[] = "client/unregisterCapability";
struct UnregisterCapabilityRequest
    : public RequestMessage<unregisterCapabilityName, UnregistrationParams, std::nullptr_t, std::nullptr_t>
{
};

inline constexpr char workspaceFoldersName[] = "workspace/workspaceFolders";
struct WorkspaceFoldersRequest
    : public RequestMessage<workspaceFoldersName, std::nullptr_t,
                            std::variant<std::vector<WorkspaceFolder>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char configurationName[] = "workspace/configuration";
struct ConfigurationRequest
    : public RequestMessage<configurationName, ConfigurationParams, std::vector<nlohmann::json>, std::nullptr_t>
{
};

inline constexpr char symbolName[] = "workspace/symbol";
struct SymbolRequest
    : public RequestMessage<symbolName, WorkspaceSymbolParams,
                            std::variant<std::vector<SymbolInformation>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char applyEditName[] = "workspace/applyEdit";
struct ApplyEditRequest
    : public RequestMessage<applyEditName, ApplyWorkspaceEditParams, ApplyWorkspaceEditResponse, std::nullptr_t>
{
};

inline constexpr char willCreateFilesName[] = "workspace/willCreateFiles";
struct WillCreateFilesRequest : public RequestMessage<willCreateFilesName, CreateFilesParams,
                                                      std::variant<WorkspaceEdit, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char willRenameFilesName[] = "workspace/willRenameFiles";
struct WillRenameFilesRequest : public RequestMessage<willRenameFilesName, RenameFilesParams,
                                                      std::variant<WorkspaceEdit, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char willDeleteFilesName[] = "workspace/willDeleteFiles";
struct WillDeleteFilesRequest : public RequestMessage<willDeleteFilesName, DeleteFilesParams,
                                                      std::variant<WorkspaceEdit, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char willSaveWaitUntilName[] = "textDocument/willSaveWaitUntil";
struct WillSaveWaitUntilRequest
    : public RequestMessage<willSaveWaitUntilName, WillSaveTextDocumentParams,
                            std::variant<std::vector<TextEdit>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char completionName[] = "textDocument/completion";
struct CompletionRequest
    : public RequestMessage<completionName, CompletionParams,
                            std::variant<std::vector<CompletionItem>, CompletionList, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char completionItemResolveName[] = "completionItem/resolve";
struct CompletionItemResolveRequest
    : public RequestMessage<completionItemResolveName, CompletionItem, CompletionItem, std::nullptr_t>
{
};

inline constexpr char hoverName[] = "textDocument/hover";
struct HoverRequest : public RequestMessage<hoverName, HoverParams, std::variant<Hover, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char signatureHelpName[] = "textDocument/signatureHelp";
struct SignatureHelpRequest : public RequestMessage<signatureHelpName, SignatureHelpParams,
                                                    std::variant<SignatureHelp, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char declarationName[] = "textDocument/declaration";
struct DeclarationRequest
    : public RequestMessage<declarationName, DeclarationParams,
                            std::variant<Location, std::vector<Location>, std::vector<LocationLink>, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char definitionName[] = "textDocument/definition";
struct DefinitionRequest
    : public RequestMessage<definitionName, DefinitionParams,
                            std::variant<Location, std::vector<Location>, std::vector<LocationLink>, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char typeDefinitionName[] = "textDocument/typeDefinition";
struct TypeDefinitionRequest
    : public RequestMessage<typeDefinitionName, TypeDefinitionParams,
                            std::variant<Location, std::vector<Location>, std::vector<LocationLink>, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char implementationName[] = "textDocument/implementation";
struct ImplementationRequest
    : public RequestMessage<implementationName, ImplementationParams,
                            std::variant<Location, std::vector<Location>, std::vector<LocationLink>, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char referencesName[] = "textDocument/references";
struct ReferencesRequest : public RequestMessage<referencesName, ReferenceParams,
                                                 std::variant<std::vector<Location>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char documentHighlightName[] = "textDocument/documentHighlight";
struct DocumentHighlightRequest
    : public RequestMessage<documentHighlightName, DocumentHighlightParams,
                            std::variant<std::vector<DocumentHighlight>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char documentSymbolName[] = "textDocument/documentSymbol";
struct DocumentSymbolRequest
    : public RequestMessage<documentSymbolName, DocumentSymbolParams,
                            std::variant<std::vector<DocumentSymbol>, std::vector<SymbolInformation>, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char codeActionName[] = "textDocument/codeAction";
struct CodeActionRequest
    : public RequestMessage<codeActionName, CodeActionParams,
                            std::variant<std::vector<std::variant<Command, CodeAction>>, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char codeActionResolveName[] = "codeAction/resolve";
struct CodeActionResolveRequest : public RequestMessage<codeActionResolveName, CodeAction, CodeAction, std::nullptr_t>
{
};

inline constexpr char codeLensName[] = "textDocument/codeLens";
struct CodeLensRequest : public RequestMessage<codeLensName, CodeLensParams,
                                               std::variant<std::vector<CodeLens>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char codeLensResolveName[] = "codeLens/resolve";
struct CodeLensResolveRequest : public RequestMessage<codeLensResolveName, CodeLens, CodeLens, std::nullptr_t>
{
};

inline constexpr char codeLensRefreshName[] = "workspace/codeLens/refresh";
struct CodeLensRefreshRequest
    : public RequestMessage<codeLensRefreshName, std::nullptr_t, std::nullptr_t, std::nullptr_t>
{
};

inline constexpr char documentLinkName[] = "textDocument/documentLink";
struct DocumentLinkRequest
    : public RequestMessage<documentLinkName, DocumentLinkParams,
                            std::variant<std::vector<DocumentLink>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char documentLinkResolveName[] = "documentLink/resolve";
struct DocumentLinkResolveRequest
    : public RequestMessage<documentLinkResolveName, DocumentLink, DocumentLink, std::nullptr_t>
{
};

inline constexpr char documentColorName[] = "textDocument/documentColor";
struct DocumentColorRequest
    : public RequestMessage<documentColorName, DocumentColorParams, std::vector<ColorInformation>, std::nullptr_t>
{
};

inline constexpr char colorPresentationName[] = "textDocument/colorPresentation";
struct ColorPresentationRequest : public RequestMessage<colorPresentationName, ColorPresentationParams,
                                                        std::vector<ColorPresentation>, std::nullptr_t>
{
};

inline constexpr char formattingName[] = "textDocument/formatting";
struct FormattingRequest : public RequestMessage<formattingName, DocumentFormattingParams,
                                                 std::variant<std::vector<TextEdit>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char rangeFormattingName[] = "textDocument/rangeFormatting";
struct RangeFormattingRequest
    : public RequestMessage<rangeFormattingName, DocumentRangeFormattingParams,
                            std::variant<std::vector<TextEdit>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char onTypeFormattingName[] = "textDocument/onTypeFormatting";
struct OnTypeFormattingRequest
    : public RequestMessage<onTypeFormattingName, DocumentOnTypeFormattingParams,
                            std::variant<std::vector<TextEdit>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char renameName[] = "textDocument/rename";
struct RenameRequest
    : public RequestMessage<renameName, RenameParams, std::variant<WorkspaceEdit, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char prepareRenameName[] = "textDocument/prepareRename";
struct PrepareRenameRequest
    : public RequestMessage<prepareRenameName, PrepareRenameParams,
                            std::variant<Range, RenamePlaceholder, RenameDefaultBehavior, std::nullptr_t>,
                            std::nullptr_t>
{
};

inline constexpr char foldingRangeName[] = "textDocument/foldingRange";
struct FoldingRangeRequest
    : public RequestMessage<foldingRangeName, FoldingRangeParams,
                            std::variant<std::vector<FoldingRange>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char selectionRangeName[] = "textDocument/selectionRange";
struct SelectionRangeRequest
    : public RequestMessage<selectionRangeName, SelectionRangeParams,
                            std::variant<std::vector<SelectionRange>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char prepareCallHierarchyName[] = "textDocument/prepareCallHierarchy";
struct PrepareCallHierarchyRequest
    : public RequestMessage<prepareCallHierarchyName, CallHierarchyPrepareParams,
                            std::variant<std::vector<CallHierarchyItem>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char callHierarchyIncomingCallsName[] = "callHierarchy/incomingCalls";
struct CallHierarchyIncomingCallsRequest
    : public RequestMessage<callHierarchyIncomingCallsName, CallHierarchyIncomingCallsParams,
                            std::variant<std::vector<CallHierarchyIncomingCall>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char callHierarchyOutgoingCallsName[] = "callHierarchy/outgoingCalls";
struct CallHierarchyOutgoingCallsRequest
    : public RequestMessage<callHierarchyOutgoingCallsName, CallHierarchyOutgoingCallsParams,
                            std::variant<std::vector<CallHierarchyOutgoingCall>, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char semanticTokensFullName[] = "textDocument/semanticTokens/full";
struct SemanticTokensFullRequest : public RequestMessage<semanticTokensFullName, SemanticTokensParams,
                                                         std::variant<SemanticTokens, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char semanticTokensFullDeltaName[] = "textDocument/semanticTokens/full/delta";
struct SemanticTokensFullDeltaRequest
    : public RequestMessage<semanticTokensFullDeltaName, SemanticTokensDeltaParams,
                            std::variant<SemanticTokens, SemanticTokensDelta, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char semanticTokensRangeName[] = "textDocument/semanticTokens/range";
struct SemanticTokensRangeRequest : public RequestMessage<semanticTokensRangeName, SemanticTokensRangeParams,
                                                          std::variant<SemanticTokens, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char semanticTokensRefreshName[] = "workspace/semanticTokens/refresh";
struct SemanticTokensRefreshRequest
    : public RequestMessage<semanticTokensRefreshName, std::nullptr_t, std::nullptr_t, std::nullptr_t>
{
};

inline constexpr char linkedEditingRangeName[] = "textDocument/linkedEditingRange";
struct LinkedEditingRangeRequest
    : public RequestMessage<linkedEditingRangeName, LinkedEditingRangeParams,
                            std::variant<LinkedEditingRanges, std::nullptr_t>, std::nullptr_t>
{
};

inline constexpr char monikerName[] = "textDocument/moniker";
struct MonikerRequest : public RequestMessage<monikerName, MonikerParams,
                                              std::variant<std::vector<Moniker>, std::nullptr_t>, std::nullptr_t>
{
};
}
