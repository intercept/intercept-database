#pragma once
#include <intercept.hpp>
#include "../intercept/src/host/common/singleton.hpp"
#include <filesystem>
#include "mariadb++/account.hpp"
#include "../intercept/src/client/headers/shared/containers.hpp"

enum class ConfigDateType {
    humanString,
    humanStringMS,
    array,
    timestamp,
    timestampString,
    timestampStringMS
};

ConfigDateType dateTypeFromString(std::string_view str);

class ConfigStatement {
public:
    r_string query;
    bool parseTinyintAsBool = false;
    ConfigDateType dateType = ConfigDateType::humanString;
};


class Config : public intercept::singleton<Config> {
    
public:
    void reloadConfig();

    ConfigStatement getStatement(r_string name) {
        auto found = statements.find(name);
        if (found == statements.end()) return ConfigStatement();
        return found->second;
    }

    r_string getQuery(r_string name) {
        auto found = statements.find(name);
        if (found == statements.end()) return r_string();
        return found->second.query;
    }

    mariadb::account_ref getAccount(r_string name) {
        auto found = accounts.find(name);
        if (found == accounts.end()) return {};
        return found->second;
    }

   std::optional<std::filesystem::path> getSchema(r_string name) {
        auto found = schemas.find(name);
        if (found == schemas.end()) return {};
        return found->second;
    }

    bool areDynamicQueriesEnabled() const {
        return dynamicQueriesEnabled;
    }

    ConfigDateType getDateType() const {
        return dateType;
    }

    bool getTinyintAsBool() const {
        return parseTinyintAsBool;
    }

    static void initCommands();
    static inline registered_sqf_function handle_cmd_reloadConfig;
    static inline registered_sqf_function handle_cmd_version;

private:
    std::map<intercept::types::r_string, mariadb::account_ref> accounts;
    std::map<intercept::types::r_string, ConfigStatement> statements;
    std::map<intercept::types::r_string, std::filesystem::path> schemas;
    bool dynamicQueriesEnabled = true;
    bool parseTinyintAsBool = false;
    ConfigDateType dateType = ConfigDateType::humanString;

};
