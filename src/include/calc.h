#include <bitset>
#include <cmath>
#include <cstring>

#include <iostream>
#include <malloc.h>

#include "omp.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "constants.h"
//#include "result.h"
#include "result.ori.h"
//#include "result_heap.h"
#include "result_writer.h"
#include "timer.h"

class Calc {

public:
    explicit Calc(float *seed_data, float* dict_data, size_t round = 1)
        :seed_data_(seed_data),
        dict_data_(dict_data),
        timer_(new Timer(round)) {

        dict_data_as_ushort_ = reinterpret_cast<unsigned short*>(dict_data_);

        seed_data_int_ = static_cast<unsigned short*>(
            memalign(kAlign16Bit, sizeof(unsigned short)*kMatrixDimension*kSeedVecNum));

        dict_data_int_ = static_cast<unsigned short*>(
            memalign(kAlign16Bit, sizeof(unsigned short)*kMatrixDimension*kDictVecNum));

        vec_index_ = static_cast<unsigned*>(malloc(sizeof(unsigned)*kDictVecNum));
        vec_index_[0] = 0;

        for (auto i = 1; i < kDictVecNum; ++i) {
            vec_index_[i] = vec_index_[i-1] + kMatrixDimension;
        }

        seed_index_points_ = static_cast<unsigned short**>(malloc(sizeof(unsigned short*)*kSeedVecNum));
        seed_index_points_[0] = seed_data_int_;

        for (auto i = 1; i < kSeedVecNum; ++i) {
            seed_index_points_[i] = seed_index_points_[i-1] + kMatrixDimension;
        }

        dict_index_points_ = static_cast<unsigned short**>(malloc(sizeof(unsigned short*)*kDictVecNum));

        for (auto &&p : all_res_) {
            p = new Top10Similarity<ResultData<unsigned>>();
        }

        calc_dict_indexes_size_.assign(THREAD_NUM, 0);
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

    void Run() {
        Timer t;
        timer_->Start();
        _PreHandleData();
        t.Start();
        _LayDataIndexFlat();
        t.Stop();
        _CalcSimilarity();
        timer_->Stop().WriteDurationLog();

        auto sum = 0;
        for (auto i = 0; i < THREAD_NUM; ++i) {
            std::cout << "slot size: " << calc_dict_indexes_size_[i] << std::endl;
            sum += calc_dict_indexes_[i].size();
        }
        std::cout << sum << std::endl;
        std::cout << dict_index_points_num_ << std::endl;

        std::cout << "lay the data flat " << t << std::endl;

        ResultWriter().Write(all_res_, dict_index_points_, dict_data_int_);
    }

    Timer& GetRunningTime() {
        return *timer_;
    }

private:

    inline void _LayDataIndexFlat() {
        for (auto i = 0; i < THREAD_NUM; ++i) {
            auto size = calc_dict_indexes_size_[i];
            for (auto j = 0; j < size; ++j) {
                dict_index_points_[dict_index_points_num_] = calc_dict_indexes_[i][j];
                std::memcpy(dict_data_as_ushort_+kMatrixDimension*dict_index_points_num_,
                    calc_dict_indexes_[i][j],
                    kMatrixDimension*sizeof(unsigned short));
                dict_index_points_num_++;
                /*
                if (dict_index_points_num_ % 100000 == 0) {

                    for (auto ori = 0; ori < kMatrixDimension; ++ori) {
                        std::cout << calc_dict_indexes_[i][j][ori] << ", ";
                    }
                    std::cout << std::endl;

                    for (auto ori = 0; ori < kMatrixDimension; ++ori) {
                        std::cout << (dict_data_as_ushort_+kMatrixDimension*(dict_index_points_num_-1))[ori] << ", ";
                    }
                    std::cout << "------" << std::endl;
                }*/
            }
        }
    }


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
                dict_data_[offset] /= norm;
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

            auto slot = omp_get_thread_num()%THREAD_NUM;

            if (sum_vec > max_dict_sum) {
                max_dict_sum = sum_vec;
            }
            if (max_dict_sum*0.945f < sum_vec) {
                calc_dict_indexes_[slot].push_back(dict_data_int_+index);
                calc_dict_indexes_size_[slot]++;
            }
        }
    }

    void _CalcSimilarity() {
        //unsigned drop = 0;
#pragma omp parallel for num_threads(THREAD_NUM)
        for(unsigned i = 0; i < kSeedVecNum; ++i) {
            //for(unsigned j = 0; j < kDictVecNum-342323; ++j) {
            for(unsigned j = 0; j < dict_index_points_num_; ++j) {
                //auto p_dict = dict_index_points_[j];
                //auto similarity = _CalcCosSim(seed_index_points_[i], p_dict);
                //all_res_[i]->InsertResData(ResultData<unsigned>{similarity, (p_dict-dict_data_int_)>>8});


                auto similarity = _CalcCosSim(seed_index_points_[i], dict_data_as_ushort_+vec_index_[j]);
                all_res_[i]->InsertResData(ResultData<unsigned>{similarity, j});
            }
        }
    }

    inline float _CalcNorm(const float* vec) {
        float l = 0.0f;

        for(auto i = 0; i < kMatrixDimension; i++) {
            l += vec[i] * vec[i];
        }
        return std::sqrt(l) + std::numeric_limits<float>::min();
    }

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
};
