#include <iostream>
#include "input.h"
#include "simulation.h"

using namespace std;

int main()
{
    string openTime = "08:00";
    auto menu = loadMenu("menu.txt");
    int memorySize = 1024;

    cout << "Choose run mode: " << endl;
    cout << "1 = Run all policies sequentially (FCFS, SJF, RR)" << endl;
    cout << "2 = Run a single policy" << endl;
    int mode;
    cin >> mode;

    if (mode == 1) {
        // Sequential run
        cout << "\n========== FCFS Simulation ==========\n";
        vector<Process> p1 = loadOrders("input.txt", menu, openTime);
        Simulator sim1(p1, FCFS, 0, memorySize, "fcfs_log.csv");
        sim1.run();
        sim1.exportGantttoCSV("gantt_fcfs.csv");

        cout << "\n========== SJF Simulation ==========\n";
        vector<Process> p2 = loadOrders("input.txt", menu, openTime);
        Simulator sim2(p2, SJF, 2, memorySize, "sjf_log.csv");
        sim2.run();
        sim2.exportGantttoCSV("gantt_sjf.csv");

        cout << "\n========== RR Simulation ==========\n";
        vector<Process> p3 = loadOrders("input.txt", menu, openTime);
        int rrQuantum = 3; // default
        Simulator sim3(p3, RR, rrQuantum, memorySize, "rr_log.csv");
        sim3.run();
        sim3.exportGantttoCSV("gantt_rr.csv");
    }
    else if (mode == 2) {
        // Single policy run
        cout << "Select scheduling policy: 1=FCFS, 2=SJF, 3=RR: ";
        int choice;
        cin >> choice;

        int quantum = 3; //default
        if (choice == 3) {
            cout << "Enter RR time quantum: ";
            cin >> quantum;
        }

        vector<Process> processes = loadOrders("input.txt", menu, openTime);

        Simulator sim(processes,
            choice == 1 ? FCFS : choice == 2 ? SJF : RR,
            quantum,
            memorySize,
            "log.csv");

        sim.run();
        sim.exportGantttoCSV("gantt.csv");
    }
    else {
        cout << "Invalid choice. Exiting." << endl;
        return 0;
    }

    cout << "Simulation finished. Check CSV log files." << endl;
    return 0;
}