#ifndef OBJECTPOOL_HH
#define OBJECTPOOL_HH

# include <cstdlib>
# include <cstddef>
# include <cstdint>  
# include <vector>
# include <tuple>

// speichert eine List von noch freien Speicherplätzen (feste Größe)
// kann diesen Speicherplatz ausgeben oder wieder als frei markieren

// sorgt bei allokation von Speicher für automatische alignness


class ObjectPool {
private:
    size_t poolSize;                    // number of Bytes the pool has allocated
    size_t objectSize;                  // size of the objects that will be stored (Trees)
    size_t indexBufferSize;             // size of the index buffer for lookup
    size_t dataBufferSize;              // size of the data buffer representing the tree
    std::vector<char*> freeSlots;       // list of free Slots for objects
    char* pool;                         // pointer to Start of the pool

public:
    ObjectPool(unsigned poolCapacity, size_t sizeObject,size_t sizeIndexBuffer, size_t sizeDataBuffer);      // poolCapacity in number of objects, sizeObject and sizeBuffer in number of Bytes
    ~ObjectPool() {std::free(pool);}

    std::tuple<char*,int*, int*> allocate();
    inline void release(char* obj) {freeSlots.push_back(obj);} // assumes obj is start of the object
};

#endif