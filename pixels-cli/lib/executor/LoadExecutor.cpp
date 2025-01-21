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
// Created by gengdy on 24-11-17.
//

#include <executor/LoadExecutor.h>
#include <iostream>
#include <encoding/EncodingLevel.h>
#include <physical/storage/LocalFS.h>
#include <load/Parameters.h>
#include <chrono>
#include <load/PixelsConsumer.h>

void LoadExecutor::execute(const bpo::variables_map& ns, const std::string& command) {
    std::string schema = ns["schema"].as<std::string>();
    std::string origin = ns["origin"].as<std::string>();
    std::string target = ns["target"].as<std::string>();
    int rowNum = ns["row_num"].as<int>();
    std::string regex = ns["row_regex"].as<std::string>();
    EncodingLevel encodingLevel = EncodingLevel::from(ns["encoding_level"].as<int>());
    bool nullPadding = ns["nulls_padding"].as<bool>();

    if(origin.back() != '/') {
        origin += "/";
    }

    Parameters parameters(schema, rowNum, regex, target, encodingLevel, nullPadding);
    LocalFS localFs;
    std::vector<std::string> fileList = localFs.listPaths(origin);
    std::vector<std::string> inputFiles, loadedFiles;
    for (auto filePath : fileList) {
        inputFiles.push_back(localFs.ensureSchemePrefix(filePath));
    }

    auto startTime = std::chrono::system_clock::now();
    //调用了load类的另外一个函数
    if (startConsumers(inputFiles, parameters, loadedFiles)) {
        std::cout << command << " is successful" << std::endl;
    } else {
        std::cout << command << " failed" << std::endl;
    }
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << "Text file in " << origin << " are loaded by 1 thread in "
                << elapsedSeconds.count() << " seconds." << std::endl;
}

//创建 PixelsConsumer 类，并执行该类的run函数
bool LoadExecutor::startConsumers(const std::vector<std::string> &inputFiles, Parameters parameters,
                                  const std::vector<std::string> &loadedFiles) {
    PixelsConsumer consumer(inputFiles, parameters, loadedFiles);

    //又run了一下
    consumer.run();
    return true;
}