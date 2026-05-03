#ifndef SIMULATION_H
#define SIMULATION_H

#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <fstream>
#include <string>
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
    vector<Process*> blockedPayment;
    vector<Process*> blockedStove;

    Process* running;
    Process* paymentProcess;

    ResourceManager stove;
    ResourceManager paymentTerminal;
    Logger logger;
    MemoryManager mem;

    SchedulingPolicy policy;
    int quantum;
    int currentTime;
    int finishedCount;
    int quantumCounter;
    int paymentTimeLeft;

    vector<GanttEntry> gantt;

    string stateToString(ProcessState state) const;
    string policyToString(SchedulingPolicy p) const;
    string formatQueue(const vector<Process*>& q) const;

    void addToQueue(vector<Process*>& q, Process* p);
    void retryMemoryBlocked();
    void startPaymentIfPossible();
    void finishPaymentIfNeeded();
    bool allocateMemoryFor(Process* p, const string& reason);
    bool acquireStoveFor(Process* p);
    void retryStoveBlocked();

public:
    Simulator(const vector<Process>& processList,
        SchedulingPolicy schedulingPolicy, int timeQuantum, int memorySize, string logfile);

    void run();
    void printGanttPerProcess() const;
    void exportGantttoCSV(string filename);
};

#endif
