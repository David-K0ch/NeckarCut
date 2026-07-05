#ifndef GLOBAL_HH
#define GLOBAL_HH

# include <vector>

# include "myQueue.hh"

inline const unsigned INFORMATION_COUNT = 4;

extern int inputLeafs; 
extern int inputTrees;
extern int arrayCapacity;      // size of data for Array representing a tree: (2*inputLeafs-1)*4       (4 ints per node)
extern int indexCapacity;      // size of lookup for Leafs: (2*inputLeafs-1)            (number of nodes)

extern Queue queue;
extern std::vector<std::string> shrunkenLeafs;

extern std::vector<bool> ws_nodesOnPath;
extern std::vector<bool> ws_visited;
extern std::vector<int> ws_pathNodes;

#endif