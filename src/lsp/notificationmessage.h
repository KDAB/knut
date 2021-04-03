#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <type_traits>

namespace Lsp {

/*!
 *  \brief Notification message (client or server
 */
template <const char *MethodName, typename NotificationParams>
struct NotificationMessage
{
    std::string jsonrpc = "2.0";
    std::string method = MethodName;
    // params could be optional, we are doing that by passing std::nullptr_t as the type
    NotificationParams params;
};

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
