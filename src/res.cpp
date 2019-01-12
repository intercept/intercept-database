#include "res.h"
#include <mariadb++/result_set.hpp>

using namespace intercept::client;
extern auto_array<ref<GameDataDBAsyncResult>> asyncWork;

game_data* createGameDataDBResult(param_archive* ar) {
    auto x = new GameDataDBResult();
    if (ar)
        x->serialize(*ar);
    return x;
}

game_data* createGameDataDBAsyncResult(param_archive* ar) {
    auto x = new GameDataDBAsyncResult();
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
    if (!res) return auto_array<game_value>();
    auto_array<game_value> result;

    while (res->next()) {
        auto_array<game_value> row;

        for (int i = 0; i < res->column_count(); ++i) {

            switch (res->column_type(i)) {
                case mariadb::value::null: row.emplace_back(game_value{}); break;
                case mariadb::value::date: row.emplace_back(res->get_date(i).str()); break;
                case mariadb::value::date_time: row.emplace_back(res->get_date_time(i).str()); break;
                case mariadb::value::time: row.emplace_back(res->get_time(i).str_time()); break;
                case mariadb::value::string: row.emplace_back(res->get_string(i)); break;
                case mariadb::value::boolean: row.emplace_back(res->get_boolean(i)); break;
                case mariadb::value::decimal: row.emplace_back(res->get_decimal(i).float32()); break;
                case mariadb::value::unsigned8: row.emplace_back(static_cast<float>(res->get_unsigned8(i))); break;
                case mariadb::value::signed8: row.emplace_back(static_cast<float>(res->get_signed8(i))); break;
                case mariadb::value::unsigned16: row.emplace_back(static_cast<float>(res->get_unsigned16(i))); break;
                case mariadb::value::signed16: row.emplace_back(static_cast<float>(res->get_signed16(i))); break;
                case mariadb::value::unsigned32: row.emplace_back(static_cast<float>(res->get_unsigned32(i))); break;
                case mariadb::value::signed32: row.emplace_back(static_cast<float>(res->get_signed32(i))); break;
                case mariadb::value::unsigned64: row.emplace_back(static_cast<float>(res->get_unsigned64(i))); break;
                case mariadb::value::signed64: row.emplace_back(static_cast<float>(res->get_signed64(i))); break;
                case mariadb::value::float32: row.emplace_back(res->get_float(i)); break;
                case mariadb::value::double64: row.emplace_back(static_cast<float>(res->get_double(i))); break;
                case mariadb::value::enumeration: row.emplace_back(res->get_string(i)); break;
                default: ;
            }            
        }
        result.emplace_back(std::move(row));
    }
	return result;
}

game_value Result::cmd_bindCallback(uintptr_t, game_value_parameter left, game_value_parameter right) {
    auto& res = left.get_as<GameDataDBAsyncResult>();

    res->data->callback = right[0];
    res->data->callbackArgs = right[1]; //#TODO call directly if result is ready
    return {};
}

game_value Result::cmd_waitForResult(uintptr_t, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBAsyncResult>();

    res->data->res.wait();

    for (int i = 0; i < asyncWork.size(); ++i) {
        if (asyncWork[i] == res) {
            asyncWork.erase(i);
            break;
        }
    }

    auto gd_res = new GameDataDBResult();
    gd_res->res = res->data->res.get();
    sqf::call(res->data->callback, { gd_res, res->data->callbackArgs });
    return gd_res;
}

void Result::initCommands() {
    auto dbType = host::register_sqf_type("DBRES"sv, "databaseResult"sv, "TODO"sv, "databaseResult"sv, createGameDataDBResult);
    GameDataDBResult_typeE = dbType.first;
    GameDataDBResult_type = dbType.second;

    auto dbTypeA = host::register_sqf_type("DBRESASYNC"sv, "databaseResultAsync"sv, "TODO"sv, "databaseResultAsync"sv, createGameDataDBResult);
    GameDataDBAsyncResult_typeE = dbTypeA.first;
    GameDataDBAsyncResult_type = dbTypeA.second;

    handle_cmd_affectedRows = client::host::register_sqf_command("dbResultAffectedRows", "TODO", Result::cmd_affectedRows, game_data_type::SCALAR, GameDataDBResult_typeE);
    handle_cmd_lastInsertId = client::host::register_sqf_command("dbResultLastInsertId", "TODO", Result::cmd_lastInsertId, game_data_type::SCALAR, GameDataDBResult_typeE);
    handle_cmd_toArray = client::host::register_sqf_command("dbResultToArray", "TODO", Result::cmd_toArray, game_data_type::ARRAY, GameDataDBResult_typeE);
    handle_cmd_bindCallback = client::host::register_sqf_command("dbBindCallback", "TODO", Result::cmd_bindCallback, game_data_type::NOTHING, GameDataDBAsyncResult_typeE, game_data_type::ARRAY);
    handle_cmd_waitForResult = client::host::register_sqf_command("dbWaitForResult", "TODO", Result::cmd_waitForResult, GameDataDBResult_typeE, GameDataDBAsyncResult_typeE);
}
