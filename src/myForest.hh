#ifndef MYFOREST_HH
#define MYFOREST_HH

# include <vector>
# include <array>
# include <string>
# include <utility>

# include "myTree.hh"
# include "objectPool.hh"

struct SiblingPairs {
    std::vector<int> pairs;
    std::vector<int> treeOffsets; // stores the starting index in pairs for each tree, ending with pairs.size()
};

class Forest {
private:
    ObjectPool* pool;
    std::vector<Tree*> trees;
public:
    // labels of current sibling pair
    int sib1;
    int sib2;

    Forest();
    Forest(Tree* tree);
    Forest(ObjectPool* pool, const Forest &otherForest);
    Forest(ObjectPool* pool, std::string &input);
    ~Forest();

    void deepCopy(const Forest&otherForest);
    Forest& operator=(const Forest &otherForest);
    inline void setPool(ObjectPool* objPool) {pool = objPool;}
    void removeTree(unsigned index);            // remove tree and release it from ObjectPool
    
    inline unsigned numberTrees() const {return trees.size();}
    inline Tree* getTree(unsigned i) const {return trees[i];}
    void setTree(unsigned i, Tree* tree);
    void addTree(Tree* tree);
    void clear();

    const std::vector<int>& getPath(int label1, int label2);       // returns all nodes on path between nodes with label1 and label2 (exclusive end nodes);
    int findComponent(int label) const;
    void findSiblings();
    SiblingPairs findAllSiblings(bool notShrunken) const;       // returns all sibling pairs (if notShrunken=true: all labels > inputLeafs are viewed as inner nodes)
    void clearInnerNodes(unsigned numLabels);

    bool isSibling(unsigned label1, unsigned label2) const;
    bool isSameTree(unsigned label1, unsigned label2) const;

    int calcLabelDistance(int label1, int label2) const;

    void cut(unsigned label, std::vector<int>* singleNodes);
    void cutPath(unsigned label1, unsigned label2, std::vector<int>* singleNodes);
    void shrink(unsigned labelChild, unsigned newLabel);
    void cutSingles(const std::vector<int>& singleLabels);

    std::string output(unsigned numLabels, std::vector<int>& singleNodes) const;
    void replaceOutput(std::string& out) const;
};

#endif