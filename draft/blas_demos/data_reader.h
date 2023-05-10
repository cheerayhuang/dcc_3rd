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
        free(dict_data_);
        free(seed_data_);
        fin_.close();
    }

    void swap(DataReader& other) {
        std::swap(seed_data_, other.seed_data_);
        std::swap(dict_data_, other.dict_data_);
        std::swap(seed_file_, other.seed_file_);
        std::swap(dict_file_, other.dict_file_);
        std::swap(fin_, other.fin_);
    }

    float* GetSeedData() const {
        return seed_data_;
    }

    float* GetDictData() const {
        return dict_data_;
    }

    void Read() {
        _ReadSeedFile();
        _ReadDictFile();
    }

private:
    void _ReadDictFile() {
        fin_.open(dict_file_);
        dict_data_ = static_cast<float*>(
            memalign(kAlign32Bit, sizeof(float)*kMatrixDimension*kDictVecNum));

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
            memalign(kAlign32Bit, sizeof(float)*kMatrixDimension*kSeedVecNum));

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
