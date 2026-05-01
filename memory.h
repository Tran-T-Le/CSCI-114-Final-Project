#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
using namespace std;

class MemoryManager
{
private:
    vector<int> memory;
    int totalSize;

public:
    MemoryManager(int size = 1024);

    int allocate(int pid, int sizeNeeded);

    void release(int pid);

    void display(int time);
};

#endif