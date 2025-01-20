//
// Created by yuly on 06.04.23.
//

#include "vector/DateColumnVector.h"

//我添加的内容
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>
// #include "MyStringdateToday.cpp"
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



void DateColumnVector::add(int value) {
        if (writeIndex >= getLength()) {
            ensureSize(writeIndex * 2, true);  // 扩展空间
        }
        dates[writeIndex] = value;  // 存储数据
        isNull[writeIndex++] = false;  // 标记数据有效
    }

    void DateColumnVector::add(int64_t value) {
        if (writeIndex >= getLength()) {
            ensureSize(writeIndex * 2, true);  // 扩展空间
        }
        set(writeIndex++, value);  // 使用 set 存储 Date 类型数据
    }

    void DateColumnVector::add(std::string &value) {
        if (writeIndex >= getLength()) {
            ensureSize(writeIndex * 2, true);  // 扩展空间
        }
        set(writeIndex++, stringDateToDay(value));  // 将字符串转换为日期并存储
    }

void DateColumnVector::ensureSize(uint64_t size, bool preserveData) {
    // 调用父类的 ensureSize
    ColumnVector::ensureSize(size, preserveData);
    // 如果新大小小于等于当前容量，直接返回
    if (size <= length) {
        return;
    }
    // 扩展数组
    int* oldDates = dates;
    dates = new int[size];  // 分配新的数组
    // 更新内存使用量
    memoryUsage += sizeof(int) * size;
    length = size;
    // 如果需要保留数据，将旧数据复制到新数组
    if (preserveData) {
        // if (isRepeating) {
        //     dates[0] = oldDates[0];  // 如果数据重复，只保留第一个元素
        // } else {
            std::copy(oldDates, oldDates + length, dates);  // 否则，复制所有数据
        //}
    }
    // 释放旧数组内存
    delete[] oldDates;
}
//

DateColumnVector::DateColumnVector(uint64_t len, bool encoding): ColumnVector(len, encoding) {
	if(encoding) {
        posix_memalign(reinterpret_cast<void **>(&dates), 32,
                       len * sizeof(int32_t));
	} else {
		this->dates = nullptr;
	}
	memoryUsage += (long) sizeof(int) * len;
}

void DateColumnVector::close() {
	if(!closed) {
		if(encoding && dates != nullptr) {
			free(dates);
		}
		dates = nullptr;
		ColumnVector::close();
	}
}

void DateColumnVector::print(int rowCount) {
	for(int i = 0; i < rowCount; i++) {
		std::cout<<dates[i]<<std::endl;
	}
}

DateColumnVector::~DateColumnVector() {
	if(!closed) {
		DateColumnVector::close();
	}
}

/**
     * Set a row from a value, which is the days from 1970-1-1 UTC.
     * We assume the entry has already been isRepeated adjusted.
     *
     * @param elementNum
     * @param days
 */
void DateColumnVector::set(int elementNum, int days) {
	if(elementNum >= writeIndex) {
		writeIndex = elementNum + 1;
	}
	dates[elementNum] = days;
	// TODO: isNull
}

void * DateColumnVector::current() {
    if(dates == nullptr) {
        return nullptr;
    } else {
        return dates + readIndex;
    }
}
