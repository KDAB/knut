#pragma once

#include "message_types.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <optional>
#include <string>

namespace Lsp {

/*!
 *  \brief Error message in the response message, following a request
 */
template <typename ErrorData>
struct ResponseError
{
    int code;
    std::string message;
    std::optional<ErrorData> data;
};

template <typename ErrorData>
void to_json(nlohmann::json &j, const ResponseError<ErrorData> &responseError)
{
    j = {{"code", responseError.code}, {"message", responseError.message}};
    if (responseError.data)
        j["data"] = responseError.data.value();
}
template <typename ErrorData>
void from_json(const nlohmann::json &j, ResponseError<ErrorData> &responseError)
{
    j.at("code").get_to(responseError.code);
    j.at("message").get_to(responseError.message);
    if (j.contains("data"))
        responseError.data = j.at("data").get<ErrorData>();
}

/*!
 *  \brief Response message following a request
 */
template <typename ResultData, typename ErrorData>
struct ResponseMessage
{
    std::string jsonrpc = "2.0";
    MessageId id;
    std::optional<ResultData> result;
    std::optional<ResponseError<ErrorData>> error;
};

template <typename ResultData, typename ErrorData>
void to_json(nlohmann::json &j, const ResponseMessage<ResultData, ErrorData> &response)
{
    j = {{"jsonrpc", response.jsonrpc}, {"id", response.id}};
    if (response.result)
        j["result"] = response.result.value();
    if (response.error)
        j["error"] = response.error.value();
}
template <typename ResultData, typename ErrorData>
void from_json(const nlohmann::json &j, ResponseMessage<ResultData, ErrorData> &response)
{
    j.at("jsonrpc").get_to(response.jsonrpc);
    j.at("id").get_to(response.id);
    if (j.contains("result"))
        response.result = j.at("result").get<ResultData>();
    if (j.contains("error"))
        response.error = j.at("error").get<ResponseError<ErrorData>>();
}

/*!
 *  \brief Request message sent to the server
 */
template <const char *MethodName, typename RequestParams, typename ResultData, typename ErrorData>
struct RequestMessage
{
    std::string jsonrpc = "2.0";
    MessageId id;
    std::string method = MethodName;
    RequestParams params;

    using Response = ResponseMessage<ResultData, ErrorData>;
    using ResponseCallback = std::function<void(Response)>;
};

template <const char *MethodName, typename RequestParams, typename ResultData, typename ErrorData>
void to_json(nlohmann::json &j, const RequestMessage<MethodName, RequestParams, ResultData, ErrorData> &request)
{
    j = {{"jsonrpc", request.jsonrpc}, {"id", request.id}, {"method", request.method}, {"params", request.params}};
}
template <const char *MethodName, typename RequestParams, typename ResultData, typename ErrorData>
void from_json(const nlohmann::json &j, RequestMessage<MethodName, RequestParams, ResultData, ErrorData> &request)
{
    j.at("jsonrpc").get_to(request.jsonrpc);
    j.at("id").get_to(request.id);
    j.at("method").get_to(request.method);
    j.at("params").get_to(request.params);
}

}
