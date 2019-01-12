#include <intercept.hpp>
//#include <mysqlx/xdevapi.h>
#include "res.h"
#include "query.h"
#include "connection.h"
#include "yaml-cpp/exceptions.h"

int intercept::api_version() { //This is required for the plugin to work.
    return 1;
}
auto_array<ref<GameDataDBAsyncResult>> asyncWork;
void intercept::register_interfaces() {
    
}

void intercept::pre_start() {
    Result::initCommands();
    Query::initCommands();
    Connection::initCommands();
    Config::initCommands();
}

void intercept::pre_init() {
    try {
        Config::get().reloadConfig();
    }
    catch (YAML::BadConversion& x) {
        sqf::diag_log ("Database config error " + x.msg + " in L" + std::to_string(x.mark.line));
        sqf::system_chat("Database config error " + x.msg + " in L" + std::to_string(x.mark.line));
    }
    catch (std::runtime_error& x) {
        sqf::diag_log(r_string("Database config error ") + x.what());
        sqf::system_chat(r_string("Database config error ") + x.what());
    }
    catch (YAML::ParserException& x) {
        sqf::diag_log("Database config error " + x.msg + " in L" + std::to_string(x.mark.line));
        sqf::system_chat("Database config error " + x.msg + " in L" + std::to_string(x.mark.line));
    }

    intercept::sqf::system_chat("Intercept database has been loaded");
}

void intercept::on_frame() {

    std::vector<ref<GameDataDBAsyncResult>> done;


    auto p = std::stable_partition(asyncWork.begin(), asyncWork.end(),
        [&](const auto& x) { return x->data->res.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready; });
    // range insert with move
    done.insert(done.end(), std::make_move_iterator(p),
        std::make_move_iterator(asyncWork.end()));
    // erase the moved-from elements.
    asyncWork.erase(p, asyncWork.end());

    for (auto& it : done) {
        auto gd_res = new GameDataDBResult();
        gd_res->res = it->data->res.get();

        sqf::call(it->data->callback, { gd_res, it->data->callbackArgs });
    }
}


/*

DB_Connection = db_createConnection [ip, port, username, password, database];
//or db_createConnection "configName" not sure if seperate config file or Arma config, probably seperate




DB_Cache_InsertQuery = db_prepareQuery 'INSERT INTO positions (name, x, y, z) VALUES (?,?,?,?)';

private _insertQuery = db_copyQuery DB_Cache_InsertQuery; //Such that you can prepare queries that you often reuse, maybe even with values already bound to the query

_insertQuery db_bindValue (name player); //Binds value to next available placeholder
_insertQuery db_bindValueArray (position player); //foreach value {Binds value to next available placeholder}. So this sets x, y and z

//db_bindNamedValue ["name", value]
//db_bindNamedValueArray [["name", value], ["name", value]]


private _asyncResult = DB_Connection db_queryAsync _insertQuery;

_asyncResult db_bindCallback [{
    params ["_result", "_arguments"];
    private _affectedRows = db_resultAffectedRows _result;
    systemChat format ["The query has completed! %1 rows affected", _affectedRows];
    
    
    //{params ["_row"]; ...} forEach _result
    
    private _result1 = db_resultGetColumn 0; //_result is DBValue type.
    db_valueGetType _result1; //"ARRAY"
    private _resultRowArray = db_valueToArray _result1; //Converts DBValue which is array of more DBValue's to a SQF array out of string/number/arrays
    systemChat ("row1: " + str _resultRowArray);
}, argument1];



*/