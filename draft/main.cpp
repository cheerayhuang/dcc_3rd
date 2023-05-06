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

#define ALGIN               (16) // 使用SIMD需要内存对齐，128bit的指令需要16位对齐，256bit的指令需要32位对齐
#define FACENUM      (1000*1000) // the total of dict vectors
#define SEEDNUM      (1000)
#define FEATSIZE     (256)

// Step 4, double-->float(在我的电脑上，sizeof(float)==4，sizeof(double)==8, sizeof(short)==2, sizeof(int)==4
//typedef float DType;
// Step 12, float-->unsigned short，定点化
typedef float DType;

//const std::string kSeedFilePath = "/home/chee/work/dcc_3rd/test/seed_vec.csv";
//const std::string kDictFilePath = "/home/chee/work/dcc_3rd/test/dict_vec.csv";
const std::string kSeedFilePath = "seed_vec.csv";
const std::string kDictFilePath = "dict_vec.csv";

constexpr float kConvertToShortDelta = 1.0f/(65535*2.0f);

float calcL(const DType * const pVec, const int len)
{
    float l = 0.0f;

    for(int i = 0; i < len; i++) {
        l += pVec[i] * pVec[i];
    }

    return sqrt(l) + FLT_MIN;
}

void NormalizeVec(float *v1, unsigned short *v2, float* v3, unsigned short *v4) {
#pragma omp parallel for num_threads(8)
    for(int i = 0; i < FACENUM; ++i) {
        float norm = calcL(v3+i*FEATSIZE, FEATSIZE);
        float norm_seed;
        if (i < SEEDNUM) {
            norm_seed = calcL(v1+i*FEATSIZE, FEATSIZE);
        }
        for(auto j = 0; j < FEATSIZE; ++j) {
            v3[i*FEATSIZE+j] /= norm;
            v4[i*FEATSIZE+j] = static_cast<unsigned short>(
                std::numeric_limits<unsigned short>::max() *
                (v3[i*FEATSIZE+j] + kConvertToShortDelta));

            if (i < SEEDNUM) {
                v1[i*FEATSIZE+j] /= norm_seed;
                v2[i*FEATSIZE+j] = static_cast<unsigned short>(
                    std::numeric_limits<unsigned short>::max() *
                    (v1[i*FEATSIZE+j] + kConvertToShortDelta));
            }
        }
    }
}

void NormalizeVec2(float *v1, unsigned short *v2, float* v3, unsigned short *v4) {
#pragma omp parallel for num_threads(8)
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
    __attribute__((aligned(ALGIN))) unsigned short vectorA_normal[SEEDNUM*FEATSIZE];

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
        all_res[i] = new Result<ResultData<unsigned>>();
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
    t_norm.Start();
    t.Start();
    NormalizeVec(vectorA, vectorA_normal, vectorB, pDB);
    //NormalizeVec2(vectorA, vectorA_normal, vectorB, pDB);
    t_norm.Stop();

    /*
    std::ostream_iterator<unsigned short> cout_iter(std::cout, ",");
    std::copy(vectorA_normal, vectorA_normal+FEATSIZE, cout_iter);
    std::cout << std::endl;
    std::copy(pDB, pDB+FEATSIZE, cout_iter);
    std::cout << std::endl;
    */


    // 3.定义计数器并开始计时

    //Normalization();

    //SearchBest(/*vectorA_normal*/ vectorA, SEEDNUM, FEATSIZE, pDB, FACENUM, all_res);
    SearchBest(vectorA_normal, SEEDNUM, FEATSIZE, pDB, FACENUM, all_res);


    // 4.打印结果
    t.Stop().WriteDurationLog();
    ResultWriter().Write(all_res);

    std::cout << "normalization " << t_norm << std::endl;
    std::cout << t << std::endl;
    // 5.释放分配的内存，防止内存泄露
    // memalign分配的内存也可以用free释放
    free(pDB);

    for(auto && p : all_res) {
        delete(p);
    }

    return 0;
}
