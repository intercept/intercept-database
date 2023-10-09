#include "logger.h"

#include <fstream>
#include <ctime>

void Logger::init(std::filesystem::path outputDir) {
    std::filesystem::create_directories(outputDir);
    outputDirectory = std::move(outputDir);
    //cleanupOldLogs(10); TODO
    refreshLogfiles();
}

void Logger::logQuery(intercept::types::r_string message) {
    std::unique_lock lock(queryLog_lock);
    if (!queryLog) return;

    pushTimestamp(*queryLog);

    *queryLog << message.c_str() << "\n";
}

void Logger::logThread(intercept::types::r_string message) {
    std::unique_lock lock(threadLog_lock);
    if (!threadLogEnabled) return;

    pushTimestamp(*threadLog);

    *threadLog << message.c_str() << "\n";
}

void Logger::pushTimestamp(std::ostream& str) const {
    auto currentTime = std::chrono::system_clock::now();
    auto millisSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    auto timeT = std::chrono::system_clock::to_time_t(currentTime);

    char buffer [84];
    auto leng = strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&timeT));
    sprintf(&buffer[leng-1], ".%03lld", millisSinceEpoch - timeT*1000);

    str << "["sv << buffer << "] "sv;
}

void Logger::refreshLogfiles() {
    // Get the current timestamp
    auto currentTime = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(currentTime);

    char timestampBuffer[20];
    strftime(timestampBuffer, sizeof(timestampBuffer), "%Y%m%d%H%M%S", localtime(&timeT));

    std::string currentTimeString = timestampBuffer;

    
    // Add the timestamp to the log file names
    std::string queryLogFileName = "query_" + currentTimeString + ".log";
    std::string threadLogFileName = "thread_" + currentTimeString + ".log";

    {
        std::unique_lock lock(queryLog_lock);
        if (queryLog) queryLog.reset();

        if (queryLogEnabled)
            queryLog = std::make_unique<std::ofstream>(outputDirectory / queryLogFileName, std::ofstream::app);
    }
    {
        std::unique_lock lock(threadLog_lock);
        if (threadLog) threadLog.reset();

        if (threadLogEnabled)
            threadLog = std::make_unique<std::ofstream>(outputDirectory / threadLogFileName, std::ofstream::app);
    }
}



