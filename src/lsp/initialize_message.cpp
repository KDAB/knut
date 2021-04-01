#include "initialize_message.h"

namespace Lsp {

void to_json(nlohmann::json &j, const InitializeResult &result)
{
    if (result.serverInfo) {
        j["serverInfo"]["name"] = result.serverInfo->name;
        if (result.serverInfo->version)
            j["serverInfo"]["version"] = result.serverInfo->version.value();
    }
}

void from_json(const nlohmann::json &j, InitializeResult &result)
{
    if (j.contains("serverInfo")) {
        InitializeResult::ServerInfo serverInfo;
        auto infoJson = j.at("serverInfo");
        infoJson.at("name").get_to(serverInfo.name);
        if (infoJson.contains("version"))
            serverInfo.version = infoJson.at("version").get<std::string>();
        result.serverInfo = serverInfo;
    }
}

}
