#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

//#include <cblas.h>
#include <mkl.h>


#include "timer.h"
#include "data_reader.h"

unsigned Dot(const unsigned* v1, const unsigned* v2) {
    unsigned res{0};
    for (auto i = 0; i < 128; ++i) {
        //res += (v1[i] >> 16) * (v2[i]>>16) + (v1[i]&0xffff) *(v2[i]&0xffff);
        res += v1[i] * v2[i];
    }

    return res;
}

unsigned Dot(const unsigned short *v1,
        const unsigned short *v2) {

    unsigned res = 0;
    for (auto i = 0; i < 256; ++i) {
        res += v1[i] * v2[i];
    }

    return res;

}

float Dot(const float * v1, const float * v2) {
    float res = 0;
    for (auto i = 0; i < 256; ++i) {
        res += v1[i] * v2[i];
    }

    return res;
}

float BlasDot(const float * v1, const float* v2) {

    float res = 0;

    //float cblas_sdot (const MKL_INT n, const float *x, const MKL_INT incx, const float *y, const MKL_INT incy);

    res = cblas_sdot(256, v1, 1, v2, 1);

    return res;

}

int main() {
    //std::vector<unsigned> v1(256, 0), v2(256, 0);
    /*
    float v1[256] {0}, v2[256]{0};
    //unsigned short v3[256] {0}, v4[256]{0};
    float v3[256] {0}, v4[256]{0};

    float *v5 = new float[256*1000*1000]{0.0f};
    */


    std::mt19937_64 rand_e;
    std::uniform_int_distribution<unsigned short> rand_distri_int{0, 999};
    //std::uniform_real_distribution<float> rand_distri{1, 4096};

    rand_e.seed(Timer::NowAsNanoseconds());

    /*
    for (auto i = 0; i < 256; ++i) {
        v3[i] = rand_distri(rand_e);
        v4[i] = rand_distri(rand_e);

        v1[i] = rand_distri(rand_e);
        v2[i] = rand_distri(rand_e);
    }
    */

    //unsigned long t_dot{0}, t_blas_dot{0};

    Timer t;

    DataReader reader;
    reader.Read();

    auto seed_data = reader.GetSeedData();
    auto dict_data = reader.GetDictData();

    std::cout << *(seed_data+236) << "," << *(seed_data+999*256+15) << std::endl;
    std::cout << *(dict_data+923666) << "," << *(dict_data+999999*256+15) << std::endl;

    std::vector<float> result(1000, 0.0f);

    // Test: int multiply
    enum {
        kMatrixDimension = 256,
        kDictVecNum = 1000*1000,
        kSeedVecNum = 1000,
        kAlign16Bit = 16,
        kAlign32Bit = 32
    };
    unsigned short *dict_data_int = static_cast<unsigned short*>(
            memalign(kAlign32Bit, sizeof(unsigned short)*kMatrixDimension*kDictVecNum));

    unsigned short *seed_data_int = static_cast<unsigned short*>(
            memalign(kAlign32Bit, sizeof(unsigned short)*kMatrixDimension*kSeedVecNum));

    unsigned result_int[1000]{0};

    t.Start();
#pragma omp parallel for num_threads(8)
    for(auto i = 0; i < 1000*1000*256; ++i) {
        if (i < 1000*256) {
            seed_data_int[i] = static_cast<unsigned short>(seed_data[i]*65535);
        }
        dict_data_int[i] = static_cast<unsigned short>(dict_data[i]*65535);
    }

#pragma omp parallel for num_threads(8)
    for (auto k = 0; k < 1000; ++k) {
        for(auto i = 0; i < 1000000; ++i) {
            //result_int[k] = Dot(reinterpret_cast<unsigned*>(seed_data_int+(k<<8)), reinterpret_cast<unsigned*>(dict_data_int+(i<<8)));
            result_int[k] = Dot(seed_data_int+(k<<8), dict_data_int+(i<<8));
        }
    }

    std::cout << "int dot elapsed: " << t.Stop() << std::endl;

    std::cout << result_int[rand_distri_int(rand_e)] << std::endl;


// Test: normal dot
    t.Start();
#pragma omp parallel for num_threads(8)
    for (auto k = 0; k < 1000; ++k) {
        size_t index_seed = (k<<8);
        size_t index_dict = 0;
        for(auto i = 0; i < 1000000; ++i, index_dict+=256) {

            result[k] = Dot(seed_data+index_seed, dict_data+index_dict);
            //result[k] = Dot(seed_data+k, dict_data+i);
        }
        //std::cout << k << std::endl;
    }
    std::cout << "dot elapsed: " << t.Stop() << std::endl;

    std::cout << result[rand_distri_int(rand_e)] << std::endl;


// Test: blas dot
    t.Start();
#pragma omp parallel for num_threads(8)
    for (auto k = 0; k < 1000; ++k)
    {
            for (auto i = 0; i < 1000000; ++i) {

                result[k] = BlasDot(seed_data+k*256, dict_data+i*256);

            }
    }
    std::cout << " blas elapsed: " <<  t.Stop() << std:: endl;

    std::cout << result[rand_distri_int(rand_e)] << std::endl;


// Test: blas vec*martrix
    float v6[1000*1000] {0};

    t.Start();
//#pragma omp parallel for num_threads(8)
    for (auto k = 0; k < 1000; ++k) {
        cblas_sgemv(CblasRowMajor, CblasNoTrans, 1000*1000, 256, 1, dict_data, 256, seed_data+k*256, 1, 0, v6, 1);
    }
    std::cout << "blas sgemv: " << t.Stop() << std::endl;

    std::cout << v6[rand_distri_int(rand_e)] << std::endl;

    return 0;
}
