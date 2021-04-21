#pragma once

#include "utils/json_utils.h"
#include "types.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

namespace Lsp {

using MessageId = std::variant<int, std::string>;

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

/*!
 *  \brief Response message following a request
 */
template <typename ResultData, typename ErrorData>
struct ResponseMessage
{
    std::string jsonrpc = "2.0";
    std::variant<int, std::string, std::nullptr_t> id;
    std::optional<ResultData> result;
    std::optional<ResponseError<ErrorData>> error;

    bool isValid() const
    {
        // We can either have a result or an error, but not both
        return (result && !error) || (!result && error);
    }
};

/*!
 *  \brief Request message sent to the server
 */
template <const char *MethodName, typename RequestParams, typename ResultData, typename ErrorData>
struct RequestMessage
{
    std::string jsonrpc = "2.0";
    MessageId id = nextId++;
    std::string method = MethodName;
    // params could be optional, we are doing that by passing std::nullptr_t as the type
    RequestParams params;

    using Response = ResponseMessage<ResultData, ErrorData>;
    using ResponseCallback = std::function<void(Response)>;

    inline static int nextId = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Serialization
///////////////////////////////////////////////////////////////////////////////
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
