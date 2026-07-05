#ifndef MYTREE_HH
#define MYTREE_HH

# include <vector>
# include <set>
# include <string>
# include <utility>

# include "global.hh"
# include "myArray.hh"
# include "objectPool.hh"

class Tree {
private:
    ObjectPool* pool;
    bool onlyInputLeafs;
    int references;             // counts the number of pointers to this tree
    Array index;                // array with index to leafs (position i stores position of leaf with label i+1 in data Array)
    Array data;                 // root is always the first entry
public:
    Tree(ObjectPool* objPool);
    Tree(ObjectPool* objPool, int* indexBuffer, int* dataBuffer);
    Tree(ObjectPool* objPool, int* indexBuffer, int* dataBuffer, Tree* other, bool isSubtreeEmptyIndex = false);
    ~Tree();

    inline ObjectPool* getPool() {return pool;}
    inline void incrementReferences() {references++;}
    void decrementReferences();

    // operations on data
    inline int getLabel(unsigned i) {return data[i*INFORMATION_COUNT];}
    void setLabel(unsigned i, int label);
    inline unsigned getParent(unsigned i) {return data[i*INFORMATION_COUNT+1];}
    inline void setParent(unsigned i, int parent) {data[i*INFORMATION_COUNT+1] = parent;}
    inline unsigned getChild1(unsigned i) {return data[i*INFORMATION_COUNT+2];}
    inline void setChild1(unsigned i, int child) {data[i*INFORMATION_COUNT+2] = child;}
    inline unsigned getChild2(unsigned i) {return data[i*INFORMATION_COUNT+3];}
    inline void setChild2(unsigned i, int child) {data[i*INFORMATION_COUNT+3] = child;}
    int getSibling(unsigned i);
    inline int findLabel(int label) {           // replace with getPosition?
        ASSERT(label > 0);           // invalid label
        return getPosition(label);
    }

    // operations on index
    inline int getPosition(int label) {
        ASSERT(label > 0);
        return index[label-1];                              // index shifted by 1 because: 0 is not valid label, labels start at 1
    }
    inline void setPosition(int label, int newPosition) {
        ASSERT(label > 0);
        index[label-1] = newPosition;                       // index shifted by 1 because: 0 is not valid label, labels start at 1
    }

    const std::vector<int>& getPath(int label1, int label2);       // returns all nodes on path between nodes with label1 and label2 (exclusive end nodes);
    int findSiblingPair();
    unsigned getNumberLeafs();
    unsigned makeNode(int label, int parent, int child1, int child2);
    Tree* clearInnerNodes(unsigned numLabels);                   // returns new Tree with set all inner nodes label to 0

    void contractEdges(unsigned i);
    Tree* shrink(unsigned i, unsigned newLabel);                // takes parent of both siblings as input and the label of the new laef in a new Tree, returns this tree
    std::pair<bool,std::pair<Tree*,Tree*>> cut(unsigned i, std::vector<int>* singleNodes);                                      // cuts the tree above the given node and returns the subtree
    std::pair<bool,Tree*> changingCut(unsigned i, std::vector<int>* singleNodes);
    std::vector<Tree*> cutPath(unsigned label1, unsigned label2, std::vector<int>* singleNodes);    // make label1, label2 siblings by cutting and contracting everything on the way
    Tree* cutSingles(const std::vector<int>& singleLabels);        // cut all labels/single nodes in the vector and return remaining Tree

    void readInput(std::string &line);       // give the function a single line from the file (it has to be chosen somewhere else)
    std::string output(unsigned numLabels);
};

#endif