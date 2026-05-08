#include <iostream>
#include <vector>
#include <string>
#include "input.h"
#include "simulation.h"

using namespace std;

int main()
{
    string openTime = "08:00";
    auto menu = loadMenu("menu.txt");
    int memorySize = 1024;

    cout << "Choose test case:\n";
    cout << "  1 = Default input.txt\n";
    cout << "  2 = Blocked Payment test\n";
    cout << "  3 = Blocked Memory test\n";
    cout << "  4 = Blocked Stove test(only applicable for Round Robin)\n";
    cout << "Select test case: ";

    int testChoice;
    cin >> testChoice;

    string inputFile;
    string logFile;
    string ganttFile;

    if (testChoice == 1)
    {
        inputFile = "input/default_input.txt";
    }
    else if (testChoice == 2)
    {
        inputFile = "input/input_payment.txt";
    }
    else if (testChoice == 3)
    {
        inputFile = "input/input_memory.txt";
    }
    else if (testChoice == 4)
    {
        inputFile = "input/input_stove.txt";
    }
    else
    {
        cout << "Invalid test case.\n";
        return 1;
    }

    cout << "\nChoose scheduling policy:\n";
    cout << "  1 = FCFS\n";
    cout << "  2 = SJF\n";
    cout << "  3 = Round Robin\n";
    cout << "Select policy: ";

    int policyChoice;
    cin >> policyChoice;

    SchedulingPolicy policy;
    string policyName;
    int quantum = 0;

    if (policyChoice == 1)
    {
        policy = FCFS;
        policyName = "fcfs";
    }
    else if (policyChoice == 2)
    {
        policy = SJF;
        policyName = "sjf";
    }
    else if (policyChoice == 3)
    {
        policy = RR;
        policyName = "rr";

        cout << "Enter RR time quantum: ";
        cin >> quantum;
    }
    else
    {
        cout << "Invalid policy.\n";
        return 1;
    }

    logFile = "output/log.csv";
    ganttFile = "output/gantt.csv";

    cout << "\nRunning simulation...\n";
    cout << "Input file: " << inputFile << endl;
    cout << "Policy: " << policyName << endl;
    if (policy == RR)
    {
        cout << "Quantum: " << quantum << endl;
    }

    vector<Process> processes = loadOrders(inputFile, menu, openTime);

    Simulator sim(processes, policy, quantum, memorySize, logFile);
    sim.run();
    sim.exportGantttoCSV(ganttFile);

    cout << "\nInformation of process\n";
    cout << "PID\tArrival\tBurst\tMemory\tState\n";

    vector<Process> originalProcesses = loadOrders(inputFile, menu, openTime);

    for (const auto &p : originalProcesses)
    {
        cout << p.pid << "\t"
             << p.arrivalReal << "\t"
             << p.burstTime << "\t"
             << p.memoryNeeded << "\t"
             << "NEW" << endl;
    }

    cout << "\nSimulation complete.\n";
    cout << "CSV log written to: " << logFile << endl;
    cout << "Gantt CSV written to: " << ganttFile << endl;

    return 0;
}