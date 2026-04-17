#include "memory.h"
#include <iostream>
using namespace std;

MemoryManager::MemoryManager(int size)
{
    totalSize = size;
    memory.assign(size, 0);
}

// First-Fit allocation
int MemoryManager::allocate(int pid, int sizeNeeded)
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
                return start;
            }
        }
        else
        {
            count = 0;
        }
    }

    return -1; // allocation failed
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
    int i = 0;
    cout << "Memory: ";

    while (i < totalSize)
    {
        int current = memory[i];
        int count = 0;

        while (i < totalSize && memory[i] == current)
        {
            count++;
            i++;
        }

        if (current == 0)
        {
            cout << "[" << count << " FREE] ";
        }
        else
        {
            cout << "[" << count << " P" << current << "] ";
        }
    }

    cout << endl;
}