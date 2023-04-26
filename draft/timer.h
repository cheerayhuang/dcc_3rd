#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream> 


using namespace std::literals::chrono_literals;

class Timer {

public:
    //Timer() = default;
    Timer(size_t round=1): round_(round) {}
    
    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    ~Timer(){
        dura_log_.close();
    }

public:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    void Start() {
        if (!dura_log_.is_open()) {
            dura_log_.open(kDurationLogPath, std::ios::out|std::ios::app);
        }
        beg_point_ = clock_.now();
    }

    Timer& Stop() {
        end_point_ = clock_.now();

        return *this;
    }

    unsigned long long AsNanoseconds() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end_point_-beg_point_).count();  
    }

    double AsSeconds() {
        return static_cast<double>(AsNanoseconds()) / static_cast<double>(std::giga::num);
    }

    double AsMiliseconds() {
        return static_cast<double>(AsNanoseconds()) / static_cast<double>(std::mega::num);
    }

    double AsMicroseconds() {
        return static_cast<double>(AsNanoseconds()) / static_cast<double>(std::kilo::num);
    }

    void WriteDurationLog(){
        dura_log_ << *this << "\n";
    }

    static TimePoint Now() {
        return std::chrono::high_resolution_clock().now();
    }
    
    static long long NowAsNanoseconds() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock().now().time_since_epoch()).count();
    }
    
    friend std::ostream& operator<<(std::ostream& cout, Timer& t);
    friend std::ofstream& operator<<(std::ofstream& fout, Timer& t);

private:

    std::chrono::high_resolution_clock clock_;
    TimePoint beg_point_;
    TimePoint end_point_;

    size_t round_;

    std::ofstream dura_log_;

    const std::string kDurationLogPath{"duration.log"};
};

std::ostream& operator<<(std::ostream& cout, Timer& t) {
    auto ori_flags = cout.flags();
    cout << std::fixed << std::setprecision(4);

    cout << "elapsed: " << t.AsNanoseconds() << " nano, " 
        << t.AsMicroseconds() << " us, "
        << t.AsMiliseconds() << " ms, " 
        <<  t.AsSeconds() << " s.";

    cout.flags(ori_flags);

    return cout;
}

std::ofstream& operator<<(std::ofstream& fout, Timer& t) {
    auto ori_flags = fout.flags();
    fout << std::fixed << std::setprecision(3);
    fout << "round:" << t.round_ << ",duration:" << t.AsSeconds();
    fout.flags(ori_flags);

    return fout;
}
