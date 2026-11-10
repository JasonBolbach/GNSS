#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

// Lock-free single-producer / single-consumer ring buffer.
//
//   Producer side (one context only): enqueue(), enqueue(ptr, n)
//   Consumer side (one context only): dequeue(), peek()
//   Either side:                      size(), isEmpty(), isFull(), maxcount()
//
// Head is written only by the consumer, tail only by the producer. Indices are
// free-running and masked on access, so no shared counter is ever read-modified-
// written by two contexts. Works for task<->task, ISR->task, or core<->core on any
// target that provides lock-free std::atomic<uint32_t> (ESP32, ARM Cortex-M3+, ...).
//
// reset() is NOT thread-safe: call it only while neither side is running.
template <typename T, uint32_t MAX_SIZE> class StaticQueue {
  static_assert(MAX_SIZE >= 2 && (MAX_SIZE & (MAX_SIZE - 1)) == 0, "MAX_SIZE must be a power of two");
  static constexpr uint32_t MASK = MAX_SIZE - 1;

  T data[MAX_SIZE];
  std::atomic<uint32_t> head{0};	// consumer-owned
  std::atomic<uint32_t> tail{0};	// producer-owned
  std::atomic<uint32_t> max{0};	// high-water mark, producer-written (statistic only)

  void noteMax(uint32_t used) {
    if (used > max.load(std::memory_order_relaxed)) max.store(used, std::memory_order_relaxed);
  }

  public:
    StaticQueue() = default;
    StaticQueue(const StaticQueue&) = delete;
    StaticQueue& operator=(const StaticQueue&) = delete;

    // ---- producer ----
    bool enqueue(const T& item) {
      const uint32_t t = tail.load(std::memory_order_relaxed);
      const uint32_t h = head.load(std::memory_order_acquire);
      if (t - h >= MAX_SIZE) return false;
      data[t & MASK] = item;
      tail.store(t + 1, std::memory_order_release);
      noteMax(t + 1 - h);
      return true;
    }

    // Bulk enqueue; returns how many items were stored (may be < n if the queue fills).
    size_t enqueue(const T* items, size_t n) {
      const uint32_t t = tail.load(std::memory_order_relaxed);
      const uint32_t h = head.load(std::memory_order_acquire);
      const uint32_t freeSlots = MAX_SIZE - (t - h);
      if (n > freeSlots) n = freeSlots;
      for (size_t i = 0; i < n; i++) data[(t + i) & MASK] = items[i];
      tail.store(t + (uint32_t)n, std::memory_order_release);
      noteMax(t + (uint32_t)n - h);
      return n;
    }

    // ---- consumer ----
    bool dequeue(T& output) {
      const uint32_t h = head.load(std::memory_order_relaxed);
      const uint32_t t = tail.load(std::memory_order_acquire);
      if (h == t) return false;
      output = data[h & MASK];
      head.store(h + 1, std::memory_order_release);
      return true;
    }

    bool peek(T& output) const {
      const uint32_t h = head.load(std::memory_order_relaxed);
      const uint32_t t = tail.load(std::memory_order_acquire);
      if (h == t) return false;
      output = data[h & MASK];
      return true;
    }

    // ---- either side (value is a snapshot and may be stale by the time you use it) ----
    int  size()    const { return (int)(tail.load(std::memory_order_acquire) - head.load(std::memory_order_acquire)); }
    bool isEmpty() const { return size() == 0; }
    bool isFull()  const { return size() >= (int)MAX_SIZE; }
    int  maxcount() const { return (int)max.load(std::memory_order_relaxed); }
    void resetmax() { max.store(0, std::memory_order_relaxed); }

    // Not thread-safe.
    void reset() {
      head.store(0, std::memory_order_relaxed);
      tail.store(0, std::memory_order_relaxed);
      max.store(0, std::memory_order_relaxed);
    }
};
