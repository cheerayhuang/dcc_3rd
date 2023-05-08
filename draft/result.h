#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <iostream>
//#include <mutex>
#include <vector>

template <typename T>
struct ResultData {

    T data;
    unsigned int index;

    using DataType = T;
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

    //using ResList = std::list<T>;
    using ResList = std::vector<T>;
    enum {
        kMaxNumOfResultData = TOP_K
    };

private:
    size_t len_ = 0;
    ResList res_list_;
    //typename ResList::iterator min_iter_ = res_list_.begin();
    typename T::DataType min_val_ = 0;

    //std::mutex insert_data_mutex_;

public:

    Result() {
        res_list_.reserve(120000);
    }

    const ResList& GetResult() const {
        return res_list_;
    }

    size_t Len() {
        return len_;
    }

    template<typename CMP_TYPE=std::greater<T>>
    void InsertResData(T&& data, CMP_TYPE cmp_op=CMP_TYPE()) {
        /*
        if (len_ == kMaxNumOfResultData && cmp_op(*min_iter_, data)){
            return ;
        }*/

        if (min_val_ > data.data) {
            return;
        }

        res_list_.push_back(std::move(data));
        len_++;

        /*
        if (len_ == kMaxNumOfResultData) {
            res_list_.erase(min_iter_);
        } else {
            len_++;
        }*/

        //std::cout << len_ << std::endl;

        if (len_ >= 100000) {
            std::nth_element(res_list_.begin(),
                res_list_.begin()+kMaxNumOfResultData-1,
                res_list_.end(),
                cmp_op);
            /*
            for (auto&& i : res_list_) {
                std::cout << i.data << ", ";
            }*/
            //std::cout << std::endl;

            res_list_.erase(res_list_.begin()+kMaxNumOfResultData, res_list_.end());
            auto min_pos = std::min_element(res_list_.begin(), res_list_.end());
            min_val_ = min_pos->data;
            len_ = kMaxNumOfResultData;
        }
    }

    void SortRes() {
        //std::cout << res_list_.size() << std::endl;

        std::nth_element(res_list_.begin(),
            res_list_.begin()+kMaxNumOfResultData-1,
            res_list_.end(),
            std::greater<T>{});
    }

    using MetaDataType = typename T::DataType;
};

template <typename T>
using Top10Similarity = Result<T, 10>;

template <typename T>
using AllResults = std::vector<Top10Similarity<T>*>;
