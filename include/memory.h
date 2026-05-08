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

    // Used for logging to show how much memory is currently used
    int getusedmemory() const
    {
        int used = 0;
        for (int i : memory)
            if (i != 0)
                used++;
        return used;
    }

    int allocate(int pid, int sizeNeeded);

    void release(int pid);

    void display(int time);
};

#endif