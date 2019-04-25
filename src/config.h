#pragma once
#include <intercept.hpp>
#include "../intercept/src/host/common/singleton.hpp"
#include <vector>
#include "mariadb++/account.hpp"
#include "../intercept/src/client/headers/shared/containers.hpp"

class Config : public intercept::singleton<Config> {
    
public:
    void reloadConfig();

    r_string getQuery(r_string name) {
        auto found = statements.find(name);
        if (found == statements.end()) return r_string();
        return found->second;
    }

    mariadb::account_ref getAccount(r_string name) {
        auto found = accounts.find(name);
        if (found == accounts.end()) return {};
        return found->second;
    }

    bool areDynamicQueriesEnabled() const {
        return dynamicQueriesEnabled;
    }

    static void initCommands();
    static inline registered_sqf_function handle_cmd_reloadConfig;
    static inline registered_sqf_function handle_cmd_version;

private:
    std::map<intercept::types::r_string, mariadb::account_ref> accounts;
    std::map<intercept::types::r_string, intercept::types::r_string> statements;
    bool dynamicQueriesEnabled = true;



};
