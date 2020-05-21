//
// Created by Jiun, Bae on 2020-05-21.
//

#ifndef PRIME_TIMER_HPP
#define PRIME_TIMER_HPP

#include <iostream>
#include <chrono>
#include <functional>


struct measure {
    using clock = std::chrono::high_resolution_clock;

    template<size_t N = 10, typename T = std::chrono::microseconds>
    class Measure {
    private:
        T duration;
    public:
        Measure() : duration() {}

        template<typename F, typename ...Args>
        auto execution(F&& f, bool verbose = false, Args&&... args) {
            if (!verbose) {
                std::cout.setstate(std::ios_base::badbit);
            }

            auto start = clock::now();

            for (size_t i = 0; i < N; i++) {
                std::forward<decltype(f)>(f)(std::forward<Args>(args)...);
            }

            this->duration = std::chrono::duration_cast<T>(clock::now() - start);
            std::cout.clear();

            return *this;
        }

        auto logging(const std::string& text) const {
            std::cout << text << " (" << N << " times)" << std::endl;
            std::cout << "\tTotal: " << this->duration.count() << unit_string() << std::endl;
            std::cout << "\tMean: " << this->duration.count() / (double)N << unit_string() << std::endl;

            return *this;
        }

        auto value() const {
            return this->duration.count();
        }

        static const char* unit_string() {
            if (std::is_same<T, std::chrono::hours>::value) {
                return "h";
            } else if (std::is_same<T, std::chrono::minutes>::value) {
                return "m";
            } else if (std::is_same<T, std::chrono::seconds>::value) {
                return "s";
            } else if (std::is_same<T, std::chrono::milliseconds>::value) {
                return "ms";
            } else if (std::is_same<T, std::chrono::microseconds>::value) {
                return "\u03BCs";
            } else if (std::is_same<T, std::chrono::nanoseconds>::value) {
                return "ns";
            }
        }
    };
};

#endif //PRIME_TIMER_HPP
