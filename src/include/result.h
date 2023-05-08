#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <iostream>
//#include <mutex>
#include <random>
#include <vector>

#include "timer.h"

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
        kMaxNumOfResultData = TOP_K,
        kResListReserveCap = 10100,
        kResListNeedSortingCap = 10000
    };

private:
    size_t len_{0};
    size_t cont_insert_num_{0};
    ResList res_list_{kResListReserveCap};
    //typename ResList::iterator min_iter_ = res_list_.begin();

    typename T::DataType min_last_10_inserted_{0}, max_last_10_inserted_{0};

    std::mt19937_64 rand_e_;
    std::uniform_int_distribution<unsigned> rand_distri_int_{0, kResListNeedSortingCap};

    unsigned drop_{0};

public:
    typename T::DataType min_val{0};

public:

    Result(){
        res_list_.reserve(kResListReserveCap);
        rand_e_.seed(Timer::NowAsNanoseconds());
    };

    Result(const Result&) = delete;
    Result(Result&&) = delete;

    Result& operator=(const Result&) = delete;
    Result& operator=(Result&&) = delete;

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

        /*
        if (min_val > data.data) {
            drop_++;
            return ;
        }*/

        //res_list_.push_back(std::move(data));
        res_list_[len_] = std::move(data);
        len_++;
        cont_insert_num_++;
        if (cont_insert_num_ == 1) {
            min_last_10_inserted_ = max_last_10_inserted_ = data.data;
            return ;
        }

        if (data.data > max_last_10_inserted_) {
            max_last_10_inserted_ = data.data;
        }
        if (data.data < min_last_10_inserted_) {
            min_last_10_inserted_ = data.data;
        }

        if (cont_insert_num_ == kMaxNumOfResultData) {
            /*
            auto new_min_val = res_list_[rand_distri_int_(rand_e_) % len_];
            while(new_min_val.data <= min_val) {
                new_min_val = res_list_[rand_distri_int_(rand_e_) % len_];
            }
            min_val = new_min_val.data;
            */
            cont_insert_num_ = 0;
            min_val = (max_last_10_inserted_ + min_last_10_inserted_) / 2;

            return ;
        }

        /*
        if (len_ == kMaxNumOfResultData) {
            res_list_.erase(min_iter_);
        } else {
            len_++;
        }*/

        if (len_ >= kResListNeedSortingCap) {
            //std::cout << "entry nth..." << std::endl;
            //Timer t;
            //t.Start();
            std::nth_element(res_list_.begin(),
                res_list_.begin()+kMaxNumOfResultData-1,
                res_list_.end(),
                cmp_op);

            //res_list_.erase(res_list_.begin()+kMaxNumOfResultData, res_list_.end());
            len_ = kMaxNumOfResultData;
            auto min_iter = std::min_element(res_list_.begin(), res_list_.begin()+len_);
            if (min_iter->data > min_val) {
                min_val = min_iter->data;
            }

            cont_insert_num_ = 0;
            //std::cout << "leave nth: " << t.Stop() << std::endl;
        }
    }

    template<typename CMP_TYPE=std::greater<T>>
    void SortRes(CMP_TYPE cmp_op=CMP_TYPE{}) {
        /*
        std::cout << "last, len_ = " << len_ << std::endl;
        std::cout << "last, drop_ = " << drop_ << std::endl;
        */
        std::nth_element(res_list_.begin(),
            res_list_.begin()+kMaxNumOfResultData-1,
            res_list_.begin()+len_,
            cmp_op);
    }

    using MetaDataType = typename T::DataType;
};

template <typename T>
using Top10Similarity = Result<T, 10>;

template <typename T>
using AllResults = std::vector<Top10Similarity<T>*>;
