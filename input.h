#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <string>

using namespace std;

struct OrderItem
{
    string name;
    int quantity;
};

struct MenuItem
{
    string name;
    int cookTime; // burst time for one item
};

enum ProcessState
{
    NEW,
    READY,
    RUNNING,
    BLOCKED_MEMORY,
    BLOCKED_RESOURCE,
    TERMINATED
};

struct Process
{
    int pid;
    string arrivalReal;
    int arrivalTime; // minutes from opening time
    int distinctItemCount;
    vector<OrderItem> items;

    int burstTime;      // total CPU/kitchen time needed
    int remainingTime;  // CPU/kitchen time left
    int memoryNeeded;   // prep-space memory requirement

    bool paymentDone;      // true after the payment terminal step finishes
    bool memoryAllocated;  // true after First-Fit memory allocation succeeds

    ProcessState state;

    Process()
    {
        pid = 0;
        arrivalReal = "";
        arrivalTime = 0;
        distinctItemCount = 0;
        burstTime = 0;
        remainingTime = 0;
        memoryNeeded = 0;
        paymentDone = false;
        memoryAllocated = false;
        state = NEW;
    }
};

vector<MenuItem> loadMenu(string filename);

int convertToMinutes(const string& timeStr);
int convertToGlobalTime(const string& realTime, const string& openTime);

int getCookTime(const string& itemName, const vector<MenuItem>& menu);
int calculateBurstTime(const vector<OrderItem>& items, const vector<MenuItem>& menu);
int calculateMemoryNeeded(const vector<OrderItem>& items);

vector<Process> loadOrders(const string& filename, const vector<MenuItem>& menu, const string& openTime);

#endif
