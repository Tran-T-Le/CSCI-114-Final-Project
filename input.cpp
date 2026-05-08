#include "input.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// 1. Will Load menu.txt
vector<MenuItem> loadMenu(string filename) {
    vector<MenuItem> menu;

    ifstream fin(filename);

    if (!fin.is_open()) {
        cout << "Error: cannot open menu file\n";
        return menu;
    }

    MenuItem item;

    while (fin >> item.name >> item.cookTime) {
        menu.push_back(item);
    }

    fin.close();
    return menu;
}

// 2. Calculates each processes busst time

// Functions to convert time to global time
int convertToMinutes(const string& timeStr) {
    int hour = stoi(timeStr.substr(0, 2));
    int minute = stoi(timeStr.substr(3, 2));

    return hour * 60 + minute;
}

int convertToGlobalTime(const string& realTime, const string& openTime) {
    return convertToMinutes(realTime) - convertToMinutes(openTime);
}

// Functions to calculate burst time
int getCookTime(const string& itemName, const vector<MenuItem>& menu) {
    for (const auto& item : menu) {
        if (item.name == itemName) {
            return item.cookTime;
        }
    }

    return 0;
}

int calculateBurstTime(const vector<OrderItem>& items,
    const vector<MenuItem>& menu) {
    int total = 0;

    for (const auto& item : items) {
        int cookTime = getCookTime(item.name, menu);

        total += cookTime * item.quantity;
    }

    return total;
}

// 3. Calculates Memory needed
int calculateMemoryNeeded(const vector<OrderItem>& items)
{
    int totalQuantity = 0;

    for (const auto& item : items)
    {
        totalQuantity += item.quantity;
    }
    return totalQuantity * 100; // each item = 100 units
}

// 4. Loads "Orders", which are processes
vector<Process> loadOrders(const string& filename, const vector<MenuItem>& menu, const string& openTime) {
    vector<Process> processes;
    ifstream fin(filename);
    
    if (!fin.is_open()) {
        cout << "Error: cannot open orders file\n";
        return processes;
    }
    int n;
    fin >> n;
    for (int i = 0; i < n; i++) {
        Process p;
        fin >> p.pid >> p.arrivalReal >> p.distinctItemCount;
        p.items.clear();
        for (int j = 0; j < p.distinctItemCount; j++) {
            OrderItem item;
            fin >> item.name >> item.quantity;
            p.items.push_back(item);
        }
        // converts time
        p.arrivalTime = convertToGlobalTime(p.arrivalReal, openTime);
        
        // calculates burst
        p.burstTime = calculateBurstTime(p.items, menu);
        p.remainingTime = p.burstTime;
        
        // calculates memory
        p.memoryNeeded = calculateMemoryNeeded(p.items);
        processes.push_back(p);
    }
    fin.close();
    return processes;
}

void printProcesses(vector<Process> processes) {
    for (const auto& p : processes) {
        cout << "P" << p.pid << ": arrival=" << p.arrivalTime
            << " burst=" << p.burstTime
            << " memory=" << p.memoryNeeded << endl;
    }
}
