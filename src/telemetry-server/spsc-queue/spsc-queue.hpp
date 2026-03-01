#ifndef CLANG_TELEMETRY_SPSC
#define CLANG_TELEMETRY_SPSC

#include <atomic>
#include <array>

#ifdef __cpp_lib_hardware_interference_size
static constexpr size_t cacheLine_ = std::hardware_destructive_interference_size;
#else
static constexpr size_t cacheLine_ = 64;
#endif

struct SPSCQueueStats
{
    size_t queue_pushed{0};
    size_t queue_dropped{0};
    size_t queue_read{0};
};

template <typename T, size_t Size = 1024>
class SPSCQueue
{
    static_assert(Size > 0 && (Size & (Size - 1)) == 0, "Size must be a power of 2");

    std::array<T, Size> buffer;
    alignas(cacheLine_) std::atomic<size_t> head{0};
    alignas(cacheLine_) std::atomic<size_t> tail{0};

public:
    bool push(const T &data)
    {
        const size_t t = tail.load(std::memory_order_relaxed);
        const size_t next_tail = (t + 1) & (Size - 1);
        if (next_tail == head.load(std::memory_order_acquire))
        {
            return false;
        }
        buffer[t] = data;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }
    bool pop(T &out)
    {
        const size_t h = head.load(std::memory_order_relaxed);
        if (h == tail.load(std::memory_order_acquire))
        {
            return false;
        }

        const size_t next_head = (h + 1) & (Size - 1);

        out = buffer[h];
        head.store(next_head, std::memory_order_release);
        return true;
    }
};

#endif