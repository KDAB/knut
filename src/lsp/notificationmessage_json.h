#pragma once

#include "json.h"
#include "notificationmessage.h"

#include <type_traits>

namespace Lsp {

///////////////////////////////////////////////////////////////////////////////
// Serialization
///////////////////////////////////////////////////////////////////////////////
template <const char *MethodName, typename NotificationParams>
void to_json(nlohmann::json &j, const NotificationMessage<MethodName, NotificationParams> &notification)
{
    j = {{"jsonrpc", notification.jsonrpc}, {"method", notification.method}};
    if constexpr (!std::is_same_v<NotificationParams, std::nullptr_t>)
        j["params"] = notification.params;
}
template <const char *MethodName, typename NotificationParams>
void from_json(const nlohmann::json &j, NotificationMessage<MethodName, NotificationParams> &notification)
{
    j.at("jsonrpc").get_to(notification.jsonrpc);
    j.at("method").get_to(notification.method);
    if constexpr (!std::is_same_v<NotificationParams, std::nullptr_t>)
        j.at("params").get_to(notification.params);
}

}
