#ifndef _SEARCHBEST_
#define _SEARCHBEST_

//#include <assert.h>
#include <cmath>
#include <float.h>
//#include <climits>
// use openblas
//#include <cblas.h>
#include <iostream>

#include <malloc.h>

//#include <jemalloc/jemalloc.h>

#include "cosine_similarity.h"
#include "result.h"
#include "result_writer.h"

enum {
        kMatrixDimension = 256,
        kDictVecNum = 1000*1000,
        kSeedVecNum = 1000,
        kAlign32Bit = 32,
        kAlign16Bit = 16
};

constexpr float kConvertToShortDelta = 1.0f/(65535*2.0f);

#define THREAD_NUM (8)
class A {
public:

    A(float *seed, float* dict) :v1(seed), v3(dict){
        v2 = static_cast<unsigned short*>(memalign(kAlign32Bit, sizeof(unsigned short)*kSeedVecNum*kMatrixDimension));

        v4 = static_cast<unsigned short*>(memalign(kAlign32Bit, sizeof(unsigned short)*kDictVecNum*kMatrixDimension));

        for (auto &&p : all_res) {
            p = new Top10Similarity<ResultData<unsigned>>();
        }
    }

    ~A() {
        free(v2);
        free(v4);

        for (auto&& p : all_res) {
            delete p;
        }
    }

    float calcL(const float* const pVec, const int len)
    {
        float l = 0.0f;

        for(int i = 0; i < len; i++) {
            l += pVec[i] * pVec[i];
        }

        return sqrt(l) + FLT_MIN;
    }

    void NormalizeVec() {
#pragma omp parallel for num_threads(THREAD_NUM)
        for(int i = 0; i < kDictVecNum; ++i) {
            float norm = calcL(v3+i*kMatrixDimension, kMatrixDimension);
            float norm_seed;
            if (i < kSeedVecNum) {
                norm_seed = calcL(v1+i*kMatrixDimension, kMatrixDimension);
            }
            for(auto j = 0; j < kMatrixDimension; ++j) {
                v3[i*kMatrixDimension+j] /= norm;
                v4[i*kMatrixDimension+j] = static_cast<unsigned short>(
                        std::numeric_limits<unsigned short>::max() *
                        (v3[i*kMatrixDimension+j] + kConvertToShortDelta));

                if (i < kSeedVecNum) {
                    v1[i*kMatrixDimension+j] /= norm_seed;
                    v2[i*kMatrixDimension+j] = static_cast<unsigned short>(
                            std::numeric_limits<unsigned short>::max() *
                            (v1[i*kMatrixDimension+j] + kConvertToShortDelta));
                }
            }
        }
    }

    unsigned CosineSimilarity(unsigned short* v2, unsigned short* v4) {

        unsigned res{0};

        for(int i = 0; i < kMatrixDimension; i++) {
           res += v2[i] * v4[i];
        }

        return res;
    }

    void SearchBest() {
#pragma omp parallel for num_threads(THREAD_NUM)
        for (auto i = 0; i < kSeedVecNum; ++i) {
            for(unsigned j = 0; j < kDictVecNum; ++j) {
                //auto similarity = CosineSimilarity(v2+i*kMatrixDimension, v4+j*kMatrixDimension);
                auto similarity = ::CosineSimilarity<unsigned>(v2+i*kMatrixDimension, v4+j*kMatrixDimension, kMatrixDimension);
                all_res[i]->InsertResData(ResultData<unsigned>{similarity, j});
            }
        }
    }

    AllResults<ResultData<unsigned>>& GetResult() {
        return all_res;
    }

private:

    float *v1, *v3;
    unsigned short* v2, *v4;

    AllResults<ResultData<unsigned>> all_res{kSeedVecNum, nullptr};
};

template <typename RE_T, typename T>
void SearchBest(const T* __restrict__ const pVecA,  // 待搜索的单个特征向量首地址
        const size_t seed_num,
        const int feat_size,  // 待搜索特征向量长度(1 x 单个特征维数)
        const T* __restrict__ const pVecDB, // 底库首地址
        const int face_num,
        T* seed_index[],
        T* dict_index[],
        AllResults<RE_T>& all_res)
{
    //assert(lenDB%lenA == 0);
    //const int featsize = lenA;
    //const int facenum  = lenDB / lenA;

    //int best_index = - INT_MAX;
    using MetaDataType = typename Top10Similarity<RE_T>::MetaDataType;
    //MetaDataType best_similarity = 0;
    //unsigned int best_similarity = 0;
#if 1
#pragma omp parallel for num_threads(THREAD_NUM)
//#pragma omp parallel for
    for (auto i = 0; i < seed_num; ++i) {
        for(unsigned j = 0; j < face_num; ++j) {
            // 普通C++代码实现的余弦相似度计算
            MetaDataType similarity = CosineSimilarity<MetaDataType>(seed_index[i], dict_index[j], feat_size);
            //T similarity = Cosine_similarity_avx(pVecA+i*featsize, pVecDB + j*featsize, featsize);
            //std::cout << "similarity:" << similarity << std::endl;
            // 使用向量化代码实现的余弦相似度计算
            //T similarity = Cosine_similarity_avx(pVecA, pVecDB + i*featsize, featsize);
            /*
            if (j == 447523) {
                std::cout << "index: " << j << ", " << similarity << std::endl;
            }*/

            /*
            if(similarity > best_similarity) {
                best_similarity = similarity;
                best_index = j;

            }*/
            //std::cout << similarity << ",";
            all_res[i]->InsertResData(RE_T{similarity, j});
            //all_res[i]->InsertResData(RE_T(similarity, j));

            /*
               std::cout << all_res[0].Len() << std::endl;
               for (auto&& d : all_res[0].GetResult()) {
               std::cout << d << ",";
               }
               std::cout << std::endl;
               */

        }
        all_res[i]->SortRes();
        //std::cout << std::endl;
    }

#endif
#if 0
    // 使用OpenBLAS
    T simAll[facenum] = {0.0f};
    cblas_sgemv(CblasRowMajor, CblasNoTrans, facenum, featsize, 1, pVecDB, featsize, pVecA, 1, 0, simAll, 1);
    // 寻找simAll里面最大的，它的序号就是要找的id
    for(int i = 0; i < facenum; i++) {
        if(simAll[i] > best_similarity) {
            best_similarity = simAll[i];
            best_index = i;
        }
    }
#endif
    //std::cout << "best index: " << best_index << std::endl;
    //return std::move(all_res);
}

#endif //!_SEARCHBEST_
