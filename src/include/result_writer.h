/*
 * 类 ResultWriter。
 * 负责向文件写入最后结果。
 */

#pragma once

#include <algorithm>
#include <fstream>
#include <string>

#include "constants.h"
#include "result.ori.h"
//#include "result.h"
//#include "result_heap.h"

template<unsigned TOP_K=kTopK>
class ResultWriter {


private:
    const std::string kResultFilePath{"result.csv"};
    const std::string kHeaderRow{"seed_index,dict_index,rank"};

    std::ofstream fout_;


public:

    ResultWriter() = default;
    ~ResultWriter() {
        fout_.close();
    }

    void WriterHeader() {
        fout_ = std::ofstream(kResultFilePath);
        fout_ << kHeaderRow << std::endl;
    }

    template<typename T,  typename RES_T>
    void Write(const AllResults<RES_T>& all_res,
        T **dict_index_points,
        T *dict_data_int) {

        WriterHeader();

        // 遍历保存了 Result 对象的数组，输出结果到文件。
        for(auto i = 0; i < all_res.size(); ++i) {
            if (all_res[i] == nullptr) continue;
            auto rank = 0;
            auto& res_list = all_res[i]->GetResult();

            //std::for_each(res_list.begin(), res_list.begin()+TOP_K,

            /*
             * 这里进行了一个简单的转换，用重排后的内存位置 index
             * 得到实际的原始数据字典的 index。
             */
            std::for_each(res_list.begin(), res_list.end(),
                    [i, this, &rank, dict_index_points, dict_data_int](auto&& data) {
                        auto index = (static_cast<unsigned>(dict_index_points[data.index]-dict_data_int) >> 8);
                        fout_ << i << "," << index
                            << "," << data.data << "," << rank << "\n";
                        rank++;
                    }
            );
            /*
            for(auto j = 0; i < TOP_K; ++i, res_list.pop()) {
                auto & data = res_list.top();
                fout_ << i << "," << data.index << "," << data.data << "," << rank << "\n";
            }*/
        }

    }
};
