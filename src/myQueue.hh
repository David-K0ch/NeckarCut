# ifndef MYQUEUE
# define MYQUEUE

# include <cstddef>
# include <vector>

# include "macros.hh"

class Queue {
private:
    size_t capacity;            // max number of items
    size_t count;               // number of items in the queue
    size_t start;               // place where the first object is
    size_t end;                 // place one after the last object
    std::vector<int> data;    // values

public:
    Queue() : capacity(0), count(0), start(0), end(0) {}

    void setCapacity(size_t neededSize) {
        size_t power = 1;
        while (power < neededSize) {
            power <<= 1;
        }
        neededSize = power;
        ASSERT(neededSize > capacity);
        data.resize(neededSize);
        capacity = neededSize;
    }
    bool empty() {
        return count==0;
    }
    void clear() {
        count = 0;
        end = start;
    }
    size_t size() {
        return count;
    }

    int& back() {
        ASSERT(count > 0);
        return data[(end+capacity-1) & (capacity-1)];
    }
    int& front() {
        ASSERT(count > 0);
        return data[start];
    }
    void push_back(int value) {
        ASSERT(count < capacity);
        data[end] = value;
        end = (end+1) & (capacity-1);
        count++;
    }
    void push_front(int value) {
        ASSERT(count < capacity);
        start = (start+capacity-1) & (capacity-1);
        data[start] = value;
        count++;
    }
    void pop_back() {
        ASSERT(count > 0);
        end = (end+capacity-1) & (capacity-1);
        count--;
    }
    void pop_front() {
        ASSERT(count > 0);
        start = (start+1) & (capacity-1);
        count--;
    }
};

# endif