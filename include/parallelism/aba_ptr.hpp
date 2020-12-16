#pragma once

#include <atomic>

template <typename T_>
class aba_ptr
{
private:
    friend class std::atomic<aba_ptr<T_>>;
    static constexpr uint64_t COUNT_MASK = 0XFFFFull << 48;

    static constexpr uint64_t POINTER_MASK = ~COUNT_MASK;

    uint64_t ptr_ = 0;

    aba_ptr(uint64_t ptr)
    {
        ptr_ = ptr;
    }

public:
    aba_ptr() = default;

    aba_ptr(T_ *ptr)
    {
        *this = ptr;
    }

    ~aba_ptr() = default;

    aba_ptr &operator=(T_ *ptr)
    {
        ptr_ &= COUNT_MASK;
        ptr_ += (uint64_t)ptr;

        return *this;
    }

    T_ *get()
    {
        return reinterpret_cast<T_ *>(ptr_ & POINTER_MASK);
    }

    T_ &operator*()
    {
        return *get();
    }

    T_ *operator->()
    {
        return get();
    }

    operator T_ *()
    {
        return get();
    }
};

namespace std
{
    template <typename T_>
    class atomic<aba_ptr<T_>>
    {
    private:
        static constexpr uint64_t COUNT_MASK = aba_ptr<T_>::COUNT_MASK;
        static constexpr uint64_t POINTER_MASK = aba_ptr<T_>::POINTER_MASK;

        std::atomic<uint64_t> ptr_;

    public:
        atomic()
        {
            ptr_ = 0;
        }
        atomic(const aba_ptr<T_> &ptr)
        {
            ptr_ = ptr.ptr_;
        }

        atomic &operator=(aba_ptr<T_> desired)
        {
            store(desired);
            return *this;
        }

        operator aba_ptr<T_>()
        {
            return load();
        }

        void store(aba_ptr<T_> desired, std::memory_order failure = std::memory_order_seq_cst)
        {
            ptr_.store(desired.ptr_, failure);
        }

        aba_ptr<T_> load(std::memory_order failure = std::memory_order_seq_cst)
        {
            return aba_ptr<T_>(ptr_.load(failure));
        }

        aba_ptr<T_> exchange(aba_ptr<T_> desired, std::memory_order failure = std::memory_order_seq_cst)
        {
            return aba_ptr<T_>(ptr_.exchange(desired, failure));
        }

        bool compare_exchange_weak(aba_ptr<T_> &expected, aba_ptr<T_> desired, std::memory_order failure = std::memory_order_seq_cst)
        {
            uint16_t count = ((expected.ptr_ & COUNT_MASK) >> 48) + 1;

            uint64_t des = ((uint64_t)count << 48) + (desired.ptr_ & POINTER_MASK);

            return ptr_.compare_exchange_weak(expected.ptr_, des, failure);
        }

        bool compare_exchange_strong(aba_ptr<T_> &expected, aba_ptr<T_> desired, std::memory_order failure = std::memory_order_seq_cst)
        {
            uint16_t count = (expected.ptr_ & COUNT_MASK) + 1;

            uint64_t des = ((uint64_t)count << 48) + (desired.ptr_ & POINTER_MASK);
            return ptr_.compare_exchange_weak(expected.ptr_, des, failure);
        }

        aba_ptr<T_> fetch_add(std::ptrdiff_t arg, std::memory_order failure = std::memory_order_seq_cst)
        {
            return aba_ptr<T_>(ptr_.fetch_add(arg, failure));
        }

        aba_ptr<T_> fetch_sub(std::ptrdiff_t arg, std::memory_order failure = std::memory_order_seq_cst)
        {
            return aba_ptr<T_>(ptr_.fetch_sub(arg, failure));
        }
    };
} // namespace std