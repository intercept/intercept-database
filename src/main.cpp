#include <intercept.hpp>
//#include <mysqlx/xdevapi.h>
#include "result.h"
#include "query.h"
#include "connection.h"

int intercept::api_version() { //This is required for the plugin to work.
    return 1;
}

void intercept::register_interfaces() {
    
}

void intercept::pre_start() {
    Result::initCommands();
    Query::initCommands();
    Connection::initCommands();
}

void intercept::pre_init() {

    //mysqlx::Session sess("localhost", 3306, "intercepttest", "password");
    //
    //auto db = sess.sql("USE ?").bind("databasename").execute();
    //
    //
    //auto query = sess.sql("SELECT * FROM ?").bind("testdata");
    //query.execute();


    intercept::sqf::system_chat("Intercept database has been loaded");
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