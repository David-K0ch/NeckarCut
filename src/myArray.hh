#ifndef MYARRAY_HH
#define MYARRAY_HH

# include <cstddef>
# include <stdexcept>
# include <algorithm>


# include "objectPool.hh"
# include "macros.hh"


// only for int & objectPool allows only objects of same and prev given size
class Array {
private:
    ObjectPool* pool;
    int* elements;                      // points to first free address
    size_t capacity;                    // number of ints that can be stored
    size_t currentSize;                 // number of stored ints

public:
    // constructors
    Array(ObjectPool* objPool, int* buffer, int capacityArray) : pool(objPool), elements(buffer), capacity(capacityArray), currentSize(0) {}
    Array(ObjectPool* objPool, int* buffer, const Array& other) : pool(objPool), elements(buffer), capacity(other.capacity), currentSize(other.currentSize) {std::copy(other.elements, other.elements + currentSize, this->elements);}
    Array(ObjectPool* objPool, int* buffer, const Array& other, bool emptyInit) : pool(objPool), elements(buffer), capacity(other.capacity), currentSize(other.currentSize) {
        if (emptyInit) {
            std::fill(this->elements, this->elements + currentSize, -1);
        } else {
            std::copy(other.elements, other.elements + currentSize, this->elements);
        }
    }
    ~Array() {}

    inline size_t size() const {return currentSize;}
    inline bool empty() const {return currentSize == 0;}

    inline void setBuffer(int* newBuffer) {
        ASSERT(currentSize == 0);
        elements = newBuffer;
    }
    inline void push_back(const int &value) {
        ASSERT(currentSize < capacity);        // array capacity exceeded
        *(elements+currentSize) = value;
        currentSize++;
    }
    inline void pop_back() {
        ASSERT(currentSize > 0);        // no object in array
        currentSize--;
    }
    inline int& back() {
        ASSERT(currentSize > 0);        // no object in array
        return *(elements+currentSize-1);
    }

    inline int& operator[](size_t i) {
        ASSERT(currentSize > i);
        ASSERT(currentSize > i && i >= 0);        // index out of range
        return *(elements+i);
    }

};

#endif