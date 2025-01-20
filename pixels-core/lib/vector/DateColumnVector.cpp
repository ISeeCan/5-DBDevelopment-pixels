//
// Created by yuly on 06.04.23.
//

#include "vector/DateColumnVector.h"

//我添加的内容
#include "MyStringdateToday.cpp"

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
