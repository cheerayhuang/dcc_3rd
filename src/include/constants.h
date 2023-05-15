/*
 * 常量定义
 */

#pragma once

#define THREAD_NUM (4)

#include <limits>

enum {
        kMatrixDimension = 256,   // 向量维度
        kDictVecNum = 1000*1000,  // 字典向量个数
        //kDictVecNum = 100000,
        kSeedVecNum = 1000,       // 种子向量个数
        //kSeedVecNum = 80,
        kAlign32Bit = 32,         // 申请内存时按32bit对齐
        kAlign16Bit = 16,         // 申请内存时按16bit对齐

        kTopK       = 10,         // 结果求前10余弦相似度

        kInt16BitMax = std::numeric_limits<unsigned short>::max()
};

// 转换为 unsigned short 时候补一个 delta 值，相当于四舍五入。
constexpr float kConvertToUInt16Delta = 1.0f/(65535*2.0f);

// float 类型最小正数值，用于必要时候加上或者初始化，防止除0错误。
constexpr float kFloatMin = std::numeric_limits<float>::min();

// 用于筛选 dict 向量的一个阈值，这个值是通过逐渐调节的，代表百分比。
constexpr float kThresholdForVectorFilter = 0.949005f;
