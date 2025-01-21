/*
 * Copyright 2024 PixelsDB.
 *
 * This file is part of Pixels.
 *
 * Pixels is free software: you can redistribute it and/or modify
 * it under the terms of the Affero GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Pixels is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Affero GNU General Public License for more details.
 *
 * You should have received a copy of the Affero GNU General Public
 * License along with Pixels.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

//
// Created by gengdy on 24-11-22.
//

#include "load/PixelsConsumer.h"
#include "encoding/EncodingLevel.h"
#include "utils/ConfigFactory.h"
#include "TypeDescription.h"
#include "vector/ColumnVector.h"
#include "vector/VectorizedRowBatch.h"
#include "physical/storage/LocalFS.h"
#include "PixelsWriterImpl.h"
#include <boost/regex.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

int PixelsConsumer::GlobalTargetPathId = 0;

PixelsConsumer::PixelsConsumer(const std::vector <std::string> &queue, const Parameters &parameters,
                               const std::vector <std::string> &loadedFiles)
                               : queue(queue), parameters(parameters), loadedFiles(loadedFiles) {}



void PixelsConsumer::run() {
    std::cout << "Start PixelsConsumer" << std::endl;
    std::string targetPath = parameters.getLoadingPath();   //获得路径
    if (targetPath.back() != '/') {                         //添加斜杠
        targetPath += '/';
    }
    std::string schemaStr = parameters.getSchema();         //获取模式
    int maxRowNum = parameters.getMaxRowNum();              //获取最大行数
    std::string regex = parameters.getRegex();              //获取正则
    EncodingLevel encodingLevel = parameters.getEncodingLevel();    //获取编码级别
    bool nullPadding = parameters.isNullsPadding();
    if (regex == "\\s") {
        regex = " ";
    }

//验收前一天又改成从文件读配置了
//虽然影响不大但是还得解决冲突...
    int pixelsStride = std::stoi(ConfigFactory::Instance().getProperty("pixel.stride"));
    int rowGroupSize = std::stoi(ConfigFactory::Instance().getProperty("row.group.size"));
    int64_t blockSize = std::stoll(ConfigFactory::Instance().getProperty("block.size"));

//    int pixelsStride = std::stoi(ConfigFactory::Instance().getProperty("pixel.stride"));
//    int rowGroupSize = std::stoi(ConfigFactory::Instance().getProperty("row.group.size"));
//    int64_t blockSize = std::stoll(ConfigFactory::Instance().getProperty("block.size"));
//     int pixelsStride = 2;       //像素步幅
//     int rowGroupSize = 100;     //行大小
//     int64_t blockSize = 1024;   //块大小


    short replication = static_cast<short>(std::stoi(ConfigFactory::Instance().getProperty("block.replication")));

    std::shared_ptr<TypeDescription> schema = TypeDescription::fromString(schemaStr);
    std::shared_ptr<VectorizedRowBatch> rowBatch = schema->createRowBatch(pixelsStride);
    std::vector<std::shared_ptr<ColumnVector>> columnVectors = rowBatch->cols;  //不知道是个啥

    std::ifstream reader;
    std::string line;

    bool initPixelsFile = true;
    std::string targetFileName = "";
    std::string targetFilePath;
    std::shared_ptr<PixelsWriter> pixelsWriter(nullptr);
    int rowCounter = 0;

    int count = 0;
    for (std::string originalFilePath : queue) {    //遍历路径
        if (!originalFilePath.empty()) {
            ++count;
            LocalFS originStorage;
            reader = originStorage.open(originalFilePath);
            if (!reader.is_open()) {        //打开文件
                std::cerr << "Error opening file: " << originalFilePath << std::endl;
                continue;
            }
            std::cout << "loading data from: " << originalFilePath << std::endl;

            while (std::getline(reader, line)) {    //逐行读取
                if (line.empty()) {
                    std::cout << "got empty line" << std::endl;
                    continue;
                }
                if (initPixelsFile) {
                    LocalFS targetStorage;
                    targetFileName = std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) + \
                                         "_" + std::to_string(this->loadedFiles.size()) + ".pxl";
                    targetFilePath = targetPath + targetFileName;
                    pixelsWriter = std::make_shared<PixelsWriterImpl>(schema, pixelsStride, rowGroupSize, targetFilePath, blockSize,
                                                                      true, encodingLevel, nullPadding,false, 1);
                }
                initPixelsFile = false;
                ++rowBatch->rowCount;
                ++rowCounter;

                std::vector<std::string> colsInLine;
                boost::sregex_token_iterator it(line.begin(), line.end(), boost::regex(regex), -1);
                for (; it != boost::sregex_token_iterator(); ++it) {
                    colsInLine.push_back(*it);
                }
                for(int i = 0; i < columnVectors.size(); ++i) {
                    if (i > colsInLine.size() || colsInLine[i].empty() || colsInLine[i] == "\\N") {
                        columnVectors[i]->addNull();
                    } else {
                        columnVectors[i]->add(colsInLine[i]);
                    }
                }

                if (rowBatch->rowCount == rowBatch->getMaxSize()) {
                    std::cout << "writing row group to file: " << targetFilePath << " rowCount:"<<rowBatch->rowCount<<std::endl;
                    pixelsWriter->addRowBatch(rowBatch);
                    rowBatch->reset();
                }

                // 创建一个新的文件
                if (rowCounter >= maxRowNum) {
                    if (rowBatch->rowCount != 0) {
                        pixelsWriter->addRowBatch(rowBatch);
                        rowBatch->reset();
                    }
                    pixelsWriter->close();
                    this->loadedFiles.push_back(targetFilePath);
                    std::cout << "Generate file: " << targetFilePath << std::endl;
                    rowCounter = 0;
                    initPixelsFile = true;
                }
            }
        }
    }
    // 剩余line写入文件
    if (rowCounter > 0) {
        if (rowBatch->rowCount != 0) {
            pixelsWriter->addRowBatch(rowBatch);
            rowBatch->reset();
        }
        pixelsWriter->close();
        this->loadedFiles.push_back(targetFilePath);
    }
    std::cout << "Exit PixelsConsumer" << std::endl;
}