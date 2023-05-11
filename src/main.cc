#include "calc.h"
#include "data_reader.h"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        argv[1] = new char[2]{'1', '\0'};
    }

    DataReader reader;
    reader.Read();

    auto seed_data = reader.MoveSeedData();
    auto dict_data = reader.MoveDictData();

    Calc calc(seed_data, dict_data, std::stoi(argv[1]));

    calc.Run();

    std::cout << calc.GetRunningTime() << std::endl;

    return 0;
}
