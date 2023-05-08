#pragma once

#define THREAD_NUM (8)

#include <limits>

enum {
        kMatrixDimension = 256,
        kDictVecNum = 1000*1000,
        //kDictVecNum = 1000,
        kSeedVecNum = 1000,
        //kSeedVecNum = 1,
        kAlign32Bit = 32,
        kAlign16Bit = 16,

        kInt16BitMax = std::numeric_limits<unsigned short>::max()
};

constexpr float kConvertToUInt16Delta = 1.0f/(65535*2.0f);
constexpr float kFloatMin = std::numeric_limits<float>::min();
