#pragma once

#include <algorithm>
#include <fstream>
#include <string>

#include "result.h"

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
    void Write(const AllResults<T>& all_res) {
        WriterHeader();

        for(auto i = 0; i < all_res.size(); ++i) {
            if (all_res[i] == nullptr) continue;
            auto rank = 0;
            const auto& res_list = all_res[i]->GetResult();

            std::for_each(res_list.begin(), res_list.begin()+TOP_K,
                    [i, this, &rank](auto&& data) {
                        fout_ << i << "," << data.index << "," << data.data << "," << rank << "\n";
                        rank++;
                    }
            );
        }

    }
};
