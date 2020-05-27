//
// Created by Jiun, Bae on 2020-05-21.
//

#ifndef POOL_HPP
#define POOL_HPP

#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

#include <memory>
#include <functional>

template <typename T>
class DefaultAllocator {
public:
    static inline void *allocate(size_t size) {
        return ::operator new(size, ::std::nothrow);
    }

    static inline void deallocate(void *pointer, size_t size) {
        ::operator delete(pointer);
    }
};

namespace object {
    template <typename T, class Allocator = DefaultAllocator<T>>
    class Pool {
    private:
        static const size_t _size;

        struct Node {
            size_t capacity{};
            void * memory{};
            Node * next;

            explicit Node(size_t capacity)
            : capacity(capacity), next(nullptr) {
                if (capacity < 1) {
                    throw std::invalid_argument("capacity must be at least 1.");
                }

                this->memory = Allocator::allocate(_size * capacity);
                if (this->memory == nullptr) {
                    throw std::bad_alloc();
                }
            }
            ~Node() {
                Allocator::deallocate(memory, _size * capacity);
            }

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;
            Node(Node&&) = delete;
            Node& operator=(Node&&) = delete;
        };

        Node first, *last;
        void * memory{};
        T* first_delete;
        size_t count_node;
        size_t capacity;
        size_t max_block_size;

        void allocate() {
            size_t size = this->count_node;
            if (assert_block(size)) {
                size = this->max_block_size;
            } else {
                size *= 2;

                if (assert_count(size)) {
                    throw std::overflow_error("cannot allocate memory, pool size is too big");
                }
                if (assert_block(size)) {
                    size = this->max_block_size;
                }
            }

            Node *node = new Node(size);
            this->last->next = node;
            this->last = node;

            this->memory = node->memory;
            this->count_node = 0;
            this->capacity = size;
        }

        [[nodiscard]] bool assert_count(size_t size) const {
            return size < this->count_node;
        }
        [[nodiscard]] bool assert_block(size_t size) const {
            return size >= this->max_block_size;
        }

    public:
        /*
            Init object::Pool, set n initial objects to ready for works
            use ::get to get object from object::Pool
        */
        explicit Pool(size_t capacity=32, size_t max_block_size=1000000)
        : first_delete(nullptr), count_node(0), capacity(capacity),
            first(capacity), max_block_size(max_block_size) {
            if (this->max_block_size < 1) {
                throw std::invalid_argument("max_block_size must be at least 1.");
            }

            this->memory = this->first.memory;
            this->last =&this->first;
        }
        ~Pool() {
            Node *node = this->first.next, *next;
            while (node) {
                next = node->next;
                delete node;
                node = next;
            }
        }

        Pool(const Pool<T, Allocator> &src) = delete;
        void operator= (const Pool<T, Allocator> &src) = delete;

        // If you want to call a non-default constructor, set init=false
        T* get(bool init=true) {
            if (this->first_delete) {
                T* result = this->first_delete;
                this->first_delete = *((T **)this->first_delete);
                new(result) T();
                return result;
            }

            if (this->count_node >= this->capacity) {
                this->allocate();
            }

            char *address = (char *)this->memory;
            address += this->count_node++ * this->_size;

            if (init) {
                return new(address) T();
            } else {
                return (T *)address;
            }
        }

        // If you want to release object without destroy, set destroy=false
        void release(T* object, bool destroy=true) {
            if (destroy) {
                object->~T();
            }

            *((T **)object) = this->first_delete;
            this->first_delete = object;
        }
    };

    template<typename T, class Allocator>
    const size_t Pool<T, Allocator>::_size
    = ((sizeof(T) + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
}

namespace thread {
    class Pool {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        std::atomic_bool stop;

    public:
        /*
            Init thread::Pool, set n workers to ready for works.
            use ::push to new task on thread::Pool
        */
        explicit Pool(size_t threads_n = std::thread::hardware_concurrency()) : stop(false) {
            if (!threads_n) throw std::invalid_argument("more than zero threads expected");

            this->workers.reserve(threads_n);
            for (; threads_n; --threads_n) {
                this->workers.emplace_back([this] {
                    while (true) {
                        std::function<void()> task = nullptr;

                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock, [this] {
                                return this->is_stop() || !this->tasks.empty();
                            });

                            if (this->is_stop() && this->tasks.empty()) {
                                return;
                            }

                            if (!this->tasks.empty()) {
                                task = std::move(this->tasks.front());
                                this->tasks.pop();
                            }
                        }

                        if (task != nullptr) {
                            task();
                        }
                    }
                });
            }
        }

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
        Pool(Pool&&) = delete;
        Pool& operator=(Pool&&) = delete;

        /*
            push function to thread::Pool parameter with emplaced bind
            By implicitly binding the given parameters at this time and returning results to std::future
            Utilizing a different point in time between parameter input and result it becomes a more elegant code.
            function std::bind and cast to std::shared_ptr for efficiency
            packaged_task is performed in parallel on shared memory resources.
        */
        template <typename F, typename... Args>
        std::future<typename std::result_of<F(Args...)>::type> push(F&& f, Args&&... args) {
            using packaged_task_t = std::packaged_task<typename std::result_of<F(Args...)>::type()>;

            std::shared_ptr<packaged_task_t> task(new packaged_task_t(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            ));

            auto res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->tasks.emplace([task]() { (*task)(); });
            }

            this->condition.notify_one();
            return res;
        }

        // release resources
        virtual ~Pool() {
            if (!this->is_stop())
                terminate();
        }

        void join() {
            while (!tasks.empty()) {}
        }

        void terminate() {
            this->stop = true;
            this->condition.notify_all();
            for (std::thread& worker : this->workers) {
                worker.join();
            }
        }

        [[nodiscard]] bool is_stop() const {
            return this->stop;
        }
    };
}

#endif //POOL_HPP
