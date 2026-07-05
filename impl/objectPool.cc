# include <stdexcept>
# include <tuple>

# include "objectPool.hh"
# include "macros.hh"

const int alignment = 64;

ObjectPool::ObjectPool(unsigned poolCapacity, size_t sizeObject, size_t sizeIndexBuffer, size_t sizeDataBuffer) {
    // adjusted size for objects
    size_t remainder = sizeObject % alignment;
    size_t adjustedObjectSize = sizeObject;
    if (remainder != 0) {
        adjustedObjectSize += (alignment - remainder);
    }
    objectSize = adjustedObjectSize;

    // adjusted size for buffer of index
    remainder = sizeIndexBuffer % alignment;
    size_t adjustedBufferSize = sizeIndexBuffer;
    if (remainder != 0) {
        adjustedBufferSize += (alignment - remainder);
    }
    indexBufferSize = adjustedBufferSize;

    // adjusted size for buffer of data
    remainder = sizeDataBuffer % alignment;
    adjustedBufferSize = sizeDataBuffer;
    if (remainder != 0) {
        adjustedBufferSize += (alignment - remainder);
    }
    dataBufferSize = adjustedBufferSize;
    
    
    // correct capacity
    poolSize = poolCapacity*(objectSize+indexBufferSize+dataBufferSize);
    
    // allocate 
    pool = static_cast<char*>(std::aligned_alloc(alignment, poolSize));
    ASSERT(pool);        // bad alloc
    // manage the list of free slots
    freeSlots.reserve(poolCapacity);
    for (unsigned i = 0; i < poolCapacity; i++) {
        freeSlots.push_back(pool + i*(objectSize+indexBufferSize+dataBufferSize));
    }
}

// return ptr to space for one object and the buffer
std::tuple<char*,int*, int*> ObjectPool::allocate() {
    ASSERT(!freeSlots.empty());             // pool capacity exceeded
    char* objPtr = freeSlots.back();
    int* indexBufPtr = reinterpret_cast<int*>(objPtr + objectSize);
    int* dataBufPtr = reinterpret_cast<int*>(objPtr + objectSize + indexBufferSize);
    freeSlots.pop_back();
    return {objPtr, indexBufPtr, dataBufPtr};
}