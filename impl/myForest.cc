# include <vector>
# include <string>
# include <iostream>
# include <stdexcept>
# include <algorithm>
# include <tuple>
# include <set>

# include "myForest.hh"
# include "myTree.hh"
# include "objectPool.hh"
# include "macros.hh"
# include "myQueue.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
Forest::Forest() {}

Forest::Forest(Tree* tree) {
    trees.clear();
    trees.push_back(tree);
    tree->incrementReferences();
    pool = tree->getPool();
}

Forest::Forest(ObjectPool* objPool, const Forest &otherForest) : pool(objPool), trees(otherForest.trees) {
    for (Tree* tree : trees) {
        tree->incrementReferences();
    }
}

Forest& Forest::operator=(const Forest& otherForest) {
    if (this != &otherForest) {
        for (Tree* tree : trees) {
            tree->decrementReferences();
        }
        pool = otherForest.pool;
        trees = otherForest.trees;
        for (Tree* tree : trees) {
            tree->incrementReferences();
        }
        sib1 = otherForest.sib1;
        sib2 = otherForest.sib2;
    }
    return *this;
}

Forest::Forest(ObjectPool* objPool, std::string &input) : pool(objPool) {
    ASSERT(trees.size() == 0);
    std::tuple<char*,int*,int*> mem = pool->allocate();
    Tree* firstTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem));
    firstTree->readInput(input);
    addTree(firstTree);
}

Forest::~Forest() {
    for (Tree* tree : trees) {
        tree->decrementReferences();
    }
    trees.clear();
}


void Forest::deepCopy(const Forest& otherForest) {
    // release all current Trees
    for (Tree* tree : trees) {
        tree->decrementReferences();
    }
    trees.clear();
    // copy new Trees
    for (Tree* tree : otherForest.trees) {
        std::tuple<char*,int*,int*> mem = pool->allocate();
        Tree* newTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem),tree); 
        addTree(newTree);
    }
}

void Forest::removeTree(unsigned index) {
    Tree* t = trees[index];
    t->decrementReferences();
    trees.erase(trees.begin() + index);
}

void Forest::setTree(unsigned i, Tree* tree) {
    trees[i]->decrementReferences();
    trees[i] = tree;
    tree->incrementReferences();
}

void Forest::addTree(Tree* tree) {
    trees.push_back(tree);
    tree->incrementReferences();
}

void Forest::clear() {
    for (Tree* tree : trees) {
        tree->decrementReferences();
    }
    trees.clear();
}


const std::vector<int>& Forest::getPath(int label1, int label2) {       // returns all nodes on path between nodes with label1 and label2 (exclusive end nodes);
    int comp1 = findComponent(label1);
    int comp2 = findComponent(label2);
    if ((comp1 != comp2) || (comp1 == -1)) {
        static const std::vector<int> empty_vector;
        return empty_vector;
    }
    return trees[comp1]->getPath(label1,label2);
}

int Forest::findComponent(int label) const {
    ASSERT(label > 0);       // invalid label
    for (int i=0 ; i < trees.size(); i++) {
        if (trees[i]->findLabel(label) != -1) {
            return i;
        }
    }
    return -1;              // label not found (e.g. the label is a single node and therefore has no tree)
}

void Forest::findSiblings() {
    for (int i=0 ; i < trees.size(); i++) {
        int parent = trees[i]->findSiblingPair();
        if (parent != -1) {
            sib1 = trees[i]->getLabel(trees[i]->getChild1(parent));
            sib2 = trees[i]->getLabel(trees[i]->getChild2(parent));
            return;
        }
    }
    // no sibling found
    sib1 = -1;
    sib2 = -1;
}

SiblingPairs Forest::findAllSiblings(bool notShrunken) const {
    SiblingPairs result;
    result.pairs.reserve(128);
    result.treeOffsets.reserve(16);

    for (Tree* t : trees) {
        result.treeOffsets.push_back(result.pairs.size());
        size_t startSize = result.pairs.size();
        
        ASSERT(queue.empty());
        // DFS like
        queue.push_back(0);
        while (queue.size() != 0) {
            int current = queue.back();
            queue.pop_back();
            // inner node
            if (t->getLabel(current) == 0 || ((notShrunken) && (t->getLabel(current) > inputLeafs))) {
                queue.push_back(t->getChild1(current));
                queue.push_back(t->getChild2(current));
            } else {
                // no sibling --> single tree
                if (t->getSibling(current) == -1) {
                    ASSERT(queue.empty());
                    continue;
                }
                // pair found
                int label1 = t->getLabel(current);
                int label2 = t->getLabel(t->getSibling(current));
                ASSERT((label1 > 0) && (label1 <= inputLeafs));
                if ((label2 > 0) && (label2 <= inputLeafs)) {
                    result.pairs.push_back(label1);
                    result.pairs.push_back(label2);
                    ASSERT(t->getLabel(queue.back()) == label2);
                    queue.pop_back();
                }
            }
        }
        queue.clear();
        
        // If we didn't add any pairs for this tree, remove the offset
        if (result.pairs.size() == startSize) {
            result.treeOffsets.pop_back();
        }
    }
    result.treeOffsets.push_back(result.pairs.size());
    return result;
}

void Forest::clearInnerNodes(unsigned numLabels) {
    for (int i=0; i<trees.size(); i++) {
        Tree* clearedTree = trees[i]->clearInnerNodes(numLabels);
        setTree(i,clearedTree);
    }
}

bool Forest::isSibling(unsigned label1, unsigned label2) const {
    ASSERT(label1 > 0 && label2 > 0);       // invalid label
    for (Tree* tree : trees) {
        int index = tree->findLabel(label1);
        if (index >= 0) {
            return ((tree->getLabel(tree->getChild1(tree->getParent(index))) == label2) || (tree->getLabel(tree->getChild2(tree->getParent(index))) == label2));
        }
    }
    ASSERT(false);      // label1 not found
    return false;
}

bool Forest::isSameTree(unsigned label1, unsigned label2) const {
    ASSERT(label1 > 0 && label2 > 0);       // invalid label
    for (Tree* tree : trees) {
        int index = tree->findLabel(label1);
        if (index >= 0) {
            return (tree->findLabel(label2) != -1);
        }
    }
    return false;       // label1 is single node and therefore has no tree
}

int Forest::calcLabelDistance(int label1, int label2) const {
    // distance equals number of nodes on shortest path between the two nodes
    // if they are in different trees, add the number of nodes up to root in both trees
    int comp1 = findComponent(label1);
    int comp2 = findComponent(label2);
    
    if (comp1 == comp2) {                   // both are in same conected component, or both single nodes
        if (comp1 == -1) {                  // both are single nodes
            return 0;
        }
        int p1 = trees[comp1]->getParent(trees[comp1]->findLabel(label1));
        ASSERT(p1 != -1);          // label1 is alone in tree
        int original_depth1 = 0;
        int temp = p1;
        while (temp != -1) {
            original_depth1++;
            temp = trees[comp1]->getParent(temp);
        }

        int p2 = trees[comp1]->getParent(trees[comp1]->findLabel(label2));
        int original_depth2 = 0;
        temp = p2;
        while (temp != -1) {
            original_depth2++;
            temp = trees[comp1]->getParent(temp);
        }

        // Align depths to find LCA
        int depth1 = original_depth1;
        int depth2 = original_depth2;
        int curr_p1 = p1;
        int curr_p2 = p2;
        while (depth1 > depth2) {
            curr_p1 = trees[comp1]->getParent(curr_p1);
            depth1--;
        }
        while (depth2 > depth1) {
            curr_p2 = trees[comp1]->getParent(curr_p2);
            depth2--;
        }
        while (curr_p1 != curr_p2) {
            curr_p1 = trees[comp1]->getParent(curr_p1);
            curr_p2 = trees[comp1]->getParent(curr_p2);
            depth1--;
        }
        return original_depth1 + original_depth2 - 2 * depth1;
    } else {                                // not in same component
        int dist = 0;
        int current = -1;
        if (comp1 != -1) {
            current = trees[comp1]->getParent(trees[comp1]->findLabel(label1));
        }
        while (current != -1) {
            dist++;
            current = trees[comp1]->getParent(current);
        }
        if (comp2 != -1) {
            current = trees[comp2]->getParent(trees[comp2]->findLabel(label2));
        }
        while (current != -1) {
            dist++;
            current = trees[comp2]->getParent(current);
        }
        return dist;
    }
}


void Forest::cut(unsigned label, std::vector<int>* singleNodes) {
    ASSERT(label > 0);      // invalid label
    int comp = findComponent(label);
    if (comp == -1) {       // label is already single node
        return;
    }
    unsigned node = trees[comp]->findLabel(label);
    std::pair<bool,std::pair<Tree*,Tree*>> cutResult = trees[comp]->cut(node, singleNodes);
    if (cutResult.first) {
        // setTree(comp,cutResult.second.first);
        // if (cutResult.second.second != nullptr) {
        //     setTree(comp,cutResult.second.second);
        // }
        if (cutResult.second.first != nullptr) {
            setTree(comp,cutResult.second.first);
            if (cutResult.second.second != nullptr) {
                addTree(cutResult.second.second);
            }
        } else {
            if (cutResult.second.second != nullptr) {
                setTree(comp,cutResult.second.second);
            } else {
                removeTree(comp);
            }
        }
    }
}

void Forest::shrink(unsigned labelChild, unsigned newLabel) {
    ASSERT(labelChild > 0);         // invalid label
    unsigned comp = findComponent(labelChild);
    ASSERT(comp >= 0);
    Tree* shrunkenTree = trees[comp]->shrink(trees[comp]->getParent(trees[comp]->findLabel(labelChild)),newLabel);
    setTree(comp,shrunkenTree);
}

void Forest::cutPath(unsigned label1, unsigned label2, std::vector<int>* singleNodes) {
    ASSERT(label1 > 0 && label2 > 0);       // invalid label
    int comp = findComponent(label1);
    ASSERT(comp >= 0);
    std::vector<Tree*> newTrees = trees[comp]->cutPath(label1, label2, singleNodes);
    ASSERT(newTrees.size() > 0);
    for (int i=0; i<newTrees.size(); i++) {
        if (i==0) {
            setTree(comp,newTrees[i]);
        } else {
            addTree(newTrees[i]);
        }
    }
}

void Forest::cutSingles(const std::vector<int>& singleLabels) {
    ASSERT(trees.size() == 1);
    Tree* remTree = trees[0]->cutSingles(singleLabels);
    setTree(0,remTree);
}

std::string Forest::output(unsigned numLabels, std::vector<int>& singleNodes) const {
    std::string out = "#";
    // single nodes
    for (int x : singleNodes) {
        out += "\n" + std::to_string(x) + ";";
    }
    int counting = 0;
    for (Tree* tree : trees) {
        out += "\n";
        out += tree->output(numLabels);
        counting++;
    }

    // replace shrunken Leafs
    replaceOutput(out);
    return out;
}

void Forest::replaceOutput(std::string& out) const {
    // replace shrunken Leafs
    ASSERT(queue.empty());
    for (int i=0;i<shrunkenLeafs.size();i++) {
        if (shrunkenLeafs[i] != std::to_string(i)) {        // i is a shrunken Leaf
            queue.push_back(i);
        }
    }
    int loopsDone = 0;
    while ((!queue.empty()) && (queue.size() > loopsDone)) {
        int i = queue.front();
        queue.pop_front();
        size_t pos = std::min({out.find("," + std::to_string(i) + ")"),out.find("(" + std::to_string(i) + ","), out.find("\n" + std::to_string(i) + ";")});        // TODO when found one option dont search for other options
        if (pos != std::string::npos) {
            out.replace(pos+1,std::to_string(i).size(),shrunkenLeafs[i]);
            loopsDone = 0;
        } else {
            queue.push_back(i);
            loopsDone++;
        }
    }
    queue.clear();
}