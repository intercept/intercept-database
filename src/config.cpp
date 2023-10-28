#include "config.h"
#include <filesystem>
#include "yaml-cpp/yaml.h"
#include "logger.h"
#include <Windows.h>


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

std::filesystem::path Config::GetCurrentDLLPath() {
    char path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)Config::initCommands, &hm) == 0)
    {
        int ret = GetLastError();
        fprintf(stderr, "GetModuleHandle failed, error = %d\\n", ret);
        // Return or however you want to handle an error.
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0)
    {
        int ret = GetLastError();
        fprintf(stderr, "GetModuleFileName failed, error = %d\\n", ret);
        // Return or however you want to handle an error.
    }

    return std::filesystem::path(path);
}

void Config::reloadConfig() {

    std::filesystem::path configFilePath(Config::GetCurrentDLLPath().parent_path().parent_path() / "config.yaml");

    if (!std::filesystem::exists(configFilePath))
        throw std::filesystem::filesystem_error("File not found", configFilePath, std::error_code());

    auto stringpath = configFilePath.string();
    YAML::Node config = YAML::LoadFile(configFilePath.string());

#pragma region accounts

    if (!config["accounts"].IsMap()) throw std::runtime_error("Config Accounts entry is not a map");

    for (const auto& it : config["accounts"]) {
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
        acc->set_connect_option(MYSQL_OPT_RECONNECT, true);
        if (value["opt_compress"].as<bool>(false))
            acc->set_connect_option(MYSQL_OPT_COMPRESS, true);
        if (value["opt_read_timeout"].IsDefined())
            acc->set_connect_option(MYSQL_OPT_READ_TIMEOUT, value["opt_read_timeout"].as<int>());
        if (value["opt_write_timeout"].IsDefined())
            acc->set_connect_option(MYSQL_OPT_WRITE_TIMEOUT, value["opt_write_timeout"].as<int>());

        if (value["opt_multi_statement"].as<bool>(false)) {
            acc->set_connect_option(MARIADB_OPT_MULTI_STATEMENTS, true);
            acc->set_connect_option(MARIADB_OPT_MULTI_RESULTS, true);
        }

        //acc->set_auto_commit(false);

        accounts[accountName] = acc;
    }

#pragma endregion accounts

#pragma region statements

    for (const auto& it : config["statements"]) {
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

    auto globalConf = config["global"];

    if (!globalConf.IsMap()) throw std::runtime_error("Config Global entry is not a map");


    dynamicQueriesEnabled = globalConf["enableDynamicQueries"].as<bool>(true);
    parseTinyintAsBool = globalConf["parseTinyintAsBool"].as<bool>(false);
    dateType = dateTypeFromString(globalConf["parseDateType"].as<r_string>("string"sv));
    DBNullEqualEmptyString = globalConf["DBNullEqualEmptyString"].as<bool>(false);
    workerCount = globalConf["workerCount"].as<size_t>(1);

    auto loggingConf = globalConf["logging"];

    if (loggingConf.IsDefined()) {
        if (!loggingConf.IsMap()) throw std::runtime_error("Config Logging entry is not a map");

        auto directory = loggingConf["directory"].as<r_string>("dbLog"sv);
        Logger::get().init(directory.operator std::basic_string_view<char>());
        Logger::get().setQueryLogEnabled(loggingConf["querylog"].as<bool>(false));
        Logger::get().setThreadLogEnabled(loggingConf["threadlog"].as<bool>(false));
    }


#pragma endregion global

    if (config["schemas"].IsMap())
        for (const auto& it : config["schemas"]) {
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
