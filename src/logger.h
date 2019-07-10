#pragma once
#include <intercept.hpp>
#include <mutex>
#include <filesystem>
#include <fstream>
#include "../intercept/src/host/common/singleton.hpp"


namespace intercept {
    namespace types {
        class r_string;
    }
}


class Logger : public intercept::singleton<Logger> {
public:
    void init(std::filesystem::path outputDirectory);
    void setQueryLogEnabled(bool isEnabled) {
        queryLogEnabled = isEnabled;
        refreshLogfiles();
    }

    void logQuery(intercept::types::r_string message);
    bool isQueryLogEnabled() const {
        return queryLogEnabled;
    }

    void setThreadLogEnabled(bool isEnabled) {
        threadLogEnabled = isEnabled;
        refreshLogfiles();
    }

    void logThread(intercept::types::r_string message);
    bool isThreadLogEnabled() const {
        return threadLogEnabled;
    }


private:
    void pushTimestamp(std::ostream& str) const;
    void refreshLogfiles();

    std::mutex queryLog_lock;
    bool queryLogEnabled = false;
    std::unique_ptr<std::ofstream> queryLog;

    std::mutex threadLog_lock;
    bool threadLogEnabled = false;
    std::unique_ptr<std::ofstream> threadLog;

    std::filesystem::path outputDirectory;


};
