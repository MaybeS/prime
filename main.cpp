#include <iostream>
#include <bitset>
#include <future>

#include <measure.hpp>

#include <gridarray.hpp>
#include <solver.hpp>

const uint64_t N = 100000000;

int main() {
    prime::Solver solver{};

    measure::Measure<1, std::chrono::milliseconds>("isPrime")
    .execute<>([&solver]() {
        solver = prime::Solver(N);
        solver.init();
    }, true).log("Initialize sieve")
    .execute([&solver]() {
        solver.iter([](uint_fast64_t i) {
            std::cout << i << ", ";
        });
    }, false).log("Get Primes")
    .total().report("results");

    return 0;
}
