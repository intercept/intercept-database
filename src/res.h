#pragma once
#include <intercept.hpp>
#include <mariadb++/result_set.hpp>
#include <future>
using namespace intercept;
using namespace intercept::types;

//#TODO rename sourcefile to queryResult. Cannot use result.cpp because mysql connector already uses that

class Result {
public:

	static game_value cmd_affectedRows(uintptr_t, game_value_parameter right);
	static game_value cmd_lastInsertId(uintptr_t, game_value_parameter right);
    static game_value cmd_toArray(uintptr_t, game_value_parameter right);
    static game_value cmd_bindCallback(uintptr_t, game_value_parameter left, game_value_parameter right);
    static game_value cmd_waitForResult(uintptr_t, game_value_parameter right);

	static void initCommands();
	static inline sqf_script_type GameDataDBResult_type;
	static inline game_data_type GameDataDBResult_typeE;
    static inline sqf_script_type GameDataDBAsyncResult_type;
    static inline game_data_type GameDataDBAsyncResult_typeE;

	static inline types::registered_sqf_function handle_cmd_affectedRows;
	static inline types::registered_sqf_function handle_cmd_lastInsertId;
    static inline types::registered_sqf_function handle_cmd_toArray;
    static inline types::registered_sqf_function handle_cmd_bindCallback;
    static inline types::registered_sqf_function handle_cmd_waitForResult;
};


class GameDataDBResult : public game_data {

public:
    GameDataDBResult() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return Result::GameDataDBResult_type; }
    ~GameDataDBResult() override {};

    bool get_as_bool() const override { return true; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("TODO"sv); return nm; }
    game_data* copy() const override { return new GameDataDBResult(*this); }
    r_string to_string() const override { return r_string("Use the dbResult* commands to read me"sv); }
    //virtual bool equals(const game_data*) const override; //#TODO isEqualTo on hashMaps would be quite nice I guess?
    const char* type_as_string() const override { return "databaseResult"; }
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

    mariadb::result_set_ref res;
};

class GameDataDBAsyncResult : public game_data {

public:
    GameDataDBAsyncResult() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return Result::GameDataDBAsyncResult_type; }
    ~GameDataDBAsyncResult() override {};

    bool get_as_bool() const override { return true; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("Use a callback or dbWaitForResult to get my result"sv); return nm; }
    game_data* copy() const override {
        return new GameDataDBAsyncResult(*this);
    } //#TODO can't do that dave
    r_string to_string() const override { return r_string("TODO"sv); }
    //virtual bool equals(const game_data*) const override; //#TODO isEqualTo on hashMaps would be quite nice I guess?
    const char* type_as_string() const override { return "databaseResultAsync"; }
    bool is_nil() const override { return false; }
    bool can_serialize() override { return false; }//Setting this to false causes a fail in scheduled and global vars

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
    struct dataT {
        std::future<mariadb::result_set_ref> res;
        game_value callback;
        game_value callbackArgs;
    };
    std::shared_ptr<dataT> data;


};