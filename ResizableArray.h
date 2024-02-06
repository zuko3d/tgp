#pragma once

#include <array>

template <typename T, size_t N>
class ResizableArray {
public:
    using value_type = T;

    void fromArray(const std::array<T, N>& arr, size_t sz) { data_ = arr; size_ = sz; }

    const T& operator[](size_t index) const { return data_[index]; }
    T& operator[](size_t index) { return data_[index]; }

    const T& at(size_t index) const { return data_[index]; }
    T& at(size_t index) { return data_[index]; }

    const T& front() const { return *data_; }
    T& front() { return *data_; }

    const T& back() const { return data_[size_ - 1]; }
    T& back() { return data_[size_ - 1]; }

    T* begin() { return &data_[0]; }
    const T* begin() const { return &data_[0]; }

    T* end() { return begin() + size_; }
    const T* end() const { return begin() + size_; }

    void push_back(const T& value) { data_.at(size_++) = value; }
    void pop_back() { --size_; }

    void erase(T* iter, size_t amount = 1) {
        const auto curEnd = end();
        const auto nextIter = iter + amount;
        while (nextIter < curEnd) {
            *iter = *nextIter;
            ++iter;
            ++nextIter;
        }
        size_--;
    }

    size_t size() const { return size_; }

    void resize(size_t sz) { assert(sz <= N); size_ = sz; }

private:
    std::array<T, N> data_;
    size_t size_ = 0;
};
