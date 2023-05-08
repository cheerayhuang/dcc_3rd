#include "timer.h"
#include <forward_list>
#include <thread>

auto func() {
    using secs_per_hour = std::ratio_multiply<std::ratio<1>, std::chrono::hours::period>;
    std::this_thread::sleep_for(1.56s);
    return secs_per_hour::num;
}

int main() {
    Timer t1;
    t1.Start();
    std::cout << func() << std::endl;
    std::cout << t1.Stop().AsNanoseconds() << std::endl;
    auto flags = std::cout.flags();
    std::cout << std::fixed << std::setprecision(9) << t1.AsSeconds() << std::endl;
    std::cout << t1.AsMiliseconds() << std::endl;
    std::cout << t1.AsMicroseconds() << std::endl;
    std::cout << t1 << std::endl;
    //std::cout.flags(flags);

    std::cout << t1.NowAsNanoseconds() << std::endl;

    return 0;
}
