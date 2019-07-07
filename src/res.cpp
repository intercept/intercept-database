#include "res.h"
#include <mariadb++/result_set.hpp>
#include "threading.h"
#include "config.h"

using namespace intercept::client;



/// date, datetime
std::tuple<std::function<game_value(const mariadb::date_time&)>,
    std::function<game_value(const mariadb::date_time&)>,
    std::function<game_value(const mariadb::time&)>
> getDateParser(ConfigDateType type) {
    switch (type) {
        
        
        case ConfigDateType::humanString: 
        return {
            [](const mariadb::date_time& date) -> game_value {
                return date.str();
            },
            [](const mariadb::date_time& dateTime) -> game_value {
                return dateTime.str();
            },
            [](const mariadb::time& time) -> game_value {
                return time.str_time();
            }
        };
        case ConfigDateType::humanStringMS: 
        return {
            [](const mariadb::date_time& date) -> game_value {
                return date.str(true);
            },
            [](const mariadb::date_time& dateTime) -> game_value {
                return dateTime.str(true);
            },
            [](const mariadb::time& time) -> game_value {
                return time.str_time(true);
            }
        };
        case ConfigDateType::array: 
        return {
            [](const mariadb::date_time& date) -> game_value {
                auto_array<game_value> res;
                res.reserve(7);

                res.emplace_back(date.year());
                res.emplace_back(date.month());
                res.emplace_back(date.day());
                res.emplace_back(date.hour()); //Probably don't need to return this on a date
                res.emplace_back(date.minute());
                res.emplace_back(date.second());
                res.emplace_back(date.millisecond());

                return res;
            },
            [](const mariadb::date_time& dateTime) -> game_value {
                auto_array<game_value> res;
                res.reserve(7);

                res.emplace_back(dateTime.year());
                res.emplace_back(dateTime.month());
                res.emplace_back(dateTime.day());
                res.emplace_back(dateTime.hour());
                res.emplace_back(dateTime.minute());
                res.emplace_back(dateTime.second());
                res.emplace_back(dateTime.millisecond());

                return res;
            },
            [](const mariadb::time& time) -> game_value {
                auto_array<game_value> res;
                res.reserve(4);
                res.emplace_back(time.hour());
                res.emplace_back(time.minute());
                res.emplace_back(time.second());
                res.emplace_back(time.millisecond());

                return res;
            }
        };
        case ConfigDateType::timestamp: 
        return {
            [](const mariadb::date_time& date) -> game_value {
                return static_cast<float>(date.mktime());
            },
            [](const mariadb::date_time& dateTime) -> game_value {
                return static_cast<float>(dateTime.mktime());
            },
            [](const mariadb::time& time) -> game_value {
                return static_cast<float>(time.mktime());
            }
        };
        case ConfigDateType::timestampString: 
        return {
            [](const mariadb::date_time& date) -> game_value {
                return std::to_string(date.mktime());
            },
            [](const mariadb::date_time& dateTime) -> game_value {
                return std::to_string(dateTime.mktime());
            },
            [](const mariadb::time& time) -> game_value {
                return std::to_string(time.mktime());
            }
        };
        case ConfigDateType::timestampStringMS: 
        return {
            [](const mariadb::date_time& date) -> game_value {
                return std::to_string(date.mktime()*1000 + date.millisecond());
            },
            [](const mariadb::date_time& dateTime) -> game_value {
                return std::to_string(dateTime.mktime()*1000 + dateTime.millisecond());
            },
            [](const mariadb::time& time) -> game_value {
                return  std::to_string(time.mktime()*1000 + time.millisecond());
            }
        };
        
    }
}




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

game_data* createGameDataDBNull(param_archive* ar) {
    auto x = new GameDataDBNull();
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
    auto& gdRes = right.get_as<GameDataDBResult>();
    auto& res = gdRes->res;
    if (!res) return auto_array<game_value>();
    auto_array<game_value> result;

    const auto [dateParser, dateTimeParser, timeParser] = getDateParser(Config::get().getDateType(gdRes->statementName));
    const auto parseTinyintAsBool =  Config::get().getTinyintAsBool(gdRes->statementName);

    while (res->next()) {
        auto_array<game_value> row;

        for (mariadb::u32 i = 0u; i < res->column_count(); ++i) {
            try {
                auto type = res->column_type(i);
                if (res->get_is_null(i)) type = mariadb::value::null;

                switch (type) {
                    case mariadb::value::null: row.emplace_back(new GameDataDBNull()); break;
                    case mariadb::value::date: row.emplace_back(dateParser(res->get_date(i))); break;
                    case mariadb::value::date_time: row.emplace_back(dateTimeParser(res->get_date_time(i))); break;
                    case mariadb::value::time: row.emplace_back(timeParser(res->get_time(i))); break;
                    case mariadb::value::string: row.emplace_back(res->get_string(i)); break;
                    case mariadb::value::boolean: row.emplace_back(res->get_boolean(i)); break;
                    case mariadb::value::decimal: row.emplace_back(res->get_decimal(i).float32()); break;
                    case mariadb::value::unsigned8: {
                        if (parseTinyintAsBool && res->column_type_raw(i) == MYSQL_TYPE_TINY)
                            row.emplace_back(static_cast<bool>(res->get_unsigned8(i)));
                        else
                            row.emplace_back(static_cast<float>(res->get_unsigned8(i)));
                    } break;
                    case mariadb::value::signed8: {
                        if (parseTinyintAsBool && res->column_type_raw(i) == MYSQL_TYPE_TINY)
                            row.emplace_back(static_cast<bool>(res->get_signed8(i)));
                        else
                            row.emplace_back(static_cast<float>(res->get_signed8(i)));
                    }  break;
                    case mariadb::value::unsigned16: row.emplace_back(static_cast<float>(res->get_unsigned16(i))); break;
                    case mariadb::value::signed16: row.emplace_back(static_cast<float>(res->get_signed16(i))); break;
                    case mariadb::value::unsigned32: row.emplace_back(static_cast<float>(res->get_unsigned32(i))); break;
                    case mariadb::value::signed32: row.emplace_back(static_cast<float>(res->get_signed32(i))); break;
                    case mariadb::value::unsigned64: row.emplace_back(static_cast<float>(res->get_unsigned64(i))); break;
                    case mariadb::value::signed64: row.emplace_back(static_cast<float>(res->get_signed64(i))); break;
                    case mariadb::value::float32: row.emplace_back(res->get_float(i)); break;
                    case mariadb::value::double64: row.emplace_back(static_cast<float>(res->get_double(i))); break;
                    case mariadb::value::enumeration: row.emplace_back(res->get_string(i)); break;
                    case mariadb::value::blob: row.emplace_back(res->get_blobString(i)); break;
                    default: ;
                }
            } catch (std::invalid_argument & x) {//get_date_time getting "NULL"
                row.emplace_back("NULL"sv); break;
            }
        }
        result.emplace_back(std::move(row));
    }
	return result;
}

game_value Result::cmd_toParsedArray(game_state& state, game_value_parameter right) {
    auto& gdRes = right.get_as<GameDataDBResult>();
    auto& res = gdRes->res;
    if (!res) return auto_array<game_value>();
    auto_array<game_value> result;

    const auto [dateParser, dateTimeParser, timeParser] = getDateParser(Config::get().getDateType(gdRes->statementName));
    const auto parseTinyintAsBool = Config::get().getTinyintAsBool(gdRes->statementName);

    while (res->next()) {
        auto_array<game_value> row;

        auto addParsedString = [&row, &state](r_string&& content) {

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
                || (content.front() == '\'' || content.front() == '"')//string
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
        };

        for (mariadb::u32 i = 0u; i < res->column_count(); ++i) {
            auto type = res->column_type(i);
            if (res->get_is_null(i)) type = mariadb::value::null;

            switch (type) {
                case mariadb::value::null: row.emplace_back(new GameDataDBNull()); break;
                case mariadb::value::date: row.emplace_back(dateParser(res->get_date(i))); break;
                case mariadb::value::date_time: row.emplace_back(dateTimeParser(res->get_date_time(i))); break;
                case mariadb::value::time: row.emplace_back(timeParser(res->get_time(i))); break;
                case mariadb::value::string: addParsedString(res->get_string(i)); break;
                case mariadb::value::blob: addParsedString(res->get_blobString(i)); break;
                case mariadb::value::boolean: row.emplace_back(res->get_boolean(i)); break;
                case mariadb::value::decimal: row.emplace_back(res->get_decimal(i).float32()); break;
                case mariadb::value::unsigned8: {
                    if (parseTinyintAsBool && res->column_type_raw(i) == MYSQL_TYPE_TINY)
                        row.emplace_back(static_cast<bool>(res->get_unsigned8(i)));
                    else
                        row.emplace_back(static_cast<float>(res->get_unsigned8(i)));
                } break;
                case mariadb::value::signed8: {
                    if (parseTinyintAsBool && res->column_type_raw(i) == MYSQL_TYPE_TINY)
                        row.emplace_back(static_cast<bool>(res->get_signed8(i)));
                    else
                        row.emplace_back(static_cast<float>(res->get_signed8(i)));
                }  break;
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

    auto dbTypeNull = host::register_sqf_type("DBNULL"sv, "dbNull"sv, "dbNull"sv, "dbNull"sv, createGameDataDBNull);
    GameDataDBNull_typeE = dbTypeNull.first;
    GameDataDBNull_type = client::host::register_compound_sqf_type({ game_data_type::STRING, GameDataDBNull_typeE });

    handle_cmd_affectedRows = client::host::register_sqf_command("dbResultAffectedRows", "TODO", Result::cmd_affectedRows, game_data_type::SCALAR, GameDataDBResult_typeE);
    handle_cmd_lastInsertId = client::host::register_sqf_command("dbResultLastInsertId", "TODO", Result::cmd_lastInsertId, game_data_type::SCALAR, GameDataDBResult_typeE);
    handle_cmd_toArray = client::host::register_sqf_command("dbResultToArray", "TODO", Result::cmd_toArray, game_data_type::ARRAY, GameDataDBResult_typeE);
    handle_cmd_toParsedArray = client::host::register_sqf_command("dbResultToParsedArray", "TODO", Result::cmd_toParsedArray, game_data_type::ARRAY, GameDataDBResult_typeE);
    handle_cmd_bindCallback = client::host::register_sqf_command("dbBindCallback", "TODO", Result::cmd_bindCallback, game_data_type::NOTHING, GameDataDBAsyncResult_typeE, game_data_type::ARRAY);
    handle_cmd_waitForResult = client::host::register_sqf_command("dbWaitForResult", "TODO", Result::cmd_waitForResult, GameDataDBResult_typeE, GameDataDBAsyncResult_typeE);

    static auto stuff = client::host::register_sqf_command("isNull", "TODO", [](game_state&, game_value_parameter) -> game_value {
        return true;
    }, game_data_type::BOOL, GameDataDBNull_typeE);

    static auto stuffb = client::host::register_sqf_command("dbNull", "TODO", [](game_state&) -> game_value {
        return new GameDataDBNull;
    }, GameDataDBNull_typeE);

    static auto stuffc = client::host::register_sqf_command("==", "TODO", [](game_state&, game_value_parameter str, game_value_parameter) -> game_value {
        return static_cast<r_string>(str).empty();
    }, game_data_type::BOOL ,game_data_type::STRING,GameDataDBNull_typeE);

    static auto stuffd = client::host::register_sqf_command("==", "TODO", [](game_state&, game_value_parameter, game_value_parameter str) -> game_value {
        return static_cast<r_string>(str).empty();
    }, game_data_type::BOOL, GameDataDBNull_typeE, game_data_type::STRING);

}

const r_string& GameDataDBNull::get_as_string() const {
    static r_string nm(""sv);
    static r_string nm2("<dbNull>"sv);
    if (Config::get().getDBNullEqualEmptyString())
        return nm;
    return nm2;
}

bool GameDataDBNull::equals(const game_data* other) const {
    if (Config::get().getDBNullEqualEmptyString())
        if (std::string_view(other->type_as_string()) == "string"sv)
            return ((game_data_string*)other)->raw_string.empty();
    return std::string_view(other->type_as_string()) == "dbNull"sv;
}
