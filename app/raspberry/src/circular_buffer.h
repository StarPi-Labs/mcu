#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "lora.h"

template<typename T, size_t Capacity>
class CircularBuffer {
public:
	CircularBuffer() : head_(0), tail_(0), count_(0) {}

	// Push an item, returns false if buffer is full
	bool push(const T& item) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (count_ >= Capacity) {
			return false; // Buffer full
		}
		buffer_[head_] = item;
		head_ = (head_ + 1) % Capacity;
		++count_;
		cv_.notify_one();
		return true;
	}

	// Pop an item, returns false if buffer is empty
	bool pop(T& item) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (count_ == 0) {
			return false; // Buffer empty
		}
		item = buffer_[tail_];
		tail_ = (tail_ + 1) % Capacity;
		--count_;
		return true;
	}

	// Try to pop with timeout (milliseconds)
	bool pop_with_timeout(T& item, int timeout_ms) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return count_ > 0; })) {
			item = buffer_[tail_];
			tail_ = (tail_ + 1) % Capacity;
			--count_;
			return true;
		}
		return false; // Timeout
	}

	// Get current count (thread-safe)
	size_t size() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return count_;
	}

	// Check if empty (thread-safe)
	bool empty() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return count_ == 0;
	}

	// Check if full (thread-safe)
	bool full() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return count_ >= Capacity;
	}

	// Clear the buffer
	void clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		head_ = 0;
		tail_ = 0;
		count_ = 0;
	}

private:
	T buffer_[Capacity];
	size_t head_;
	size_t tail_;
	size_t count_;
	mutable std::mutex mutex_;
	std::condition_variable cv_;
};

// Specialization for LoRaCommandPacket with default capacity
using CommandBuffer = CircularBuffer<LoRaCommandPacket, 32>;