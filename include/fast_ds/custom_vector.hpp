#pragma once

#include <cstddef>
#include <utility> // required for std::move, std::forward, and std::move_if_noexcept
#include <new> // required for ::operator new and placement new
#include <stdexcept> // required for std::out_of_range
#include <type_traits> // required for std::is_nothrow_move_constructible
#include <iostream> // required for std::ostream


namespace fast_ds
{
    // template to create class CustomVector that can hold any data type T
    template <typename T, bool EnableTrace = false>
    class CustomVector
    {
    public:
        // creates default constructor with private data members data_, size_, and capacity_ initialized to nullptr, 0, and 0 respectively
        CustomVector() {
            trace("INIT", "Created empty vector (size: 0, capacity: 0)");
        }

        // 1. Destructor
        ~CustomVector()
        {
            trace("DESTROY", "Destroying vector and freeing memory raw heap block");
            // clear old elements and free raw memory
            clear();
            ::operator delete(data_); // Free the allocated memory without calling destructors
        }

        // 2. Copy Constructor (Deep Copy)
        CustomVector(const CustomVector &other)
        {
            trace("COPY_CTOR", "Allocating fresh buffer to deep-copy " + std::to_string(other.size_) + "elements");

            if (other.size_ > 0) {
                data_ = static_cast<T*>(::operator new(other.size_ * sizeof(T))); // Allocate raw memory for the new vector
                capacity_ = other.size_; // Set capacity to the size of the other vector
                for (std::size_t i = 0; i < other.size_; ++i) {
                    new (data_ + i) T(other.data_[i]); // Placement new to copy construct each element
                    ++size_; // Increment size after constructing each element
                }
            }

            print_memory_bar();
        }

        // 3. Copy Assignment (copy-and-swap idiom)
        CustomVector& operator=(CustomVector& other) // Pass by value to leverage copy constructor
        {
            trace("COPY_ASSIGN", "Executing copy-and-swap assignment");
            if (this != &other) { // Check for self-assignment
                CustomVector temp(other); // Create a temporary copy of the other vector
                swap(temp); // Swap the contents of this vector with the temporary copy
            }
            return *this; // Return the current object
        }

        // 4. Move Constructor (0(1) pointer steal)
        CustomVector(CustomVector &&other) noexcept
            : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
        {
            other.data_ = nullptr; // Leave the moved-from object in a valid state
            other.size_ = 0;
            other.capacity_ = 0;
            trace("MOVE_CTOR", "Stole heap buffer in 0(1) time, (source reset to nulptr)");
            print_memory_bar();
        }

        // 5. Move Assignment (0(1) pointer steal)
        CustomVector& operator=(CustomVector&& other) noexcept
        {
            trace("MOVE_ASSIGN", "Cleaning current buffer and stealing source pointer");
            if (this != &other) { // Check for self-assignment
                clear(); // Clear current elements and free memory
                ::operator delete(data_); // Free the old raw memory

                // Steal the resources from the other vector
                data_ = other.data_;
                size_ = other.size_;
                capacity_ = other.capacity_;

                // Leave the moved-from object in a valid state
                other.data_ = nullptr;
                other.size_ = 0;
                other.capacity_ = 0;
                print_memory_bar();
            }
            return *this; // Return the current object
        }

        void swap(CustomVector& other) noexcept
        {
            std::swap(data_, other.data_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
        }

        // --- Core Modifiers ---
        template <typename... Args>
        T& emplace_back(Args &&...args) {
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
            trace("EMPLACE", "Constructed new element at index [" + std::to_string(size_ - 1) + "]");
            print_memory_bar();
            return data_[size_ - 1];
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
                trace("Pop", "Destroyed element at index [" + std::to_string(size_) + "]");
                print_memory_bar();
            }
        }

        void clear() noexcept
        {
            // Call destructors for all elements
            for (std::size_t i = 0; i < size_; ++i) {
                data_[i].~T();
            }
            size_ = 0;
            trace("CLEAR", "Cleared all elements");
            print_memory_bar();
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
            if constexpr (EnableTrace) {
                std::cout
                    << "\n   [REALLOCATE] Capacity Full (" << capacity_
                    << "), reallocating to new capacity: " << new_capacity << "\n"
                    << "    |-- 1. ::operator new(" << new_capacity * sizeof(T)
                    << ") bytes on the heap for uninitialized elements of type T\n"
                    << "    |-- 2. Placement-move " << size_
                    << " existing elements into the new block\n"
                    << "    |-- 3. Destroy the old elements & ::operator delete old block\n";
            }
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

        void trace(const std::string &action, const std::string &message) const
        {
            if constexpr (EnableTrace) {
                std::cout << "[" << action << "] : " << message << "\n";
            }
        }

        void print_memory_bar() const
        {
            if constexpr (EnableTrace) {
                std::cout << "    Heap Buffer (" << static_cast<const void*>(data_) << "): ";
                for (std::size_t i = 0; i < capacity_; ++i) {
                    if (i < size_) {
                        if constexpr (std::is_arithmetic_v<T>) {
                            std::cout << "[" << data_[i] << "]"; // Occupied slot with value
                        } else {
                            std::cout << "[#]"; // Occupied slot with non-arithmetic type
                        }
                    } else {
                        std::cout << "[_]"; // Unoccupied slot
                    }
                }
                std::cout << " (size: " << size_ << ", capacity: " << capacity_ << ")\n";
            }
        }

        T *data_ = nullptr;
        std::size_t size_ = 0;
        std::size_t capacity_ = 0;
    };

} // namespace fast_ds
