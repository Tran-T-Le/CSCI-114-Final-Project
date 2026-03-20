#include "memory.h"
#include <iostream>
using namespace std;

MemoryManager::MemoryManager(int size)
{
    totalSize = size;
    memory.assign(size, 0);
}

// First-Fit allocation
bool MemoryManager::allocate(int pid, int sizeNeeded)
{
    int count = 0;
    int start = -1;

    for (int i = 0; i < totalSize; i++)
    {
        if (memory[i] == 0)
        {
            if (count == 0)
                start = i;

            count++;

            if (count == sizeNeeded)
            {
                // allocate
                for (int j = start; j < start + sizeNeeded; j++)
                {
                    memory[j] = pid;
                }
                return true;
            }
        }
        else
        {
            count = 0;
        }
    }

    return false;
}

// release memory
void MemoryManager::release(int pid)
{
    for (int i = 0; i < totalSize; i++)
    {
        if (memory[i] == pid)
        {
            memory[i] = 0;
        }
    }
}

// debug
void MemoryManager::display()
{
    int used = 0;

    for (int x : memory)
    {
        if (x != 0)
            used++;
    }

    cout << "Memory used: " << used << "/" << totalSize << endl;
}