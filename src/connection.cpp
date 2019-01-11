#include "connection.h"
#include <mariadb++/connection.hpp>
#include "query.h"
#include "res.h"

using namespace intercept::client;


class GameDataDBConnection : public game_data {

public:
    GameDataDBConnection() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return Connection::GameDataDBConnection_type; }
    ~GameDataDBConnection() override {};

    bool get_as_bool() const override { return true; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("stuff"sv); return nm; }
    game_data* copy() const override { return new GameDataDBConnection(*this); } //#TODO make sure this works
    r_string to_string() const override {
        if (!session) return r_string("<no session>"sv);
        if (!session->connected()) return r_string("<not connected>"sv);
        return "<connected to database: " + session->schema() + ">";        
    }
    //virtual bool equals(const game_data*) const override; //#TODO isEqualTo on hashMaps would be quite nice I guess?
    const char* type_as_string() const override { return "databaseConnection"; }
    bool is_nil() const override { return false; }
    bool can_serialize() override { return true; }//Setting this to false causes a fail in scheduled and global vars

    serialization_return serialize(param_archive& ar) override {
        game_data::serialize(ar);
        //size_t entryCount;
        //if (ar._isExporting) entryCount = map.size();
        //ar.serialize("entryCount"sv, entryCount, 1);
        //#TODO add array serialization functions
        //ar._p1->add_array_entry()
        //if (!ar._isExporting) {
        //
        //    for (int i = 0; i < entryCount; ++i) {
        //        s
        //    }
        //
        //
        //
        //}
        return serialization_return::no_error;
    }

    mariadb::connection_ref session;
};

game_data* createGameDataDBConnection(param_archive* ar) {
    auto x = new GameDataDBConnection();
    if (ar)
        x->serialize(*ar);
    return x;
}

game_value Connection::cmd_createConnectionArray(uintptr_t, game_value_parameter right) {
    //#TODO error checking
    r_string ip = right[0];
    int port = right[1];
    r_string user = right[2];
    r_string pw = right[3];
    r_string db = right[4];

    auto acc = mariadb::account::create(ip, user, pw, db, port);
    

    auto newCon = new GameDataDBConnection();

    newCon->session = mariadb::connection::create(acc);


    return newCon;
}

game_value Connection::cmd_query(uintptr_t, game_value_parameter con, game_value_parameter qu) {
    
    auto session = con.get_as<GameDataDBConnection>()->session;
    auto query = qu.get_as<GameDataDBQuery>();

    auto statement = session->create_statement(query->queryString);

    uint32_t idx = 0;
    for (auto& it : query->boundValues) {
        
        switch (it.type_enum()) {
            case game_data_type::SCALAR: statement->set_float(idx++, static_cast<float>(it)); break;
            case game_data_type::BOOL: statement->set_boolean(idx++, static_cast<bool>(it)); break;
            case game_data_type::STRING: statement->set_string(idx++, static_cast<r_string>(it)); break;
            default: ;
        }
    }
    auto res = statement->query();
    
    auto gd_res = new GameDataDBResult();
    gd_res->res = res;

    return gd_res;
}

void Connection::initCommands() {
    
    auto dbType = host::register_sqf_type("DBCON"sv, "databaseConnection"sv, "TODO"sv, "databaseConnection"sv, createGameDataDBConnection);
    GameDataDBConnection_typeE = dbType.first;
    GameDataDBConnection_type = dbType.second;


    handle_cmd_createConnection = host::register_sqf_command("db_createConnection", "TODO", Connection::cmd_createConnectionArray, GameDataDBConnection_typeE, game_data_type::ARRAY);
    handle_cmd_query = host::register_sqf_command("db_query", "TODO", Connection::cmd_query, Query::GameDataDBQuery_typeE, GameDataDBConnection_typeE, Query::GameDataDBQuery_typeE);
}
