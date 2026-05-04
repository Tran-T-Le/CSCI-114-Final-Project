#ifndef SIMULATION_H
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
    vector<Process *> ready;
    vector<Process *> paidQueue;

    vector<Process *> blockedMemory;
    vector<Process *> blockedStove;
    vector<Process *> blockedPayment;

    Process *running;

    vector<ResourceManager> stoves;

    ResourceManager PaymentTerminal;

    Logger logger;
    MemoryManager mem;
    SchedulingPolicy policy;

    int quantum;
    int currentTime;
    int finishedCount;
    int quantumCounter;

    int paymentTimeLeft;
    int paymentStartTime;
    static const int PAYMENT_TIME = 2;

    vector<GanttEntry> gantt;

    void Systemstate();

    Process *findProcessByPid(int pid);
    bool hasStove(int pid) const;
    bool acquireStove(Process *p);
    void releaseStove(int pid);

public:
    Simulator(const vector<Process> &processList,
              SchedulingPolicy schedulingPolicy,
              int timeQuantum,
              int memorySize,
              string logfile);

    string policyToString(SchedulingPolicy p);
    string formatQueue(const vector<Process *> &q);

    void run();
    void printGanttPerProcess() const;
    void exportGantttoCSV(string filename);
};

#endif