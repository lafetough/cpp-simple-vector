#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include "array_ptr.h"
#include <algorithm>
#include <iostream>



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
        this->move_fill(begin(), end(), Type{});
    }

    SimpleVector(const SimpleVector& other) {
        this->Resize(other.GetSize());
        std::copy(other.begin(), other.end(), this->begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
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
        :capacity_(obj.reserved_)
    {
    }

    SimpleVector(SimpleVector&& other)
    {
        Resize(other.GetSize());
        std::move(other.begin(), other.end(), this->begin());
        other.Clear();
    }


    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        //const Type ret = array_[index];
        return array_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    //template <typename ForwardIt, typename T>
    //void move_fill(ForwardIt first, ForwardIt last, const T& value) {
    //    while (first != last) {
    //        *first = value;
    //        ++first;
    //    }
    //}

    template <typename ForwardIt, typename T>
    void move_fill(ForwardIt first, ForwardIt last, T&& value) {
        while (first != last) {
            *first = std::move(value);
            ++first;
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {

        if (new_size <= size_) {
            size_ = new_size;
            return;
        }

        if (new_size <= capacity_) {
            move_fill(this->array_.Get() + size_, this->array_.Get() + new_size, Type{});
            size_ = new_size;
            return;
        }



        ArrayPtr<Type>temp_arr(new_size);
        std::move(this->begin(), this->end(), temp_arr.Get());

        /*for (auto it = temp_arr.Get() + size_; it != temp_arr.Get() + new_size; ++it) {

            std::swap(*it, Type{});
        }*/

        std::generate(temp_arr.Get() + size_, temp_arr.Get() + new_size, []() {return Type{};});
        //move_fill(temp_arr.Get() + size_, temp_arr.Get() + new_size, Type{});
        this->array_.swap(temp_arr);
        size_ = new_size;
        capacity_ = new_size;

    }


    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return (array_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return (array_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return (array_.Get() + size_);
    }

    void PushBack(const Type& item) {

        size_t new_size;
        if (size_ >= capacity_) {
            !size_ ? new_size = 1 : new_size = size_ + 1;
            Resize(new_size);
            array_[size_ - 1] = item;
        }
        else {
            array_[size_] = item;
            ++size_;
        }
    }

    void PushBack(Type&& item) {

        size_t new_size;
        if (size_ >= capacity_) {
            !size_ ? new_size = 1 : new_size = size_ + 1;
            Resize(new_size);
            std::swap(array_[size_ - 1], item);
        }
        else {
            std::swap(array_[size_], item);
            ++size_;
        }
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

        auto dist = std::distance(pos, cend());

        if (size_ == 0 && size_ >= capacity_) {
            Resize(1);
            array_[0] = value;
            return &array_[0];
        }

        if (pos == end() && size_ >= capacity_) {
            Resize(size_ + 1);
            *(end() - 1) = value;
            return (end() - 1);
        }

        if (size_ >= capacity_) {
            ArrayPtr<Type> temp_arr(dist);

            std::copy(const_cast<Iterator>(pos), end(), temp_arr.Get());

            array_[dist] = value;

            Resize(size_ + 1);

            std::copy_backward(temp_arr.Get(), temp_arr.Get() + dist, end());
        }
        else {
            std::copy_backward(const_cast<Iterator>(pos), end(), end() + 1);
            *const_cast<Iterator>(pos) = value;
            ++size_;
        }

        return &array_[dist];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {

        auto dist = std::distance(pos, cend());

        if (size_ == 0 && size_ >= capacity_) {
            Resize(1);
            std::swap(array_[0], value);
            return &array_[0];
        }

        if (pos == end() && size_ >= capacity_) {
            Resize(size_ + 1);
            std::swap(*(end() - 1), value);
            return (end() - 1);
        }

        if (size_ >= capacity_) {

            ArrayPtr<Type> temp_arr(dist);
            std::move(const_cast<Iterator>(pos), end(), temp_arr.Get());
            std::swap(array_[size_ - dist], value);
            Resize(size_ + 1);
            std::move_backward(temp_arr.Get(), temp_arr.Get() + dist, end());

        }
        else {
            std::move_backward(const_cast<Iterator>(pos), end(), end() + 1);
            std::swap(*const_cast<Iterator>(pos), value);
            ++size_;
        }

        return &array_[dist];
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(size_ != 0);
        std::move(const_cast<Iterator>(pos + 1), end(), const_cast<Iterator>(pos));
        Resize(size_ - 1);
        return const_cast<Iterator>(pos);
    }

    void Reserve(size_t new_capacity) {

        if (capacity_ < new_capacity) {
            size_t init_size = size_;
            Resize(new_capacity);
            size_ = init_size;
        }
    }


private:
    size_t size_ = {};
    ArrayPtr<Type> array_ = {};
    size_t capacity_ = {};

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

