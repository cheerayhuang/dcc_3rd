/*
 * 类Calc， 主要计算过程。
 */

#pragma once

#include <bitset>
#include <cmath>
#include <cstring>
#include <iostream>
#include <malloc.h>

#include "omp.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "constants.h"
#include "result.ori.h"
#include "result_writer.h"
#include "timer.h"

// 另外一种 result 排序实现，最后没有采用。
//#include "result.h"

class Calc {

public:
    explicit Calc(float *seed_data, float* dict_data, size_t round = 1)
        :seed_data_(seed_data),
        dict_data_(dict_data),
        timer_(new Timer(round)) {

        // 将之前申请的读入 dict 向量的内存重复利用，
        // 重解释转换之后，作为 unsigned short 的内存指针使用。
        // 这块内存会用于存放筛选后的 dict 向量，
        // 把筛选后的 dict 向量重排之后，连续存放在这块内存里。
        dict_data_as_ushort_ = reinterpret_cast<unsigned short*>(dict_data_);

        // 存放转换为 unsigned short 之后的 seed 向量数据。以 16bit 对齐的方式申请内存。
        seed_data_int_ = static_cast<unsigned short*>(
            memalign(kAlign16Bit, sizeof(unsigned short)*kMatrixDimension*kSeedVecNum));

        // 存放转换为 unsigned short 之后的 dict 向量数据。以 16bit 对齐的方式申请内存。
        dict_data_int_ = static_cast<unsigned short*>(
            memalign(kAlign16Bit, sizeof(unsigned short)*kMatrixDimension*kDictVecNum));

        // 由于维度 256 是定值。这里可以把指针加法的步长先提前准备好。放在 vec_index_ 里。
        vec_index_ = static_cast<unsigned*>(malloc(sizeof(unsigned)*kDictVecNum));
        vec_index_[0] = 0;

        for (auto i = 1; i < kDictVecNum; ++i) {
            vec_index_[i] = vec_index_[i-1] + kMatrixDimension;
        }

        // 提前把固定步长 256 的存放 seed 向量的内存指针提前准备好，存放在 seed_index_points_ 里。
        // 其实就是提前放好上述 seed_data_int_ + 0，+256， +512... ... 的指针。
        seed_index_points_ = static_cast<unsigned short**>(malloc(sizeof(unsigned short*)*kSeedVecNum));
        seed_index_points_[0] = seed_data_int_;

        for (auto i = 1; i < kSeedVecNum; ++i) {
            seed_index_points_[i] = seed_index_points_[i-1] + kMatrixDimension;
        }

        // 提前把固定步长 256 的存放 dict 向量的内存指针提前准备好，存放在 seed_index_points_ 里。
        dict_index_points_ = static_cast<unsigned short**>(malloc(sizeof(unsigned short*)*kDictVecNum));

        // 存放 seed 向量与 dict 向量的相似度结果。
        // Top10Similarity 是 Result 类的别名。
        for (auto &&p : all_res_) {
            p = new Top10Similarity<ResultData<unsigned>>();
        }

        // 存放真正需要计算的 dict 向量的 index ， 按照线程分成了不同的 slot 存放。
        calc_dict_indexes_size_.assign(THREAD_NUM, 0);

        logger_ = spdlog::stdout_color_mt("Calc");
        logger_->set_level(spdlog::level::info);
    }

    Calc(Calc&&) = delete;
    Calc(const Calc&&) = delete;

    Calc& operator=(const Calc&) = delete;
    Calc& operator=(Calc&&) = delete;

    ~Calc() {
        free(dict_data_);
        free(seed_data_);
        free(seed_data_int_);
        free(dict_data_int_);
        free(vec_index_);
        free(seed_index_points_);
        free(dict_index_points_);

        for(auto&& p : all_res_) {
            delete p;
        }
        delete timer_;
    }

    /*
     * 程序运行的主函数。
     * 包括3个步骤：
     * 1. 数据预处理。
     * 2. 数据内存重排。
     * 3. 计算相似度。
     */
    void Run() {
        timer_->Start();

        // 数据预处理。
        _PreHandleData();

        // 真正需要计算的 dict 数据进行内存重排，保证连续提高效率。
        _LayDataIndexFlat();

        // 计算相似度。
        _CalcSimilarity();

        timer_->Stop().WriteDurationLog();

        /*
        auto sum = 0;
        for (auto i = 0; i < THREAD_NUM; ++i) {
            //std::cout << "slot size: " << calc_dict_indexes_size_[i] << std::endl;
            logger_->info("slot {} size: {}", i, calc_dict_indexes_size_[i]);
            sum += calc_dict_indexes_[i].size();
        }
        logger_->info("the actual num of dict vec calculated: {}", sum);
        */
        ResultWriter<kTopK>().Write(all_res_, dict_index_points_, dict_data_int_);
    }

    Timer& GetRunningTime() {
        return *timer_;
    }

private:

    /*
     * 对真正需要计算的 dict 向量进行内存重排。
     *
     * 筛选出需要真正计算的 dict 向量之后，
     * 由于它们的 index 不是连续的，在内存中也不是连续的。
     * 所以需要进行重排。
     */
    inline void _LayDataIndexFlat() {

        // 由于预处理数据开启多线程，为了避免锁，
        // 这里使用对应于每个线程的一个数组(slot)
        // 来存放筛选后的 dict 向量。
        for (auto i = 0; i < THREAD_NUM; ++i) {
            auto size = calc_dict_indexes_size_[i];
            for (auto j = 0; j < size; ++j) {
                // 把不同 slot 的向量指针都存放到一个数组指针中。
                dict_index_points_[dict_index_points_num_] = calc_dict_indexes_[i][j];

                // 把不同 slot 的向量数据，重新排列到一个完整连续的数据内存中。
                // 这里没有重新申请新的内存，而是直接使用了读取数据时申请的 float 存储的内存。
                //
                //    dict_data_int_                                                        dict_data_as_ushort_
                //   +--------------------------------------------+                    +--------------------------------------------+
                //   |                                            |                    |                                            |
                //   |                                            +-------------------->                                            |
                //   +--------------------------------------------+                    +--------------------------------------------+
                //   |                                            |                    |                                            |
                //   |                                            |     +-------------->                                            |
                //   |                                            |     |              +--------------------------------------------+
                //   |                                            |     |              |                                            |
                //   |                                            |     |    +--------->                                            |
                //   |                                            |     |    |         +--------------------------------------------+
                //   |                                            |     |    |         |                                            |
                //   |                                            |     |    |         |                                            |
                //   |                                            |     |    |         |                                            |
                //   |                                            |     |    |         |                                            |
                //   +--------------------------------------------+     |    |         |                                            |
                //   |                                            +-----+    |         |                                            |
                //   |                                            |          |         |                                            |
                //   +--------------------------------------------+          |         |                                            |
                //   |                                            |          |         |                                            |
                //   |                                            |          |         |                                            |
                //   |                                            |          |         |                                            |
                //   +--------------------------------------------+          |         |                                            |
                //   |                                            +----------+         |                                            |
                //   +--------------------------------------------+                    +--------------------------------------------+
                //
                std::memcpy(dict_data_as_ushort_+kMatrixDimension*dict_index_points_num_,
                    calc_dict_indexes_[i][j],
                    kMatrixDimension*sizeof(unsigned short));
                dict_index_points_num_++;
            }
        }
    }

    /*
     * 读取数据后的预处理函数。
     * 主要做了3个事情：
     * 1. 模归一化。将后续的余弦相似度计算变成乘法计算。
     * 2. 将 float 数据转换为 unsigned short。映射到 [1, 65535] 之间的整数。
     * 3. 用上题目 98% 准确度条件。筛选真正需要计算的向量。
     */
    void _PreHandleData() {
        unsigned max_dict_sum{0};
#pragma omp parallel for num_threads(THREAD_NUM)
        for(auto i = 0; i < kDictVecNum; ++i) {
            size_t index = vec_index_[i];
            float norm = _CalcNorm(dict_data_+index);
            float norm_seed = kFloatMin;

            if (i < kSeedVecNum) {
                norm_seed = _CalcNorm(seed_data_+index);
            }

            unsigned sum_vec{0};
            for(auto j = 0; j < kMatrixDimension; ++j) {
                size_t offset = index+j;

                // 模归一化
                dict_data_[offset] /= norm;

                // 映射为 unsigned short 整数。
                dict_data_int_[offset] = static_cast<unsigned short>(
                    kInt16BitMax *
                    (dict_data_[offset] + kConvertToUInt16Delta));

                sum_vec += dict_data_int_[offset];

                if (i < kSeedVecNum) {
                    seed_data_[offset] /= norm_seed;
                    seed_data_int_[offset] = static_cast<unsigned short>(
                        kInt16BitMax *
                        (seed_data_[offset] + kConvertToUInt16Delta));
                }
            }

            // 进行搜索截肢，删除模归一化之后，
            // 进行相似度乘法计算值 “可能” 过小的向量。
            auto slot = omp_get_thread_num()%THREAD_NUM;

            if (sum_vec > max_dict_sum) {
                max_dict_sum = sum_vec;
            }
            if (max_dict_sum*kThresholdForVectorFilter < sum_vec) {
                // 将符合条件的 dict 指针保存起来。
                // 由于是多线程，这里需要保存到每个线程对应的 slot 里。
                // 这样做避免了锁。
                calc_dict_indexes_[slot].push_back(dict_data_int_+index);

                // 同时计算好 slot 的大小，避免后续重复计算。
                calc_dict_indexes_size_[slot]++;
            }
        }
    }

    /*
     * 计算相似度的主过程函数。
     *
     * 外层循环是 seed 向量的数量。
     * 内层循环时 dict 向量的数量，
     * 经过上述预处理函数截肢之后，小于 1000*1000 了。
     * 经过上述重排函数处理之后，这些向量依然保持内存连续。
     */
    void _CalcSimilarity() {
#pragma omp parallel for num_threads(THREAD_NUM)
        for(unsigned i = 0; i < kSeedVecNum; ++i) {
            for(unsigned j = 0; j < dict_index_points_num_; ++j) {

                // 计算相似度。
                // 这里直接使用了两个 index 预先存放的指针或者步长，
                // 节约任何一次不必要的重复计算。
                auto similarity = _CalcCosSim(seed_index_points_[i], dict_data_as_ushort_+vec_index_[j]);

                // 将相似度计算结果插入每个 seed 向量对应的 Result 类中。并自动保证永远存放最大的10个。
                // 见 Result 类文件注释。
                all_res_[i]->InsertResData(ResultData<unsigned>{similarity, j});
            }
        }
    }

    /*
     * 计算模的函数。
     */
    inline float _CalcNorm(const float* vec) {
        float l = 0.0f;

        for(auto i = 0; i < kMatrixDimension; i++) {
            l += vec[i] * vec[i];
        }
        return std::sqrt(l) + std::numeric_limits<float>::min();
    }

    /*
     * 计算相似度的函数。
     *
     * 经过处理之后只需要计算两个 unsigned short 整数乘法。
     */
    inline unsigned _CalcCosSim(unsigned short* vec1, unsigned short* vec2) {
        unsigned res{0};

        for (auto i =0; i < kMatrixDimension; ++i){
            res += vec1[i] * vec2[i];
        }

        return res;
    }

private:
    float *seed_data_;
    float *dict_data_;

    unsigned short *dict_data_as_ushort_;

    unsigned short*  seed_data_int_;
    unsigned short*  dict_data_int_;

    unsigned *vec_index_;

    unsigned short **seed_index_points_;

    unsigned short **dict_index_points_;
    unsigned dict_index_points_num_{0};

    std::vector<std::vector<unsigned short*>> calc_dict_indexes_{THREAD_NUM};
    std::vector<unsigned> calc_dict_indexes_size_;

    Timer* timer_;

    AllResults<ResultData<unsigned>> all_res_{kSeedVecNum, nullptr};

    std::shared_ptr<spdlog::logger> logger_;
};
