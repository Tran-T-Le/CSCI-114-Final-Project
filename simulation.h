#include <iostream>
#include <vector>
#include "memory.h"
#include "input.h"
#include "scheduler.h"

using namespace std;

void runSimulation(vector<Process> &processes, SchedulingPolicy policy, int quantum, int memorySize)
{
    MemoryManager mem(memorySize);
    vector<Process *> ready;
    vector<Process *> blocked;

    Process *running = nullptr;

    int time = 0;
    int done = 0;
    int quantumCounter = 0;

    while (done < processes.size())
    {
        cout << "\nTime " << time << endl;

        // Retry blocked processes first
        vector<Process *> temp;
        for (auto p : blocked)
        {
            if (mem.allocate(p->pid, p->memoryNeeded))
            {
                p->state = READY;
                ready.push_back(p);
                cout << "P" << p->pid << " UNBLOCKED\n";
            }
            else
            {
                temp.push_back(p);
            }
        }
        blocked = temp;

        // Then check newly arrived processes
        for (auto &p : processes)
        {
            if (p.arrivalTime == time && p.state == NEW)
            {
                if (mem.allocate(p.pid, p.memoryNeeded))
                {
                    p.state = READY;
                    ready.push_back(&p);
                    cout << "P" << p.pid << " READY\n";
                }
                else
                {
                    p.state = BLOCKED_MEMORY;
                    blocked.push_back(&p);
                    cout << "P" << p.pid << " BLOCKED_MEMORY\n";
                }
            }
        }

        // If CPU is free, scheduler selects a process
        if (running == nullptr)
        {
            running = selectProcess(ready, policy);
            if (running != nullptr)
            {
                running->state = RUNNING;
                quantumCounter = 0;
            }
        }

        // Run process
        if (running != nullptr)
        {
            cout << "Running P" << running->pid << endl;
            running->remainingTime--;
            quantumCounter++;

            if (running->remainingTime == 0)
            {
                running->state = TERMINATED;
                mem.release(running->pid);
                cout << "P" << running->pid << " DONE\n";
                running = nullptr;
                done++;
                quantumCounter = 0;
            }
            else if (policy == RR && quantumCounter == quantum)
            {
                running->state = READY;
                ready.push_back(running);
                cout << "P" << running->pid << " TIME SLICE EXPIRED\n";
                running = nullptr;
                quantumCounter = 0;
            }
        }
        else
        {
            cout << "CPU IDLE\n";
        }

        mem.display();
        time++;
    }
}