#include <iostream>
#include "input.h"
#include "simulation.h"

using namespace std;

int main()
{
    string openTime = "08:00";
    auto menu = loadMenu("menu.txt");
    int memorySize = 1024;

    cout << "Choose run mode:\n";
    cout << "  1 = Run all three policies sequentially (FCFS, SJF, RR)\n";
    cout << "  2 = Run a single policy\n";
    cout << "  3 = Stress-test: forces memory-blocking (small memory, many big orders)\n";
    int mode;
    cin >> mode;

    if (mode == 1)
    {
        cout << "\n========== FCFS ==========\n";
        vector<Process> p1 = loadOrders("input.txt", menu, openTime);
        Simulator sim1(p1, FCFS, 0, memorySize, "fcfs_log.csv");
        sim1.run();
        sim1.exportGantttoCSV("gantt_fcfs.csv");

        cout << "\n========== SJF ==========\n";
        vector<Process> p2 = loadOrders("input.txt", menu, openTime);
        Simulator sim2(p2, SJF, 0, memorySize, "sjf_log.csv");
        sim2.run();
        sim2.exportGantttoCSV("gantt_sjf.csv");

        cout << "\n========== RR (quantum=3) ==========\n";
        vector<Process> p3 = loadOrders("input.txt", menu, openTime);
        Simulator sim3(p3, RR, 3, memorySize, "rr_log.csv");
        sim3.run();
        sim3.exportGantttoCSV("gantt_rr.csv");
    }
    else if (mode == 2)
    {
        cout << "Select policy: 1=FCFS  2=SJF  3=RR : ";
        int choice; cin >> choice;

        int quantum = 0;
        if (choice == 3)
        {
            cout << "Enter RR time quantum: ";
            cin >> quantum;
        }

        vector<Process> processes = loadOrders("input.txt", menu, openTime);
        Simulator sim(processes,
            choice == 1 ? FCFS : choice == 2 ? SJF : RR,
            quantum, memorySize, "log.csv");
        sim.run();
        sim.exportGantttoCSV("gantt.csv");
    }

    else if (mode == 3)
    {
        cout << "Select policy: 1=FCFS  2=SJF  3=RR : ";
        int choice; cin >> choice;

        int quantum = 0;
        if (choice == 3)
        {
            cout << "Enter RR time quantum: ";
            cin >> quantum;
        }

        int stressMem = 600;
        cout << "\n[STRESS TEST] Memory limited to " << stressMem
            << " units — late arrivals will be BLOCKED_MEMORY until space frees.\n\n";

        vector<Process> processes = loadOrders("input_stress.txt", menu, openTime);
        Simulator sim(processes,
            choice == 1 ? FCFS : choice == 2 ? SJF : RR,
            quantum, stressMem, "stress_log.csv");
        sim.run();
        sim.exportGantttoCSV("gantt_stress.csv");
    }
    else
    {
        cout << "Invalid choice.\n";
        return 1;
    }

    cout << "\nSimulation complete. CSV logs written.\n";
    return 0;
}