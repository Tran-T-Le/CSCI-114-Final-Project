#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include "memory.h"
#include "input.h"
#include "scheduler.h"
using namespace std;

struct GanttEntry
{
    int pid;
    int start;
    int end;
};

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

    vector<GanttEntry> gantt;

public:
    Simulator(const vector<Process> &processList,
              SchedulingPolicy schedulingPolicy,
              int timeQuantum,
              int memorySize);
    void printGanttPerProcess() const;
    void run();
};