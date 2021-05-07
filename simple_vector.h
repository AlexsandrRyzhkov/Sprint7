#pragma once
#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <iterator>

class ReserveProxyObj {
public:
    ReserveProxyObj() = default;
    
    ReserveProxyObj(size_t capacity_to_reserve)
    : new_capacity_(capacity_to_reserve) {
    }
    
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return new_capacity_;
    }
    
private:
    size_t new_capacity_= 0;
};

//template <typename Type>
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_(size), capacity_(size), items_(size) {
        std::fill(begin(), end(), Type());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : size_(size), capacity_(size), items_(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size()), items_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }
    
    SimpleVector(const SimpleVector& other) 
        : size_(other.GetSize()), capacity_(other.GetCapacity()), items_(other.GetSize()) {
            std::copy(other.begin(), other.end(), begin());   
    }
   
  SimpleVector(SimpleVector&& other)
  : size_(other.GetSize()),
    capacity_(other.GetCapacity()),
    items_(other.GetSize()) 
    {
    std::move(other.begin(), other.end(), &items_[0]);
    other.size_ = 0;
    other.capacity_ = 0;
    }
   
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }
    
    SimpleVector(ReserveProxyObj buff) 
       : capacity_(buff.GetCapacity()), items_(buff.GetCapacity()) {
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
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is too much");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is too much");
        }
        return items_[index];
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if(this != &rhs) {
           auto temp(rhs);
           swap(temp);
        }
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& other) {
        if(this != &other) {
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            std::move(other.begin(), other.end(), &items_[0]);
        }
        return *this;
    }
    
    void PushBack(const Type& item) {
       if (size_ >= capacity_) {
            if (capacity_ == 0) ++capacity_;
            ArrayPtr<Type> new_items(capacity_ * 2);
            std::copy(begin(), end(), new_items.Get());
            new_items[size_] = item;
            items_.swap(new_items);
            ++size_;
            capacity_ *= 2;
        } else {
            items_[size_] = item;
            ++size_;
        }
    }
    
    void PushBack(Type&& item) {
       if (size_ >= capacity_) {
            if (capacity_ == 0) ++capacity_;
            ArrayPtr<Type> new_items(capacity_ * 2);
            std::move(begin(), end(), new_items.Get());
            new_items[size_] = std::move(item);
            items_.swap(new_items);
            capacity_ *= 2;
        } else {
            items_[size_] = std::move(item);
           }
        ++size_;
    }
    
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t x = pos - begin();
        if (size_ < capacity_) {
            std::copy_backward(pos, cend(), &items_[size_]);
            items_[x] = value;
        } else {
            if(capacity_ == 0) ++capacity_;
            ArrayPtr<Type> new_items(2 * capacity_);
            std::copy(cbegin(), pos, new_items.Get());
            new_items[x] = value;
            std::copy_backward(pos, cend(), &new_items.Get()[size_ + 1]);
            items_.swap(new_items);
            capacity_ *= 2;
        }
            ++size_;
            return &items_[x];
    }

Iterator Insert(ConstIterator pos, Type&& value) {
        size_t x = pos - begin();
        Iterator mutable_pos = begin() + (pos - cbegin());
        if (size_ < capacity_) {
            std::move_backward(mutable_pos, end(), &items_[size_]);
            items_[x] = std::move(value);
        } else {
            if(capacity_ == 0) ++capacity_;
            ArrayPtr<Type> new_items(2 * capacity_);
            std::move(begin(), mutable_pos, new_items.Get());
            new_items.Get()[x] = std::move(value);
            std::move_backward(mutable_pos, end(), &new_items.Get()[size_ + 1]);
            items_.swap(new_items);
            capacity_ *= 2;
        }
            ++size_;
            return &items_[x];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_ ;
    }

    // Удаляет элемент вектора в указанной позиции
     Iterator Erase(ConstIterator pos) {
        size_t x = pos - begin();
        Iterator mutable_pos = begin() + (pos - cbegin());
        ArrayPtr<Type> new_items(size_);
        std::move(mutable_pos, end(), new_items.Get());
        std::move(&new_items.Get()[1], &new_items.Get()[size_], &items_[x]);
        --size_;
        return &items_[x];
    }
    
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (size_ > new_size) {
            size_ = new_size;
        }
        if (new_size <= capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                 items_[i] = Type();
            }
        }
        else {
            //new_size = std::max(new_size, 2 * capacity_);
            SimpleVector<Type> new_items(std::max(new_size, 2 * capacity_));
            std::move(begin(), end(), new_items.begin());
            std::fill(new_items.begin()+size_, new_items.end(), Type());
            items_.swap(new_items.items_);
            capacity_ = new_items.GetCapacity();
            size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &items_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return &items_[size_];
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(rhs.begin(), rhs.end(),
                                        lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(),
                                        lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end()));
}
