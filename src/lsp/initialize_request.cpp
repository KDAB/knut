#include "initialize_request.h"

#include "utils/json_utils.h"

namespace Lsp {

void to_json(nlohmann::json &j, const InitializeParams::ClientInfo &clientInfo)
{
    j = {{"name", clientInfo.name}};
    optional_to_json(j, "version", clientInfo.version);
}

void from_json(const nlohmann::json &j, InitializeParams::ClientInfo &clientInfo)
{
    j.at("name").get_to(clientInfo.name);
    optional_from_json(j, "version", clientInfo.version);
}

void to_json(nlohmann::json &j, const InitializeResult::ServerInfo &serverInfo)
{
    j = {{"name", serverInfo.name}};
    optional_to_json(j, "version", serverInfo.version);
}

void from_json(const nlohmann::json &j, InitializeResult::ServerInfo &serverInfo)
{
    j.at("name").get_to(serverInfo.name);
    optional_from_json(j, "version", serverInfo.version);
}

void to_json(nlohmann::json &j, const InitializeResult &result)
{
    optional_to_json(j, "serverInfo", result.serverInfo);
}

void from_json(const nlohmann::json &j, InitializeResult &result)
{
    optional_from_json(j, "serverInfo", result.serverInfo);
}

void to_json(nlohmann::json &j, const InitializeParams &request)
{
    j = {{"processId", request.processId},
         {"capabilities", request.capabilities}};
    optional_to_json(j, "clientInfo", request.clientInfo);
    optional_to_json(j, "locale", request.locale);
    optional_enum_to_json(j, "trace", request.trace);
    optional_to_json(j, "workspaceFolders", request.workspaceFolders);
}

void from_json(const nlohmann::json &j, InitializeParams &request)
{
    j.at("processId").get_to(request.processId);
    j.at("capabilities").get_to(request.capabilities);
    optional_from_json(j, "clientInfo", request.clientInfo);
    optional_from_json(j, "locale", request.locale);
    optional_enum_from_json(j, "trace", request.trace);
    optional_from_json(j, "workspaceFolders", request.workspaceFolders);
}

void to_json(nlohmann::json &j, const ClientCapabilities &object)
{
    optional_to_json(j, "workspace", object.workspace);
    optional_to_json(j, "textDocument", object.textDocument);
    optional_to_json(j, "window", object.window);
    optional_to_json(j, "general", object.general);
}

void from_json(const nlohmann::json &j, ClientCapabilities &object)
{
    optional_from_json(j, "workspace", object.workspace);
    optional_from_json(j, "textDocument", object.textDocument);
    optional_from_json(j, "window", object.window);
    optional_from_json(j, "general", object.general);
}

void to_json(nlohmann::json &j, const ClientCapabilities::Workspace &object)
{
    optional_to_json(j, "applyEdit", object.applyEdit);
    optional_to_json(j, "workspaceEdit", object.workspaceEdit);
    optional_to_json(j, "didChangeConfiguration", object.didChangeConfiguration);
    optional_to_json(j, "didChangeWatchedFiles", object.didChangeWatchedFiles);
    optional_to_json(j, "symbol", object.symbol);
    optional_to_json(j, "executeCommand", object.executeCommand);
    optional_to_json(j, "workspaceFolders", object.workspaceFolders);
    optional_to_json(j, "configuration", object.configuration);
    optional_to_json(j, "semanticTokens", object.semanticTokens);
    optional_to_json(j, "codeLens", object.codeLens);
    optional_to_json(j, "fileOperations", object.fileOperations);
}

void from_json(const nlohmann::json &j, ClientCapabilities::Workspace &object)
{
    optional_from_json(j, "applyEdit", object.applyEdit);
    optional_from_json(j, "workspaceEdit", object.workspaceEdit);
    optional_from_json(j, "didChangeConfiguration", object.didChangeConfiguration);
    optional_from_json(j, "didChangeWatchedFiles", object.didChangeWatchedFiles);
    optional_from_json(j, "symbol", object.symbol);
    optional_from_json(j, "executeCommand", object.executeCommand);
    optional_from_json(j, "workspaceFolders", object.workspaceFolders);
    optional_from_json(j, "configuration", object.configuration);
    optional_from_json(j, "semanticTokens", object.semanticTokens);
    optional_from_json(j, "codeLens", object.codeLens);
    optional_from_json(j, "fileOperations", object.fileOperations);
}

void to_json(nlohmann::json &j, const ClientCapabilities::Workspace::FileOperations &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "didCreate", object.didCreate);
    optional_to_json(j, "willCreate", object.willCreate);
    optional_to_json(j, "didRename", object.didRename);
    optional_to_json(j, "willRename", object.willRename);
    optional_to_json(j, "didDelete", object.didDelete);
    optional_to_json(j, "willDelete", object.willDelete);
}

void from_json(const nlohmann::json &j, ClientCapabilities::Workspace::FileOperations &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "didCreate", object.didCreate);
    optional_from_json(j, "willCreate", object.willCreate);
    optional_from_json(j, "didRename", object.didRename);
    optional_from_json(j, "willRename", object.willRename);
    optional_from_json(j, "didDelete", object.didDelete);
    optional_from_json(j, "willDelete", object.willDelete);
}

void to_json(nlohmann::json &j, const WorkspaceEditClientCapabilities::ChangeAnnotationSupport &object)
{
    optional_to_json(j, "groupsOnLabel", object.groupsOnLabel);
}

void from_json(const nlohmann::json &j, WorkspaceEditClientCapabilities::ChangeAnnotationSupport &object)
{
    optional_from_json(j, "groupsOnLabel", object.groupsOnLabel);
}

void to_json(nlohmann::json &j, const WorkspaceEditClientCapabilities &object)
{
    optional_to_json(j, "documentChanges", object.documentChanges);
    optional_to_json(j, "resourceOperations", object.resourceOperations);
    optional_to_json(j, "failureHandling", object.failureHandling);
    optional_to_json(j, "normalizesLineEndings", object.normalizesLineEndings);
}

void from_json(const nlohmann::json &j, WorkspaceEditClientCapabilities &object)
{
    optional_from_json(j, "documentChanges", object.documentChanges);
    optional_from_json(j, "resourceOperations", object.resourceOperations);
    optional_from_json(j, "failureHandling", object.failureHandling);
    optional_from_json(j, "normalizesLineEndings", object.normalizesLineEndings);
}

void to_json(nlohmann::json &j, const DidChangeConfigurationClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DidChangeConfigurationClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DidChangeWatchedFilesClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DidChangeWatchedFilesClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const WorkspaceSymbolClientCapabilities::SymbolKind &object)
{
    optional_to_json(j, "valueSet", object.valueSet);

}

void from_json(const nlohmann::json &j, WorkspaceSymbolClientCapabilities::SymbolKind &object)
{
    optional_from_json(j, "valueSet", object.valueSet);
}

void to_json(nlohmann::json &j, const WorkspaceSymbolClientCapabilities::TagSupport &object)
{
    optional_to_json(j, "valueSet", object.valueSet);
}

void from_json(const nlohmann::json &j, WorkspaceSymbolClientCapabilities::TagSupport &object)
{
    optional_from_json(j, "valueSet", object.valueSet);
}

void to_json(nlohmann::json &j, const WorkspaceSymbolClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, WorkspaceSymbolClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const ExecuteCommandClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, ExecuteCommandClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const SemanticTokensWorkspaceClientCapabilities &object)
{
    optional_to_json(j, "refreshSupport", object.refreshSupport);
}

void from_json(const nlohmann::json &j, SemanticTokensWorkspaceClientCapabilities &object)
{
    optional_from_json(j, "refreshSupport", object.refreshSupport);
}

void to_json(nlohmann::json &j, const CodeLensWorkspaceClientCapabilities &object)
{
    optional_to_json(j, "refreshSupport", object.refreshSupport);
}

void from_json(const nlohmann::json &j, CodeLensWorkspaceClientCapabilities &object)
{
    optional_from_json(j, "refreshSupport", object.refreshSupport);
}

void to_json(nlohmann::json &j, const ShowMessageRequestClientCapabilities::MessageActionItem &object)
{
    optional_to_json(j, "additionalPropertiesSupport", object.additionalPropertiesSupport);
}

void from_json(const nlohmann::json &j, ShowMessageRequestClientCapabilities::MessageActionItem &object)
{
    optional_from_json(j, "additionalPropertiesSupport", object.additionalPropertiesSupport);
}

void to_json(nlohmann::json &j, const ShowMessageRequestClientCapabilities &object)
{
    optional_to_json(j, "messageActionItem", object.messageActionItem);
}

void from_json(const nlohmann::json &j, ShowMessageRequestClientCapabilities &object)
{
    optional_from_json(j, "messageActionItem", object.messageActionItem);
}

void to_json(nlohmann::json &j, const ShowDocumentClientCapabilities &object)
{
    optional_to_json(j, "support", object.support);
}

void from_json(const nlohmann::json &j, ShowDocumentClientCapabilities &object)
{
    optional_from_json(j, "support", object.support);

}

void to_json(nlohmann::json &j, const RegularExpressionsClientCapabilities &object)
{
    j = { {"engine", object.engine} };
    optional_to_json(j, "version", object.version);
}

void from_json(const nlohmann::json &j, RegularExpressionsClientCapabilities &object)
{
    j.at("engine").get_to(object.engine);
    optional_from_json(j, "version", object.version);
}

void to_json(nlohmann::json &j, const MarkdownClientCapabilities &object)
{
    j = { {"parser", object.parser} };
    optional_to_json(j, "version", object.version);
}

void from_json(const nlohmann::json &j, MarkdownClientCapabilities &object)
{
    j.at("parser").get_to(object.parser);
    optional_from_json(j, "version", object.version);
}

void to_json(nlohmann::json &j, const TextDocumentSyncClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "willSave", object.willSave);
    optional_to_json(j, "willSaveWaitUntil", object.willSaveWaitUntil);
    optional_to_json(j, "didSave", object.didSave);
}

void from_json(const nlohmann::json &j, TextDocumentSyncClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "willSave", object.willSave);
    optional_from_json(j, "willSaveWaitUntil", object.willSaveWaitUntil);
    optional_from_json(j, "didSave", object.didSave);

}

void to_json(nlohmann::json &j, const CompletionClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    // TODO optional_to_json(j, "completionItem", object.completionItem);
    // TODO optional_to_json(j, "completionItemKind", object.completionItemKind);
    optional_to_json(j, "contextSupport", object.contextSupport);
}

void from_json(const nlohmann::json &j, CompletionClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    //TODO optional_from_json(j, "completionItem", object.completionItem);
    //TODO optional_from_json(j, "completionItemKind", object.completionItemKind);
    optional_from_json(j, "contextSupport", object.contextSupport);
}

void to_json(nlohmann::json &j, const HoverClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "contentFormat", object.contentFormat);
}

void from_json(const nlohmann::json &j, HoverClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "contentFormat", object.contentFormat);
}

void to_json(nlohmann::json &j, const SignatureHelpClientCapabilities::SignatueInformation::ParameterInformation &object)
{
    optional_to_json(j, "labelOffsetSupport", object.labelOffsetSupport);
}

void from_json(const nlohmann::json &j, SignatureHelpClientCapabilities::SignatueInformation::ParameterInformation &object)
{
    optional_from_json(j, "labelOffsetSupport", object.labelOffsetSupport);
}

void to_json(nlohmann::json &j, const SignatureHelpClientCapabilities::SignatueInformation &object)
{
    optional_to_json(j, "documentationFormat", object.documentationFormat);
    optional_to_json(j, "activeParameterSupport", object.activeParameterSupport);
}

void from_json(const nlohmann::json &j, SignatureHelpClientCapabilities::SignatueInformation &object)
{
    optional_from_json(j, "documentationFormat", object.documentationFormat);
    optional_from_json(j, "activeParameterSupport", object.activeParameterSupport);
}

void to_json(nlohmann::json &j, const SignatureHelpClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "contextSupport", object.contextSupport);
}

void from_json(const nlohmann::json &j, SignatureHelpClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "contextSupport", object.contextSupport);
}

void to_json(nlohmann::json &j, const DeclarationClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "linkSupport", object.linkSupport);
}

void from_json(const nlohmann::json &j, DeclarationClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "linkSupport", object.linkSupport);
}

void to_json(nlohmann::json &j, const DefinitionClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "linkSupport", object.linkSupport);
}

void from_json(const nlohmann::json &j, DefinitionClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "linkSupport", object.linkSupport);
}

void to_json(nlohmann::json &j, const TypeDefinitionClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "linkSupport", object.linkSupport);
}

void from_json(const nlohmann::json &j, TypeDefinitionClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "linkSupport", object.linkSupport);
}

void to_json(nlohmann::json &j, const ReferenceClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, ReferenceClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DocumentHighlightClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DocumentHighlightClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DocumentSymbolClientCapabilities::SymbolKinds &object)
{
    optional_to_json(j, "valueSet", object.valueSet);
}

void from_json(const nlohmann::json &j, DocumentSymbolClientCapabilities::SymbolKinds &object)
{
    optional_from_json(j, "valueSet", object.valueSet);
}

void to_json(nlohmann::json &j, const DocumentSymbolClientCapabilities::TagSupport &object)
{
    optional_to_json(j, "valueSet", object.valueSet);
}

void from_json(const nlohmann::json &j, DocumentSymbolClientCapabilities::TagSupport &object)
{
    optional_from_json(j, "valueSet", object.valueSet);
}

void to_json(nlohmann::json &j, const DocumentSymbolClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "symbolKind", object.symbolKind);
    optional_to_json(j, "hierarchicalDocumentSymbolSupport", object.hierarchicalDocumentSymbolSupport);
    optional_to_json(j, "tagSupport", object.tagSupport);
}

void from_json(const nlohmann::json &j, DocumentSymbolClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "symbolKind", object.symbolKind);
    optional_from_json(j, "hierarchicalDocumentSymbolSupport", object.hierarchicalDocumentSymbolSupport);
    optional_from_json(j, "tagSupport", object.tagSupport);
}

void to_json(nlohmann::json &j, const CodeActionClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    // TODO optional_to_json(j, "codeActionLiteralSupport", object.codeActionLiteralSupport);
    optional_to_json(j, "isPreferredSupport", object.isPreferredSupport);
    optional_to_json(j, "disabledSupport", object.disabledSupport);
    optional_to_json(j, "dataSupport", object.dataSupport);
    optional_to_json(j, "resolveSupport", object.resolveSupport);
    optional_to_json(j, "honorsChangeAnnotations", object.honorsChangeAnnotations);
}

void from_json(const nlohmann::json &j, CodeActionClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    // TODO optional_from_json(j, "codeActionLiteralSupport", object.codeActionLiteralSupport);
    optional_from_json(j, "isPreferredSupport", object.isPreferredSupport);
    optional_from_json(j, "disabledSupport", object.disabledSupport);
    optional_from_json(j, "dataSupport", object.dataSupport);
    optional_from_json(j, "resolveSupport", object.resolveSupport);
    optional_from_json(j, "honorsChangeAnnotations", object.honorsChangeAnnotations);
}

void to_json(nlohmann::json &j, const CodeLensClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, CodeLensClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DocumentLinkClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "tooltipSupport", object.tooltipSupport);
}

void from_json(const nlohmann::json &j, DocumentLinkClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "tooltipSupport", object.tooltipSupport);
}

void to_json(nlohmann::json &j, const DocumentColorClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DocumentColorClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DocumentFormattingClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DocumentFormattingClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DocumentRangeFormattingClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DocumentRangeFormattingClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const DocumentOnTypeFormattingClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, DocumentOnTypeFormattingClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const RenameClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "prepareSupport", object.prepareSupport);
    //TODO optional_to_json(j, "prepareSupportDefaultBehavior", object.prepareSupportDefaultBehavior);
    optional_to_json(j, "honorsChangeAnnotations", object.honorsChangeAnnotations);
}

void from_json(const nlohmann::json &j, RenameClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "prepareSupport", object.prepareSupport);
    //TODO optional_from_json(j, "prepareSupportDefaultBehavior", object.prepareSupportDefaultBehavior);
    optional_from_json(j, "honorsChangeAnnotations", object.honorsChangeAnnotations);
}

void to_json(nlohmann::json &j, const PublishDiagnosticsClientCapabilities &object)
{
    optional_to_json(j, "relatedInformation", object.relatedInformation);
    //TODO optional_to_json(j, "tagSupport", object.tagSupport);
    optional_to_json(j, "versionSupport", object.versionSupport);
    optional_to_json(j, "codeDescriptionSupport", object.codeDescriptionSupport);
    optional_to_json(j, "dataSupport", object.dataSupport);
}

void from_json(const nlohmann::json &j, PublishDiagnosticsClientCapabilities &object)
{
    optional_from_json(j, "relatedInformation", object.relatedInformation);
    //TODO optional_from_json(j, "tagSupport", object.tagSupport);
    optional_from_json(j, "versionSupport", object.versionSupport);
    optional_from_json(j, "codeDescriptionSupport", object.codeDescriptionSupport);
    optional_from_json(j, "dataSupport", object.dataSupport);
}

void to_json(nlohmann::json &j, const FoldingRangeClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "rangeLimit", object.rangeLimit);
    optional_to_json(j, "lineFoldingOnly", object.lineFoldingOnly);
}

void from_json(const nlohmann::json &j, FoldingRangeClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "rangeLimit", object.rangeLimit);
    optional_from_json(j, "lineFoldingOnly", object.lineFoldingOnly);
}

void to_json(nlohmann::json &j, const SelectionRangeClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, SelectionRangeClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const LinkedEditingRangeClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, LinkedEditingRangeClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const CallHierarchyClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, CallHierarchyClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const SemanticTokensClientCapabilities::Requests::Full &object)
{
    optional_to_json(j, "delta", object.delta);
}

void from_json(const nlohmann::json &j, SemanticTokensClientCapabilities::Requests::Full &object)
{
    optional_from_json(j, "delta", object.delta);
}

void to_json(nlohmann::json &j, const SemanticTokensClientCapabilities::Requests &object)
{
    optional_to_json(j, "range", object.range);
//TODO variant serialization    optional_to_json(j, "full", object.full);
}

void from_json(const nlohmann::json &j, SemanticTokensClientCapabilities::Requests &object)
{
    optional_from_json(j, "range", object.range);
//TODO variant serialization    optional_from_json(j, "full", object.full);
}

void to_json(nlohmann::json &j, const SemanticTokensClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_to_json(j, "requests", object.requests);
    optional_to_json(j, "overlappingTokenSupport", object.overlappingTokenSupport);
    optional_to_json(j, "multilineTokenSupport", object.multilineTokenSupport);
}

void from_json(const nlohmann::json &j, SemanticTokensClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
    optional_from_json(j, "requests", object.requests);
    optional_from_json(j, "overlappingTokenSupport", object.overlappingTokenSupport);
    optional_from_json(j, "multilineTokenSupport", object.multilineTokenSupport);
}

void to_json(nlohmann::json &j, const MonikerClientCapabilities &object)
{
    optional_to_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void from_json(const nlohmann::json &j, MonikerClientCapabilities &object)
{
    optional_from_json(j, "dynamicRegistration", object.dynamicRegistration);
}

void to_json(nlohmann::json &j, const TextDocumentClientCapabilities &object)
{
    optional_to_json(j, "synchronization", object.synchronization);
    optional_to_json(j, "completion", object.completion);
    optional_to_json(j, "hover", object.hover);
    optional_to_json(j, "signatureHelp", object.signatureHelp);
    optional_to_json(j, "declaration", object.declaration);
    optional_to_json(j, "definition", object.definition);
    optional_to_json(j, "typeDefinition", object.typeDefinition);
    optional_to_json(j, "references", object.references);
    optional_to_json(j, "documentHighlight", object.documentHighlight);
    optional_to_json(j, "documentSymbol", object.documentSymbol);
    optional_to_json(j, "codeAction", object.codeAction);
    optional_to_json(j, "codeLens", object.codeLens);
    optional_to_json(j, "documentLink", object.documentLink);
    optional_to_json(j, "colorProvider", object.colorProvider);
    optional_to_json(j, "formatting", object.formatting);
    optional_to_json(j, "rangeFormatting", object.rangeFormatting);
    optional_to_json(j, "onTypeFormatting", object.onTypeFormatting);
    optional_to_json(j, "rename", object.rename);
    optional_to_json(j, "publishDiagnostics", object.publishDiagnostics);
    optional_to_json(j, "foldingRange", object.foldingRange);
    optional_to_json(j, "selectionRange", object.selectionRange);
    optional_to_json(j, "linkedEditingRange", object.linkedEditingRange);
    optional_to_json(j, "callHierarchy", object.callHierarchy);
    optional_to_json(j, "semanticTokens", object.semanticTokens);
    optional_to_json(j, "moniker", object.moniker);
}

void from_json(const nlohmann::json &j, TextDocumentClientCapabilities &object)
{
    optional_from_json(j, "synchronization", object.synchronization);
    optional_from_json(j, "completion", object.completion);
    optional_from_json(j, "hover", object.hover);
    optional_from_json(j, "signatureHelp", object.signatureHelp);
    optional_from_json(j, "declaration", object.declaration);
    optional_from_json(j, "definition", object.definition);
    optional_from_json(j, "typeDefinition", object.typeDefinition);
    optional_from_json(j, "references", object.references);
    optional_from_json(j, "documentHighlight", object.documentHighlight);
    optional_from_json(j, "documentSymbol", object.documentSymbol);
    optional_from_json(j, "codeAction", object.codeAction);
    optional_from_json(j, "codeLens", object.codeLens);
    optional_from_json(j, "documentLink", object.documentLink);
    optional_from_json(j, "colorProvider", object.colorProvider);
    optional_from_json(j, "formatting", object.formatting);
    optional_from_json(j, "rangeFormatting", object.rangeFormatting);
    optional_from_json(j, "onTypeFormatting", object.onTypeFormatting);
    optional_from_json(j, "rename", object.rename);
    optional_from_json(j, "publishDiagnostics", object.publishDiagnostics);
    optional_from_json(j, "foldingRange", object.foldingRange);
    optional_from_json(j, "selectionRange", object.selectionRange);
    optional_from_json(j, "linkedEditingRange", object.linkedEditingRange);
    optional_from_json(j, "callHierarchy", object.callHierarchy);
    optional_from_json(j, "semanticTokens", object.semanticTokens);
    optional_from_json(j, "moniker", object.moniker);
}

void to_json(nlohmann::json &j, const ClientCapabilities::Window &object)
{
    optional_to_json(j, "workDoneProgress", object.workDoneProgress);
    optional_to_json(j, "showMessage", object.showMessage);
    optional_to_json(j, "showDocument", object.showDocument);
}

void from_json(const nlohmann::json &j, ClientCapabilities::Window &object)
{
    optional_from_json(j, "workDoneProgress", object.workDoneProgress);
    optional_from_json(j, "showMessage", object.showMessage);
    optional_from_json(j, "showDocument", object.showDocument);
}

void to_json(nlohmann::json &j, const ClientCapabilities::General &object)
{
    optional_to_json(j, "regularExpressions", object.regularExpressions);
    optional_to_json(j, "markdown", object.markdown);
}

void from_json(const nlohmann::json &j, ClientCapabilities::General &object)
{
    optional_from_json(j, "regularExpressions", object.regularExpressions);
    optional_from_json(j, "markdown", object.markdown);
}


}
