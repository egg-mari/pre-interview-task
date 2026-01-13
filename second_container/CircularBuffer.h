#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

template<typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(std::size_t capacity)
        : data_(capacity), capacity_(capacity), head_(0), tail_(0), size_(0) {
        if (capacity_ == 0) {
            throw std::invalid_argument("capacity must be > 0");
        }
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }

private:
    std::vector<T> data_;
    std::size_t capacity_;
    std::size_t head_; // 가장 오래된 요소의 인덱스
    std::size_t tail_; // 다음에 삽입할 위치
    std::size_t size_;
};
