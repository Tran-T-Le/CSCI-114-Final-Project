#include <iostream>
#include <vector>
#include "memory.h"
#include "input.h"
#include "scheduler.h"
using namespace std;

class Simulator
{
private:
    vector<Process> processes;
    vector<Process *> ready;
    vector<Process *> blocked;
    Process *running;

    MemoryManager mem;
    SchedulingPolicy policy;
    int quantum;
    int currentTime;
    int finishedCount;
    int quantumCounter;

public:
    Simulator(const vector<Process> &processList,
              SchedulingPolicy schedulingPolicy,
              int timeQuantum,
              int memorySize);

    void run();
};