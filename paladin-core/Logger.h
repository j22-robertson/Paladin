//
// Created by James Robertson on 01/02/2026.
//

#ifndef PALADIN_LOGGER_H
#define PALADIN_LOGGER_H
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <fstream>
#include <filesystem>
//Ref: https://www.geeksforgeeks.org/cpp/logging-system-in-cpp/

enum LogContext
{
    DEBUG,
    INFO,
    WARN,
    ERR,
    FATAL,
};

class PaladinLogger {
    //TODO: Create a nice general logger
public:
    PaladinLogger(const PaladinLogger&) = delete;
    PaladinLogger& operator=(const PaladinLogger&) = delete;
    PaladinLogger();
    ~PaladinLogger();

    void log(LogContext context, const std::string& message);
    static PaladinLogger& Get();
private:
    static std::string log_context_to_string(LogContext context);
    std::ofstream paladin_log_file;
    std::filesystem::path paladin_log_folder;
    std::filesystem::path paladin_log_file_path;
};


#endif //PALADIN_LOGGER_H
