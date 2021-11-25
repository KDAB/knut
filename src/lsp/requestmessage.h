#pragma once

#include <functional>
#include <optional>
#include <string>
#include <variant>

namespace Lsp {

using MessageId = std::variant<int, std::string>;

/**
 *  \brief Error message in the response message, following a request
 */
template <typename ErrorData>
struct ResponseError
{
    int code;
    std::string message;
    std::optional<ErrorData> data;
};

/**
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

/**
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
    using Result = ResultData;
    using ResponseCallback = std::function<void(Response)>;

    inline static int nextId = 1;
};
}
