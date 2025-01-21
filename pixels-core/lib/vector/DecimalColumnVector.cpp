//
// Created by yuly on 05.04.23.
//

#include "vector/DecimalColumnVector.h"
#include "duckdb/common/types/decimal.hpp"

/**
 * The decimal column vector with precision and scale.
 * The values of this column vector are the unscaled integer value
 * of the decimal. For example, the unscaled value of 3.14, which is
 * of the type decimal(3,2), is 314. While the precision and scale
 * of this decimal are 3 and 2, respectively.
 *
 * <p><b>Note: it only supports short decimals with max precision
 * and scale 18.</b></p>
 *
 * Created at: 05/03/2022
 * Author: hank
 */
// 我添加了
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <iomanip> 
void DecimalColumnVector::add(std::string &val) {
    if (writeIndex >= length) {
            ensureSize(writeIndex * 2, true);
        }

    double dval = parseDecimal(val);
    vector[writeIndex] = static_cast<long>(dval);
    isNull[writeIndex] = false;
    writeIndex++;
}

double DecimalColumnVector::parseDecimal(const std::string &val) {
        size_t pos = 0;
        double result = std::stod(val, &pos);
        long it = (long)std::round(result * std::pow(10, scale));
        return double(it);
}

void DecimalColumnVector::add(float value) {
    add(static_cast<double>(value));  // 将 float 转换为 double 然后调用 add(double)
}

void DecimalColumnVector::add(double value) {
    if (writeIndex >= getLength()) {
        ensureSize(writeIndex * 2, true);  // 扩展空间
    }

    // 将输入值舍入到所需精度
    double roundedValue = round(value * pow(10, scale)) / pow(10, scale);

    // 检查精度是否超出
    if (std::numeric_limits<double>::digits10 < precision) {
        throw std::invalid_argument("value exceeds the allowed precision " + std::to_string(precision));
    }

    int index = writeIndex++;
    vector[index] = static_cast<long>(roundedValue);  // 将舍入后的值转换为 long 类型
    isNull[index] = false;
}


DecimalColumnVector::DecimalColumnVector(int precision, int scale, bool encoding): ColumnVector(VectorizedRowBatch::DEFAULT_SIZE, encoding) {
    DecimalColumnVector(VectorizedRowBatch::DEFAULT_SIZE, precision, scale, encoding);
}

DecimalColumnVector::DecimalColumnVector(uint64_t len, int precision, int scale,
                                         bool encoding)
    : ColumnVector(len, encoding) {
    // decimal column vector has no encoding so we don't allocate memory to
    // this->vector
    this->vector = nullptr;
    this->precision = precision;
    this->scale = scale;

    using duckdb::Decimal;
    if (precision <= Decimal::MAX_WIDTH_INT16) {
        physical_type_ = PhysicalType::INT16;
        posix_memalign(reinterpret_cast<void **>(&vector), 32,
                       len * sizeof(int16_t));
        memoryUsage += (uint64_t)sizeof(int16_t) * len;
    } else if (precision <= Decimal::MAX_WIDTH_INT32) {
        physical_type_ = PhysicalType::INT32;
        posix_memalign(reinterpret_cast<void **>(&vector), 32,
                       len * sizeof(int32_t));
        memoryUsage += (uint64_t)sizeof(int32_t) * len;
    } else if (precision <= Decimal::MAX_WIDTH_INT64) {
        physical_type_ = PhysicalType::INT64;
        memoryUsage += (uint64_t)sizeof(uint64_t) * len;
    } else if (precision <= Decimal::MAX_WIDTH_INT128) {
        physical_type_ = PhysicalType::INT128;
        memoryUsage += (uint64_t)sizeof(uint64_t) * len;
    } else {
        throw std::runtime_error(
            "Decimal precision is bigger than the maximum supported width");
    }
}

void DecimalColumnVector::close() {
    if (!closed) {
        ColumnVector::close();
        if (physical_type_ == PhysicalType::INT16 ||
            physical_type_ == PhysicalType::INT32) {
            free(vector);
        }
        vector = nullptr;
    }
}

void DecimalColumnVector::print(int rowCount) {
//    throw InvalidArgumentException("not support print Decimalcolumnvector.");
    for(int i = 0; i < rowCount; i++) {
        std::cout<<vector[i]<<std::endl;
    }
}

DecimalColumnVector::~DecimalColumnVector() {
    if(!closed) {
        DecimalColumnVector::close();
    }
}

void * DecimalColumnVector::current() {
    if(vector == nullptr) {
        return nullptr;
    } else {
        return vector + readIndex;
    }
}

int DecimalColumnVector::getPrecision() {
	return precision;
}


int DecimalColumnVector::getScale() {
	return scale;
}
