/*
 * 数据读入类 DataReader。
 *
 * 直接放弃 double 类型，采用 float 读入，放弃部分精度。
 */

#pragma once

#include <string>
#include <fstream>
#include <limits>
#include <memory>

#include <malloc.h>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "constants.h"


class DataReader {

public:
    DataReader(std::string dict_file="dict_vec.csv",
        std::string seed_file="seed_vec.csv")
        : dict_file_(dict_file), seed_file_(seed_file) {
            logger_ = spdlog::stdout_color_mt("DataReader");
            logger_->set_level(spdlog::level::info);
        }

    DataReader(DataReader&& other) {
        seed_data_ = other.seed_data_;
        dict_data_ = other.dict_data_;

        fin_ = std::move(other.fin_);
        seed_file_ = std::move(other.seed_file_);
        dict_file_ = std::move(other.dict_file_);
    }

    DataReader& operator=(DataReader&& other) {
        swap(other);
        return *this;
    }

    ~DataReader() {
        fin_.close();
    }

    void swap(DataReader& other) {
        std::swap(seed_data_, other.seed_data_);
        std::swap(dict_data_, other.dict_data_);
        std::swap(seed_file_, other.seed_file_);
        std::swap(dict_file_, other.dict_file_);
        std::swap(fin_, other.fin_);
    }

    float* MoveSeedData() {
        auto res = seed_data_;
        seed_data_ = nullptr;

        return res;
    }

    float* MoveDictData() {
        auto res = dict_data_;
        dict_data_ = nullptr;

        return res;
    }

    void Read() {
        _ReadSeedFile();
        _ReadDictFile();
    }

private:
    /*
     * 下面两个函数分别读取 dict 的 csv 文件
     * 和 seed 的 csv 文件。
     *
     * 直接申请的 float 类型的指针。
     *
     * 整个过程没有做数据预处理。
     *
     */


    void _ReadDictFile() {
        fin_.open(dict_file_);

        /* 以 16bit 对齐的方式申请存储 float 内容的内存。
         * 尽管 float 类型时32bit的。这样做的目的是后续这块内存还要作为
         * unsigned short使用。
         */

        dict_data_ = static_cast<float*>(
            memalign(kAlign16Bit, sizeof(float)*kMatrixDimension*kDictVecNum));

        char delimiter;
        for (auto i = 0; i < kDictVecNum; ++i) {
            fin_.ignore(kStreamMaxSize, ',');
            for (auto j = 0; j < kMatrixDimension; ++j) {
                fin_ >> dict_data_[i*kMatrixDimension+j];
                if (j < kMatrixDimension-1) {
                    fin_ >> delimiter;
                }
            }
            if ((i+1) % 100000 == 0)
                logger_->info("Finish reading dict file line {}.", i+1);
        }
        fin_.close();
    }

    void _ReadSeedFile() {
        fin_.open(seed_file_);
        seed_data_ = static_cast<float*>(
            memalign(kAlign16Bit, sizeof(float)*kMatrixDimension*kSeedVecNum));

        char delimiter;
        for (auto i = 0; i < kSeedVecNum; ++i) {
            fin_.ignore(kStreamMaxSize, ',');
            for (auto j = 0; j < kMatrixDimension; ++j) {
                fin_ >> seed_data_[i*kMatrixDimension+j];
                if (j < kMatrixDimension-1) {
                    fin_ >> delimiter;
                }
            }
            //logger_->info("Finish reading seed file line {}.", i+1);
        }
        fin_.close();
    }

private:
    float *seed_data_ = nullptr;
    float *dict_data_ = nullptr;

    std::ifstream fin_;

    std::string dict_file_, seed_file_;

    std::shared_ptr<spdlog::logger> logger_;

    enum {
        kStreamMaxSize = std::numeric_limits<std::streamsize>::max()
    };

    /*
    enum {
        kMatrixDimension = 256,
        kDictVecNum = 1000*1000,
        kSeedVecNum = 1000,
        kAlign32Bit = 32
    };*/


};
