/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "requestmessage.h"
#include "utils/json.h"

#include <type_traits>

namespace Lsp {

template <typename ErrorData>
void to_json(nlohmann::json &j, const ResponseError<ErrorData> &responseError)
{
    j = {{"code", responseError.code}, {"message", responseError.message}};
    optional_to_json(j, "data", responseError.data);
}
template <typename ErrorData>
void from_json(const nlohmann::json &j, ResponseError<ErrorData> &responseError)
{
    j.at("code").get_to(responseError.code);
    j.at("message").get_to(responseError.message);
    optional_from_json(j, "data", responseError.data);
}

template <typename ResultData, typename ErrorData>
void to_json(nlohmann::json &j, const ResponseMessage<ResultData, ErrorData> &response)
{
    j = {{"jsonrpc", response.jsonrpc}, {"id", response.id}};
    optional_to_json(j, "result", response.result);
    optional_to_json(j, "error", response.error);
}
template <typename ResultData, typename ErrorData>
void from_json(const nlohmann::json &j, ResponseMessage<ResultData, ErrorData> &response)
{
    j.at("jsonrpc").get_to(response.jsonrpc);
    j.at("id").get_to(response.id);
    optional_from_json(j, "result", response.result);
    optional_from_json(j, "error", response.error);
}

template <const char *MethodName, typename RequestParams, typename ResultData, typename ErrorData>
void to_json(nlohmann::json &j, const RequestMessage<MethodName, RequestParams, ResultData, ErrorData> &request)
{
    j = {{"jsonrpc", request.jsonrpc}, {"id", request.id}, {"method", request.method}};
    if constexpr (!std::is_same_v<RequestParams, std::nullptr_t>)
        j["params"] = request.params;
}
template <const char *MethodName, typename RequestParams, typename ResultData, typename ErrorData>
void from_json(const nlohmann::json &j, RequestMessage<MethodName, RequestParams, ResultData, ErrorData> &request)
{
    j.at("jsonrpc").get_to(request.jsonrpc);
    j.at("id").get_to(request.id);
    j.at("method").get_to(request.method);
    if constexpr (!std::is_same_v<RequestParams, std::nullptr_t>)
        j.at("params").get_to(request.params);
}

}
