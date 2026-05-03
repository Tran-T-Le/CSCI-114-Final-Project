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
    vector<Process*> blocked;
    Process* running;

    ResourceManager CPUStove;
    Logger logger;

    MemoryManager mem;
    SchedulingPolicy policy;
    int quantum;
    int currentTime;
    int finishedCount;
    int quantumCounter;

    vector<GanttEntry> gantt;

public:
    Simulator(const vector<Process>& processList,
        SchedulingPolicy schedulingPolicy, int timeQuantum, int memorySize, string logfile);

    // Utility functions
    string policyToString(SchedulingPolicy p);
    string formatQueue(const vector<Process*>& q);

    // Main simulation function
    void printGanttPerProcess() const;
    void run();

    void exportGantttoCSV(string filename);
};