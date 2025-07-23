#include "IO/Log.hpp"

// std::stringstream Log::stream;
std::string Log::filename;

void Log::init()
{
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);
    filename = "planeturem-log-" + getYearString() + "-" + getMonthString() + "-" + padTimeString(std::to_string(datetime.tm_mday)) + "-"
        + padTimeString(std::to_string(datetime.tm_hour)) + "-" + padTimeString(std::to_string(datetime.tm_min)) + "-" + padTimeString(std::to_string(datetime.tm_sec));
}

void Log::push(const std::string& string)
{
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);
    std::string timeString = "[" + padTimeString(std::to_string(datetime.tm_hour)) + ":" + padTimeString(std::to_string(datetime.tm_min)) +
        ":" + padTimeString(std::to_string(datetime.tm_sec)) + "] ";

    std::filesystem::path dir(sago::getDataHome() + "/Planeturem");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    dir = std::filesystem::path(sago::getDataHome() + "/Planeturem/Logs");
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    std::fstream out(sago::getDataHome() + "/Planeturem/Logs/" + filename + ".txt", std::ios::out | std::ios::app);

    out << timeString << string;

    out.close();
}

std::string Log::getYearString()
{
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);

    return std::to_string(1900 + datetime.tm_year);
}

std::string Log::getMonthString()
{
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);

    std::string string = std::to_string(1 + datetime.tm_mon);

    return padTimeString(string);
}

std::string Log::padTimeString(const std::string timeString)
{
    if (timeString.size() < 2)
    {
        return "0" + timeString;
    }
    return timeString;
}