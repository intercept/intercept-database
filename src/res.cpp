#include "res.h"
#include <mysqlx/xdevapi.h>

using namespace intercept::client;


game_data* createGameDataDBResult(param_archive* ar) {
    auto x = new GameDataDBResult();
    if (ar)
        x->serialize(*ar);
    return x;
}


game_value Result::cmd_affectedRows(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;

    return res.getAffectedItemsCount();
}

game_value Result::cmd_lastInsertId(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;

    return res.getAutoIncrementValue();
}

game_value Result::cmd_toArray(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;
    auto_array<game_value> result;

    for (auto& it : res) {
        auto_array<game_value> row;

        for (int i = 0; i < it.colCount(); ++i) {

            switch (it.get(i).getType()) {
                case mysqlx::Value::VNULL: row.emplace_back(game_value{}); break;
                case mysqlx::Value::UINT64: row.emplace_back(it.get(i).get<uint64_t>()); break;
                case mysqlx::Value::INT64: row.emplace_back(static_cast<float>(it.get(i).get<int64_t>())); break;
                case mysqlx::Value::FLOAT: row.emplace_back(it.get(i).get<float>()); break;
                case mysqlx::Value::DOUBLE:row.emplace_back(static_cast<float>(it.get(i).get<double>())); break;
                case mysqlx::Value::BOOL: row.emplace_back(it.get(i).get<bool>()); break;
				case mysqlx::Value::STRING: row.emplace_back(static_cast<std::string>(it.get(i).get<mysqlx::string>())); break;
                case mysqlx::Value::DOCUMENT: break;
                case mysqlx::Value::RAW: break;
                case mysqlx::Value::ARRAY: break;
                default: ;
            }            
        }
        result.emplace_back(std::move(row));
    }
	return game_value(result);
}

void ::Result::initCommands() {
    
    auto dbType = host::register_sqf_type("DBQUERRY"sv, "databaseQuery"sv, "TODO"sv, "databaseQuery"sv, createGameDataDBResult);
    GameDataDBResult_typeE = dbType.first;
    GameDataDBResult_type = dbType.second;


    handle_cmd_affectedRows = client::host::register_sqf_command("db_resultAffectedRows", "TODO", Result::cmd_affectedRows, game_data_type::SCALAR, GameDataDBResult_typeE);
    handle_cmd_lastInsertId = client::host::register_sqf_command("db_resultLastInsertId", "TODO", Result::cmd_lastInsertId, game_data_type::SCALAR, GameDataDBResult_typeE);
    handle_cmd_toArray = client::host::register_sqf_command("db_resultToArray", "TODO", Result::cmd_toArray, game_data_type::ARRAY, GameDataDBResult_typeE);


}
