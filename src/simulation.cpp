#include "simulation.h"

void Simulator::Systemstate()
{
    cout << endl
         << "---- SYSTEM STATE ----" << endl;

    cout << "Running: ";
    if (running)
        cout << "P" << running->pid << endl;
    else
        cout << "NONE" << endl;

    cout << "Ready Queue: " << formatQueue(ready) << endl;
    cout << "Blocked (Memory): " << formatQueue(blockedMemory) << endl;
    cout << "Blocked (Stove): " << formatQueue(blockedStove) << endl;
    cout << "Blocked (Payment): " << formatQueue(blockedPayment) << endl;

    cout << "---------------------" << endl;
    ;
}

Simulator::Simulator(const vector<Process> &processList,
                     SchedulingPolicy schedulingPolicy,
                     int timeQuantum,
                     int memorySize,
                     string logfile)
    : processes(processList),
      policy(schedulingPolicy),
      quantum(timeQuantum),
      mem(memorySize),
      PaymentTerminal("PaymentTerminal"),
      logger(logfile)
{
    currentTime = 0;
    finishedCount = 0;
    quantumCounter = 0;
    running = nullptr;

    paymentTimeLeft = 0;
    paymentStartTime = -1;

    stoves.push_back(ResourceManager("Stove1"));
    stoves.push_back(ResourceManager("Stove2"));
}

Process *Simulator::findProcessByPid(int pid)
{
    for (auto &p : processes)
    {
        if (p.pid == pid)
            return &p;
    }
    return nullptr;
}

bool Simulator::hasStove(int pid) const
{
    for (const auto &stove : stoves)
    {
        if (stove.getHolder() == pid)
            return true;
    }
    return false;
}

bool Simulator::acquireStove(Process *p)
{
    if (hasStove(p->pid))
        return true;

    for (auto &stove : stoves)
    {
        if (!stove.isBusy())
        {
            stove.request(p->pid);
            return true;
        }
    }

    return false;
}

void Simulator::releaseStove(int pid)
{
    for (auto &stove : stoves)
    {
        if (stove.getHolder() == pid)
        {
            stove.release();
            return;
        }
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
    string s;
    for (auto p : q)
        s += "P" + to_string(p->pid) + " ";
    if (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

string Simulator::getStoveStatus(int index)
{
    if (stoves[index].isBusy())
        return "P" + to_string(stoves[index].getHolder());
    return "FREE";
}

string Simulator::getPaymentStatus()
{
    if (PaymentTerminal.isBusy())
        return "P" + to_string(PaymentTerminal.getHolder());
    return "FREE";
}

void Simulator::run()
{
    auto stoveStatus = [&](int index)
    {
        if (stoves[index].isBusy())
            return "P" + to_string(stoves[index].getHolder());
        return string("FREE");
    };

    auto paymentStatus = [&]()
    {
        if (PaymentTerminal.isBusy())
            return "P" + to_string(PaymentTerminal.getHolder());
        return string("FREE");
    };

    // There are 5 queue types: ready, blockedMemory, blockedStove, blockedPayment, paidQueue

    blockedPayment.clear(); // important: payment blocking happens before memory allocation
    paidQueue.clear();      // this is queue before memory allocation, once memory is allocated, it will move to ready queue
    blockedMemory.clear();  // if memory is blocked, it will remain in this queue until available
    ready.clear();          // ready queue is for processes that have passed payment and memory allocation, but waiting for stove or CPU scheduling
    blockedStove.clear();   // When scheduler selects a process but stove is not available, it will be put in this queue until stove is available

    running = nullptr; // to check if there is a process currently running on CPU
    gantt.clear();     // use for grant chart in log

    // Initialize time and counters

    currentTime = 0;       // time for simulation
    finishedCount = 0;     // to track how many processes have finished, when it equals total process count, we can end the simulation
    quantumCounter = 0;    // quantum counter for RR scheduling
    paymentTimeLeft = 0;   // to track remaining payment time ( PAYMENT_TIME is constant= 2, so it will count down from 2 to 0)
    paymentStartTime = -1; // to track when the payment started, used to calculate paymentTimeLeft

    while (finishedCount < (int)processes.size())
    {
        cout << "\n========== Time " << currentTime << " ==========\n";
        // I. Check payment completion first before anything else //

        // 1. Update current payment, if done paying, move to paidQueue before memory allocation
        if (PaymentTerminal.isBusy())
        {
            int pid = PaymentTerminal.getHolder();
            Process *p = findProcessByPid(pid);
            //
            if (p != nullptr)
            {
                if (currentTime > paymentStartTime)
                    paymentTimeLeft--;
                // If payment just completed
                if (paymentTimeLeft == 0)
                {
                    // this is for log and console output
                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " PAYMENT_DONE -> RELEASE_PAYMENT" << endl;

                    logger.log(currentTime, p->pid, "PAYMENT_DONE", "PAYING",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));

                    // Free up payment terminal and move process to paidQueue
                    PaymentTerminal.release();
                    paymentTimeLeft = 0;
                    paymentStartTime = -1;

                    paidQueue.push_back(p);
                }
                else
                {
                    // If still paying, just log the status
                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " PAYING" << endl;

                    logger.log(currentTime, p->pid, "PAYING", "PAYING",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
            }
        }

        // 2. Check for processes waiting for payment in blockedPayment, if payment terminal is free, start payment for the first one
        if (!PaymentTerminal.isBusy() && !blockedPayment.empty())
        {
            Process *p = blockedPayment.front();
            blockedPayment.erase(blockedPayment.begin());

            PaymentTerminal.request(p->pid);
            p->state = BLOCKED_RESOURCE;

            paymentTimeLeft = PAYMENT_TIME;
            paymentStartTime = currentTime;

            cout << "[t=" << currentTime << "] P" << p->pid
                 << " START PAYMENT -> ACQUIRE_PAYMENT" << endl;

            logger.log(currentTime, p->pid, "ACQUIRE_PAYMENT", "PAYING",
                       stoveStatus(0), stoveStatus(1), paymentStatus(),
                       mem.getusedmemory(), formatQueue(ready),
                       formatQueue(blockedMemory), formatQueue(blockedPayment),
                       formatQueue(blockedStove));
        }

        // II. Paid queue -> Memory allocation -> Ready queue or Blocked_memory queue
        {
            vector<Process *> stillPaid;
            // Only processes that in paidQ
            for (auto *p : paidQueue)
            {
                int slot = mem.allocate(p->pid, p->memoryNeeded);

                if (slot != -1)
                {
                    p->state = READY;
                    ready.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " MEMORY_OK -> READY" << endl;

                    logger.log(currentTime, p->pid, "MEMORY_ALLOC", "READY",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
                else
                {
                    p->state = BLOCKED_MEMORY;
                    blockedMemory.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << "memory needed: " << p->memoryNeeded << " MEMORY_FAIL -> "
                         << " BLOCKED_MEMORY" << endl;

                    logger.log(currentTime, p->pid, "BLOCKED_MEMORY", "BLOCKED",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
            }

            paidQueue = stillPaid;
        }

        // III. Check new arrivals, move to blockedPayment or directly to paidQueue if payment terminal is free
        for (auto &p : processes)
        {
            if (p.arrivalTime != currentTime || p.state != NEW)
                continue;

            cout << "[t=" << currentTime << "] P" << p.pid
                 << " ARRIVE" << endl;

            logger.log(currentTime, p.pid, "ARRIVE", "NEW",
                       stoveStatus(0), stoveStatus(1), paymentStatus(),
                       mem.getusedmemory(), "-", "-", "-", "-");

            p.state = BLOCKED_RESOURCE;

            if (!PaymentTerminal.isBusy())
            {
                PaymentTerminal.request(p.pid);
                paymentTimeLeft = PAYMENT_TIME;
                paymentStartTime = currentTime;

                cout << "[t=" << currentTime << "] P" << p.pid
                     << " START PAYMENT -> ACQUIRE_PAYMENT" << endl;

                logger.log(currentTime, p.pid, "ACQUIRE_PAYMENT", "PAYING",
                           stoveStatus(0), stoveStatus(1), paymentStatus(),
                           mem.getusedmemory(), formatQueue(ready),
                           formatQueue(blockedMemory), formatQueue(blockedPayment),
                           formatQueue(blockedStove));
            }
            else
            {
                blockedPayment.push_back(&p);

                cout << "[t=" << currentTime << "] P" << p.pid
                     << " WAIT_PAYMENT" << endl;

                logger.log(currentTime, p.pid, "WAIT_PAYMENT", "BLOCKED",
                           stoveStatus(0), stoveStatus(1), paymentStatus(),
                           mem.getusedmemory(), formatQueue(ready),
                           formatQueue(blockedMemory), formatQueue(blockedPayment),
                           formatQueue(blockedStove));
            }
        }
        // IV. Retry stove blocked processes, if stove is available, move to ready queue
        {
            vector<Process *> stillBlocked;

            for (auto *p : blockedStove)
            {
                if (hasStove(p->pid) || acquireStove(p))
                {
                    p->state = READY;
                    ready.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " STOVE_OK -> READY" << endl;

                    logger.log(currentTime, p->pid, "STOVE_RETRY_OK", "READY",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
                else
                {
                    stillBlocked.push_back(p);
                }
            }

            blockedStove = stillBlocked;
        }
        // V. Retry memory blocked processes, if memory is available, move to ready queue
        {
            vector<Process *> stillBlocked;

            for (auto *p : blockedMemory)
            {
                int slot = mem.allocate(p->pid, p->memoryNeeded);

                if (slot != -1)
                {
                    p->state = READY;
                    ready.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " MEMORY_RETRY_OK -> READY" << endl;

                    logger.log(currentTime, p->pid, "MEMORY_RETRY", "READY",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
                else
                {
                    stillBlocked.push_back(p);
                }
            }

            blockedMemory = stillBlocked;
        }

        // VI. schedule from ready queue and check stove availability, to move to running or blockedStove
        if (running == nullptr)
        {
            Process *p = selectProcess(ready, policy);

            if (p != nullptr)
            {
                if (hasStove(p->pid) || acquireStove(p))
                {
                    p->state = RUNNING;
                    running = p;
                    quantumCounter = 0;

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " SCHEDULE using " << policyToString(policy) << " -> acquire stove and RUN"
                         << "| remaining burst time: " << p->remainingTime << endl;

                    logger.log(currentTime, p->pid, "SCHEDULE", "RUNNING",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
                else
                {
                    p->state = BLOCKED_RESOURCE;
                    blockedStove.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " WAIT_STOVE" << endl;

                    logger.log(currentTime, p->pid, "WAIT_STOVE", "BLOCKED",
                               stoveStatus(0), stoveStatus(1), paymentStatus(),
                               mem.getusedmemory(), formatQueue(ready),
                               formatQueue(blockedMemory), formatQueue(blockedPayment),
                               formatQueue(blockedStove));
                }
            }
        }

        // VII. Execute running process and check for finish or time slice expiration
        if (running != nullptr)
        {
            running->remainingTime--;

            if (policy == RR)
                quantumCounter++;

            cout << "[t=" << currentTime << "] P" << running->pid
                 << " RUN"
                 << "| remaining burst time: " << running->remainingTime << endl;

            logger.log(currentTime, running->pid, "RUN", "RUNNING",
                       stoveStatus(0), stoveStatus(1), paymentStatus(),
                       mem.getusedmemory(), formatQueue(ready),
                       formatQueue(blockedMemory), formatQueue(blockedPayment),
                       formatQueue(blockedStove));

            if (gantt.empty() || gantt.back().pid != running->pid)
                gantt.push_back({running->pid, currentTime, currentTime + 1});
            else
                gantt.back().end++;

            if (running->remainingTime == 0)
            {
                running->state = TERMINATED;

                releaseStove(running->pid);
                mem.release(running->pid);

                cout << "[t=" << currentTime << "] P" << running->pid
                     << " FINISH" << endl;

                logger.log(currentTime, running->pid, "FINISH", "TERMINATED",
                           stoveStatus(0), stoveStatus(1), paymentStatus(),
                           mem.getusedmemory(), formatQueue(ready),
                           formatQueue(blockedMemory), formatQueue(blockedPayment),
                           formatQueue(blockedStove));

                running = nullptr;
                quantumCounter = 0;
                finishedCount++;
            }
            else if (policy == RR && quantumCounter >= quantum)
            {
                running->state = READY;
                ready.push_back(running);

                cout << "[t=" << currentTime << "] P" << running->pid
                     << " TIME_SLICE -> READY" << endl;

                logger.log(currentTime, running->pid, "TIME_SLICE", "READY",
                           stoveStatus(0), stoveStatus(1), paymentStatus(),
                           mem.getusedmemory(), formatQueue(ready),
                           formatQueue(blockedMemory), formatQueue(blockedPayment),
                           formatQueue(blockedStove));

                running = nullptr;
                quantumCounter = 0;
            }
        }
        else
        {
            cout << "[t=" << currentTime << "] CPU IDLE" << endl;
        }

        mem.display(currentTime);
        Systemstate();

        currentTime++;
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

    for (const auto &p : processes)
        timeline[p.pid] = vector<char>(maxTime, '-');

    for (const auto &g : gantt)
        for (int t = g.start; t < g.end; t++)
            timeline[g.pid][t] = 'X';

    for (const auto &p : processes)
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

    if (!file.is_open())
    {
        cout << "Error: cannot open " << filename << endl;
        return;
    }

    if (gantt.empty())
    {
        file << "pid\n";
        file.close();
        return;
    }

    int maxTime = gantt.back().end;

    // Header: pid,0,1,2,3,...
    file << "pid";
    for (int t = 0; t < maxTime; t++)
        file << "," << t;
    file << "\n";

    // Each process has one row
    for (const auto &p : processes)
    {
        file << "P" << p.pid;

        for (int t = 0; t < maxTime; t++)
        {
            bool isRunning = false;

            for (const auto &g : gantt)
            {
                if (g.pid == p.pid && t >= g.start && t < g.end)
                {
                    isRunning = true;
                    break;
                }
            }

            file << "," << (isRunning ? "X" : "");
        }

        file << "\n";
    }

    file.close();
}