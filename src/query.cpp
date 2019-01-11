#include "query.h"
#include <mariadb++/statement.hpp>

using namespace intercept::client;


game_data* createGameDataDBQuery(param_archive* ar) {
    auto x = new GameDataDBQuery();
    if (ar)
        x->serialize(*ar);
    return x;
}


game_value Query::cmd_prepareQuery(uintptr_t, game_value_parameter right) {

    auto query = new GameDataDBQuery();

    query->queryString = right;

    return query;
}

game_value Query::cmd_bindValue(uintptr_t, game_value_parameter left, game_value_parameter right) {
    auto query = left.get_as<GameDataDBQuery>();

    query->boundValues.emplace_back(right);
    return {};
}
game_value Query::cmd_bindValueArray(uintptr_t, game_value_parameter left, game_value_parameter right) {
    auto query = left.get_as<GameDataDBQuery>();

    for (auto& it : right.to_array())
        query->boundValues.emplace_back(it);

    return {};
}
game_value Query::cmd_bindNamedValue(uintptr_t, game_value_parameter left, game_value_parameter right) {
    return {};
}
game_value Query::cmd_bindNamedValueArray(uintptr_t, game_value_parameter left, game_value_parameter right) {
    return {};
}

void Query::initCommands() {
    
    auto dbType = host::register_sqf_type("DBQUERRY"sv, "databaseQuery"sv, "TODO"sv, "databaseQuery"sv, createGameDataDBQuery);
    GameDataDBQuery_typeE = dbType.first;
    GameDataDBQuery_type = dbType.second;


    handle_cmd_prepareQuery = client::host::register_sqf_command("db_prepareQuery", "TODO", Query::cmd_prepareQuery, dbType.first, game_data_type::STRING);

    //#TODO only accept string,scalar,bool
    
    handle_cmd_bindValue = client::host::register_sqf_command("db_bindValue", "TODO", Query::cmd_bindValue, game_data_type::NOTHING, dbType.first, game_data_type::ANY);
    handle_cmd_bindValueArray = client::host::register_sqf_command("db_bindValueArray", "TODO", Query::cmd_bindValueArray, game_data_type::NOTHING, dbType.first, game_data_type::ARRAY);
    //handle_cmd_bindNamedValue = client::host::register_sqf_command("db_bindNamedValue", "TODO", Query::cmd_bindNamedValue, game_data_type::NOTHING, dbType.first, game_data_type::ANY);
    //handle_cmd_bindNamedValueArray = client::host::register_sqf_command("db_bindNamedValueArray", "TODO", Query::cmd_bindNamedValueArray, game_data_type::NOTHING, dbType.first, game_data_type::ARRAY);


}
