
#include "simulation.h"

Simulator::Simulator(const vector<Process> &processList,
                     SchedulingPolicy schedulingPolicy,
                     int timeQuantum,
                     int memorySize)
    : processes(processList), policy(schedulingPolicy), quantum(timeQuantum), mem(memorySize)
{
    currentTime = 0;
    finishedCount = 0;
    quantumCounter = 0;
    running = nullptr;
}

void Simulator::printGanttPerProcess() const
{
    if (gantt.empty())
    {
        cout << "\n========== FINAL GANTT (PER PROCESS) ==========\n";
        cout << "No execution history.\n";
        return;
    }

    int maxTime = gantt.back().end;
    map<int, vector<char>> timeline;

    // Create all rows first
    for (const auto &p : processes)
    {
        timeline[p.pid] = vector<char>(maxTime, '-');
    }

    // Mark running slots
    for (const auto &g : gantt)
    {
        for (int t = g.start; t < g.end; t++)
        {
            timeline[g.pid][t] = 'X';
        }
    }

    for (const auto &p : processes)
    {
        cout << "P" << p.pid << ":  ";
        for (int t = 0; t < maxTime; t++)
        {
            cout << timeline[p.pid][t] << " ";
        }
        cout << "\n";
    }
}
string Simulator::policyToString(SchedulingPolicy p)
{
    switch (p)
    {
    case FCFS:
        return "FCFS";
    case SJF:
        return "SJF";
    case RR:
        return "RR";
    default:
        return "UNKNOWN";
    }
}
string Simulator::formatQueue(const vector<Process *> &q)
{
    if (q.empty())
        return "EMPTY";

    string s = "";
    for (auto p : q)
    {
        s += "P" + to_string(p->pid) + " ";
    }

    if (!s.empty() && s.back() == ' ')
        s.pop_back();

    return s;
}

void Simulator::run()
{
    vector<Process *> ready;
    vector<Process *> blocked;

    Process *running = nullptr;

    int time = 0;
    int done = 0;
    int quantumCounter = 0;

    while (done < (int)processes.size())
    {
        // Retry blocked processes first
        vector<Process *> stillBlocked;
        for (auto p : blocked)
        {
            int start = mem.allocate(p->pid, p->memoryNeeded);

            if (start != -1)
            {
                p->state = READY;
                ready.push_back(p);

                cout << "[t=" << time << "] job=P" << p->pid
                     << " RETRY after BLOCKED_MEMORY; memory allocated ["
                     << start << ".." << start + p->memoryNeeded - 1 << "]"
                     << " -> READY; ReadyQ=" << formatQueue(ready) << "\n";
            }
            else
            {
                stillBlocked.push_back(p);
            }
        }
        blocked = stillBlocked;

        // Check newly arrived processes
        for (auto &p : processes)
        {
            if (p.arrivalTime == time && p.state == NEW)
            {
                int start = mem.allocate(p.pid, p.memoryNeeded);

                if (start != -1)
                {
                    p.state = READY;
                    ready.push_back(&p);

                    cout << "[t=" << time << "] job=P" << p.pid
                         << " ARRIVED; memory allocated ["
                         << start << ".." << start + p.memoryNeeded - 1 << "]"
                         << " -> READY; ReadyQ=" << formatQueue(ready) << "\n";
                }
                else
                {
                    p.state = BLOCKED_MEMORY;
                    blocked.push_back(&p);

                    cout << "[t=" << time << "] job=P" << p.pid
                         << " ARRIVED; insufficient memory"
                         << " -> BLOCKED_MEMORY; BlockedQ=" << formatQueue(blocked) << "\n";
                }
            }
        }

        // If CPU is free, scheduler selects a process
        if (running == nullptr)
        {
            running = selectProcess(ready, policy);

            if (running != nullptr)
            {
                running->state = RUNNING;
                quantumCounter = 0;
                if (policy == RR)
                {
                    cout << "[t=" << time << "] scheduler(" << policyToString(policy)
                         << ") SELECTED job=P" << running->pid
                         << " with time quantum=" << quantum
                         << " (ReadyQ=" << formatQueue(ready) << ")\n";
                }
                else
                {
                    cout << "[t=" << time << "] scheduler(" << policyToString(policy)
                         << ") SELECTED job=P" << running->pid
                         << " (ReadyQ=" << formatQueue(ready) << ")\n";
                }
            }
        }

        // Run process
        if (running != nullptr)
        {
            cout << "[t=" << time << "] job=P" << running->pid
                 << " RUNNING; remaining_burst_time=" << running->remainingTime << "\n";

            running->remainingTime--;
            quantumCounter++;

            if (gantt.empty())
            {
                gantt.push_back({running->pid, time, time + 1});
            }
            else if (gantt.back().pid != running->pid || gantt.back().end != time)
            {
                gantt.push_back({running->pid, time, time + 1});
            }
            else
            {
                gantt.back().end++;
            }

            if (running->remainingTime == 0)
            {
                running->state = TERMINATED;
                mem.release(running->pid);

                cout << "[t=" << time << "] job=P" << running->pid
                     << " FINISHED -> TERMINATED\n";

                running = nullptr;
                done++;
                quantumCounter = 0;
            }
            else if (policy == RR && quantumCounter == quantum)
            {
                running->state = READY;
                ready.push_back(running);

                cout << "[t=" << time << "] job=P" << running->pid
                     << " TIME_SLICE_EXPIRED -> READY; ReadyQ="
                     << formatQueue(ready) << "\n";

                running = nullptr;
                quantumCounter = 0;
            }
        }
        else
        {
            cout << "[t=" << time << "] CPU IDLE\n";
        }

        mem.display(time);
        cout << "\n";
        time++;
    }
}