#pragma once

#include <sstream>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <format>

#include <platform_folders.h>

namespace Log
{
    void init();

    void push(const std::string& string);

    template <class... Args>
    inline void push(std::format_string<Args...> string, Args&&... args)
    {
        std::string formattedString = std::format(string, std::forward<Args>(args)...);
        push(formattedString);
    }

    std::string getYearString();
    std::string getMonthString();
    std::string padTimeString(const std::string timeString);

    extern std::string filename;
}