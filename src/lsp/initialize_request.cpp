#include "initialize_request.h"

#include "utils/json_utils.h"

namespace Lsp {

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

}
