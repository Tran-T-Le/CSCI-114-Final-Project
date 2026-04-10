
#include "simulation.h"

Simulator::Simulator(const vector<Process> &processList,
                     SchedulingPolicy schedulingPolicy,
                     int timeQuantum,
                     int memorySize)
    : processes(processList), policy(schedulingPolicy), quantum(timeQuantum), mem(memorySize)
{
    currentTime = 0;
    finishedCount = 0;
    quantumCounter = 0;
    running = nullptr;
}
void Simulator::run()
{
    vector<Process *> ready;
    vector<Process *> blocked;

    Process *running = nullptr;

    int time = 0;
    int done = 0;
    int quantumCounter = 0;

    while (done < processes.size())
    {
        cout << "\n========== TIME " << time << " ==========\n";

        // Retry blocked processes first
        vector<Process *> temp;
        for (auto p : blocked)
        {
            if (mem.allocate(p->pid, p->memoryNeeded))
            {
                p->state = READY;
                ready.push_back(p);
                cout << "job=P" << p->pid << " state=BLOCKED_MEMORY->READY " << endl;
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
                    cout << "job=P" << p.pid << "Arrived-> " << "state = READY " << endl;
                }
                else
                {
                    p.state = BLOCKED_MEMORY;
                    blocked.push_back(&p);
                    cout << "job=P" << p.pid << "Arrived-> " << "state = BLOCKED_MEMORY " << endl;
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
            cout << "job=P" << running->pid << " state=RUNNING " << "remaining=" << running->remainingTime << endl;
            running->remainingTime--;
            quantumCounter++;

            if (running->remainingTime == 0)
            {
                running->state = TERMINATED;
                mem.release(running->pid);
                cout << "job=P" << running->pid << " FINISHED->state = TERMINATED " << endl;
                running = nullptr;
                done++;
                quantumCounter = 0;
            }
            else if (policy == RR && quantumCounter == quantum)
            {
                running->state = READY;
                ready.push_back(running);
                cout << "job=P" << running->pid << " TIME_SLICE_EXPIRED->state = READY " << endl;
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