#ifndef _SEARCHBEST_
#define _SEARCHBEST_

//#include <assert.h>
#include <cmath>
#include <float.h>
//#include <climits>
// use openblas
//#include <cblas.h>
#include <iostream>

//#include <jemalloc/jemalloc.h>

#include "cosine_similarity.h"
#include "result_writer.h"

#define THREAD_NUM (4)

// Step 1, g++ main.cpp search_best.cpp cosine_similarity.cpp -std=c++11
// Step 2, g++ main.cpp search_best.cpp cosine_similarity.cpp -std=c++11 -O3
// Step 3, g++ main.cpp search_best.cpp cosine_similarity.cpp -std=c++11 -O3 -Ofast -ffast-math
template <typename RE_T, typename T>
void SearchBest(const T* __restrict__ const pVecA,  // 待搜索的单个特征向量首地址
        const size_t seed_num,
        const int feat_size,  // 待搜索特征向量长度(1 x 单个特征维数)
        const T* __restrict__ const pVecDB, // 底库首地址
        const int face_num,
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
    // Step 5, 加上OpenMP
    //GCC很聪明，OpenMP默认线程数就是多核处理器的核心数量，不必显示指定
    //OpenMP起线程，收回线程也是有开销的，所以要合理安排每个线程的任务量大小，不宜放入内层for循环（任务量太小划不来）
#pragma omp parallel for num_threads(THREAD_NUM)
//#pragma omp parallel for
    for (auto i = 0; i < seed_num; ++i) {
        //all_res[i] = new Result<RE_T>();
        for(unsigned j = 0; j < face_num; ++j) {
            // 普通C++代码实现的余弦相似度计算
            MetaDataType similarity = CosineSimilarity<MetaDataType>(pVecA+i*feat_size, pVecDB + j*feat_size, feat_size);
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
    }

#endif
#if 0
    // Step 12，使用OpenBLAS
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
