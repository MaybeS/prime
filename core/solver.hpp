//
// Created by Jiun Bae on 2020/05/25.
//

#ifndef PRIME_SOLVER_HPP
#define PRIME_SOLVER_HPP

#include <gridarray.hpp>
#include <pool.hpp>

namespace prime {
    template <typename T = uint64_t>
    class Solver {
    public:
        Solver() = default;
        explicit Solver(T size)
        : size(size), sieve(std::make_shared<GridArray<T>>(size)) {}

        void init() {
            thread::Pool thread_pool{};

            for (uint64_t i = 2; i < size; i++) {
                thread_pool.push([&](uint64_t i) {
                    uint64_t ii = i * i;
                    for (uint_fast64_t j = ii; j < size; j+= i) {
                        sieve->operator[](j-1) |= true;
                    }
                }, i);
            }

            thread_pool.join();
        }

        void iter(const std::function<void(T)>& f) {
            for (uint64_t i = 2; i < size; i++) {
                if (!sieve->operator[](i - 1)) {
                    f(i - 1);
                }
            }
        }

    private:
        T size;
        std::shared_ptr<GridArray<T>> sieve;
    };
}

#endif //PRIME_SOLVER_HPP
