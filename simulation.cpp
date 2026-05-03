#include "simulation.h"
#include <algorithm>

Simulator::Simulator(const vector<Process>& processList,
    SchedulingPolicy schedulingPolicy, int timeQuantum, int memorySize, string logfile)
    : processes(processList), running(nullptr), paymentProcess(nullptr),
      stove("Stove"), paymentTerminal("PaymentTerminal"), logger(logfile), mem(memorySize),
      policy(schedulingPolicy), quantum(timeQuantum), currentTime(0), finishedCount(0),
      quantumCounter(0), paymentTimeLeft(0)
{
}

string Simulator::stateToString(ProcessState state) const
{
    switch (state)
    {
    case NEW:
        return "NEW";
    case READY:
        return "READY";
    case RUNNING:
        return "RUNNING";
    case BLOCKED_MEMORY:
        return "BLOCKED_MEMORY";
    case BLOCKED_RESOURCE:
        return "BLOCKED_RESOURCE";
    case TERMINATED:
        return "TERMINATED";
    default:
        return "UNKNOWN";
    }
}

string Simulator::policyToString(SchedulingPolicy p) const
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

string Simulator::formatQueue(const vector<Process*>& q) const
{
    if (q.empty())
    {
        return "EMPTY";
    }

    string s;
    for (auto p : q)
    {
        s += "P" + to_string(p->pid) + " ";
    }

    if (!s.empty() && s.back() == ' ')
    {
        s.pop_back();
    }

    return s;
}

void Simulator::addToQueue(vector<Process*>& q, Process* p)
{
    if (find(q.begin(), q.end(), p) == q.end())
    {
        q.push_back(p);
    }
}

bool Simulator::allocateMemoryFor(Process* p, const string& reason)
{
    if (p->memoryAllocated)
    {
        p->state = READY;
        addToQueue(ready, p);
        logger.log(currentTime, p->pid, reason, "READY", "MEMORY_ALREADY_ALLOCATED", p->memoryNeeded, formatQueue(ready));
        return true;
    }

    int start = mem.allocate(p->pid, p->memoryNeeded);

    if (start != -1)
    {
        p->memoryAllocated = true;
        p->state = READY;
        addToQueue(ready, p);
        logger.log(currentTime, p->pid, reason, "READY", "MEMORY_ALLOCATED_START_" + to_string(start), p->memoryNeeded, formatQueue(ready));
        cout << "[t=" << currentTime << "] P" << p->pid << " allocated " << p->memoryNeeded
             << " memory units using First-Fit.\n";
        return true;
    }

    p->state = BLOCKED_MEMORY;
    addToQueue(blockedMemory, p);
    logger.log(currentTime, p->pid, reason, "BLOCKED_MEMORY", "MEMORY_FULL", p->memoryNeeded, formatQueue(blockedMemory));
    cout << "[t=" << currentTime << "] P" << p->pid << " blocked: not enough contiguous memory.\n";
    return false;
}

void Simulator::retryMemoryBlocked()
{
    vector<Process*> stillBlocked;

    for (auto p : blockedMemory)
    {
        int start = mem.allocate(p->pid, p->memoryNeeded);

        if (start != -1)
        {
            p->memoryAllocated = true;
            p->state = READY;
            addToQueue(ready, p);
            logger.log(currentTime, p->pid, "RETRY_MEMORY", "READY", "MEMORY_ALLOCATED_START_" + to_string(start), p->memoryNeeded, formatQueue(ready));
            cout << "[t=" << currentTime << "] P" << p->pid << " unblocked from memory wait.\n";
        }
        else
        {
            stillBlocked.push_back(p);
        }
    }

    blockedMemory = stillBlocked;
}

void Simulator::startPaymentIfPossible()
{
    if (paymentProcess != nullptr || blockedPayment.empty() || paymentTerminal.isBusy())
    {
        return;
    }

    Process* p = blockedPayment.front();
    blockedPayment.erase(blockedPayment.begin());
    paymentTerminal.removeWaiting(p->pid);

    if (paymentTerminal.request(p->pid))
    {
        paymentProcess = p;
        paymentTimeLeft = 1; // one simulated minute at the payment terminal
        p->state = BLOCKED_RESOURCE;
        logger.log(currentTime, p->pid, "ACQUIRE_RESOURCE", "BLOCKED_RESOURCE", "PaymentTerminal", p->memoryNeeded, paymentTerminal.getQueueString());
        cout << "[t=" << currentTime << "] P" << p->pid << " acquired PaymentTerminal.\n";
    }
}

void Simulator::finishPaymentIfNeeded()
{
    if (paymentProcess == nullptr)
    {
        return;
    }

    paymentTimeLeft--;

    if (paymentTimeLeft <= 0)
    {
        Process* p = paymentProcess;
        paymentTerminal.release(p->pid);
        p->paymentDone = true;
        paymentProcess = nullptr;

        logger.log(currentTime, p->pid, "RELEASE_RESOURCE", "BLOCKED_RESOURCE", "PaymentTerminal", p->memoryNeeded, paymentTerminal.getQueueString());
        cout << "[t=" << currentTime << "] P" << p->pid << " released PaymentTerminal.\n";

        allocateMemoryFor(p, "PAYMENT_DONE");
        startPaymentIfPossible();
    }
}

bool Simulator::acquireStoveFor(Process* p)
{
    if (stove.request(p->pid))
    {
        logger.log(currentTime, p->pid, "ACQUIRE_RESOURCE", "RUNNING", "Stove", p->memoryNeeded, stove.getQueueString());
        return true;
    }

    p->state = BLOCKED_RESOURCE;
    addToQueue(blockedStove, p);
    logger.log(currentTime, p->pid, "BLOCKED_RESOURCE", "BLOCKED_RESOURCE", "Stove_BUSY", p->memoryNeeded, stove.getQueueString());
    cout << "[t=" << currentTime << "] P" << p->pid << " blocked: Stove is busy.\n";
    return false;
}

void Simulator::retryStoveBlocked()
{
    if (blockedStove.empty() || stove.isBusy())
    {
        return;
    }

    vector<Process*> stillBlocked;

    for (auto p : blockedStove)
    {
        if (!stove.isBusy() && stove.request(p->pid))
        {
            p->state = READY;
            addToQueue(ready, p);
            logger.log(currentTime, p->pid, "RETRY_RESOURCE", "READY", "Stove", p->memoryNeeded, formatQueue(ready));
        }
        else
        {
            stillBlocked.push_back(p);
        }
    }

    blockedStove = stillBlocked;
}

void Simulator::run()
{
    const int MAX_TIME = 10000;

    cout << "\nPolicy: " << policyToString(policy);
    if (policy == RR)
    {
        cout << " | Quantum: " << quantum;
    }
    cout << "\n";

    while (finishedCount < (int)processes.size() && currentTime < MAX_TIME)
    {
        cout << "\n========== Time " << currentTime << " ==========" << endl;

        // 1. Retry processes that were blocked only because memory was unavailable.
        retryMemoryBlocked();

        // 2. Retry processes that were blocked only because the stove was unavailable.
        retryStoveBlocked();

        // 3. Bring in new arrivals.
        for (auto& p : processes)
        {
            if (p.arrivalTime == currentTime && p.state == NEW)
            {
                cout << "[t=" << currentTime << "] P" << p.pid << " arrived at restaurant kiosk.\n";
                logger.log(currentTime, p.pid, "ARRIVAL", "NEW", "KIOSK", p.memoryNeeded, "-");

                if (paymentTerminal.isBusy() || paymentProcess != nullptr || !blockedPayment.empty())
                {
                    p.state = BLOCKED_RESOURCE;
                    addToQueue(blockedPayment, &p);
                    paymentTerminal.addWaiting(p.pid);
                    logger.log(currentTime, p.pid, "BLOCKED_RESOURCE", "BLOCKED_RESOURCE", "PaymentTerminal_BUSY", p.memoryNeeded, paymentTerminal.getQueueString());
                    cout << "[t=" << currentTime << "] P" << p.pid << " blocked: PaymentTerminal is busy.\n";
                }
                else
                {
                    p.state = BLOCKED_RESOURCE;
                    addToQueue(blockedPayment, &p);
                }
            }
        }

        // 4. Start one payment-terminal job if the terminal is free.
        startPaymentIfPossible();

        // 5. If CPU is free, pick the next ready process.
        if (running == nullptr)
        {
            running = selectProcess(ready, policy);

            if (running != nullptr)
            {
                if (!acquireStoveFor(running))
                {
                    running = nullptr;
                }
                else
                {
                    running->state = RUNNING;
                    quantumCounter = 0;
                    logger.log(currentTime, running->pid, "SCHEDULE", "RUNNING", policyToString(policy), running->memoryNeeded, formatQueue(ready));
                    cout << "[t=" << currentTime << "] Scheduler selected P" << running->pid
                         << " using " << policyToString(policy) << ".\n";
                }
            }
        }

        // 6. Execute one time unit on the one logical CPU.
        if (running != nullptr)
        {
            running->remainingTime--;
            quantumCounter++;

            logger.log(currentTime, running->pid, "RUN", "RUNNING", "CPU/Stove", running->memoryNeeded, formatQueue(ready));
            cout << "[t=" << currentTime << "] P" << running->pid << " RUNNING, remaining time = "
                 << running->remainingTime << ".\n";

            if (gantt.empty() || gantt.back().pid != running->pid || gantt.back().end != currentTime)
            {
                gantt.push_back({ running->pid, currentTime, currentTime + 1 });
            }
            else
            {
                gantt.back().end++;
            }

            if (running->remainingTime <= 0)
            {
                running->state = TERMINATED;
                stove.release(running->pid);
                mem.release(running->pid);
                running->memoryAllocated = false;

                logger.log(currentTime, running->pid, "FINISH", "TERMINATED", "Release_Stove_And_Memory", 0, formatQueue(ready));
                cout << "[t=" << currentTime << "] P" << running->pid
                     << " TERMINATED. Memory and Stove released.\n";

                running = nullptr;
                finishedCount++;
                quantumCounter = 0;
            }
            else if (policy == RR && quantumCounter >= quantum)
            {
                running->state = READY;
                stove.release(running->pid);
                addToQueue(ready, running);
                logger.log(currentTime, running->pid, "TIME_SLICE_EXPIRED", "READY", "RR_RELEASE_STOVE", running->memoryNeeded, formatQueue(ready));
                cout << "[t=" << currentTime << "] P" << running->pid
                     << " time slice expired. Moved back to READY.\n";

                running = nullptr;
                quantumCounter = 0;
            }
        }
        else
        {
            logger.log(currentTime, -1, "CPU_IDLE", "IDLE", "CPU", 0, formatQueue(ready));
            cout << "[t=" << currentTime << "] CPU IDLE.\n";
        }

        // 7. Advance resource work that happens outside the CPU.
        finishPaymentIfNeeded();

        mem.display(currentTime);
        currentTime++;
    }

    if (currentTime >= MAX_TIME)
    {
        cout << "\nERROR: Simulation stopped because it reached the safety time limit.\n";
        cout << "This usually means a process is stuck in a queue.\n";
    }

    cout << "\n========== FINAL GANTT CHART ==========" << endl;
    printGanttPerProcess();
}

void Simulator::printGanttPerProcess() const
{
    if (gantt.empty())
    {
        cout << "No execution history.\n";
        return;
    }

    int maxTime = gantt.back().end;
    map<int, vector<char>> timeline;

    for (const auto& p : processes)
    {
        timeline[p.pid] = vector<char>(maxTime, '-');
    }

    for (const auto& g : gantt)
    {
        for (int t = g.start; t < g.end; t++)
        {
            timeline[g.pid][t] = 'X';
        }
    }

    cout << "Time: ";
    for (int t = 0; t < maxTime; t++)
    {
        cout << setw(2) << t << " ";
    }
    cout << "\n";

    for (const auto& p : processes)
    {
        cout << "P" << p.pid << ": ";
        for (int t = 0; t < maxTime; t++)
        {
            cout << setw(2) << timeline.at(p.pid)[t] << " ";
        }
        cout << "\n";
    }
}

void Simulator::exportGantttoCSV(string filename)
{
    ofstream file(filename);
    file << "pid,start,end" << endl;

    for (auto& g : gantt)
    {
        file << g.pid << "," << g.start << "," << g.end << "\n";
    }

    file.close();
}
