#pragma once

#include <algorithm>
#include <fstream>
#include <string>

#include "result.ori.h"
//#include "result.h"
//#include "result_heap.h"

template<unsigned TOP_K=10>
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

    template<typename T>
    void Write(const AllResults<T>& all_res,
        unsigned short **dict_index_points,
        unsigned short *dict_data_int) {

        WriterHeader();

        for(auto i = 0; i < all_res.size(); ++i) {
            if (all_res[i] == nullptr) continue;
            auto rank = 0;
            auto& res_list = all_res[i]->GetResult();

            //std::for_each(res_list.begin(), res_list.begin()+TOP_K,
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
