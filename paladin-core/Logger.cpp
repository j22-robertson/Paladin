//
// Created by James Robertson on 01/02/2026.
//
#include "Logger.h"

PaladinLogger::PaladinLogger()
{
    std::filesystem::path root_path = PALADIN_ROOT_DIR;
    paladin_log_folder = root_path / "paladin-logs";
    if (std::filesystem::exists(paladin_log_folder))
    {
        paladin_log_file_path = paladin_log_folder / "paladin-log.txt";
        if (!paladin_log_file.is_open())
        {

            paladin_log_file.open(paladin_log_file_path);
            paladin_log_file.clear();
        }
    }
    else
    {
        if (!std::filesystem::create_directory(paladin_log_folder))
        {
            std::cout << "Failed to create Directory : " << paladin_log_folder << std::endl;
        }
        else
        {
            paladin_log_file_path = paladin_log_folder / "paladin-log.txt";

            if (!paladin_log_file.is_open())
            {
                paladin_log_file.open(paladin_log_file_path, std::ios::app);
            }
        }
    }
}

PaladinLogger::~PaladinLogger()
{
    paladin_log_file.close();
}

void PaladinLogger::Log(LogContext context, const std::string& message)
{
    std::time_t now = std::time(nullptr);

    const std::tm* time_info = std::localtime(&now);
    char time_stamp[20];
    strftime(time_stamp, sizeof(time_stamp), "%Y-%m-%d %H:%M:%S", time_info);

    std::stringstream log_entry;
    log_entry << "[" << time_stamp << "] " << ContextToString(context)<<":" << message << std::endl;
    std::cout << log_entry.str();

    if (paladin_log_file.is_open())
    {
        paladin_log_file << log_entry.str();
        paladin_log_file.flush();
    }
}

PaladinLogger& PaladinLogger::Get()
{
    static PaladinLogger instance;
    return instance;
}

std::string PaladinLogger::ContextToString(LogContext context)
{
    switch (context) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARN:
        return "WARNING";
    case ERR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}