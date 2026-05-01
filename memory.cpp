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
void MemoryManager::display(int time)
{
    cout << "[t=" << time << "] Memory: ";

    int i = 0;
    while (i < (int)memory.size())
    {
        int pid = memory[i];
        int start = i;

        while (i < (int)memory.size() && memory[i] == pid)
        {
            i++;
        }

        int end = i - 1;

        if (pid == 0)
            cout << "[" << start << ".." << end << " FREE] ";
        else
            cout << "[" << start << ".." << end << " P" << pid << "] ";
    }

    cout << "\n";
}