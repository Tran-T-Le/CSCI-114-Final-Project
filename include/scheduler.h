#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "input.h"

using namespace std;

enum SchedulingPolicy
{
    FCFS,
    SJF,
    RR
};

Process* selectProcess(vector<Process*>& ready, SchedulingPolicy policy);

#endif