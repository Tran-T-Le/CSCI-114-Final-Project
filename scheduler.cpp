#include "scheduler.h"

Process *selectProcess(vector<Process *> &ready, SchedulingPolicy policy)
{
    if (ready.empty())
        return nullptr;

    // FCFS and RR: take the first process in ready queue
    if (policy == FCFS || policy == RR)
    {
        Process *p = ready.front();
        ready.erase(ready.begin());
        return p;
    }

    // SJF: choose process with shortest remaining time
    if (policy == SJF)
    {
        int idx = 0;
        for (int i = 1; i < ready.size(); i++)
        {
            if (ready[i]->remainingTime < ready[idx]->remainingTime)
            {
                idx = i;
            }
        }

        Process *p = ready[idx];
        ready.erase(ready.begin() + idx);
        return p;
    }

    return nullptr;
}