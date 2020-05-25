//
// Created by Jiun Bae on 2020/05/25.
//

#ifndef PRIME_GRIDARRAY_HPP
#define PRIME_GRIDARRAY_HPP

#include <valarray>

#include <pool.hpp>

template <typename T = uint64_t, size_t BLOCK = 10000, class Allocator = DefaultAllocator<bool>>
class GridArray {
public:
    class GridArray_ {
    public:
        void* memory{};

        explicit GridArray_(T size)
        : memory(nullptr) {
            this->memory = Allocator::allocate(sizeof(bool) * size);
            if (this->memory == nullptr) {
                throw std::bad_alloc();
            }
        }
    };

    explicit GridArray(T size)
    : size(size), containers((int) ((size - 1) / BLOCK) + 1) {
        if (size < 1) {
            throw std::invalid_argument("GridArray must be at least 1.");
        }

        for (auto& container : this->containers) {
            container = new GridArray_(BLOCK);
        }
    }

    bool& operator[](T index) {
        auto axis = static_cast<size_t>(index / BLOCK);
        auto flag = static_cast<size_t>(index % BLOCK);

        return *(((bool *)this->containers[axis]->memory) + flag);
    }

private:
    T size;
    std::valarray<GridArray_*> containers;
};

#endif //PRIME_GRIDARRAY_HPP
