#include <iostream>
#include <bitset>
#include <queue>
#include <future>

#include "lib/pool.hpp"
#include "lib/measure.hpp"

const uint64_t  N = 100000;
const size_t THREAD_N = 4;

int main() {
    auto array = std::make_shared<std::bitset<N>>();
    object::Pool<std::bitset<N>> object_pool;

    measure::Measure<1, std::chrono::milliseconds>()
    .execute<>([&array, &object_pool]() {
        thread::Pool thread_pool(THREAD_N);

        for (uint64_t i = 2; i < N; i++) {
            thread_pool.push([&array, &object_pool](uint64_t i) {
                uint64_t ii = i * i;
                auto temp = object_pool.get();
                for (uint64_t j = ii; j < N; j += i) {
                    temp->set(j - 1, true);
                };
                (*array) |= (*temp);
            }, i);
        }

        thread_pool.join();
    }).log("Initialize sieve")
    .execute([&array]() {
        for (uint64_t i = 2; i < N; i++) {
            if (!(*array)[i - 1]) {
                std::cout << i << "\t";
            }
        }
    }, false).log("Get Primes")
    .total().report("results");

    return 0;
}
