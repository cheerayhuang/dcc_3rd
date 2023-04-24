//main.cpp
#include <iostream>
#include <fstream>
#include <malloc.h>

#include <iterator>
#include <algorithm>
#include <limits>
#include <string>


#include "timer.h"
#include "search_best.h"

#define ALGIN               (32) // 使用SIMD需要内存对齐，128bit的指令需要16位对齐，256bit的指令需要32位对齐
#define FACENUM      (1000*1000) // 底库中存有100万张人脸特征向量
#define SEEDNUM      (1)
//#define FEATSIZE           (512) // 每个人脸特征向量的维度是512维，每一维是一个DType类型的浮点数
//Step 11，用PCA做特征选择，压缩512维到256维
#define FEATSIZE             (256) // 每个人脸特征向量的维度是256维，每一维是一个DType类型的浮点数

// Step 4, double-->float(在我的电脑上，sizeof(float)==4，sizeof(double)==8, sizeof(short)==2, sizeof(int)==4
//typedef float DType;
// Step 12, float-->unsigned short，定点化
typedef float DType;

//const std::string kSeedFilePath = "/home/chee/work/dcc_3rd/test/seed_vec.csv";
//const std::string kDictFilePath = "/home/chee/work/dcc_3rd/test/dict_vec.csv";
const std::string kSeedFilePath = "seed_vec.csv";
const std::string kDictFilePath = "dict_vec.csv";

float calcL(const DType * const pVec, const int len)
{
    float l = 0.0f;

    for(int i = 0; i < len; i++) {
        l += pVec[i] * pVec[i];
    }

    return sqrt(l) + FLT_MIN;
}

int main(int argc, char* argv[])
{
    // 1.定义当前脸的特征，并初始化
    __attribute__((aligned(ALGIN))) DType vectorA[SEEDNUM * FEATSIZE];
    __attribute__((aligned(ALGIN))) unsigned short vectorA_norml[SEEDNUM*FEATSIZE];


    /*
       cout << sizeof(unsigned short) << endl;
       cout << std::numeric_limits<unsigned int>::max() << endl;
       cout << std::numeric_limits<unsigned long>::max() << endl;

       return 0;
       */


    auto fin = ifstream(kSeedFilePath); 
    char delimiter;
    for(auto i = 0; i < SEEDNUM; ++i)
        for(int j = 0; j < FEATSIZE; ++j) { 
            //vectorA[i] = static_cast<DType>(FACENUM/2*FEATSIZE + i) / (FACENUM * FEATSIZE);
            fin >> vectorA[i*FEATSIZE + j];
            if (i < FEATSIZE-1) {
                fin >> delimiter;
            }
        }
    fin.close();

    /*
       std::ostream_iterator<DType> cout_iter(std::cout, ","); 
       std::copy(begin(vectorA), end(vectorA), cout_iter);
       std::cout << std::endl;
       */

    // 模归一化
    for(int i = 0; i < SEEDNUM; ++i) {
        const float norm = calcL(vectorA+i*FEATSIZE, FEATSIZE);
        for(auto j = 0; j < FEATSIZE; ++j) {
            vectorA[i*FEATSIZE+j] /= norm;
            vectorA_norml[i*FEATSIZE+j] = static_cast<unsigned short>(
                std::numeric_limits<unsigned short>::max() * (vectorA[i*FEATSIZE+j] + 1.0f/(65535.0f*2.0f))); 
        }
    }

    // 2.定义底库中所有脸的特征向量，并初始化
    // 为了使用SIMD优化，使用memalign申请对齐了的内存，牺牲了代码的可移植性
    //DType* pDB = reinterpret_cast<DType*>(memalign(ALGIN, sizeof(DType)*FACENUM*FEATSIZE));
    unsigned short* pDB = reinterpret_cast<unsigned short*>(memalign(ALGIN, sizeof(unsigned short)*FACENUM*FEATSIZE));
    if(!pDB) {
        std::cout << "out of memory\n";
        return -1;
    }

    __attribute__((aligned(ALGIN))) DType vectorB[FEATSIZE];
    // 验证内存是否对齐
    //printf("vectorA[%p], pDB[%p].\n", vectorA, pDB);
    //cout << (vectorA-pDB) << endl;
    //cout << ((vectorA-pDB) % 32) << endl;
    fin.open(kDictFilePath);
    for(int i = 0; i < FACENUM; i++) {
        for(int j = 0; j < FEATSIZE; j++) {
            //pDB[i*FEATSIZE+j] = static_cast<DType>(i*FEATSIZE + j) / (FACENUM * FEATSIZE);
            fin >> vectorB[j];
            //fin >> pDB[i*FEATSIZE+j];
            if (j < FEATSIZE-1) {
                fin >> delimiter;
            }
        }

        /*
           if (i == 4 || i == 9) {
           std::copy(pDB+i*FEATSIZE, pDB+(i+1)*FEATSIZE, cout_iter);
           std::cout << std::endl;
           }*/

        // 模归一化
        const float norm = calcL(vectorB, FEATSIZE);
        //const float norm = calcL(pDB+i*FEATSIZE, FEATSIZE);
        for(int j = 0; j < FEATSIZE; j++) {
            //pDB[i*FEATSIZE+j] /= norm;
            vectorB[j] /= norm;
            pDB[i*FEATSIZE+j] = static_cast<unsigned short>(
                std::numeric_limits<unsigned short>::max()*(vectorB[j]+1.0f/(65535.0f*2.0f)));
        }
    }
    fin.close();

    // 3.定义计数器并开始计时
    Timer t;

    //int best_index = SearchBest(static_cast<DType*>(vectorA), FEATSIZE, pDB, FACENUM*FEATSIZE);
    //int best_index = SearchBest(static_cast<unsigned short*>(vectorA_norml), FEATSIZE, pDB, FACENUM*FEATSIZE);
    auto all_res = SearchBest<ResultData<unsigned long>>(static_cast<unsigned short*>(vectorA_norml), SEEDNUM, FEATSIZE, pDB, FACENUM*FEATSIZE);
    //auto all_res = SearchBest(static_cast<DType*>(vectorA), SEEDNUM, FEATSIZE, pDB, FACENUM*FEATSIZE);

    // 4.打印结果
    //std::cout << "Best face index is: " << best_index << std::endl;
    std::cout << "Find the best face index eat: " << t.elapsed_micro() << "us" << std::endl;
    std::cout << "PER Cosine_similarity call eat: " << t.elapsed_nano() / FACENUM << "ns" << std::endl;

    ResultWriter().Write(all_res);
    //printf("double[%d], float[%d], short[%d], int[%d].\n", (int)sizeof(double), (int)sizeof(float), (int)sizeof(short), (int)sizeof(int));

    // 5.释放分配的内存，防止内存泄露
    // memalign分配的内存也可以用free释放
    free(pDB);

    return 0;
}
