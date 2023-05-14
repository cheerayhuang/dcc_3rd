//main.cpp
#include <algorithm>
#include <fstream>
#include <iostream>
#include <malloc.h>

#include <iterator>
#include <limits>
#include <string>

#include "timer.h"
#include "search_best.h"

#define ALGIN               (32) // 使用SIMD需要内存对齐，128bit的指令需要16位对齐，256bit的指令需要32位对齐
#define FACENUM      (1000*1000) // the total of dict vectors
//#define FACENUM      (20000) // the total of dict vectors
//#define SEEDNUM      (1)
#define SEEDNUM      (1000)
#define FEATSIZE     (256)

typedef float DType;

//const std::string kSeedFilePath = "/home/chee/work/dcc_3rd/test/seed_vec.csv";
//const std::string kDictFilePath = "/home/chee/work/dcc_3rd/test/dict_vec.csv";
const std::string kSeedFilePath = "seed_vec.csv";
const std::string kDictFilePath = "dict_vec.csv";

float calcL(const float* const pVec, const int len) {
    float l = 0.0f;

    for(int i = 0; i < len; i++) {
        l += pVec[i] * pVec[i];
    }

    return sqrt(l) + FLT_MIN;
}

void NormalizeVec(float *v1, unsigned short *v2, float* v3, unsigned short *v4,
        unsigned short* seed_index[],
        unsigned short* dict_index[]) {

#pragma omp parallel for num_threads(THREAD_NUM)
    for(int i = 0; i < kDictVecNum; ++i) {
        size_t index = i*kMatrixDimension;

        dict_index[i] = v4+index;
        float norm = calcL(v3+index, kMatrixDimension);
        float norm_seed;
        if (i < kSeedVecNum) {
            seed_index[i] = v2+index;
            norm_seed = calcL(v1+index, kMatrixDimension);
        }

        for(auto j = 0; j < kMatrixDimension; ++j) {
            size_t offset = index + j;
            v3[offset] /= norm;
            dict_index[i][j] = static_cast<unsigned short>(
                    std::numeric_limits<unsigned short>::max() *
                    (v3[offset] + kConvertToShortDelta));

            if (i < kSeedVecNum) {
                v1[offset] /= norm_seed;
                seed_index[i][j] = static_cast<unsigned short>(
                        std::numeric_limits<unsigned short>::max() *
                        (v1[offset] + kConvertToShortDelta));
            }
        }
    }

}

void NormalizeVec2(float *v1, unsigned short *v2, float* v3, unsigned short *v4) {
#pragma omp parallel for num_threads(THREAD_NUM)
    for(int i = 0; i < FACENUM; ++i) {
        unsigned long norm{0}, norm_seed{0};
        for (auto j = 0; j < FEATSIZE; ++j) {
            if (i < SEEDNUM) {
                v2[i*FEATSIZE+j] = static_cast<unsigned short>(
                    std::numeric_limits<unsigned short>::max() *
                    (v1[i*FEATSIZE+j] + kConvertToShortDelta));

                norm_seed += static_cast<unsigned long>(v2[i*FEATSIZE+j]) * v2[i*FEATSIZE+j];
            }
            v4[i*FEATSIZE+j] = static_cast<unsigned short>(
                    std::numeric_limits<unsigned short>::max() *
                    (v3[i*FEATSIZE+j] + kConvertToShortDelta));

            norm += static_cast<unsigned long>(v4[i*FEATSIZE+j]) * v4[i*FEATSIZE+j];
        }

        for(auto j = 0; j < FEATSIZE; ++j) {
            v4[i*FEATSIZE+j] = static_cast<unsigned short>(
                std::numeric_limits<unsigned short>::max() *
                (v4[i*FEATSIZE+j]/std::sqrt(norm)));

            if (i < SEEDNUM) {
                v2[i*FEATSIZE+j] = static_cast<unsigned short>(
                    std::numeric_limits<unsigned short>::max() *
                    (v2[i*FEATSIZE+j]/std::sqrt(norm_seed)));
            }
        }
    }
}

int main(int argc, char* argv[])
{
    __attribute__((aligned(ALGIN))) DType vectorA[SEEDNUM * FEATSIZE];
    __attribute__((aligned(ALGIN))) unsigned short vectorA_norm[SEEDNUM*FEATSIZE];

    if (argc == 1) {
        argv[1] = new char[2]{'1', '\0'};
    }

    AllResults<ResultData<unsigned>> all_res(SEEDNUM, nullptr);
    //AllResults<ResultData<DType>> all_res(SEEDNUM, nullptr);

    auto fin = std::ifstream(kSeedFilePath);
    char delimiter;
    size_t line_no;
    for(auto i = 0; i < SEEDNUM; ++i) {
        // fin >> line_no >> delimiter;
        //all_res[i] = new Result<ResultData<DType>>();
        all_res[i] = new Top10Similarity<ResultData<unsigned>>();
        for(int j = 0; j < FEATSIZE; ++j) {
            fin >> vectorA[i*FEATSIZE + j];
            if (j < FEATSIZE-1) {
                fin >> delimiter;
            }
        }
    }
    fin.close();

    // 使用SIMD优化，使用memalign申请对齐了的内存，牺牲了代码的可移植性
    //DType* pDB = reinterpret_cast<DType*>(memalign(ALGIN, sizeof(DType)*FACENUM*FEATSIZE));
    unsigned short* pDB = static_cast<unsigned short*>(memalign(ALGIN, sizeof(unsigned short)*FACENUM*FEATSIZE));
    if(!pDB) {
        std::cout << "out of memory\n";
        return -1;
    }

    DType* vectorB = static_cast<DType*>(memalign(ALGIN, sizeof(DType)*FACENUM*FEATSIZE));
    fin.open(kDictFilePath);
    for(int i = 0; i < FACENUM; i++) {
        // fin >> line_no >> delimiter;
        for(int j = 0; j < FEATSIZE; j++) {
            //pDB[i*FEATSIZE+j] = static_cast<DType>(i*FEATSIZE + j) / (FACENUM * FEATSIZE);
            fin >> vectorB[i*FEATSIZE+j];
            //fin >> pDB[i*FEATSIZE+j];
            if (j < FEATSIZE-1) {
                fin >> delimiter;
            }
        }
    }
    fin.close();

    // 模归一化
    Timer t(std::stoi(argv[1]));
    Timer t_norm;
    //A a(vectorA, vectorB);
    unsigned short *seed_index[kSeedVecNum];
    //unsigned short* (*dict_index) [kDictVecNum];
    unsigned short** dict_index = static_cast<unsigned short**>(malloc(sizeof(unsigned short*)*kDictVecNum));

    t_norm.Start();
    t.Start();
    //a.NormalizeVec();
    NormalizeVec(vectorA, vectorA_norm, vectorB, pDB, seed_index, dict_index);
    t_norm.Stop();

    /*
    for (auto && i : vectorA_norm) {
        std::cout << i << std::endl;
    }*/

    /*
    std::ostream_iterator<unsigned short> cout_iter(std::cout, ",");
    std::copy(vectorA_norm, vectorA_norm+FEATSIZE, cout_iter);
    std::cout << std::endl;
    std::copy(pDB, pDB+FEATSIZE, cout_iter);
    std::cout << std::endl;
    */

    //a.SearchBest();
    SearchBest(vectorA_norm, SEEDNUM, FEATSIZE, pDB, FACENUM, seed_index, dict_index, all_res);

    t.Stop().WriteDurationLog();
    ResultWriter().Write(all_res);

    std::cout << "normalization " << t_norm << std::endl;
    std::cout << t << std::endl;

    free(pDB);
    free(vectorB);

    for(auto && p : all_res) {
        delete(p);
    }

    free(dict_index);

    return 0;
}
