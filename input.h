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
    int cookTime; // burst time
};

enum ProcessState
{
    NEW, // initialize state
    READY,
    RUNNING,
    BLOCKED_MEMORY,
    TERMINATED
};

struct Process
{
    int pid;
    string arrivalReal;
    int arrivalTime; // in minutes from open time
    int distinctItemCount;
    vector<OrderItem> items;
    int burstTime; // total cook time
    int remainingTime;
    int memoryNeeded; // memory needed based on items

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
        state = NEW;
    }
};

vector<MenuItem> loadMenu(string filename);

// Functions to convert time to global time
int convertToMinutes(const string& timeStr);
int convertToGlobalTime(const string& realTime, const string& openTime);
// Functions to calculate burst time
int getCookTime(const string& itemName, const vector<MenuItem>& menu);
int calculateBurstTime(const vector<OrderItem>& items,
    const vector<MenuItem>& menu);

// Function to calculate memory needed
int calculateMemoryNeeded(const vector<OrderItem>& items);

// Function to load orders (processes)
vector<Process> loadOrders(const string& filename,
    const vector<MenuItem>& menu,
    const string& openTime);

#endif