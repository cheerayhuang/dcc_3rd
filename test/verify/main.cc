#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>

constexpr unsigned kAnsFileLineNum = 1*10000;

int main() {

    std::ifstream fin("result.csv.std");
    std::string headline;

    std::getline(fin, headline);

    std::unordered_map<unsigned, std::unordered_set<unsigned>> std_ans;

    unsigned ignore_num = 0;
    char delimiter;
    unsigned index, dict_index;


    for (auto i = 0; i < kAnsFileLineNum; ++i) {

        fin >> index >> delimiter >> dict_index >> delimiter;
        fin >> ignore_num >> delimiter >> ignore_num;

        //std::cout << std_ans.count(index) << std::endl;
        //std::cout << index << "," << dict_index << std::endl;

        if (std_ans.count(index) == 0) {
            std_ans[index] = std::unordered_set<unsigned>{dict_index};
            continue;
        }
        std_ans[index].insert(dict_index);

        //std::cout << std_ans[index].size() << std::endl;

    }
    fin.close();

    /*
    for (auto&& i : std_ans) {
        for(auto &&j : i.second) {
            std::cout << j << std::endl;
        }
    }*/

    fin.open("result.csv");
    std::getline(fin, headline);

    unsigned wrong_num = 0;

    for (auto i = 0; i < kAnsFileLineNum; ++i) {
        fin >> index >> delimiter >> dict_index >> delimiter;
        fin >> ignore_num >> delimiter >> ignore_num;

        if (std_ans[index].find(dict_index) == std_ans[index].end()) {
            wrong_num++;
            std::cout << "data index: " << index << " should not be similar with dict index: " << dict_index << std::endl;
        }

    }

    std::cout << "wrong num: " << wrong_num << std::endl;

    return 0;
}
