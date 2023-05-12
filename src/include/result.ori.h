/*
 * 结果存储相关类 ResultData 、Result的实现
 */

#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <iostream>
//#include <mutex>
#include <utility>
#include <vector>

/*
 * 简单定义了每一个结果数据类型。
 * 包含一个相似度数值结果 data，
 * 以及该结果在字典向量中的 index。
 */
template <typename T>
struct ResultData {

    T data;
    unsigned int index;

    using DataType = T;

    inline void swap(ResultData& other) {
        std::swap(data, other.data);
        std::swap(index, other.index);
    }
};

// 重载数值比较操作符，方便后续的排序操作使用。
template <typename T>
bool operator ==(const ResultData<T> &op1, const ResultData<T> &op2) {
    return op1.data == op2.data;
}

template <typename T>
bool operator <(const ResultData<T> &op1, const ResultData<T> &op2) {
    return  op1.data < op2.data;
}

template <typename T>
bool operator >(const ResultData<T> &op1, const ResultData<T> &op2) {
    return  op1.data > op2.data;
}

using namespace std::rel_ops;

/*
 * 定义 Result 类。
 * 里面的 res_list_ 成员是一个 vector，
 * 用于保存每一个种子向量与所有字典向量的相似度计算结果。
 */

template<typename T, unsigned TOP_K>
class Result {

    //using ResList = std::list<T>;
    using ResList = std::vector<T>;
    enum {
        kMaxNumOfResultData = TOP_K
    };

private:
    size_t len_ = 0;
    ResList res_list_;
    typename ResList::iterator min_iter_;
    //typename T::DataType min_val_ = 0;

    //std::mutex insert_data_mutex_;

public:

    Result() {
        // 每次只需要保存最大的10个就好。
        // reserve 20个 unsigned short 大小的内存，
        // 防止 vector 增长再分配内存消耗时间。
        res_list_.reserve(kMaxNumOfResultData*2);
        min_iter_ = res_list_.begin();
    }

    const ResList& GetResult() const {
        return res_list_;
    }

    size_t Len() {
        return len_;
    }

    /*
     * 插入一个相似度计算结果。
     * 该方法在插入的同时，保证了永远只保留最大的10个值。
     */
    template<typename CMP_TYPE=std::greater<T>>
    void InsertResData(T&& data, CMP_TYPE cmp_op=CMP_TYPE()) {

        // 当存的结果到了10个之后，把要插入的新值
        // 与 mini_iter_ 做比较，如果比已经保存的结果中
        // 的最小值还小，直接丢弃。
        //
        // 否则直接将最小值替换为这个新值。
        // 然后更新最小值，保持永远只存最大的10个。
        if (len_ == kMaxNumOfResultData) {
            if (cmp_op(*min_iter_, data)) {
                return ;
            }
            *min_iter_ = std::move(data);
            min_iter_ = std::min_element(res_list_.begin(), res_list_.end());
            return ;
        }

        // 不够10个的时候, 直接在 vector 尾部插入就好了。
        res_list_.push_back(std::move(data));
        len_++;

        /*
         * 下面的代码使用了插入排序代替 上面的 min_element。
         * 本质上都是 O(DictNum * Top_K) 。
         */

        /*
        auto i = res_list_.rbegin();
        for ( ; i != res_list_.rend(); ++i) {
            if (cmp_op(*i, data)) {
                break;
            }
        }

        if (len_ == kMaxNumOfResultData) {
            data.swap(*(--i));
        } else {
            res_list_.insert((--i).base(), std::move(data));
            len_++;
        }*/
    }

    using MetaDataType = typename T::DataType;
};

/*
 * 一些类型别名的定义，方便代码使用。
 * 都是对上面 Result 类的别名定义。
 */

template <typename T>
using Top10Similarity = Result<T, 10>;

// 用于保存1000个 seed 向量结果的数组，
// 每个元素都是一个 Result<T, 10> 。
template <typename T>
using AllResults = std::vector<Top10Similarity<T>*>;
