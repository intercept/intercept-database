#include "logger.h"

#include <fstream>
#include <ctime>
#include <Windows.h>

void Logger::init(std::filesystem::path outputDir) {
    std::filesystem::create_directories(outputDir);
    outputDirectory = std::move(outputDir);
    refreshLogfiles();
}

void Logger::logQuery(intercept::types::r_string message) {
    std::unique_lock lock(queryLog_lock);
    if (!queryLog) return;

    std::time_t t = std::time(0);   // get time now
    std::tm* now = std::localtime(&t);

    auto currentTime = std::chrono::system_clock::now();
    auto millisSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    auto timeT = std::chrono::system_clock::to_time_t(currentTime);

    char buffer [84];
    auto leng = strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&timeT));
    sprintf(&buffer[leng-1], ".%03lld", millisSinceEpoch - timeT*1000);

    *queryLog << "["sv << buffer << "] "sv << message.c_str() << "\n";
}

void Logger::refreshLogfiles() {
    std::unique_lock lock(queryLog_lock);
    if (queryLog) queryLog.reset();

    if (queryLogEnabled)
        queryLog = std::make_unique<std::ofstream>(outputDirectory/"query.log", std::ofstream::app);
}
