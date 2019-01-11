#include "res.h"
#include <mariadb++/result_set.hpp>

using namespace intercept::client;


game_data* createGameDataDBResult(param_archive* ar) {
    auto x = new GameDataDBResult();
    if (ar)
        x->serialize(*ar);
    return x;
}


game_value Result::cmd_affectedRows(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;

    return res->row_count();
}

game_value Result::cmd_lastInsertId(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;

    return res->get_last_insert_id();
}

game_value Result::cmd_toArray(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;
    auto_array<game_value> result;

    while (res->next()) {
        auto_array<game_value> row;

        for (int i = 0; i < res->column_count(); ++i) {

            switch (res->column_type(i)) {
                case mariadb::value::null: row.emplace_back(game_value{}); break;
                case mariadb::value::date: row.emplace_back(res->get_string(i)); break;
                case mariadb::value::date_time: row.emplace_back(res->get_string(i)); break;
                case mariadb::value::time: row.emplace_back(res->get_string(i)); break;
                case mariadb::value::string: row.emplace_back(res->get_string(i)); break;
                case mariadb::value::boolean: row.emplace_back(res->get_boolean(i)); break;
                case mariadb::value::decimal: row.emplace_back(res->get_decimal(i).float32()); break;
                case mariadb::value::unsigned8: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::signed8: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::unsigned16: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::signed16: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::unsigned32: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::signed32: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::unsigned64: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::signed64: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::float32: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::double64: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::enumeration: row.emplace_back(res->get_string(i)); break;
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
