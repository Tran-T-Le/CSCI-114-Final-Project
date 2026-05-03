#include "simulation.h"

void Simulator::Systemstate() {
    cout << endl << "---- SYSTEM STATE ----" << endl;

    cout << "Running: ";
    if (running) cout << "P" << running->pid << endl;
    else cout << "NONE" << endl;

    cout << "Ready Queue: " << formatQueue(ready) << endl;
    cout << "Blocked (Memory): " << formatQueue(blockedMemory) << endl;
    cout << "Blocked (Stove): " << formatQueue(blockedStove) << endl;
    cout << "Blocked (Payment): " << formatQueue(blockedPayment) << endl;

    cout << "---------------------" << endl;;
}

Simulator::Simulator(const vector<Process>& processList,
    SchedulingPolicy schedulingPolicy, int timeQuantum, int memorySize, string logfile)
    : processes(processList), policy(schedulingPolicy), quantum(timeQuantum),
    mem(memorySize), CPUStove("Stove"), PaymentTerminal("PaymentTerminal"), logger(logfile)
{
    currentTime = 0;
    finishedCount = 0;
    quantumCounter = 0;
    running = nullptr;
}

string Simulator::policyToString(SchedulingPolicy p)
{
    switch (p)
    {
    case FCFS: return "FCFS";
    case SJF:  return "SJF";
    case RR:   return "RR";
    default:   return "UNKNOWN";
    }
}

string Simulator::formatQueue(const vector<Process*>& q)
{
    if (q.empty()) return "EMPTY";
    string s;
    for (auto p : q) s += "P" + to_string(p->pid) + " ";
    if (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

void Simulator::run()
{
    blockedMemory.clear();
    blockedStove.clear();
    blockedPayment.clear();

    ready.clear();
    running = nullptr;

    quantumCounter = 0;
    int time = 0;
    int done = 0;

    while (done < (int)processes.size())
    {
        cout << "\n========== Time " << time << " ==========\n";

        {
            vector<Process*> still;
            for (auto* p : blockedMemory)
            {
                int slot = mem.allocate(p->pid, p->memoryNeeded);
                if (slot != -1)
                {
                    p->state = READY;
                    ready.push_back(p);
                    logger.log(time, p->pid, "MEM_RETRY", "READY",
                        "MEM", p->memoryNeeded, formatQueue(ready));
                }
                else still.push_back(p);
            }
            blockedMemory = still;
        }

        {
            vector<Process*> still;
            for (auto* p : blockedPayment)
            {
                if (!PaymentTerminal.isBusy())
                {
                    PaymentTerminal.request(p->pid);
                    PaymentTerminal.release();

                    int slot = mem.allocate(p->pid, p->memoryNeeded);

                    if (slot == -1)
                    {
                        p->state = BLOCKED_MEMORY;
                        blockedMemory.push_back(p);
                    }
                    else
                    {
                        p->state = READY;
                        ready.push_back(p);
                    }

                    logger.log(time, p->pid, "PAYMENT_DONE", "READY",
                        "PaymentTerminal", p->memoryNeeded, formatQueue(ready));
                }
                else
                {
                    still.push_back(p);
                }
            }
            blockedPayment = still;
        }

        {
            vector<Process*> still;
            for (auto* p : blockedStove)
            {
                if (!CPUStove.isBusy())
                {
                    CPUStove.request(p->pid);
                    p->state = READY;
                    ready.push_back(p);

                    logger.log(time, p->pid, "RETRY_RESOURCE", "READY",
                        "Stove", p->memoryNeeded, formatQueue(ready));
                }
                else
                {
                    still.push_back(p);
                }
            }
            blockedStove = still;
        }

        for (auto& p : processes)
        {
            if (p.arrivalTime != time || p.state != NEW) continue;

            cout << "[t=" << time << "] P" << p.pid << " arrived.\n";

            if (!PaymentTerminal.request(p.pid))
            {
                p.state = BLOCKED_RESOURCE;
                blockedPayment.push_back(&p);

                logger.log(time, p.pid, "BLOCKED_RESOURCE", "BLOCKED_RESOURCE",
                    "PaymentTerminal", p.memoryNeeded, formatQueue(blockedPayment));
            }
            else
            {
                logger.log(time, p.pid, "ACQUIRE_RESOURCE", "BLOCKED_RESOURCE",
                    "PaymentTerminal", p.memoryNeeded, "-");

                PaymentTerminal.release();

                int slot = mem.allocate(p.pid, p.memoryNeeded);

                if (slot == -1)
                {
                    p.state = BLOCKED_MEMORY;
                    blockedMemory.push_back(&p);
                }
                else
                {
                    p.state = READY;
                    ready.push_back(&p);
                }
            }
        }

        if (running == nullptr)
        {
            Process* candidate = selectProcess(ready, policy);

            if (candidate != nullptr)
            {
                if (!CPUStove.request(candidate->pid))
                {
                    candidate->state = BLOCKED_RESOURCE;
                    
                    blockedStove.push_back(candidate);

                    logger.log(time, candidate->pid, "BLOCKED_RESOURCE", "BLOCKED_RESOURCE",
                        "Stove", candidate->memoryNeeded,
                        formatQueue(blockedStove));
                }
                else
                {
                    candidate->state = RUNNING;
                    running = candidate;
                    quantumCounter = 0;

                    logger.log(time, running->pid, "SCHEDULE", "RUNNING",
                        policyToString(policy), running->memoryNeeded, formatQueue(ready));
                }
            }
        }

        if (running != nullptr)
        {
            running->remainingTime--;
            if (policy == RR) quantumCounter++;

            logger.log(time, running->pid, "RUN", "RUNNING",
                "STOVE", running->memoryNeeded, formatQueue(ready));

            if (gantt.empty() || gantt.back().pid != running->pid)
                gantt.push_back({ running->pid, time, time + 1 });
            else
                gantt.back().end++;

            if (running->remainingTime == 0)
            {
                running->state = TERMINATED;

                CPUStove.release();
                mem.release(running->pid);

                logger.log(time, running->pid, "FINISH", "TERMINATED",
                    "RELEASE_ALL", 0, formatQueue(ready));

                running = nullptr;
                done++;
                quantumCounter = 0;
            }


            else if (policy == RR && quantumCounter >= quantum)
            {
                CPUStove.release();

                running->state = READY;
                ready.push_back(running);

                logger.log(time, running->pid, "TIME_SLICE", "READY",
                    "RR", running->memoryNeeded, formatQueue(ready));

                running = nullptr;    
                quantumCounter = 0;        
            }
        }
        else
        {
            cout << "[t=" << time << "] CPU IDLE\n";
        }
        
        mem.display(time);
        Systemstate();
        time++;
    }

    printGanttPerProcess();
}

void Simulator::printGanttPerProcess() const
{
    cout << "\n========== FINAL GANTT ==========\n";

    if (gantt.empty())
    {
        cout << "No execution history.\n";
        return;
    }

    int maxTime = gantt.back().end;
    map<int, vector<char>> timeline;

    for (const auto& p : processes)
        timeline[p.pid] = vector<char>(maxTime, '-');

    for (const auto& g : gantt)
        for (int t = g.start; t < g.end; t++)
            timeline[g.pid][t] = 'X';

    for (const auto& p : processes)
    {
        cout << "P" << p.pid << ": ";
        for (int t = 0; t < maxTime; t++)
            cout << timeline[p.pid][t] << " ";
        cout << "\n";
    }
}

void Simulator::exportGantttoCSV(string filename)
{
    ofstream file(filename);
    file << "pid,start,end\n";

    for (auto& g : gantt)
        file << g.pid << "," << g.start << "," << g.end << "\n";

    file.close();
}