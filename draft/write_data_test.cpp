#include <iostream>
#include <algorithm>
#include <mutex> 
#include <vector>

#include "result_writer.h"

int main() {
    Result r;

    r.InsertResData(5.5f, 1);
    r.InsertResData(5.5123f, 3);
    r.InsertResData(5.5124f, 2);

    auto res = r.GetResult();
    auto res_index = r.GetIndexList();

    auto echo_f = [](const auto & data) {
        std::cout << data << ", ";
    };

    std::for_each(
        res.begin(), 
        res.end(),
        echo_f 
    );

    std::for_each(
        res_index.begin(), 
        res_index.end(),
        echo_f 
    );

    ResultWriter::AllResults all_res{2};

    all_res[0] = &r;

    //all_res.emplace_back(Result());
    //all_res.emplace_back();
    all_res[1] = new Result();
    
    all_res[1]->InsertResData(3.14f, 3);
    all_res[1]->InsertResData(3.1415f, 2);
    all_res[1]->InsertResData(3.14159f, 4);
    all_res[1]->InsertResData(3.141592f, 5);
    ResultWriter().Write(all_res);

    
    std::vector<char*> v_test{nullptr, nullptr};
    std::cout << v_test.size() << std::endl;

    for (auto&& x: v_test) {
        std::cout << reinterpret_cast<uintptr_t>(x) << std::endl;
    }

    return 0;
}
