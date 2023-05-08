#include <cmath>

#include <malloc.h>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "constants.h"
#include "result.h"
#include "result_writer.h"
#include "timer.h"

class Calc {

public:
    explicit Calc(float *seed_data, float* dict_data, size_t round = 1)
        :seed_data_(seed_data),
        dict_data_(dict_data),
        timer_(new Timer(round)) {

        seed_data_int_ = static_cast<unsigned short*>(
            memalign(kAlign16Bit, sizeof(unsigned short)*kMatrixDimension*kSeedVecNum));

        dict_data_int_ = static_cast<unsigned short*>(
            memalign(kAlign16Bit, sizeof(unsigned short)*kMatrixDimension*kDictVecNum));

        vec_index_ = static_cast<unsigned*>(malloc(sizeof(unsigned)*kDictVecNum));
        vec_index_[0] = 0;
        for (auto i = 1; i < kDictVecNum; ++i) {
            vec_index_[i] = vec_index_[i-1] + kMatrixDimension;
        }

        for (auto &&p : all_res_) {
            p = new Top10Similarity<ResultData<unsigned>>();
        }

    }

    Calc(Calc&&) = delete;
    Calc(const Calc&&) = delete;

    Calc& operator=(const Calc&) = delete;
    Calc& operator=(Calc&&) = delete;

    ~Calc() {
        free(seed_data_int_);
        free(dict_data_int_);

        for(auto&& p : all_res_) {
            delete p;
        }
        delete timer_;
    }

    void Run() {
        Timer t_norm;
        Timer t_calc;
        t_norm.Start();
        timer_->Start();
        _DataPreHandler();
        t_norm.Stop();

        t_calc.Start();
        _CalcSimilarity();
        timer_->Stop().WriteDurationLog();
        t_calc.Stop();

        std::cout << "prehandle: " << t_norm << std::endl;
        std::cout << "calc: " << t_calc << std::endl;

        ResultWriter().Write(all_res_);
    }

    Timer& GetRunningTime() {
        return *timer_;
    }

private:
    void _DataPreHandler() {
#pragma omp parallel for num_threads(THREAD_NUM)
        for(auto i = 0; i < kDictVecNum; ++i) {
            size_t index = vec_index_[i];
            float norm = _CalcNorm(dict_data_+index);
            float norm_seed = kFloatMin;

            if (i < kSeedVecNum) {
                norm_seed = _CalcNorm(seed_data_+index);
            }

            for(auto j = 0; j < kMatrixDimension; ++j) {
                size_t offset = index+j;
                dict_data_[offset] /= norm;
                dict_data_int_[offset] = static_cast<unsigned short>(
                    kInt16BitMax *
                    (dict_data_[offset] + kConvertToUInt16Delta));

                if (i < kSeedVecNum) {
                    seed_data_[offset] /= norm_seed;
                    seed_data_int_[offset] = static_cast<unsigned short>(
                        kInt16BitMax *
                        (seed_data_[offset] + kConvertToUInt16Delta));
                }
            }
        }
    }

    void _CalcSimilarity() {
#pragma omp parallel for num_threads(THREAD_NUM)
        for(unsigned i = 0; i < kSeedVecNum; ++i) {
            for(unsigned j = 0; j < kDictVecNum; ++j) {
                auto similarity = _CalcCosSim(seed_data_int_+vec_index_[i], dict_data_int_+vec_index_[j]);

                if (similarity < all_res_[i]->min_val) {
                    continue;
                }

                all_res_[i]->InsertResData(ResultData<unsigned>{similarity, j});
            }
            all_res_[i]->SortRes();
        }
    }

    inline float _CalcNorm(const float* vec) {
        float l = 0.0f;

        for(auto i = 0; i < kMatrixDimension; i++) {
            l += vec[i] * vec[i];
            //l += std::pow(vec[i], 2);
        }
        return std::sqrt(l) + std::numeric_limits<float>::min();
    }

    inline unsigned _CalcCosSim(unsigned short* v1, unsigned short* v2) {
        unsigned res{0};

        for (auto i =0; i < kMatrixDimension; ++i){
            res += v1[i] * v2[i];
        }

        return res;
    }

private:
    float *seed_data_;
    float *dict_data_;

    unsigned short*  seed_data_int_;
    unsigned short*  dict_data_int_;

    unsigned* vec_index_;

    Timer* timer_;

    AllResults<ResultData<unsigned>> all_res_{kSeedVecNum, nullptr};
};
