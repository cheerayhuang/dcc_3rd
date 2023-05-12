#include "calc.h"
#include "data_reader.h"



int main(int argc, char* argv[]) {
    if (argc == 1) {
        argv[1] = new char[2]{'1', '\0'};
    }

    DataReader reader;
    reader.Read();

    /*
     * 读取数据类，读完之后，将数据指针交出来。
     *
     * TODO: 由于有很多申请内存、重排、拷贝内存等细节操作，
     * 所以本代码目前都没有使用智能指针。后续评估性能影响之后可以改进。
     */

    auto seed_data = reader.MoveSeedData();
    auto dict_data = reader.MoveDictData();

    /*
     * 封装了一个简单的计算过程对象。传递数据进去，进行计算。
     */
    Calc calc(seed_data, dict_data, std::stoi(argv[1]));
    calc.Run();

    std::cout << calc.GetRunningTime() << std::endl;

    return 0;
}
