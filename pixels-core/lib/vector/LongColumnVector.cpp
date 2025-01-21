//
// Created by liyu on 3/17/23.
//

//Exemple Implement


#include "vector/LongColumnVector.h"
#include <algorithm>

LongColumnVector::LongColumnVector(uint64_t len, bool encoding, bool isLong): ColumnVector(len, encoding) {
    if(isLong) {
        posix_memalign(reinterpret_cast<void **>(&longVector), 32,
                       len * sizeof(int64_t));
        intVector = nullptr;
    } else {
        longVector = nullptr;
        posix_memalign(reinterpret_cast<void **>(&intVector), 32,
                       len * sizeof(int32_t));
    }

    this->isLong = isLong;
    memoryUsage += (long) sizeof(long) * len;
}

void LongColumnVector::close() {
	if(!closed) {
		ColumnVector::close();
		if(encoding && longVector != nullptr) {
			free(longVector);
		}
		if(encoding && intVector != nullptr) {
			free(intVector);
		}
		longVector = nullptr;
		intVector = nullptr;
	}
}

void LongColumnVector::print(int rowCount) {
	throw InvalidArgumentException("not support print longcolumnvector.");
//    for(int i = 0; i < rowCount; i++) {
//        std::cout<<longVector[i]<<std::endl;
//		std::cout<<intVector[i]<<std::endl;
//    }
}

LongColumnVector::~LongColumnVector() {
	if(!closed) {
		LongColumnVector::close();
	}
}

void * LongColumnVector::current() {
    if(isLong) {
        if(longVector == nullptr) {
            return nullptr;
        } else {
            return longVector + readIndex;
        }
    } else {
        if(intVector == nullptr) {
            return nullptr;
        } else {
            return intVector + readIndex;
        }
    }
}


//这些 add 函数属于 LongColumnVector 类，它们用于向列中添加不同类型的值。
//类中可能会存储不同类型的数据（例如：bool, int, int64_t, string 等），
//而这些 add 方法实现了根据输入数据类型将数据添加到相应的存储容器中的功能。

void LongColumnVector::add(std::string &value) {
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "true") {
        add(1);
    } else if (value == "false") {
        add(0);
    } else {
        add(std::stol(value));
    }
}

void LongColumnVector::add(bool value) {
    add(value ? 1 : 0);
}

void LongColumnVector::add(int64_t value) {
    if (writeIndex >= length) {
        ensureSize(writeIndex * 2, true);
    }
    int index = writeIndex++;
    if(isLong) {            //为啥只有他进行了额外设置
        longVector[index] = value;
    } else {
        intVector[index] = value;
    }
    isNull[index] = false;
}

void LongColumnVector::add(int value) {
    if (writeIndex >= length) {
        ensureSize(writeIndex * 2, true);
    }
    int index = writeIndex++;
    if(isLong) {
        longVector[index] = value;
    } else {
        intVector[index] = value;
    }
    isNull[index] = false;
}

/*扩展列的存储空间，以便能容纳更多的数据。
此函数根据当前存储的 length（列的当前容量）与需要的 size（目标容量）来决定是否需要分配更多内存，
并在必要时执行内存扩展。
*/

void LongColumnVector::ensureSize(uint64_t size, bool preserveData) {
    ColumnVector::ensureSize(size, preserveData);
    if (length < size) {
        if (isLong) {
            long *oldVector = longVector;
            posix_memalign(reinterpret_cast<void **>(&longVector), 32,
                           size * sizeof(int64_t));
            if (preserveData) {
                std::copy(oldVector, oldVector + length, longVector);
            }
            delete[] oldVector;
            memoryUsage += (long) sizeof(long) * (size - length);
            resize(size);
        } else {
            long *oldVector = intVector;
            posix_memalign(reinterpret_cast<void **>(&intVector), 32,
                           size * sizeof(int32_t));
            if (preserveData) {
                std::copy(oldVector, oldVector + length, intVector);
            }
            delete[] oldVector;
            memoryUsage += (long) sizeof(int) * (size - length);
            resize(size);
        }
    }
}

bool LongColumnVector::isLongVector() {
    return isLong;
}
