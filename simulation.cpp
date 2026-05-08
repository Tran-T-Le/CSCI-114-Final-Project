#include "simulation.h"

void Simulator::Systemstate() {
    cout << endl << "---- SYSTEM STATE ----" << endl;

    cout << "Running: ";
    if (running) {
        cout << "P" << running->pid << endl;
    }
    else {
        cout << "NONE" << endl;
    }

    cout << "Ready Queue: " << formatQueue(ready) << endl;
    cout << "Blocked (Memory): " << formatQueue(blockedMemory) << endl;
    cout << "Blocked (Stove): " << formatQueue(blockedStove) << endl;
    cout << "Blocked (Payment): " << formatQueue(blockedPayment) << endl;

    cout << "---------------------" << endl;
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

Process *Simulator::findProcessByPid(int pid) {
    for (auto &p : processes) {
        if (p.pid == pid) {
            return &p;
        }
    }
    return nullptr;
}

bool Simulator::hasStove(int pid) const {
    for (const auto &stove : stoves) {
        if (stove.getHolder() == pid) {
            return true;
        }
    }
    return false;
}

bool Simulator::acquireStove(Process *p) {
    if (hasStove(p->pid)) {
        return true;
    }

    for (auto &stove : stoves) {
        if (!stove.isBusy()) {
            stove.request(p->pid);
            return true;
        }
    }
    return false;
}

void Simulator::releaseStove(int pid) {
    for (auto &stove : stoves) {
        if (stove.getHolder() == pid) {
            stove.release();
            return;
        }
    }
}

string Simulator::policyToString(SchedulingPolicy p) {
    switch (p) {
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

string Simulator::formatQueue(const vector<Process *> &q) {
    if (q.empty()) {
        return "EMPTY";
    }
    string s;
    for (auto p : q) {
        s += "P" + to_string(p->pid) + " ";
    }
    if (!s.empty() && s.back() == ' ') {
        s.pop_back();
    }
    return s;
}

void Simulator::run() {
    blockedMemory.clear();
    blockedStove.clear();
    blockedPayment.clear();
    paidQueue.clear();
    ready.clear();
    running = nullptr;
    gantt.clear();

    currentTime = 0;
    finishedCount = 0;
    quantumCounter = 0;

    paymentTimeLeft = 0;
    paymentStartTime = -1;

    while (finishedCount < (int)processes.size()) {
        cout << "\n========== Time " << currentTime << " ==========\n";

        // 1. Update current payment
        if (PaymentTerminal.isBusy()) {
            // Get the process currently paying
            int pid = PaymentTerminal.getHolder();
            Process *p = findProcessByPid(pid);

            if (p != nullptr) {
                // Decrease payment time
                if (currentTime > paymentStartTime) {
                    paymentTimeLeft--;
                }
                // Check if payment is done, if so, release terminal and move to paid queue
                if (paymentTimeLeft == 0) {
                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " PAYMENT_DONE" << endl;
                    logger.log(currentTime, p->pid, "PAYING", "PAYING",
                               "PaymentTerminal", mem.getusedmemory(), formatQueue(ready));

                    PaymentTerminal.release();
                    paymentTimeLeft = 0;
                    paymentStartTime = -1;

                    paidQueue.push_back(p);
                }
                    
                else {
                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " PAYING" << endl;
                    logger.log(currentTime, p->pid, "PAYING", "PAYING",
                               "PaymentTerminal", mem.getusedmemory(), formatQueue(ready));
                }
            }
        }

        // 2. Start old blocked payment first to add to paid queue before handling new arrivals
        if (!PaymentTerminal.isBusy() && !blockedPayment.empty()) {
            Process *p = blockedPayment.front();
            blockedPayment.erase(blockedPayment.begin());

            // Start payment
            PaymentTerminal.request(p->pid);
            p->state = BLOCKED_RESOURCE;

            paymentTimeLeft = PAYMENT_TIME;
            paymentStartTime = currentTime;

            cout << "[t=" << currentTime << "] P" << p->pid
                 << " START_PAYMENT" << endl;
            logger.log(currentTime, p->pid, "START_PAYMENT", "PAYING",
                       "PaymentTerminal", mem.getusedmemory(), formatQueue(ready));
        }
        // 3. Paid queue -> Memory allocation -> Ready/Blocked
        {
            // We need to separate those that can be allocated memory from those that can't
            vector<Process *> stillPaid;

            for (auto *p : paidQueue) {
                int slot = mem.allocate(p->pid, p->memoryNeeded);

                if (slot != -1) {
                    p->state = READY;
                    ready.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " MEMORY_OK -> READY" << endl;
                    logger.log(currentTime, p->pid, "MEMORY_ALLOC", "READY",
                               "-", mem.getusedmemory(), formatQueue(ready));
                }
                    
                else {
                    p->state = BLOCKED_MEMORY;
                    blockedMemory.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " BLOCKED_MEMORY" << endl;
                    logger.log(currentTime, p->pid, "BLOCKED_MEMORY", "BLOCKED",
                               "-", mem.getusedmemory(), formatQueue(ready));
                }
            }
            // Update paidQueue to only contain those still waiting for memory
            paidQueue = stillPaid;
        }
        // 4. Handle new arrivals
        for (auto &p : processes) {
            if (p.arrivalTime != currentTime || p.state != NEW) {
                continue;
            }

            cout << "[t=" << currentTime << "] P" << p.pid
                 << " ARRIVE" << endl;
            logger.log(currentTime, p.pid, "ARRIVE", "NEW",
                       "-", mem.getusedmemory(), "-");

            p.state = BLOCKED_RESOURCE;

            if (!PaymentTerminal.isBusy()) {
                PaymentTerminal.request(p.pid);
                paymentTimeLeft = PAYMENT_TIME;
                paymentStartTime = currentTime;

                cout << "[t=" << currentTime << "] P" << p.pid
                     << " START_PAYMENT" << endl;
                logger.log(currentTime, p.pid, "START_PAYMENT", "PAYING",
                           "PaymentTerminal", mem.getusedmemory(), formatQueue(ready));
            }
                
            else {
                blockedPayment.push_back(&p);

                cout << "[t=" << currentTime << "] P" << p.pid
                     << " WAIT_PAYMENT" << endl;
                logger.log(currentTime, p.pid, "WAIT_PAYMENT", "BLOCKED",
                           "PaymentTerminal", mem.getusedmemory(), formatQueue(ready));
            }
        }

        // 5. Retry memory blocked
        {
            vector<Process *> stillBlocked;

            for (auto *p : blockedMemory) {
                int slot = mem.allocate(p->pid, p->memoryNeeded);

                if (slot != -1) {
                    p->state = READY;
                    ready.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " MEMORY_RETRY_OK -> READY" << endl;
                    logger.log(currentTime, p->pid, "MEMORY_RETRY", "READY",
                               "-", mem.getusedmemory(), formatQueue(ready));
                }
                    
                else {
                    stillBlocked.push_back(p);
                }
            }

            blockedMemory = stillBlocked;
        }

        // 6. Retry stove blocked
        {
            vector<Process *> stillBlocked;

            for (auto *p : blockedStove) {
                if (hasStove(p->pid) || acquireStove(p)) {
                    p->state = READY;
                    ready.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " STOVE_OK -> READY" << endl;
                    logger.log(currentTime, p->pid, "STOVE_RETRY_OK", "READY",
                               "-", mem.getusedmemory(), formatQueue(ready));
                }
                    
                else {
                    stillBlocked.push_back(p);
                }
            }

            blockedStove = stillBlocked;
        }

        // 7. Schedule from ready queue
        if (running == nullptr) {
            Process *p = selectProcess(ready, policy);

            if (p != nullptr) {
                if (hasStove(p->pid) || acquireStove(p)) {
                    p->state = RUNNING;
                    running = p;
                    quantumCounter = 0;

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " SCHEDULE -> RUNNING" << "| remaining burst time: " << p->remainingTime << endl;
                    logger.log(currentTime, p->pid, "SCHEDULE", "RUNNING",
                               "-", mem.getusedmemory(), formatQueue(ready));
                }
                    
                else {
                    p->state = BLOCKED_RESOURCE;
                    blockedStove.push_back(p);

                    cout << "[t=" << currentTime << "] P" << p->pid
                         << " WAIT_STOVE" << endl;
                    logger.log(currentTime, p->pid, "WAIT_STOVE", "BLOCKED",
                               "-", mem.getusedmemory(), formatQueue(ready));
                }
            }
        }

        // 8. Run current process
        if (running != nullptr) {
            running->remainingTime--;

            if (policy == RR) {
                quantumCounter++;
            }

            cout << "[t=" << currentTime << "] P" << running->pid
                 << " RUN" << "| remaining burst time: " << running->remainingTime << endl;
            logger.log(currentTime, running->pid, "RUN", "RUNNING",
                       "-", mem.getusedmemory(), formatQueue(ready));

            if (gantt.empty() || gantt.back().pid != running->pid) {
                gantt.push_back({running->pid, currentTime, currentTime + 1});
            }
            else {
                gantt.back().end++;
            }

            if (running->remainingTime == 0) {
                running->state = TERMINATED;

                releaseStove(running->pid);
                mem.release(running->pid);

                cout << "[t=" << currentTime << "] P" << running->pid
                     << " FINISH" << endl;
                logger.log(currentTime, running->pid, "FINISH", "TERMINATED",
                           "-", mem.getusedmemory(), formatQueue(ready));

                running = nullptr;
                quantumCounter = 0;
                finishedCount++;
            }
            
            else if (policy == RR && quantumCounter >= quantum) {
                running->state = READY;
                ready.push_back(running);

                cout << "[t=" << currentTime << "] P" << running->pid
                     << " TIME_SLICE -> READY" << endl;
                logger.log(currentTime, running->pid, "TIME_SLICE", "READY",
                           "-", mem.getusedmemory(), formatQueue(ready));
                running = nullptr;
                quantumCounter = 0;
            }
        }
            
        else {
            cout << "[t=" << currentTime << "] CPU IDLE" << endl;
        }

        mem.display(currentTime);
        Systemstate();

        currentTime++;
    }

    printGanttPerProcess();
}

void Simulator::printGanttPerProcess() const {
    cout << "\n========== FINAL GANTT ==========\n";

    if (gantt.empty()) {
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

    for (const auto &p : processes) {
        cout << "P" << p.pid << ": ";
        for (int t = 0; t < maxTime; t++) {
            cout << timeline[p.pid][t] << " ";
        }
        cout << "\n";
    }
}

void Simulator::exportGantttoCSV(string filename) {
    ofstream file(filename);
    file << "pid,start,end\n";

    for (auto &g : gantt)
        file << g.pid << "," << g.start << "," << g.end << "\n";

    file.close();
}
