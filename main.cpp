#include <iostream>
#include <bitset>
#include <queue>
#include <future>

#include "lib/pool.hpp"
#include "lib/timer.hpp"

const uint64_t  N = 100000000;
const size_t THREAD_N = 1;

int main() {
    auto array = std::make_shared<std::bitset<N>>();

    measure::Measure<1, std::chrono::milliseconds>().execution([&array]() {
        thread::Pool pool(THREAD_N);
        std::queue<std::future<void>> tasks;

        for (uint64_t i = 2; i < N; i++) {
            tasks.emplace(pool.push([&array](uint64_t i) {
                uint64_t ii = i * i;
                auto temp = std::make_shared<std::bitset<N>>();
                for (uint64_t j = ii; j < N; j+=i) {
                    temp->set(j - 1, true);
                };
                (*array) |= (*temp);
            }, i));
        }

        while (!tasks.empty()) {
            tasks.front().get();
            tasks.pop();
        }
    }).logging("init");

    measure::Measure<1, std::chrono::milliseconds>().execution([&array]() {
        thread::Pool pool(THREAD_N);
        std::queue<std::future<void>> tasks;

        for (uint64_t i = 2; i < N; i++) {
            tasks.emplace(pool.push([&array, i]() {
                if (!(*array)[i - 1]) {
                    std::cout << i << "\t";
                }
            }));
        }

        while (!tasks.empty()) {
            tasks.front().get();
            tasks.pop();
        }
    }, false).logging("calc");
    return 0;
}
