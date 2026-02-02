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
#include <cstdlib>
#ifdef PALADIN_LOG_ENABLE
#define PALADIN_LOG(context, message) PaladinLogger::Get().Log(context,message);

#else
#define PALADIN_LOG(context, message)
#endif


//Ref: https://www.geeksforgeeks.org/cpp/logging-system-in-cpp/

enum LogContext
{
    DEBUG,
    INFO,
    WARN,
    ERR,
    FATAL,
};

// https://www.geeksforgeeks.org/cpp/convert-wstring-to-string-in-c/
///TODO: Use wcstombs_s instead of wcstombs
inline std::string ConvertWString(std::wstring& wstr)
{
    std::size_t len = wcstombs(nullptr, wstr.c_str(), 0)+1;

    char* buffer = new char[len];

    wcstombs(buffer, wstr.c_str(), len);

    std::string str(buffer);

    delete[] buffer;
    return str;
}

inline std::string ErrorResult(std::string message, long result)
{
    std::stringstream ss;
    ss << std::hex << result << " : "<< message;
    return ss.str();
}

class PaladinLogger {
    //TODO: Create a nice general logger
public:
    PaladinLogger(const PaladinLogger&) = delete;
    PaladinLogger& operator=(const PaladinLogger&) = delete;
    PaladinLogger();
    ~PaladinLogger();

    void Log(LogContext context, const std::string& message);
    static PaladinLogger& Get();
private:
    static std::string ContextToString(LogContext context);
    std::ofstream paladin_log_file;
    std::filesystem::path paladin_log_folder;
    std::filesystem::path paladin_log_file_path;
};


#endif //PALADIN_LOGGER_H
