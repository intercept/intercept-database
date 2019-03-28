#pragma once
#include <intercept.hpp>
using namespace intercept;
using namespace intercept::types;



class Connection {
public:

    static game_value cmd_createConnectionArray(game_state&, game_value_parameter right);
    static game_value cmd_createConnectionConfig(game_state&, game_value_parameter right);
    static game_value cmd_execute(game_state&, game_value_parameter con, game_value_parameter qu);
    static game_value cmd_executeAsync(game_state&, game_value_parameter con, game_value_parameter qu);
    static game_value cmd_ping(game_state&, game_value_parameter con);




	static void initCommands();
	static inline sqf_script_type GameDataDBConnection_type;
	static inline game_data_type GameDataDBConnection_typeE;


    static inline types::registered_sqf_function handle_cmd_createConnection;
    static inline types::registered_sqf_function handle_cmd_createConnectionConfig;
    static inline types::registered_sqf_function handle_cmd_execute;
    static inline types::registered_sqf_function handle_cmd_executeAsync;
    static inline types::registered_sqf_function handle_cmd_ping;
};