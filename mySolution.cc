# include <fstream>
# include <iostream>
# include <vector>
# include <array>
# include <set>
# include <string>

# include "myForest.hh"
# include "global.hh"
# include "macros.hh"
# include "myQueue.hh"

// best solution so far
Forest bestForest;
std::vector<int> bestSingleNodes;
int minTrees;               // number of trees in the best solution

// globals
int inputLeafs;
int inputTrees;
int arrayCapacity;
int indexCapacity;
std::vector<std::string> shrunkenLeafs;         // stores the "translation" of the leafs that were shrunken at the start of the program (space: inputLeafs, shrunkenLeafs[i] = "(a,b)"/"i")
Queue queue;

// Eingabe
std::vector<Forest> forests;
std::vector<int> forestOrdering;
std::vector<SiblingPairs> cachedInputSiblingPairs;

std::vector<bool> ws_nodesOnPath;
std::vector<bool> ws_visited;
std::vector<int> ws_pathNodes;


// forward declarations
void orderingInput(std::vector<int>& ordering, Forest& f0, int nextTreeNumber);

// find max Tree between Forest 0 and Forest "number"
void recursiveFunc(Forest& f0, Forest& fnumb, int number, ObjectPool* objPool, int highestLabel, std::vector<int>& singleNodes) {
    if (f0.numberTrees() + singleNodes.size() >= minTrees) {
        return;
    }
    
    fnumb.findSiblings();   
    std::array<int,2> siblings = {fnumb.sib1,fnumb.sib2};
    if (siblings[0] == -1) {
        // max Tree
        if (number == inputTrees-1) {
            bestForest.deepCopy(f0);                // bestForest = f0
            bestSingleNodes = singleNodes;
            minTrees = f0.numberTrees() + singleNodes.size();
            return;
        }
        // compare with next tree
        number++;
        f0.clearInnerNodes(inputLeafs);
        orderingInput(forestOrdering, f0, number);

        // cut all single nodes already
        Forest f_next(objPool, forests[forestOrdering[number]]);
        f_next.cutSingles(singleNodes);             // takes the labels not the positions

        recursiveFunc(f0, f_next, number, objPool, inputLeafs, singleNodes);

    // recursive
    } else {
        // a, b in different connected components in F0
        if (!f0.isSameTree(siblings[0],siblings[1])) {
            // cut the "not-shrunken-leafs" first
            if (siblings[0] > inputLeafs) {
                int temp = siblings[0];
                siblings[0] = siblings[1];
                siblings[1] = temp;
            }
            // remove edge to a
            {
                Forest f0_newA(objPool, f0);
                Forest f1_newA(objPool, fnumb);
                int sizeSingleNodes = singleNodes.size();
                f0_newA.cut(siblings[0], &singleNodes);
                f1_newA.cut(siblings[0], nullptr);
                recursiveFunc(f0_newA, f1_newA, number, objPool, highestLabel, singleNodes);
                while (sizeSingleNodes != singleNodes.size()) {
                    singleNodes.pop_back();
                }

            }
            // remove edge to b
            {
                Forest f0_newB(objPool, f0);
                Forest f1_newB(objPool, fnumb);
                int sizeSingleNodes = singleNodes.size();
                f0_newB.cut(siblings[1], &singleNodes);
                f1_newB.cut(siblings[1], nullptr);
                recursiveFunc(f0_newB, f1_newB, number, objPool, highestLabel, singleNodes);
                while (sizeSingleNodes != singleNodes.size()) {
                    singleNodes.pop_back();
                }
            }
        }
        // a, b also siblings in F0
        else if (f0.isSibling(siblings[0],siblings[1])) {
            highestLabel++;
            f0.shrink(siblings[0],highestLabel);
            fnumb.shrink(siblings[0],highestLabel);
            recursiveFunc(f0,fnumb, number, objPool, highestLabel, singleNodes);
        }
        // a, b in same connected component but not silings in F0
        else {
            // cut the "not-shrunken-leafs" first
            if (siblings[0] > inputLeafs) {
                int temp = siblings[0];
                siblings[0] = siblings[1];
                siblings[1] = temp;
            }
            // remove edge to a
            {
                Forest f0_newA(objPool, f0);
                Forest f1_newA(objPool, fnumb);
                int sizeSingleNodes = singleNodes.size();
                f0_newA.cut(siblings[0], &singleNodes);
                f1_newA.cut(siblings[0], nullptr);
                recursiveFunc(f0_newA, f1_newA, number, objPool, highestLabel, singleNodes);
                while (sizeSingleNodes != singleNodes.size()) {
                    singleNodes.pop_back();
                }
            }
            // remove edge to b
            {
                Forest f0_newB(objPool, f0);
                Forest f1_newB(objPool, fnumb);
                int sizeSingleNodes = singleNodes.size();
                f0_newB.cut(siblings[1], &singleNodes);
                f1_newB.cut(siblings[1], nullptr);
                recursiveFunc(f0_newB, f1_newB, number, objPool, highestLabel, singleNodes);
                while (sizeSingleNodes != singleNodes.size()) {
                    singleNodes.pop_back();
                }
            }
            // path
            {   
                int predictedSingleNodesCount = singleNodes.size();
                int comp = f0.findComponent(siblings[0]);
                if (comp != -1) {
                    Tree* tree = f0.getTree(comp);
                    const std::vector<int>& path = tree->getPath(siblings[0], siblings[1]);
                    
                    std::fill(ws_visited.begin(), ws_visited.end(), false);
                    for (int node : path) {
                        ws_visited[node] = true;
                    }
                    ws_visited[tree->findLabel(siblings[0])] = true;
                    ws_visited[tree->findLabel(siblings[1])] = true;
                    
                    static std::vector<bool> isSingle(inputLeafs + 1, false);
                    std::fill(isSingle.begin(), isSingle.end(), false);
                    for (int s : singleNodes) {
                        isSingle[s] = true;
                    }
                    
                    auto countActiveLeaves = [&](auto& self, Tree* t, int node, int& foundLeafLabel) -> int {
                        if (node == -1) return 0;
                        int lbl = t->getLabel(node);
                        if (lbl > 0 && lbl <= inputLeafs) {
                            if (!isSingle[lbl]) {
                                foundLeafLabel = lbl;
                                return 1;
                            }
                            return 0;
                        }
                        int leftFound = -1;
                        int leftCount = self(self, t, t->getChild1(node), leftFound);
                        if (leftCount > 1) return 2;
                        
                        int rightFound = -1;
                        int rightCount = self(self, t, t->getChild2(node), rightFound);
                        if (leftCount + rightCount > 1) return 2;
                        
                        if (leftCount == 1) foundLeafLabel = leftFound;
                        else if (rightCount == 1) foundLeafLabel = rightFound;
                        
                        return leftCount + rightCount;
                    };

                    for (int node : path) {
                        int c1 = tree->getChild1(node);
                        int c2 = tree->getChild2(node);
                        int sideChild = -1;
                        if (c1 != -1 && !ws_visited[c1]) sideChild = c1;
                        else if (c2 != -1 && !ws_visited[c2]) sideChild = c2;
                        
                        if (sideChild != -1) {
                            int leafLabel = -1;
                            int activeCount = countActiveLeaves(countActiveLeaves, tree, sideChild, leafLabel);
                            if (activeCount == 1) {
                                if (leafLabel != -1 && !isSingle[leafLabel]) {
                                    predictedSingleNodesCount++;
                                }
                            }
                        }
                    }
                }
                
                if (f0.numberTrees() + predictedSingleNodesCount < minTrees) {
                    Forest f0_newAB(objPool, f0);
                    Forest f1_newAB(objPool, fnumb);
                    int sizeSingleNodes = singleNodes.size();
                    f0_newAB.cutPath(siblings[0],siblings[1],&singleNodes);
                    f1_newAB.cutPath(siblings[0],siblings[1],nullptr);  
                    recursiveFunc(f0_newAB, f1_newAB, number, objPool, highestLabel, singleNodes);
                    while (sizeSingleNodes != singleNodes.size()) {
                        singleNodes.pop_back();
                    }
                }
            }
        }
    }
    return;
}

void findAndShrinkSiblings (std::vector<std::string>& strings) {
    size_t pos = 0;
    size_t len = strings[0].size();
    std::vector<size_t> pairPos;
    while (pos < len) {
        pos = strings[0].find_first_of('(', pos);
        size_t o = strings[0].find_first_of('(', pos+1);
        size_t c = strings[0].find_first_of(')', pos+1);
        if (c < o) {        // found possible sibling pair
            pairPos.clear();
            ASSERT(pos < len);
            // create also the "inverse" pair
            std::string pair1 = strings[0].substr(pos,c-pos+1);
            size_t mid = pair1.find(',');
            ASSERT(pair1[0] == '(' && pair1[pair1.size()-1] == ')');
            std::string number1 = pair1.substr(1,mid-1);
            std::string number2 = pair1.substr(mid+1,pair1.size()-mid-2);
            std::string pair2 = "(" + number2 + "," + number1 + ")";
    
            pairPos.push_back(pos);
            
            // search in other trees
            std::vector<size_t> pairPos {pos};
            bool included = true;
            int i = 1;
            while (included && i<strings.size()) {
                size_t posI = std::min(strings[i].find(pair1),strings[i].find(pair2));
                if (posI != std::string::npos) {
                    pairPos.push_back(posI);
                } else {
                    included = false;
                }
                i++;
            }
            // shrink the pair
            if (included) {
                for (int j=0; j<strings.size(); j++) {
                    strings[j].replace(pairPos[j],pair1.size(),number1);
                }
                // write in shrunken Leafs
                int entryPosition = std::stoi(number1);
                std::string oldEntry = shrunkenLeafs[entryPosition];
                std::string newEntry;
                if (oldEntry == number1) {              // not overwritten yet
                    newEntry = pair1;
                } else {
                    size_t posInPair = std::min(pair1.find("," + number1 + ")"), pair1.find("(" + number1 + ","));
                    newEntry = pair1;
                    newEntry.replace(posInPair+1,number1.size(),oldEntry);
                }
                shrunkenLeafs[entryPosition] = newEntry;
                
                pos = 0;
                len = strings[0].size();
            } else {
                pos = o;
            }
        } else {            // no pair
            pos = o;
        }
    }

}   

int estimateTreeDistance(Forest& f1, Forest& f2) {
    // for all sibling pairs (in both forests) add the calculated distances in the other forest
    int distance = 0;
    // find all sibling pairs in f1
    SiblingPairs siblingPairs1 = f1.findAllSiblings(false);
    // calculate distance for every pair in f2
    for (size_t i = 0; i < siblingPairs1.pairs.size(); i += 2) {
        distance += f2.calcLabelDistance(siblingPairs1.pairs[i], siblingPairs1.pairs[i+1]);
    }
    // find all sibling pairs in f2
    SiblingPairs siblingPairs2 = f2.findAllSiblings(false);
    // calculate distance for every pair in f1
    for (size_t i = 0; i < siblingPairs2.pairs.size(); i += 2) {
        distance += f1.calcLabelDistance(siblingPairs2.pairs[i], siblingPairs2.pairs[i+1]);
    }
    return distance;
}

int estimateTreeDistanceCached(Forest& f1, const SiblingPairs& siblingPairs1, Forest& f2, const SiblingPairs& siblingPairs2) {
    int distance = 0;
    // calculate distance for every pair in f2
    for (size_t i = 0; i < siblingPairs1.pairs.size(); i += 2) {
        distance += f2.calcLabelDistance(siblingPairs1.pairs[i], siblingPairs1.pairs[i+1]);
    }
    // calculate distance for every pair in f1
    for (size_t i = 0; i < siblingPairs2.pairs.size(); i += 2) {
        distance += f1.calcLabelDistance(siblingPairs2.pairs[i], siblingPairs2.pairs[i+1]);
    }
    return distance;
}

void orderingInput(std::vector<int>& ordering, Forest& f0, int nextTreeNumber) {
    if (nextTreeNumber >= inputTrees - 1) {
        return;
    }
    int maxIndex = 0;
    int maxDist = -1;
    SiblingPairs siblingPairs0 = f0.findAllSiblings(false);
    for (int i=nextTreeNumber; i<inputTrees; i++) {
        int targetIdx = forestOrdering[i];
        int dist = estimateTreeDistanceCached(f0, siblingPairs0, forests[targetIdx], cachedInputSiblingPairs[targetIdx]);
        if (dist > maxDist) {
            maxIndex = i;
            maxDist = dist;
        }
    }
    ASSERT(maxIndex >= nextTreeNumber);

    int temp = ordering[nextTreeNumber];
    ordering[nextTreeNumber] = ordering[maxIndex];
    ordering[maxIndex] = temp;
}

int main(int argc, char** argv) {
    std::vector<int> singleNodes;
    int highestLabel = 0;
    forests.clear();
    // read in
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line[0] == '#') {
            if (line[1] == 'p') {
                unsigned temp = line.find(' ', 3);
                // set global values
                inputTrees = std::stoi(line.substr(3,temp-3));
                inputLeafs = std::stoi(line.substr(temp+1,line.size()-temp-1));
                ws_nodesOnPath.assign(2 * inputLeafs, false);
                ws_visited.assign(2 * inputLeafs, false);
                ws_pathNodes.reserve(2 * inputLeafs);
                arrayCapacity = (2*inputLeafs-1)*4;               // set array Capacity
                indexCapacity = (2*inputLeafs-1);
                forests.reserve(inputTrees);                        // reserve space for every Input Forest
                shrunkenLeafs.reserve(inputLeafs+1);
                queue.setCapacity(2*inputLeafs);
                break;   
            }
            continue;
        }
    }
    // define shrunkenLeafs
    std::string stringLeaf;
    for (int i=0; i<inputLeafs+1; i++) {
        stringLeaf = std::to_string(i);
        shrunkenLeafs.push_back(stringLeaf);
    }
    // initilize ObjectPool         
    ObjectPool objectPool(4*inputLeafs*inputTrees+inputLeafs+2*inputTrees, sizeof(Tree), indexCapacity*4, arrayCapacity*4);      // capacity * 4 because 4 Bytes per int        // TODO size
    bestForest.setPool(&objectPool);
    
    // read in Trees
    std::vector<std::string> treeStrings;
    while (std::getline(std::cin, line)) {
        if (line[0] != '#') {
            treeStrings.push_back(line);
        }
    }
    // shrink siblings already
    findAndShrinkSiblings(treeStrings);

    // build forests
    for (std::string s : treeStrings) {
        forests.emplace_back(&objectPool, s);
    }
    
    // cache input sibling pairs
    cachedInputSiblingPairs.resize(forests.size());
    for (size_t i = 0; i < forests.size(); i++) {
        cachedInputSiblingPairs[i] = forests[i].findAllSiblings(false);
    }
    // order forests
    int maximum = -1;
    int distance;
    std::pair<int,int> indices;
    for(int i=0; i<forests.size(); i++) {
        for (int j=i+1; j<forests.size(); j++) {
            distance = estimateTreeDistance(forests[i],forests[j]);
            if (distance > maximum) {
                maximum = distance;
                indices = {i,j};
            }
        }
    }
    ASSERT(indices.first!=indices.second);

    for(int i=0;i<inputTrees;i++) {
        forestOrdering.push_back(i);
    }
    // swap the order so that the first two entries are best
    forestOrdering[0] = indices.first;
    forestOrdering[indices.first] = 0;
    int temp = forestOrdering[1];
    for (int i=1;i<inputTrees;i++) {
        if (forestOrdering[i] == indices.second) {
            forestOrdering[1] = indices.second;
            forestOrdering[i] = temp;
            i = inputTrees;
        }
    }



    minTrees = inputLeafs-1;
    highestLabel = inputLeafs;
    
    // solve 
    recursiveFunc(forests[forestOrdering[0]],forests[forestOrdering[1]], 1, &objectPool, highestLabel, singleNodes);

    // print and return solution 
    if (minTrees == inputLeafs-1) {
        std::cout << "(1,2);" << std::endl;
        for (int i=3; i<=inputLeafs; i++) {
            std::cout << i << ";" << std::endl;
        }
    } else {
        std::cout << bestForest.output(inputLeafs, bestSingleNodes) << "\n";
    }

    // clear all remaining forests
    bestForest.clear();
    for (int i=forests.size()-1;i>=0;i--) {
        forests[i].clear();
    }
    return 0;
}