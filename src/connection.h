#pragma once
#include <intercept.hpp>
#include "mariadb++/connection.hpp"
#include "res.h"
#include "query.h"
using namespace intercept;
using namespace intercept::types;



class Connection {
public:
    static bool throwQueryError(game_state& gs, mariadb::connection_ref connection, size_t errorID, r_string errorMessage, r_string queryString, std::optional<sourcedocpos> sourcePosition = {});
    static [[nodiscard]] GameDataDBAsyncResult* pushAsyncQuery(game_state& gs, mariadb::connection_ref connection, ref<GameDataDBQuery> query);

    static game_value cmd_createConnectionArray(game_state&, game_value_parameter right);
    static game_value cmd_createConnectionConfig(game_state&, game_value_parameter right);
    static game_value cmd_execute(game_state&, game_value_parameter con, game_value_parameter qu);
    static game_value cmd_executeAsync(game_state&, game_value_parameter con, game_value_parameter qu);
    static game_value cmd_ping(game_state&, game_value_parameter con);
    static game_value cmd_isConnected(game_state&, game_value_parameter con);
    static game_value cmd_addErrorHandler(game_state&, game_value_parameter con, game_value_parameter handler);
    static game_value cmd_loadSchema(game_state&, game_value_parameter con, game_value_parameter name);



	static void initCommands();
	static inline sqf_script_type GameDataDBConnection_type;
	static inline game_data_type GameDataDBConnection_typeE;


    static inline types::registered_sqf_function handle_cmd_createConnection;
    static inline types::registered_sqf_function handle_cmd_createConnectionConfig;
    static inline types::registered_sqf_function handle_cmd_execute;
    static inline types::registered_sqf_function handle_cmd_executeAsync;
    static inline types::registered_sqf_function handle_cmd_ping;
    static inline types::registered_sqf_function handle_cmd_isConnected;
    static inline types::registered_sqf_function handle_cmd_addErrorHandler;
    static inline types::registered_sqf_function handle_cmd_loadSchema;
};