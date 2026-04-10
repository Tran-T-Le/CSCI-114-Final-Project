#include <iostream>
#include "input.h"
#include "simulation.h"

using namespace std;

int main()
{
    string openTime = "08:00";
    auto menu = loadMenu("menu.txt");
    int memorySize = 400;

    // vector<Process> p1 = loadOrders("input.txt", menu, openTime);
    // cout << "FCFS Simulation:\n";
    // runSimulation(p1, FCFS, 2, memorySize);

    // cout << "\n\nSJF Simulation:\n";
    // vector<Process> p2 = loadOrders("input.txt", menu, openTime);
    // runSimulation(p2, SJF, 2, memorySize);

    cout << "[config] scheduler=RR quantum=3\n";
    cout << "[config] total_memory=1024\n";
    vector<Process> p3 = loadOrders("input.txt", menu, openTime);
    for (const auto &p : p3)
    {
        cout << "P" << p.pid << ": arrival=" << p.arrivalTime
             << " burst=" << p.burstTime
             << " memory=" << p.memoryNeeded << endl;
    }
    Simulator sim(p3, RR, 3, 1024);
    sim.run();

    return 0;
}