#include <iostream>
#include "input.h"
#include "simulation.h"

using namespace std;

int main()
{
    string openTime = "08:00";
    auto menu = loadMenu("menu.txt");
    int memorySize = 1024;

    vector<Process> p1 = loadOrders("input.txt", menu, openTime);
    cout << "FCFS Simulation:\n";
    runSimulation(p1, FCFS, 2, memorySize);

    // cout << "\n\nSJF Simulation:\n";
    // vector<Process> p2 = loadOrders("input.txt", menu, openTime);
    // runSimulation(p2, SJF, 2, memorySize);

    // cout << "\n\nRR Simulation:\n";
    // vector<Process> p3 = loadOrders("input.txt", menu, openTime);
    // runSimulation(p3, RR, 2, memorySize);

    return 0;
}