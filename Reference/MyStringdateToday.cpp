#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

int stringDateToDay(const std::string& date) {
    std::tm tm = {};
    std::istringstream ss(date);
    
    // 解析日期字符串，格式为 yyyy-MM-dd
    ss >> std::get_time(&tm, "%Y-%m-%d");
    
    if (ss.fail()) {
        throw std::invalid_argument("Invalid date format");
    }

    // 将 tm 转换为 time_t
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    // 计算从 1970-01-01 开始的天数
    auto epoch = std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point{});
    return static_cast<int>((std::chrono::system_clock::to_time_t(tp) - epoch) / (60 * 60 * 24));
}

//这已经是旧版了，最新版在lib的verctor源文件目录下