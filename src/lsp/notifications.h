#pragma once

#include "notificationmessage.h"
#include "types.h"

namespace Lsp {

inline constexpr char cancelRequestName[] = "$/cancelRequest";
struct CancelRequestNotification : public NotificationMessage<cancelRequestName, CancelParams>
{
};

inline constexpr char progressName[] = "$/progress";
struct ProgressNotification : public NotificationMessage<progressName, ProgressParams>
{
};

inline constexpr char initializedName[] = "initialized";
struct InitializedNotification : public NotificationMessage<initializedName, InitializedParams>
{
};

inline constexpr char exitName[] = "exit";
struct ExitNotification : public NotificationMessage<exitName, std::nullptr_t>
{
};

inline constexpr char logTraceName[] = "$/logTrace";
struct LogTraceNotification : public NotificationMessage<logTraceName, LogTraceParams>
{
};

inline constexpr char setTraceName[] = "$/setTrace";
struct SetTraceNotification : public NotificationMessage<setTraceName, SetTraceParams>
{
};

inline constexpr char showMessageName[] = "window/showMessage";
struct ShowMessageNotification : public NotificationMessage<showMessageName, ShowMessageParams>
{
};

inline constexpr char logMessageName[] = "window/logMessage";
struct LogMessageNotification : public NotificationMessage<logMessageName, LogMessageParams>
{
};

inline constexpr char cancelName[] = "window/workDoneProgress/cancel";
struct CancelNotification : public NotificationMessage<cancelName, WorkDoneProgressCancelParams>
{
};

inline constexpr char eventName[] = "telemetry/event";
struct EventNotification : public NotificationMessage<eventName, nlohmann::json>
{
};

inline constexpr char didChangeWorkspaceFoldersName[] = "workspace/didChangeWorkspaceFolders";
struct DidChangeWorkspaceFoldersNotification
    : public NotificationMessage<didChangeWorkspaceFoldersName, DidChangeWorkspaceFoldersParams>
{
};

inline constexpr char didChangeConfigurationName[] = "workspace/didChangeConfiguration";
struct DidChangeConfigurationNotification
    : public NotificationMessage<didChangeConfigurationName, DidChangeConfigurationParams>
{
};

inline constexpr char didChangeWatchedFilesName[] = "workspace/didChangeWatchedFiles";
struct DidChangeWatchedFilesNotification
    : public NotificationMessage<didChangeWatchedFilesName, DidChangeWatchedFilesParams>
{
};

inline constexpr char didCreateFilesName[] = "workspace/didCreateFiles";
struct DidCreateFilesNotification : public NotificationMessage<didCreateFilesName, CreateFilesParams>
{
};

inline constexpr char didRenameFilesName[] = "workspace/didRenameFiles";
struct DidRenameFilesNotification : public NotificationMessage<didRenameFilesName, RenameFilesParams>
{
};

inline constexpr char didDeleteFilesName[] = "workspace/didDeleteFiles";
struct DidDeleteFilesNotification : public NotificationMessage<didDeleteFilesName, DeleteFilesParams>
{
};

inline constexpr char didOpenName[] = "textDocument/didOpen";
struct DidOpenNotification : public NotificationMessage<didOpenName, DidOpenTextDocumentParams>
{
};

inline constexpr char didChangeName[] = "textDocument/didChange";
struct DidChangeNotification : public NotificationMessage<didChangeName, DidChangeTextDocumentParams>
{
};

inline constexpr char willSaveName[] = "textDocument/willSave";
struct WillSaveNotification : public NotificationMessage<willSaveName, WillSaveTextDocumentParams>
{
};

inline constexpr char didSaveName[] = "textDocument/didSave";
struct DidSaveNotification : public NotificationMessage<didSaveName, DidSaveTextDocumentParams>
{
};

inline constexpr char didCloseName[] = "textDocument/didClose";
struct DidCloseNotification : public NotificationMessage<didCloseName, DidCloseTextDocumentParams>
{
};

inline constexpr char publishDiagnosticsName[] = "textDocument/publishDiagnostics";
struct PublishDiagnosticsNotification : public NotificationMessage<publishDiagnosticsName, PublishDiagnosticsParams>
{
};
}
