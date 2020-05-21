#include <iostream>
#include <bitset>
#include <queue>
#include <future>

#include "lib/pool.hpp"
#include "lib/measure.hpp"

const uint64_t  N = 1000000;
const size_t THREAD_N = 12;

int main() {
    auto array = std::make_shared<std::bitset<N>>();

    measure::Measure<1, std::chrono::milliseconds>()
    .execute<>([&array]() {
        thread::Pool pool(THREAD_N);

        for (uint64_t i = 2; i < N; i++) {
            pool.push([&array](uint64_t i) {
                uint64_t ii = i * i;
                auto temp = std::make_shared<std::bitset<N>>();
                for (uint64_t j = ii; j < N; j += i) {
                    temp->set(j - 1, true);
                };
                (*array) |= (*temp);
            }, i);
        }

        pool.join();
    }).log("Initialize sieve")
    .execute([&array]() {
        for (uint64_t i = 2; i < N; i++) {
            if (!(*array)[i - 1]) {
                std::cout << i << "\t";
            }
        }
    }, false).log("Print Primes")
    .report("isPrime");

    return 0;
}
