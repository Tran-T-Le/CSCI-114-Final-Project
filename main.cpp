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
    // Debug: print loaded processes
    for (const auto &p : p1)
    {
        cout << "P" << p.pid << ": arrival=" << p.arrivalTime
             << " burst=" << p.burstTime
             << " memory=" << p.memoryNeeded << endl;
    }

    cout << "\n========== FCFS Simulation ==========\n";
    cout << "[config] scheduler=FCFS\n";
    cout << "[config] memory_size=" << memorySize << "\n";
    Simulator sim1(p1, FCFS, 0, memorySize);
    sim1.run();

    vector<Process> p2 = loadOrders("input.txt", menu, openTime);
    cout << "\n========== SJF Simulation ==========\n";
    cout << "[config] scheduler=SJF\n";
    cout << "[config] memory_size=" << memorySize << "\n";
    Simulator sim2(p2, SJF, 2, memorySize);
    sim2.run();

    vector<Process> p3 = loadOrders("input.txt", menu, openTime);
    cout << "\n========== RR Simulation ==========\n";
    cout << "[config] scheduler=RR\n";
    cout << "[config] memory_size=" << memorySize << "\n";
    Simulator sim(p3, RR, 3, 1024);
    sim.run();

    // Print Gantt charts to compare
    cout << "\nGantt Chart FCFS:\n";
    sim1.printGanttPerProcess();
    cout << "\nGantt Chart SJF:\n";
    sim2.printGanttPerProcess();
    cout << "\nGantt Chart RR:\n";
    sim.printGanttPerProcess();

    return 0;
}