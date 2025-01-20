//
// Created by liyu on 12/23/23.
//

#include "vector/TimestampColumnVector.h"
#include "MyStringdateToday.cpp"

// 我添加了
void TimestampColumnVector::add(long micros) {
    if (writeIndex >= getLength()) {
        ensureSize(writeIndex * 2, true);  // 扩展空间
    }
    set(writeIndex++, micros);  // 将微秒时间戳存储到当前位置
}
void TimestampColumnVector::add(const std::chrono::system_clock::time_point& value) {
    if (writeIndex >= getLength()) {
        ensureSize(writeIndex * 2, true);  // 扩展空间
    }
    long micros = std::chrono::duration_cast<std::chrono::microseconds>(value.time_since_epoch()).count();
    set(writeIndex++, micros);  // 将微秒时间戳存储到当前位置
}
void TimestampColumnVector::add(const std::string& value) {
    if (writeIndex >= getLength()) {
        ensureSize(writeIndex * 2, true);  // 扩展空间
    }
    long micros = stringTimestampToMicros(value);  // 将字符串时间戳转换为微秒
    set(writeIndex++, micros);  // 存储微秒时间戳
}
void TimestampColumnVector::ensureSize(int size, bool preserveData) {
    // 调用父类的 ensureSize
    ColumnVector::ensureSize(size, preserveData);

    // 如果新大小小于等于当前容量，直接返回
    if (size <= length) {
        return;
    }

    // 扩展数组
    long* oldTimes = times;
    times = new long[size];  // 分配新的数组

    // 初始化新数组（与 Java 中的 Arrays.fill() 类似）
    // 使用 const 定义一个常量，表示默认的时间戳值（微秒）
    const long DEFAULT_TIMESTAMP_MICROS = 0;  // 也可以选择其他合理的默认值

    std::fill(times, times + size, DEFAULT_TIMESTAMP_MICROS);

    // 更新内存使用量
    memoryUsage += sizeof(long) * size;
    length = size;

    // 如果需要保留数据，将旧数据复制到新数组
    if (preserveData) {
        // if (isRepeating) {
        //     times[0] = oldTimes[0];  // 如果数据重复，只保留第一个元素
        // } else {
            std::copy(oldTimes, oldTimes + length, times);  // 否则，复制所有数据
        // }
    }

    // 释放旧数组内存
    delete[] oldTimes;
}


TimestampColumnVector::TimestampColumnVector(int precision, bool encoding): ColumnVector(VectorizedRowBatch::DEFAULT_SIZE, encoding) {
    TimestampColumnVector(VectorizedRowBatch::DEFAULT_SIZE, precision, encoding);
}

TimestampColumnVector::TimestampColumnVector(uint64_t len, int precision, bool encoding): ColumnVector(len, encoding) {
    this->precision = precision;
    if(encoding) {
        posix_memalign(reinterpret_cast<void **>(&this->times), 64,
                       len * sizeof(long));
    } else {
        this->times = nullptr;
    }
}


void TimestampColumnVector::close() {
    if(!closed) {
        ColumnVector::close();
        if(encoding && this->times != nullptr) {
            free(this->times);
        }
        this->times = nullptr;
    }
}

void TimestampColumnVector::print(int rowCount) {
    throw InvalidArgumentException("not support print longcolumnvector.");
//    for(int i = 0; i < rowCount; i++) {
//        std::cout<<longVector[i]<<std::endl;
//		std::cout<<intVector[i]<<std::endl;
//    }
}

TimestampColumnVector::~TimestampColumnVector() {
    if(!closed) {
        TimestampColumnVector::close();
    }
}

void * TimestampColumnVector::current() {
    if(this->times == nullptr) {
        return nullptr;
    } else {
        return this->times + readIndex;
    }
}

/**
     * Set a row from a value, which is the days from 1970-1-1 UTC.
     * We assume the entry has already been isRepeated adjusted.
     *
     * @param elementNum
     * @param days
 */
void TimestampColumnVector::set(int elementNum, long ts) {
    if(elementNum >= writeIndex) {
        writeIndex = elementNum + 1;
    }
    times[elementNum] = ts;
    // TODO: isNull
}