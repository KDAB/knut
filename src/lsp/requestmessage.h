#pragma once

#include "message_types.h"

#include <nlohmann/json.hpp>

#include <string>

namespace Lsp {

template <const char *MethodName, typename RequestParams>
struct RequestMessage
{
    std::string jsonrpc = "2.0";
    MessageId id;
    std::string method = MethodName;
    RequestParams params;
};

template <const char *MethodName, typename RequestParams>
static void to_json(nlohmann::json &j, const RequestMessage<MethodName, RequestParams> &data)
{
    j = {{"jsonrpc", data.jsonrpc}, {"id", data.id}, {"method", data.method}, {"params", data.params}};
}
template <const char *MethodName, typename RequestParams>
static void from_json(const nlohmann::json &j, RequestMessage<MethodName, RequestParams> &data)
{
    j.at("jsonrpc").get_to(data.jsonrpc);
    j.at("id").get_to(data.id);
    j.at("method").get_to(data.method);
    j.at("params").get_to(data.params);
}

}
