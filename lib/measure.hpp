//
// Created by Jiun, Bae on 2020-05-21.
//

#ifndef PRIME_MEASURE_HPP
#define PRIME_MEASURE_HPP

#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <chrono>
#include <functional>
#include <list>
#include <tuple>
#include <cstring>
#include <cpuid.h>

template<std::size_t...> struct seq{};

template<std::size_t N, std::size_t... Is>
struct gen_seq : gen_seq<N-1, N-1, Is...>{};

template<std::size_t... Is>
struct gen_seq<0, Is...> : seq<Is...>{};

template<class Ch, class Tr, class Tuple, std::size_t... Is>
void print_tuple(std::basic_ostream<Ch,Tr>& os, Tuple const& t, seq<Is...>){
    using swallow = int[];
    (void)swallow{0, (void(os << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
}

template<class Ch, class Tr, class... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t)
-> std::basic_ostream<Ch, Tr>& {
    os << "(";
    print_tuple(os, t, gen_seq<sizeof...(Args)>());
    return os << ")";
}

struct measure {
    using clock = std::chrono::high_resolution_clock;

    template<size_t N = 10, typename T = std::chrono::microseconds>
    class Measure {
    private:
        T duration;
        std::list<std::stringstream*> logs;

    public:
        explicit Measure() : duration() {}

        template<typename F, typename ...Args>
        auto execute(F&& f, bool verbose = false, Args&&... args) {
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

        auto log(const char* title, std::ostream& ostream = std::cout) {
            auto stream = new std::stringstream();

            (*stream) << title << " (" << N << " times)" << std::endl;
            (*stream) << "\tTotal: " << this->duration.count() << unit_string() << std::endl;
            (*stream) << "\tMean: " << this->duration.count() / (double)N << unit_string() << std::endl;

            ostream << stream->str();
            this->logs.push_back(stream);

            return *this;
        }

        void report(const char* pathname, const char* extension = ".log") const {
            if (!std::filesystem::exists(pathname)) {
                std::filesystem::create_directory(pathname);
            }

            auto path = std::filesystem::path(pathname) /
                    std::to_string(std::chrono::duration_cast<T>(clock::now().time_since_epoch()).count());
            path.replace_extension(extension);
            std::ofstream ostream(path.string());

            if (ostream.is_open()) {
                {   // report CPU BRAND STRING
                    char CPU_BRAND_STRING[0x40];
                    unsigned int CPU_INFO[4] = {0,};

                    __cpuid(0x80000000, CPU_INFO[0], CPU_INFO[1], CPU_INFO[2], CPU_INFO[3]);
                    unsigned int nExIDs = CPU_INFO[0];

                    for (unsigned int i = 0x80000000; i <= nExIDs; ++i) {
                        __cpuid(i, CPU_INFO[0], CPU_INFO[1], CPU_INFO[2], CPU_INFO[3]);

                        if (i == 0x80000002)
                            memcpy(CPU_BRAND_STRING, CPU_INFO, sizeof(CPU_INFO));
                        else if (i == 0x80000003)
                            memcpy(CPU_BRAND_STRING + 16, CPU_INFO, sizeof(CPU_INFO));
                        else if (i == 0x80000004)
                            memcpy(CPU_BRAND_STRING + 32, CPU_INFO, sizeof(CPU_INFO));
                    }
                    ostream << "CPU Type: " << CPU_BRAND_STRING << std::endl;
                }
                {
                    for (auto log : this->logs) {
                        ostream << log->str() << std::endl;
                    }
                }
                ostream.close();
            }
        }

        void clear() {
            this->duration = std::chrono::duration_values<T>::zero();
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

#endif //PRIME_MEASURE_HPP
