# include <vector>
# include <string>
# include <utility>
# include <tuple>
# include <iostream>
# include <deque>
# include <set>
# include <stdexcept>
# include <algorithm>

# include "myTree.hh"
# include "global.hh"
# include "macros.hh"
# include "myArray.hh"
# include "objectPool.hh"
# include "myQueue.hh"

// Use a vector as data structure
// for each node use multiple entries to store information

// number of informations stored per node (label, parent, child1, child2)
// label: >0 => leaf with same label,  =0 => inner node,  <-1 => "shrunken leaf"
// parent, child1, child2: => returns the index of the node  (-1 => no parent, -1 => no child)


/////////////////////////////////////////////////////////////////////////////////////////////////////
Tree::Tree(ObjectPool* objPool) : pool(objPool), onlyInputLeafs(true), references(0), index(objPool, nullptr, indexCapacity), data(objPool, nullptr, arrayCapacity) {
}

Tree::Tree(ObjectPool* objPool, int* indexBuffer, int* dataBuffer) : pool(objPool), onlyInputLeafs(true), references(0), index(objPool, indexBuffer, indexCapacity), data(objPool, dataBuffer, arrayCapacity) {
    for (int i=0; i<indexCapacity; i++) {
        index.push_back(-1);
    }
}

Tree::Tree(ObjectPool* objPool, int* indexBuffer, int* dataBuffer, Tree* other, bool isSubtreeEmptyIndex) : pool(objPool), onlyInputLeafs(other->onlyInputLeafs), references(0), index(objPool, indexBuffer, other->index, isSubtreeEmptyIndex), data(objPool, dataBuffer, other->data) {
}

Tree::~Tree() {
    pool->release(reinterpret_cast<char*>(this));
}


void Tree::decrementReferences() {
    ASSERT(references > 0);
    references--;
    if (references == 0) {
        this->~Tree();
    }
}

// Setter
void Tree::setLabel(unsigned i, int label) {
    int oldLabel = getLabel(i);
    if (oldLabel > 0) {
        setPosition(oldLabel,-1);
    }
    if (label > 0) {
        setPosition(label,i);
    }
    data[i*INFORMATION_COUNT] = label;
}

int Tree::getSibling(unsigned i) {
    int parent = getParent(i);
    if ((parent == -1) || (getLabel(i) == -1)) {
        return -1;
    } else {
        if (getChild1(parent) == i) {
            return getChild2(parent);
        } else {
            return getChild1(parent);
        }
    }
}


const std::vector<int>& Tree::getPath(int label1, int label2) {
    ASSERT((findLabel(label1) != -1) && (findLabel(label2) != -1));
    ws_pathNodes.clear();

    int n1 = findLabel(label1);
    int n2 = findLabel(label2);
    int p1 = getParent(n1);
    int p2 = getParent(n2);
    ASSERT(p1 != -1 && p2 != -1);

    // Compute depths of p1 and p2 (distance to root)
    int depth1 = 0;
    int temp = p1;
    while (temp != -1) {
        depth1++;
        temp = getParent(temp);
    }

    int depth2 = 0;
    temp = p2;
    while (temp != -1) {
        depth2++;
        temp = getParent(temp);
    }

    // Align depths to find LCA while collecting path nodes
    int curr_p1 = p1;
    int curr_p2 = p2;
    int d1 = depth1;
    int d2 = depth2;

    while (d1 > d2) {
        ws_pathNodes.push_back(curr_p1);
        curr_p1 = getParent(curr_p1);
        d1--;
    }
    
    // Store nodes from p2's side to add at the end
    while (d2 > d1) {
        ws_pathNodes.push_back(curr_p2);
        curr_p2 = getParent(curr_p2);
        d2--;
    }

    while (curr_p1 != curr_p2) {
        ws_pathNodes.push_back(curr_p1);
        ws_pathNodes.push_back(curr_p2);
        curr_p1 = getParent(curr_p1);
        curr_p2 = getParent(curr_p2);
    }

    return ws_pathNodes;
}


int Tree::findSiblingPair() {
    // DFS like
    queue.push_back(0);
    while (queue.size() != 0) {
        int current = queue.back();
        queue.pop_back();
        // inner node
        if (getLabel(current) == 0) {
            queue.push_back(getChild1(current));
            queue.push_back(getChild2(current));
        } else {
            // no sibling
            if (getSibling(current) == -1) {
                continue;
            }
            // pair found
            if ((getLabel(current) > 0) && (getLabel(getSibling(current)) > 0)) {
                queue.clear();
                return getParent(current);
            }
        }
    }
    // no pair found
    queue.clear();
    return -1;
}

// count leafs --> TODO wrong!? (but also not needed)
unsigned Tree::getNumberLeafs() {
    unsigned numberLeafs = 0;
    for(unsigned i=0; i<data.size()/INFORMATION_COUNT; i++) {
        if ((getLabel(i) > 0) && ((getParent(i) == -1) || (getLabel(getParent(i)) == 0))) {
            numberLeafs++;
        }
    }
    return numberLeafs;
}

// make a new node and return the index of the node (index of the label)
unsigned Tree::makeNode(int label, int parent, int child1, int child2) {
    unsigned length = data.size()/INFORMATION_COUNT;
    data.push_back(label);
    data.push_back(parent);
    data.push_back(child1);
    data.push_back(child2);
    if (label > 0) {
        setPosition(label,length);
    }
    return length;
}


Tree* Tree::clearInnerNodes(unsigned numLabels) {
    std::tuple<char*,int*, int*> mem = pool->allocate();
    Tree* clearedTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem),this);
    if (onlyInputLeafs) {
        return clearedTree;             // TODO do i have to make a new tree even in this case?
    }
    for (int i=0; i<data.size()/INFORMATION_COUNT;i++) {
        if (getLabel(i) > numLabels) {
            clearedTree->setLabel(i,0);
        }
    }
    clearedTree->onlyInputLeafs = true;
    return clearedTree;
}

// gets the i-th node and contracts the edges above
void Tree::contractEdges(unsigned i) {
    unsigned current = getParent(i);
    // if single node: do nothing
    if (current == -1) {
        return;
    }
    // go path up the tree until reached the root as long as the nodes only have one child
    while ((getChild1(current) == -1) || (getChild2(current) == -1)){
        // only do something if current is inner node (TODO necessary?)
        if (getLabel(current) == 0) {
            // child1 is nullptr
            if (getChild1(current) == -1) {
                int newNode = getChild2(current);
                // current is root of the tree
                if (getParent(current) == -1) {
                    setLabel(0,getLabel(newNode));
                    setChild1(0,getChild1(newNode));
                    setChild2(0,getChild2(newNode));
                    if (getChild1(newNode) != -1) {
                        setParent(getChild1(newNode),0);
                    }
                    if (getChild2(newNode) != -1) {             // maybe only check one child -> should always be the same
                        setParent(getChild2(newNode),0);
                    }
                    return;
                }
                // current is not root
                setParent(newNode,getParent(current));          // change parent pointer of the child (to skip current)
                // current is child1 of his parent
                if (getChild1(getParent(current)) == current) {
                    setChild1(getParent(current),newNode);      // change the child pointer of the parent (to skip current)
                // current is child2 of his parent
                } else {
                    setChild2(getParent(current),newNode);      // change the child pointer of the parent (to skip current)
                }
            // child2 is nullptr
            } else {
                int newNode = getChild1(current);
                // current is root of the tree
                if (getParent(current) == -1) {
                    setLabel(0,getLabel(newNode));
                    setChild1(0,getChild1(newNode));
                    setChild2(0,getChild2(newNode));
                    if (getChild1(newNode) != -1) {
                        setParent(getChild1(newNode),0);
                    }
                    if (getChild2(newNode) != -1) {             // maybe only check one child -> should always be the same
                        setParent(getChild2(newNode),0);
                    }
                    return;
                }
                setParent(newNode,getParent(current));          // change parent pointer of the child (to skip current)
                // current is child1 of his parent
                if (getChild1(getParent(current)) == current) {
                    setChild1(getParent(current),newNode);      // change the child pointer of the parent (to skip current)
                // current is child2 of his parent
                } else {
                    setChild2(getParent(current),newNode);      // change the child pointer of the parent (to skip current)
                }
            }
        }        
        current = getParent(current);
    }
}


Tree* Tree::shrink(unsigned i, unsigned newLabel) {
    ASSERT(newLabel > inputLeafs);
    std::tuple<char*,int*, int*> mem = pool->allocate();
    Tree* newTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem),this);
    newTree->setLabel(i,newLabel);                     // shrunken leafs get a label   -- real leafs have children = -1
    newTree->onlyInputLeafs = false;
    return newTree;
}

// cut the node and return the two new trees of both parts
std::pair<bool,std::pair<Tree*,Tree*>> Tree::cut(unsigned i, std::vector<int>* singleNodes) {
    int parent = getParent(i);
    if (parent == -1) {       // if cut above root: do nothing
        return {false,{this,this}};
    }
    // tree that gets the node cut of
    // adjust the parent
    std::tuple<char*,int*, int*> mem1 = pool->allocate();
    Tree* newTree1 = new (std::get<0>(mem1)) Tree(pool,std::get<1>(mem1),std::get<2>(mem1),this);

    if (newTree1->getChild1(parent) == i) {
        newTree1->setChild1(parent, -1);
    } else {
        newTree1->setChild2(parent, -1);
    }

    // remove label of i from index
    ASSERT(getLabel(i) > 0);
    newTree1->setPosition(getLabel(i),-1);
    // contract the tree above
    newTree1->contractEdges(i);                                                   // maybe enough to just adjust the parent??
    if (newTree1->getChild1(0) == -1) {
        ASSERT(newTree1->getChild2(0) == -1);
        ASSERT(newTree1->getLabel(0) > 0);
        if (singleNodes != nullptr) {
            singleNodes->push_back(newTree1->getLabel(0));
        }
        newTree1->~Tree();
        newTree1 = nullptr;
    }


    // check if it is single node that gets cut off
    if (getChild1(i) == -1) {
        ASSERT(getLabel(i) > 0 && getLabel(i) <= inputLeafs);
        if (singleNodes != nullptr) {
            singleNodes->push_back(getLabel(i));
        }
        return {true,{newTree1,nullptr}};
    }

    // tree that is cut off from the original tree
    // make new tree - copy everything but initialize index to -1 and change only the root node
    std::tuple<char*,int*,int*> mem2 = pool->allocate();
    Tree* newTree2 = new (std::get<0>(mem2)) Tree(pool,std::get<1>(mem2),std::get<2>(mem2),this,true);
    // update root
    newTree2->setLabel(0,getLabel(i));
    newTree2->setChild1(0,getChild1(i));
    newTree2->setChild2(0,getChild2(i));  

    // update index and parents of children
    ASSERT(queue.empty());
    if (getChild1(i) != -1) {                   //if one child is -1, the other should be as well
        ASSERT(getChild2(i) != -1);
        // update parents
        newTree2->setParent(getChild1(i),0);
        newTree2->setParent(getChild2(i),0);
        // queue for index
        queue.push_back(getChild1(i));
        queue.push_back(getChild2(i));
    }
    while(!queue.empty()) {
        int current = queue.back();
        queue.pop_back();
        ASSERT(getLabel(current) > 0);
        newTree2->setPosition(getLabel(current),current);
        if (newTree1 != nullptr) {
            newTree1->setPosition(getLabel(current),-1);
        }        
        if (getChild1(current) != -1) {
            ASSERT(getChild2(current) != -1);
            queue.push_back(getChild1(current));
            queue.push_back(getChild2(current));
        }
    } 
    queue.clear();
    return {true,{newTree1,newTree2}};
}

// cut the node and change the tree itself aswell
std::pair<bool,Tree*> Tree::changingCut(unsigned i, std::vector<int>* singleNodes) {           // used by cutPath               // TODO resulting single nodes
    int parent = getParent(i);
    if (parent == -1) {       // if cut above root: do nothing
        return {false,this};
    }
    int current = parent;
    // adjust the parent
    if (getChild1(parent) == i) {
        setChild1(parent, -1);
    } else {
        setChild2(parent, -1);
    }
    // remove label of i from index
    if (getLabel(i) != 0) {
        setPosition(getLabel(i),-1);
    }
    // contract the tree above
    contractEdges(i);                                                   // maybe enough to just adjust the parent??


    // check if it is single node that gets cut off
    if (getChild1(i) == -1) {
        ASSERT(getLabel(i) > 0 && getLabel(i) <= inputLeafs);
        if (singleNodes != nullptr) {
            singleNodes->push_back(getLabel(i));
        }
        return {true,nullptr};          // TODO could write "return {false,nullptr};"
    }

    // tree that is cut off from the original tree
    // make new tree - copy everything but initialize index to -1 and change only the root node
    std::tuple<char*,int*,int*> mem = pool->allocate();
    Tree* newTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem),this,true);
    // update root
    newTree->setLabel(0,getLabel(i));
    newTree->setChild1(0,getChild1(i));
    newTree->setChild2(0,getChild2(i));

    // update index and parents of children
    ASSERT(queue.empty());
    if (getChild1(i) != -1) {                   //if one child is -1, the other should be as well
        ASSERT(getChild2(i) != -1);
        // update parents
        newTree->setParent(getChild1(i),0);
        newTree->setParent(getChild2(i),0);
        // queue for index
        queue.push_back(getChild1(i));
        queue.push_back(getChild2(i));
    }
    while(!queue.empty()) {
        int current = queue.back();
        queue.pop_back();
        ASSERT(getLabel(current) >= 0);
        if (getLabel(current) != 0) {
            newTree->setPosition(getLabel(current),current);
            setPosition(getLabel(current),-1);
        } 
        if (getChild1(current) != -1) {
            ASSERT(getChild2(current) != -1);
            queue.push_back(getChild1(current));
            queue.push_back(getChild2(current));
        }
    }  
    queue.clear();

    return {true,newTree};
}

std::vector<Tree*> Tree::cutPath(unsigned label1, unsigned label2, std::vector<int>* singleNodes) {
    std::vector<Tree*> newTrees;
    
    // Clear workspace flags and path buffer
    std::fill(ws_visited.begin(), ws_visited.end(), false);
    ws_pathNodes.clear();

    int n1 = findLabel(label1);
    int n2 = findLabel(label2);
    int p1 = getParent(n1);
    int p2 = getParent(n2);
    ASSERT(p1 != -1 && p2 != -1);

    // Compute depths of p1 and p2 (distance to root)
    int depth1 = 0;
    int temp = p1;
    while (temp != -1) {
        depth1++;
        temp = getParent(temp);
    }

    int depth2 = 0;
    temp = p2;
    while (temp != -1) {
        depth2++;
        temp = getParent(temp);
    }

    // Align depths to find LCA while collecting path nodes
    int curr_p1 = p1;
    int curr_p2 = p2;
    int d1 = depth1;
    int d2 = depth2;

    while (d1 > d2) {
        ws_visited[curr_p1] = true;
        ws_pathNodes.push_back(curr_p1);
        curr_p1 = getParent(curr_p1);
        d1--;
    }
    
    while (d2 > d1) {
        ws_visited[curr_p2] = true;
        ws_pathNodes.push_back(curr_p2);
        curr_p2 = getParent(curr_p2);
        d2--;
    }

    while (curr_p1 != curr_p2) {
        ws_visited[curr_p1] = true;
        ws_pathNodes.push_back(curr_p1);
        ws_visited[curr_p2] = true;
        ws_pathNodes.push_back(curr_p2);
        curr_p1 = getParent(curr_p1);
        curr_p2 = getParent(curr_p2);
    }

    // create copy of the tree. This will be the changed tree after all cuts
    std::tuple<char*,int*,int*> mem = pool->allocate();
    Tree* remTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem),this);
    newTrees.push_back(remTree);
    // cut all the nodes
    for (unsigned node: ws_pathNodes) {
        int c1 = getChild1(node);
        int c2 = getChild2(node);
        // check if c1 is not on path
        if (c1 != -1 && !ws_visited[c1] && getLabel(c1) != label1 && getLabel(c1) != label2) {
            std::pair<bool,Tree*> cutResult = remTree->changingCut(c1, singleNodes);
            if ((cutResult.first) && (cutResult.second != nullptr)) {
                newTrees.push_back(cutResult.second);
            }                
        } else if (c2 != -1 && !ws_visited[c2] && getLabel(c2) != label1 && getLabel(c2) != label2) {                                          
            std::pair<bool,Tree*> cutResult = remTree->changingCut(c2, singleNodes);                   // child2 is not on path
            if ((cutResult.first) && (cutResult.second != nullptr)) {
                newTrees.push_back(cutResult.second);
            }                                         
        }
    }
    return newTrees;
}


Tree* Tree::cutSingles(const std::vector<int>& singleLabels) {
    std::vector<Tree*> newTrees;
    // create copy of the tree. This will be the changed tree after all cuts
    std::tuple<char*,int*,int*> mem = pool->allocate();
    Tree* remTree = new (std::get<0>(mem)) Tree(pool,std::get<1>(mem),std::get<2>(mem),this);
    // cut all labels in vector
    for (int label : singleLabels) {
        ASSERT(label > 0 && label <= inputLeafs);
        std::pair<bool,Tree*> cutResult = remTree->changingCut(findLabel(label),nullptr);
        ASSERT(cutResult.first);
    }
    return remTree;
}

std::string Tree::output(unsigned numLabels) {
    std::string out = "";

    if (getLabel(0) > 0 && getLabel(0) <= numLabels) {
        out += std::to_string(getLabel(0));
        out += ";";
    } else {
        ASSERT(queue.empty());
        out += "(,)";
        queue.push_back(getChild2(0));
        queue.push_back(getChild1(0));

        while (!queue.empty()) {
            int current = queue.back();
            queue.pop_back();
            unsigned pos = std::min(out.find(",)"),out.find("(,"));

            if (getLabel(current) > 0 && getLabel(current) <= numLabels) {
                out.insert(pos+1, std::to_string(getLabel(current)));
            } else {
                out.insert(pos+1, "(,)");
                queue.push_back(getChild2(current));
                queue.push_back(getChild1(current));
            }
        }
        out += ";";
    }
    return out;
}

// read an input line and create the tree
void Tree::readInput(std::string &line) {
    if (line[0] != '(') {           // single leaf (e.g. 6;)
        makeNode(std::stoi(line.substr(0,line.size()-1)),-1,-1,-1);
        return;
    }

    ASSERT(line[0] == '(');
    
    // queue: nodes where not all children are correctly defined
    ASSERT(queue.empty());
    queue.push_back(makeNode(0,-1,0,0));       // root
    for (unsigned i=0; i<line.size()-1; i++) {
        if (line[i] == '(') {
            // add the two children
            unsigned parent = queue.back();
            unsigned child1 = makeNode(0,parent,0,0);
            setChild1(parent,child1);
            unsigned child2 = makeNode(0,parent,0,0);
            setChild2(parent,child2);
            // remove the parent from unfinished nodes
            queue.pop_back();
            // add children to unfinished nodes
            queue.push_back(child2);
            queue.push_back(child1);
        }
        else if ((line[i] == ')') or (line[i] == ',')) {
            continue;
        }
        else if (std::isdigit(line[i])) {
            // read the label (possibly multible digits)
            unsigned endOfLabel = std::min(line.find(')', i),line.find(',', i));
            int label = std::stoi(line.substr(i,endOfLabel-i));
            // give the node the label
            ASSERT(!queue.empty());
            unsigned currentNode = queue.back();
            setLabel(currentNode,label);
            setChild1(currentNode,-1);
            setChild2(currentNode,-1);
            // remove the node from unfinishedNodes
            queue.pop_back();
            // adjust the reading position (for multiple digits labels)
            i = endOfLabel-1;
        }
        else {
            ASSERT(false);      // unexpected symbol
        }
    }
    queue.clear();
}