#pragma once

#include <cstddef>
#include <utility> // required for std::move, std::forward, and std::move_if_noexcept
#include <new> // required for ::operator new and placement new
#include <stdexcept> // required for std::out_of_range


namespace fast_ds
{
    // template to create class CustomVector that can hold any data type T
    template <typename T>
    class CustomVector
    {
    public:
        // creates default constructor with private data members data_, size_, and capacity_ initialized to nullptr, 0, and 0 respectively
        CustomVector() = default;
        ~CustomVector()
        {
            // clear old elements and free raw memory
            clear();
            ::operator delete(data_); // Free the allocated memory without calling destructors
        }

        // --- Core Modifiers ---
        template <typename... Args>
        void emplace_back(Args &&...args) {
            if (size_ >= capacity_) {
                //Implement growth logic
                std::size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
                reallocate(new_capacity);
            }

            // Placement new: constructs T directly at memory address (data_ + size_)
            // perfectly forward the arguments to the constructor of T
            // instead of asking os for new memory, we are using the memory we already have allocated for the vector
            new (data_ + size_) T(std::forward<Args>(args)...);
            ++size_; // Increment size after constructing the new element
        }
        void push_back(const T &value)
        {
            //Implement growth logic and placement new
            emplace_back(value); // Use emplace_back to handle the logic
        }

        void push_back(T &&value)
        {
            //Implement move semantics
            emplace_back(std::move(value)); // Use emplace_back to handle the logic
        }

        void pop_back()
        {
            if (size_ > 0) {
                // Call the destructor of the last element
                --size_;
                data_[size_].~T();
            }
        }

        void clear() noexcept
        {
            // Call destructors for all elements
            for (std::size_t i = 0; i < size_; ++i) {
                data_[i].~T();
            }
            size_ = 0;
        }

        void reserve(std::size_t new_capacity)
        {
            if (new_capacity > capacity_) {
                reallocate(new_capacity);
            }
        }

        // --- Element Access ---
        T& operator[](std::size_t index) noexcept
        {
            return data_[index];
        }

        const T& operator[](std::size_t index) const noexcept
        {
            return data_[index];
        }

        T& at(std::size_t index)
        {
            if (index >= size_) {
                throw std::out_of_range("CustomVector::at: index out of range");
            }
            return data_[index];
        }

        const T& at(std::size_t index) const
        {
            if (index >= size_) {
                throw std::out_of_range("CustomVector::at: index out of range");
            }
            return data_[index];
        }

        [[nodiscard]] std::size_t size() const noexcept { return size_; }
        [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    
    // T* *data is a pointer to memory on the heap, std::size_t size is the number of elements in the vector, std::size_t capacity is the total number of elements that can be stored in the vector without reallocating memory
    private:
        void reallocate(std::size_t new_capacity)
        {
            // Allocate raw, uninitialized bytes on the heap for new_capacity elements of type T without calling T's constructor
            T* new_data = static_cast<T*>(::operator new(new_capacity * sizeof(T)));

            // Move or copy existing elements into the uninitialized buffer
            for (std::size_t i = 0; i < size_; ++i) {
                new (new_data + i) T(std::move_if_noexcept(data_[i])); 
            }

            // Destroy the old elements in the original buffer and free the old raw buffer
            // manual creation of objects with placement new requires manual destruction of objects and freeing of raw memory
            for (std::size_t i = 0; i < size_; ++i) {
                data_[i].~T(); // Call the destructor for each element
            }
            ::operator delete(data_); // Free the old raw memory

            // Update pointers and capacity to the new buffer
            data_ = new_data;
            capacity_ = new_capacity;
        }
        T *data_ = nullptr;
        std::size_t size_ = 0;
        std::size_t capacity_ = 0;
    };

} // namespace fast_ds
