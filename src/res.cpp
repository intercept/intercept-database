#include "res.h"
#include <mariadb++/result_set.hpp>
#include "threading.h"

using namespace intercept::client;

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

game_value Result::cmd_affectedRows(game_state&, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;

    return res->row_count();
}

game_value Result::cmd_lastInsertId(game_state&, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;

    return res->get_last_insert_id();
}

game_value Result::cmd_toArray(game_state&, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;
    if (!res) return auto_array<game_value>();
    auto_array<game_value> result;

    while (res->next()) {
        auto_array<game_value> row;

        for (mariadb::u32 i = 0u; i < res->column_count(); ++i) {
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

game_value Result::cmd_toParsedArray(game_state& state, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBResult>()->res;
    if (!res) return auto_array<game_value>();
    auto_array<game_value> result;

    while (res->next()) {
        auto_array<game_value> row;

        for (size_t i = 0u; i < res->column_count(); ++i) {

            switch (res->column_type(i)) {
            case mariadb::value::null: row.emplace_back(game_value{}); break;
            case mariadb::value::date: row.emplace_back(res->get_date(i).str()); break;
            case mariadb::value::date_time: row.emplace_back(res->get_date_time(i).str()); break;
            case mariadb::value::time: row.emplace_back(res->get_time(i).str_time()); break;
            case mariadb::value::string: {
                auto content = res->get_string(i);

                if (content.front() == '[') {//array
                    //attempt parse
                    auto arrayContent = sqf::parse_simple_array(content);

                    if (state.get_evaluator()->_errorType == game_state::game_evaluator::evaluator_error_type::gen
                        || 
                        (!arrayContent.empty() && arrayContent.front().is_null())
                        ) {
                        state.get_evaluator()->_errorType = game_state::game_evaluator::evaluator_error_type::ok; //Force ignore
                        row.emplace_back(std::move(content));
                    } else
                        row.emplace_back(game_value(arrayContent));
                } else if (
                    content.front() == 't'//true
                    || content.front() == 'T'//True
                    || content.front() == 'f'//false
                    || content.front() == 'F'//False
                    || (content.front() >= '0' && content.front() <= '9')//number
                    ) {

                    auto arrayContent = sqf::parse_simple_array(r_string("["sv)+content+"]"sv);

                    if (state.get_evaluator()->_errorType == game_state::game_evaluator::evaluator_error_type::gen
                        || arrayContent.empty() 
                        || arrayContent.front().is_null()) {
                        state.get_evaluator()->_errorType = game_state::game_evaluator::evaluator_error_type::ok; //Force ignore
                        row.emplace_back(std::move(content));
                    } else
                        row.emplace_back(arrayContent.front());
                } else {
                    row.emplace_back(std::move(content));
                }
            } break;
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
            default:;
            }
        }
        result.emplace_back(std::move(row));
    }
    return result;
}

game_value Result::cmd_bindCallback(game_state&, game_value_parameter left, game_value_parameter right) {
    auto& res = left.get_as<GameDataDBAsyncResult>();

    res->data->callback = right[0];
    res->data->callbackArgs = right[1]; //#TODO call directly if result is ready
    return {};
}

game_value Result::cmd_waitForResult(game_state&, game_value_parameter right) {
    auto& res = right.get_as<GameDataDBAsyncResult>();

    res->data->fut.wait();

    std::unique_lock l(Threading::get().asyncWorkMutex);
    auto& tasks = Threading::get().completedAsyncTasks;
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i] == res) {
            tasks.erase(tasks.begin()+i);
            break;
        }
    }
    if (tasks.empty())
        Threading::get().hasCompletedAsyncWork = false;
    l.unlock();


    auto gd_res = new GameDataDBResult();
    gd_res->res = res->data->res;
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
    handle_cmd_toParsedArray = client::host::register_sqf_command("dbResultToParsedArray", "TODO", Result::cmd_toParsedArray, game_data_type::ARRAY, GameDataDBResult_typeE);
    handle_cmd_bindCallback = client::host::register_sqf_command("dbBindCallback", "TODO", Result::cmd_bindCallback, game_data_type::NOTHING, GameDataDBAsyncResult_typeE, game_data_type::ARRAY);
    handle_cmd_waitForResult = client::host::register_sqf_command("dbWaitForResult", "TODO", Result::cmd_waitForResult, GameDataDBResult_typeE, GameDataDBAsyncResult_typeE);
}
