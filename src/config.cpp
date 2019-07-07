#include "config.h"
#include <filesystem>
#include "yaml-cpp/yaml.h"


namespace YAML {
    template<>
    struct convert<r_string> {
        static Node encode(const r_string& rhs) {
            Node node;
            node = static_cast<std::string>(rhs);
            return node;
        }

        static bool decode(const Node& node, r_string& rhs) {
            if (!node.IsScalar()) {
                return false;
            }

            rhs = node.as<std::string>();
            return true;
        }
    };
}


ConfigDateType dateTypeFromString(std::string_view str) {
    if (str == "string")
        return ConfigDateType::humanString;
    if (str == "stringMS")
        return ConfigDateType::humanStringMS;
    if (str == "array")
        return ConfigDateType::array;
    if (str == "timestamp")
        return ConfigDateType::timestamp;
    if (str == "timestampString")
        return ConfigDateType::timestampString;
    if (str == "timestampStringMS")
        return ConfigDateType::timestampStringMS;
    throw std::invalid_argument("Invalid value passed as parseDateTime entry");
}

void Config::reloadConfig() {
    std::filesystem::path configFilePath("@InterceptDB/config.yaml");

    if (!std::filesystem::exists(configFilePath))
        throw std::filesystem::filesystem_error("File not found", configFilePath, std::error_code());

    auto stringpath = configFilePath.string();
    YAML::Node config = YAML::LoadFile(configFilePath.string());

#pragma region accounts

    if (!config["accounts"].IsMap()) throw std::runtime_error("Config Accounts entry is not a map");

    for (auto& it : config["accounts"]) {
        r_string accountName = it.first.as<r_string>();
        auto value = it.second;

        if (!value["ip"]) throw std::runtime_error("Undefined account value: ip");
        if (!value["username"]) throw std::runtime_error("Undefined account value: username");
        if (!value["password"]) throw std::runtime_error("Undefined account value: password");
        if (!value["database"]) throw std::runtime_error("Undefined account value: database");

        auto ip = value["ip"].as<r_string>();
        auto user = value["username"].as<r_string>();
        auto password = value["password"].as<r_string>();
        auto database = value["database"].as<r_string>();
        auto port = value["port"].as<int>(3306);

        auto acc = mariadb::account::create(ip, user, password, database, port);
        acc->set_connect_option(MYSQL_SET_CHARSET_NAME, r_string("utf8mb4"sv));
        //acc->set_connect_option(MARIADB_OPT_MULTI_STATEMENTS, true);
        //acc->set_connect_option(MARIADB_OPT_MULTI_RESULTS, true);
        //acc->set_auto_commit(false);

        accounts[accountName] = acc;
    }

#pragma endregion accounts

#pragma region statements

    for (auto& it : config["statements"]) {
        r_string stmtName = it.first.as<r_string>();
        if (it.second.IsMap()) {
            if (!it.second["query"]) throw std::runtime_error(("statement "+stmtName+" has no 'query' entry").c_str());

            ConfigStatement stmt;
            stmt.query = it.second["query"].as<r_string>();
            if (it.second["parseTinyintAsBool"].IsDefined())
                stmt.parseTinyintAsBool = it.second["parseTinyintAsBool"].as<bool>();
            if (it.second["parseDateType"].IsDefined())
                stmt.dateType = dateTypeFromString(it.second["parseDateType"].as<r_string>());

            statements[stmtName] = std::move(stmt);
        } else {
            statements[stmtName] = {it.second.as<r_string>()};
        }
    }

#pragma endregion statements

#pragma region global

    if (!config["global"].IsMap()) throw std::runtime_error("Config Global entry is not a map");


    dynamicQueriesEnabled = config["global"]["enableDynamicQueries"].as<bool>(true);
    parseTinyintAsBool = config["global"]["parseTinyintAsBool"].as<bool>(false);
    dateType = dateTypeFromString(config["global"]["parseDateType"].as<r_string>("string"sv));
    DBNullEqualEmptyString = config["global"]["DBNullEqualEmptyString"].as<bool>(false);

#pragma endregion global

    if (config["schemas"].IsMap())
        for (auto& it : config["schemas"]) {
            r_string schemaName = it.first.as<r_string>();
            auto value = it.second;

            schemas[schemaName] = std::filesystem::path("@InterceptDB"sv) / value.as<std::string>();
        }


}


game_value cmd_reloadConfig(game_state&) {



    try {
        Config::get().reloadConfig();
    }
    catch (YAML::BadConversion& x) {
        return "error " + x.msg + " in L" + std::to_string(x.mark.line);
    }
    catch (YAML::ParserException & x) {
        return "error " + x.msg + " in L" + std::to_string(x.mark.line);
    }
    catch (std::runtime_error& x) {
        return r_string("error ") + x.what();
    }
    return {};
}

void Config::initCommands() {
    
    handle_cmd_reloadConfig = intercept::client::host::register_sqf_command("dbReloadConfig", "TODO", cmd_reloadConfig, game_data_type::STRING);
    handle_cmd_version = intercept::client::host::register_sqf_command("dbVersion", "TODO", [](game_state&) -> game_value {
                                return "1.5";
                            }, game_data_type::STRING);
}
