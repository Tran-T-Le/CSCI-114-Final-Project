#include "simulation.h"

// Constructor
Simulator::Simulator(const vector<Process>& processList,
    SchedulingPolicy schedulingPolicy, int timeQuantum, int memorySize, string logfile)
    : processes(processList), policy(schedulingPolicy), quantum(timeQuantum),
    mem(memorySize), CPUStove("Stove"), logger(logfile)
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
    for (const auto& p : processes)
    {
        timeline[p.pid] = vector<char>(maxTime, '-');
    }

    // Mark running slots
    for (const auto& g : gantt)
    {
        for (int t = g.start; t < g.end; t++)
        {
            timeline[g.pid][t] = 'X';
        }
    }

    for (const auto& p : processes)
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

string Simulator::formatQueue(const vector<Process*>& q)
{
    if (q.empty()) return "EMPTY";
    string s;
    for (auto p : q) s += "P" + to_string(p->pid) + " ";
    if (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

// Main simulation loop
void Simulator::run()
{
    vector<Process*> ready;
    vector<Process*> blocked;
    Process* running = nullptr;

    int quantumCounter = 0;
    int time = 0;
    int done = 0;

    while (done < processes.size())
    {
        // Retry blocked processes
        vector<Process*> stillBlocked;
        for (auto p : blocked) {
            int start = mem.allocate(p->pid, p->memoryNeeded);
            if (start != -1) {
                p->state = READY;
                ready.push_back(p);
                logger.log(time, p->pid, "RETRY", "READY", "MEM", p->memoryNeeded, formatQueue(ready));
            }
            else {
                stillBlocked.push_back(p);
            }
        }
        blocked = stillBlocked;

        // Check new arrivals
        for (auto& p : processes) {
            if (p.arrivalTime == time && p.state == NEW) {
                int start = mem.allocate(p.pid, p.memoryNeeded);
                if (start != -1) {
                    p.state = READY;
                    ready.push_back(&p);
                    logger.log(time, p.pid, "ARRIVAL", "READY", "MEM", p.memoryNeeded, formatQueue(ready));
                }
                else {
                    p.state = BLOCKED_MEMORY;
                    blocked.push_back(&p);
                    logger.log(time, p.pid, "ARRIVAL", "BLOCKED", "MEM", p.memoryNeeded, formatQueue(blocked));
                }
            }
        }

        // Scheduler picks process
        if (running == nullptr) {
            running = selectProcess(ready, policy);
            if (running != nullptr) {
                if (CPUStove.isBusy() && CPUStove.getHolder() != running->pid) {
                    running->state = BLOCKED_MEMORY;
                    blocked.push_back(running);
                    running = nullptr;
                    continue;
                }
                if (!CPUStove.request(running->pid)) {
                    running->state = BLOCKED_MEMORY;
                    blocked.push_back(running);
                    running = nullptr;
                    continue;
                }
                running->state = RUNNING;
                quantumCounter = 0;
                logger.log(time, running->pid, "SCHEDULE", "RUNNING", policyToString(policy),
                    running->memoryNeeded, formatQueue(ready));
            }
        }

        // Run process
        if (running != nullptr) {
            running->remainingTime--;
            if (policy == RR) {
                quantumCounter++;
            }

            logger.log(time, running->pid, "RUN", "RUNNING", "STOVE", running->memoryNeeded, formatQueue(ready));

            if (gantt.empty() || gantt.back().pid != running->pid)
                gantt.push_back({ running->pid, time, time + 1 });
            else
                gantt.back().end++;

            if (running->remainingTime == 0) {
                running->state = TERMINATED;
                mem.release(running->pid);
                CPUStove.release();

                logger.log(time, running->pid, "FINISH", "TERMINATED", "STOVE", 0, formatQueue(ready));
                
                running = nullptr;
                done++;
                quantumCounter = 0;
            }
            else if (policy == RR && quantumCounter == quantum) {
                running->state = READY;
                ready.push_back(running);
                logger.log(time, running->pid, "TIME_SLICE", "READY", "RR", running->memoryNeeded, formatQueue(ready));
                running = nullptr;
                quantumCounter = 0;
            }
        }
        else {
            cout << "[t=" << time << "] CPU IDLE\n";
        }

        mem.display(time);
        cout << endl;
        time++;
    }
}

// Export Gantt chart CSV
void Simulator::exportGantttoCSV(string filename) {
    ofstream file(filename);
    file << "pid,start,end" << endl;
    for (auto& g : gantt)
        file << g.pid << "," << g.start << "," << g.end << "\n";
    file.close();
}