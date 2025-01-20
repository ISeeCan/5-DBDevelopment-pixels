#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>

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

const long MICROS_PER_SEC = 1000000;
const long NANOS_PER_MICROS = 1000;

long stringTimestampToMicros(const std::string& timestamp) {
    std::tm tm = {};
    std::istringstream ss(timestamp);
    
    // 解析日期时间字符串，格式为 "yyyy-MM-dd HH:mm:ss[.SSSSSS]"
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        throw std::invalid_argument("Invalid timestamp format");
    }

    // 处理毫秒和微秒部分
    long microseconds = 0;
    if (ss.peek() == '.') {
        ss.ignore(); // 跳过小数点

        std::string fractionalPart;
        ss >> fractionalPart;

        // 根据小数部分的长度，计算微秒值
        size_t len = fractionalPart.length();
        if (len > 6) {
            throw std::invalid_argument("Fractional part too long");
        }
        microseconds += std::stol(fractionalPart) * (MICROS_PER_SEC / (long)pow(10, len));
    }

    // 将 tm 转换为 time_t
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    // 计算自1970年1月1日以来的微秒
    long seconds = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    return seconds * MICROS_PER_SEC + microseconds;
}