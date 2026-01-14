#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <iterator>

template<typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(std::size_t capacity)
        : data_(capacity), cap_(capacity), head_(0), tail_(0), sz_(0) {
        if (cap_ == 0) throw std::runtime_error("capacity 0");
    }

    std::size_t size() const { return sz_; }
    std::size_t capacity() const { return cap_; }
    bool empty() const { return sz_ == 0; }

    void push_back(const T& item) {
        data_[tail_] = item;

        if (sz_ < cap_) {
            sz_++;
        } else {
            head_ = (head_ + 1) % cap_;
        }
        tail_ = (tail_ + 1) % cap_;
    }

    void pop_front() {
        if (sz_ == 0) throw std::runtime_error("empty");
        head_ = (head_ + 1) % cap_;
        sz_--;
    }

    T& front() {
        if (sz_ == 0) throw std::runtime_error("empty");
        return data_[head_];
    }

    const T& front() const {
        if (sz_ == 0) throw std::runtime_error("empty");
        return data_[head_];
    }

    T& back() {
        if (sz_ == 0) throw std::runtime_error("empty");
        std::size_t last = (tail_ + cap_ - 1) % cap_;
        return data_[last];
    }

    const T& back() const {
        if (sz_ == 0) throw std::runtime_error("empty");
        std::size_t last = (tail_ + cap_ - 1) % cap_;
        return data_[last];
    }

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(CircularBuffer<T>* b, std::size_t off) : b_(b), off_(off) {}

        reference operator*() const {
            std::size_t idx = (b_->head_ + off_) % b_->cap_;
            return b_->data_[idx];
        }

        iterator& operator++() { off_++; return *this; }
        bool operator!=(const iterator& other) const { return off_ != other.off_ || b_ != other.b_; }

    private:
        CircularBuffer<T>* b_;
        std::size_t off_;
    };

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, sz_); }

private:
    std::vector<T> data_;
    std::size_t cap_;
    std::size_t head_;
    std::size_t tail_;
    std::size_t sz_;
};
