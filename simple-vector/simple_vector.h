#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include "array_ptr.h"
#include <algorithm>
#include <iostream>
#include <numeric>


class ReserveProxyObj {
public:

    ReserveProxyObj() = default;

    ReserveProxyObj(size_t capacity_to_reserve)
        :reserved_(capacity_to_reserve)
    {

    }

    size_t reserved_ = 0;

};

ReserveProxyObj Reserve(size_t new_capacity) {
    return ReserveProxyObj(new_capacity);
}


template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        :size_(size), array_(ArrayPtr<Type>(size)), capacity_(size)
    {
        std::generate(begin(), end(), []() {return Type{};});
    }

    SimpleVector(const SimpleVector& other) {
        this->Reserve(other.GetCapacity());
        std::copy(other.begin(), other.end(), begin());
        size_ = other.GetSize();
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this == rhs) {
            return *this;
        }
        SimpleVector<Type> temp(rhs);
        this->swap(temp);
        return *this;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        :size_(size), array_(ArrayPtr<Type>(size)), capacity_(size)
    {
        std::fill(this->begin(), this->end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), array_(ArrayPtr<Type>(init.size())), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), this->begin());
    }

    SimpleVector(ReserveProxyObj obj)
        :array_(ArrayPtr<Type>(obj.reserved_)), capacity_(obj.reserved_)
    {
    }

    SimpleVector(SimpleVector&& other)
    {
        this->swap(other);
        other.Clear();
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        return array_[index];
    }


    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return array_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {

        if (new_size <= size_) {
            size_ = new_size;
            return;
        }

        if (new_size <= capacity_) {
            std::generate(end(), (begin() + new_size), []() {return Type{};});
            size_ = new_size;
            return;
        }

        Reserve(new_size * 2);
        std::generate(end(), begin() + new_size, []() {return Type{};});
        size_ = new_size;

    }

    Iterator begin() noexcept {
        return array_.Get();
    }

    Iterator end() noexcept {
        return (array_.Get() + size_);
    }

    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    ConstIterator end() const noexcept {
        return (array_.Get() + size_);
    }

    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    ConstIterator cend() const noexcept {
        return (array_.Get() + size_);
    }

    void PushBack(const Type& item) {
        PushBackProcess(item);
    }

    void PushBack(Type&& item) {
        PushBackProcess(item);
    }

    void PopBack() noexcept {
        if (size_ == 0) {
            return;
        }
        Resize(size_ - 1);
    }

    void swap(SimpleVector& other) noexcept {
        this->array_.swap(other.array_);
        std::swap(this->capacity_, other.capacity_);
        std::swap(this->size_, other.size_);
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        return InsertProcess(pos, value);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        return InsertProcess(pos, value);
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        if (!(pos <= cend()) && !(pos >= cbegin())) {
            throw std::out_of_range("Iterator is out if range");
        }
        assert(size_ != 0);
        std::move(const_cast<Iterator>(pos + 1), end(), const_cast<Iterator>(pos));
        Resize(size_ - 1);
        return const_cast<Iterator>(pos);
    }

    void Reserve(size_t new_capacity) {

        if (capacity_ < new_capacity) {
            ArrayPtr<Type> temp(new_capacity);
            std::move(begin(), end(), temp.Get());
            array_.swap(temp);
            capacity_ = new_capacity;
        }
    }


private:
    size_t size_ = {};
    ArrayPtr<Type> array_ = {};
    size_t capacity_ = {};

    Iterator InsertProcess(ConstIterator pos, Type& value) {

        if (!(pos <= cend()) && !(pos >= cbegin())) {
            throw std::out_of_range("Iterator is out if range");
        }

        auto dist = std::distance(cbegin(), pos);

        if (size_ == 0 && size_ >= capacity_) {
            Resize(1);
            array_[0] = std::move(value);
            return &array_[0];
        }

        if (pos == end() && size_ >= capacity_) {
            Resize(size_ + 1);
            *(std::prev(end(), 1)) = std::move(value);
            return (std::prev(end(), 1));
        }

        if (size_ >= capacity_) {

            ArrayPtr<Type> temp_arr(size_ + 1);
            std::move_backward(const_cast<Iterator>(pos), end(), temp_arr.Get() + size_ + 1);

            std::move(begin(), const_cast<Iterator>(pos), temp_arr.Get());

            temp_arr[dist] = std::move(value);

            ++size_;
            ++capacity_;

            array_.swap(temp_arr);


        }
        else {
            std::move_backward(const_cast<Iterator>(pos), end(), end() + 1);
            *const_cast<Iterator>(pos) = std::move(value);
            ++size_;
        }

        return &array_[dist];

    }

    void PushBackProcess(Type& item) {
        size_t new_size = size_ + 1;
        if (size_ >= capacity_) {
            Reserve(std::max(1, static_cast<int>(capacity_ * 2)));
            array_[size_] = std::move(item);
            ++size_;
        }
        else {
            array_[size_] = std::move(item);
            ++size_;
        }
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

