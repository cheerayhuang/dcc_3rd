#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <iostream>
//#include <mutex>
#include <utility>
#include <vector>

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

template<typename T, unsigned TOP_K>
class Result {

    using ResList = std::list<T>;
    //using ResList = std::vector<T>;
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
        //res_list_.reserve(kMaxNumOfResultData*2);
        min_iter_ = res_list_.begin();
    }

    const ResList& GetResult() const {
        return res_list_;
    }

    size_t Len() {
        return len_;
    }

    template<typename CMP_TYPE=std::greater<T>>
    void InsertResData(T&& data, CMP_TYPE cmp_op=CMP_TYPE()) {
        if (len_ == kMaxNumOfResultData) {
            if (cmp_op(*min_iter_, data)) {
                return ;
            }
            *min_iter_ = std::move(data);
            min_iter_ = std::min_element(res_list_.begin(), res_list_.end());
            return ;
        }

        res_list_.push_back(std::move(data));
        len_++;

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

template <typename T>
using Top10Similarity = Result<T, 10>;

template <typename T>
using AllResults = std::vector<Top10Similarity<T>*>;
