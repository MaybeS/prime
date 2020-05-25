#include <iostream>
#include <bitset>
#include <future>

#include <pool.hpp>
#include <measure.hpp>

#include <gridarray.hpp>

const uint64_t N = 10000;
const size_t THREAD_N = 4;

int main() {
    auto sieve = GridArray<uint_fast64_t, 100>(N);
    measure::Measure<100, std::chrono::milliseconds>()
    .execute<>([&sieve]() {
        thread::Pool thread_pool(THREAD_N);

        for (uint64_t i = 2; i < N; i++) {
            thread_pool.push([&sieve](uint64_t i) {
                uint64_t ii = i * i;

                for (uint_fast64_t j = ii; j < N; j+= i) {
                    sieve[j - 1] |= true;
                }
            }, i);
        }

        thread_pool.join();
    }, true).log("Initialize sieve")
    .execute([&sieve]() {
        for (uint64_t i = 2; i < N; i++) {
            if (!sieve[i - 1]) {
                std::cout << i << "\t";
            }
        }
    }, false).log("Get Primes")
    .total().report("results");

    return 0;
}
