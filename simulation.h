#ifndef SIMULATION_H  // [CHANGE 2] added missing include guard
#define SIMULATION_H

#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <fstream>
#include "memory.h"
#include "input.h"
#include "scheduler.h"
#include "resource.h"
#include "logger.h"

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
    vector<Process*> ready;

    vector<Process*> blockedMemory;
    vector<Process*> blockedStove;
    vector<Process*> blockedPayment;

    Process* running;

    ResourceManager CPUStove;   
    ResourceManager PaymentTerminal;    

    Logger logger;
    MemoryManager mem;
    SchedulingPolicy policy;
    int quantum;
    int currentTime;
    int finishedCount;
    int quantumCounter;

    vector<GanttEntry> gantt;

    void Systemstate();

public:
    Simulator(const vector<Process>& processList,
        SchedulingPolicy schedulingPolicy, int timeQuantum, int memorySize, string logfile);

    string policyToString(SchedulingPolicy p);
    string formatQueue(const vector<Process*>& q);

    void printGanttPerProcess() const;
    void run();
    void exportGantttoCSV(string filename);
};

#endif