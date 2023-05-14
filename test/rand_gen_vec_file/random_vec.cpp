#include <chrono>
#include <fstream>
#include <random>
#include <string>

#include "args.h"
#include "timer.h"

enum class VecType {
    kSeedVector = 8,
    kDictVector = 9

};

class TestGenerator {

private:

    std::mt19937_64 rand_e_;
    std::uniform_real_distribution<double> rand_distri_{0.0, 1.0};
    std::string seed_file_, dict_file_;

    const unsigned int kDimetion = FLAGS_dimetion;
    const unsigned int kSeedVecTotal = FLAGS_seed_total;
    const unsigned int kDictVecTotal = FLAGS_dict_total;

public:

    TestGenerator(std::string seed_file=FLAGS_seed_file, std::string dict_file=FLAGS_dict_file):
        seed_file_(std::move(seed_file)),
        dict_file_(std::move(dict_file)) {

        rand_e_.seed(Timer::NowAsNanoseconds());
    }

    TestGenerator(const TestGenerator&) = delete;
    TestGenerator(TestGenerator&&) = delete;

    TestGenerator& operator=(const TestGenerator&) = delete;
    TestGenerator& operator=(TestGenerator&&) = delete;

    bool GenVectors(VecType vec_type) {
        auto nums = kSeedVecTotal;
        auto fout = std::ofstream(seed_file_, std::ios::app | std::ios::out);
        if (vec_type == VecType::kDictVector) {
            nums = kDictVecTotal;
            fout.close();
            fout = std::ofstream(dict_file_, std::ios::app | std::ios::out);
        }

        for (auto i = 0; i < nums; ++i) {
            for (auto j = 0; j < kDimetion; ++j) {
               fout << rand_distri_(rand_e_);
               if (j == kDimetion-1) {
                   fout << std::endl;
               } else {
                   fout << ",";
               }
            }
        }

        fout.close();
        return true;
    }
};


int main(int argc, char **argv) {

    gflags::SetVersionString("0.1.0");
    gflags::SetUsageMessage("Generate vectors stored in files for testing.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);


    /*
    VecType v_type = VecType::kSeedVector;
    std::cout << static_cast<int>(v_type) << std::endl;
    */

    TestGenerator t;
    //t.GenVectors(v_type);
    t.GenVectors(VecType::kDictVector);

    return 0;
}
