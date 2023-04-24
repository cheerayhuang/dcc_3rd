#include <iostream>
#include <random>
#include <limits>
#include <fstream>
#include <algorithm>
#include <iterator>

#include "timer.h"

using namespace std;

int main( ){

    cout << sizeof(unsigned int) << endl;
    cout << sizeof(unsigned long) << endl;
    cout << sizeof(unsigned long long) << endl;
    cout << std::numeric_limits<unsigned int>::max() << endl;
    cout << std::numeric_limits<unsigned long>::max() << endl;

    shuffle_order_engine<linear_congruential_engine<unsigned int, 32767, 1777, 2147483647>, 100> e;
    default_random_engine e2;
    std::mt19937_64 e3;
    uniform_real_distribution<double> u(0.0, 1.0);

    Timer t1;

    e3.seed(t1.NowAsNanoseconds());
    for(int i=0; i<10; ++i)
        cout << u(e3) << endl;


    std::ifstream fin("./seed_vec.csv");

    double arr[256];
    char delimiter;

    for(auto i = 0; i < 255; ++i) {
        fin >> arr[i] >> delimiter;
    }
    fin >> arr[255];

    std::ostream_iterator<double> o_iter(cout, "  ");
    std::copy(std::begin(arr), std::end(arr), o_iter);

    

    return 0;
}
