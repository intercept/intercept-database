#pragma once
#include <intercept.hpp>
using namespace intercept;
using namespace intercept::types;

class Query {
public:

	static game_value cmd_prepareQuery(uintptr_t, game_value_parameter right);
	static game_value cmd_bindValue(uintptr_t, game_value_parameter left, game_value_parameter right);
	static game_value cmd_bindValueArray(uintptr_t, game_value_parameter left, game_value_parameter right);
	static game_value cmd_bindNamedValue(uintptr_t, game_value_parameter left, game_value_parameter right);
	static game_value cmd_bindNamedValueArray(uintptr_t, game_value_parameter left, game_value_parameter right);




	static void initCommands();
	static inline sqf_script_type GameDataDBQuery_type;
	static inline game_data_type GameDataDBQuery_typeE;

	static inline types::registered_sqf_function handle_cmd_prepareQuery;
	static inline types::registered_sqf_function handle_cmd_bindValue;
	static inline types::registered_sqf_function handle_cmd_bindValueArray;
	static inline types::registered_sqf_function handle_cmd_bindNamedValue;
	static inline types::registered_sqf_function handle_cmd_bindNamedValueArray;
};

class GameDataDBQuery : public game_data {

public:
    GameDataDBQuery() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return Query::GameDataDBQuery_type; }
    ~GameDataDBQuery() override {};

    bool get_as_bool() const override { return true; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("TODO"sv); return nm; }
    game_data* copy() const override { return new GameDataDBQuery(*this); } //#TODO make sure this works
    r_string to_string() const override { return r_string("TODO"sv); }
    //virtual bool equals(const game_data*) const override; //#TODO isEqualTo on hashMaps would be quite nice I guess?
    const char* type_as_string() const override { return "databaseQuery"; }
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

    auto_array<game_value> boundValues;
    r_string queryString;
};