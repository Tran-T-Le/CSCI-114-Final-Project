# So Phở So Good — MiniOS Simulator

## Team Information

**Course:** CSCI 114 — Operating Systems  
**Project:** Final Project — MiniOS Simulator  
**Team:** Team 3  
**Members:** Angel Tiffany Gago Manuel, Nhu Quynh Do, Tran Thanh Dat Le

---

## Project Overview

**So Phở So Good** is a restaurant-themed operating system simulation written in C++. The project is not a real OS kernel. It is a user-space simulator where customer food orders act like operating-system processes.

The simulation demonstrates:

- multiple processes/jobs moving through states
- FCFS, SJF, and Round Robin scheduling
- one logical CPU
- fixed-size memory allocation using First-Fit
- memory blocking when there is not enough contiguous memory
- two shared resources: `PaymentTerminal` and `Stove`
- resource blocking when a shared resource is busy
- readable logs showing state transitions, scheduling choices, memory allocation, memory release, resource acquisition, and resource release

---

## Restaurant-to-Operating-System Mapping

| Restaurant Idea | OS Concept | Meaning in This Project |
|---|---|---|
| Customer order | Process / job | Each order becomes one `Process` object. |
| Order number | PID | Unique process ID used in memory, logs, and scheduling. |
| Order arrival time | Arrival time | Time when the process enters the simulation. |
| Food items ordered | Process workload | Items determine CPU burst time and memory needed. |
| Kitchen staff time | CPU time | Only one process can run at a time. |
| Cooking time | CPU burst time | Total execution time required by the process. |
| Remaining cooking time | Remaining CPU time | Time left before the process terminates. |
| Food prep space | Memory | A process must receive memory before it can run. |
| Waiting order line | Ready queue | Processes that have memory and are ready to run. |
| Payment terminal | Shared resource / I/O device | Only one order can use it at a time. |
| Stove | Shared resource / device | A running process must acquire it before execution. |
| Completed order | Terminated process | The process finished and releases memory/resources. |

---

## Main Files

| File | Purpose |
|---|---|
| `main.cpp` | Program entry point. Lets user run all policies or one selected policy. |
| `input.h` / `input.cpp` | Defines `Process`, menu items, order loading, burst-time calculation, and memory-needed calculation. |
| `scheduler.h` / `scheduler.cpp` | Implements FCFS, SJF, and Round Robin selection behavior. |
| `memory.h` / `memory.cpp` | Implements fixed-size contiguous memory with First-Fit allocation. |
| `resource.h` / `resource.cpp` | Implements shared resource ownership and waiting queues. |
| `simulation.h` / `simulation.cpp` | Main scheduler loop, state transitions, blocking, resource use, logging, and Gantt chart generation. |
| `logger.h` / `logger.cpp` | Writes simulation events to CSV log files. |
| `menu.txt` | Food menu and cook times. |
| `input.txt` | Default process/order input file. |

---

## Process / PCB Design

Each customer order is stored as a `Process` object in `input.h`.

Important fields include:

| Field | Meaning |
|---|---|
| `pid` | Process/order ID. |
| `arrivalReal` | Clock time from input file, such as `08:00`. |
| `arrivalTime` | Converted time in minutes from opening time. |
| `items` | Food items in the order. |
| `burstTime` | Total CPU/cooking time needed. |
| `remainingTime` | CPU/cooking time still left. |
| `memoryNeeded` | Memory required before the process can enter the ready queue. |
| `paymentDone` | Whether the process has finished using the `PaymentTerminal`. |
| `memoryAllocated` | Whether First-Fit memory allocation already succeeded. |
| `state` | Current process state. |

---

## Process States

The project uses these process states:

| State | Meaning |
|---|---|
| `NEW` | The process exists in the input file but has not arrived yet. |
| `READY` | The process has finished payment, has memory, and is waiting for CPU time. |
| `RUNNING` | The scheduler selected the process and it is currently using the CPU/Stove. |
| `BLOCKED_MEMORY` | The process cannot run because memory allocation failed. |
| `BLOCKED_RESOURCE` | The process is waiting for a shared resource, such as `PaymentTerminal` or `Stove`. |
| `TERMINATED` | The process finished execution and released its memory/resources. |

Basic process flow:

```text
NEW
  ↓ arrival
BLOCKED_RESOURCE  ← waiting for PaymentTerminal
  ↓ payment done
BLOCKED_MEMORY    ← only if First-Fit memory allocation fails
  ↓ memory allocated
READY
  ↓ scheduler chooses process
RUNNING
  ↓ finished
TERMINATED
```

For Round Robin, a process can also move:

```text
RUNNING → READY
```

when its time quantum expires.

---

## Scheduling Policies

The program implements three scheduling policies.

### 1. FCFS — First Come, First Served

FCFS uses the ready queue in FIFO order. The process that enters the ready queue first is selected first.

In code:

```cpp
if (policy == FCFS || policy == RR) {
    Process* p = ready.front();
    ready.erase(ready.begin());
    return p;
}
```

### 2. SJF — Shortest Job First

SJF chooses the ready process with the smallest `remainingTime`. In this project, SJF is non-preemptive because the process keeps running until it finishes.

In code, SJF scans the ready queue and picks the shortest remaining job.

### 3. RR — Round Robin

Round Robin gives each process a fixed time quantum. If the process does not finish within that quantum, it is moved back to the ready queue.

The user can choose the RR quantum in single-policy mode, and the default quantum is `3` in run-all mode.

In the fixed version, Round Robin no longer hangs. When the time slice expires, the process releases the `Stove`, returns to `READY`, and can be scheduled again later.

---

## Memory Management

### Total Memory Size

The simulation uses fixed memory size:

```cpp
int memorySize = 1024;
```

So the simulated system has **1024 memory units**.

### Memory Representation

Memory is represented as a vector of integers:

```cpp
vector<int> memory;
```

| Value | Meaning |
|---|---|
| `0` | Free memory cell. |
| PID | Memory cell owned by that process. |

Example output:

```text
[t=15] Memory: [0..399 P101] [400..599 P103] [600..1023 FREE]
```

### First-Fit Allocation

The baseline memory policy is **First-Fit contiguous allocation**.

The memory manager scans memory from the beginning and looks for the first free block large enough for the process. If it finds enough contiguous space, the process receives that block. If not, the process moves to `BLOCKED_MEMORY`.

### Memory Needed Formula

The current project uses:

```cpp
memoryNeeded = totalQuantity * 100;
```

So each ordered item quantity requires **100 memory units**.

Example:

```text
Pho 3 + MilkTea 1 = 4 total items
4 * 100 = 400 memory units
```

### Memory Release

When a process terminates, the memory manager releases all memory cells owned by that PID. This allows blocked memory processes to retry allocation later.

---

## Shared Resources

The fixed version includes two explicit shared resources.

### 1. PaymentTerminal

The `PaymentTerminal` acts like a shared I/O device. A process must use the payment terminal before it can request memory and enter the ready queue.

Only one process can use it at a time. If multiple orders arrive at the same time, one process uses the terminal and the others enter `BLOCKED_RESOURCE`.

Example output:

```text
[t=2] P102 acquired PaymentTerminal.
[t=2] P103 blocked: PaymentTerminal is busy.
[t=3] P103 released PaymentTerminal.
```

### 2. Stove

The `Stove` acts like the resource needed for actual CPU/kitchen execution. A process must acquire the stove before it can run.

The stove is released when:

- the process terminates, or
- a Round Robin time slice expires

This keeps the Round Robin simulation from getting stuck.

---

## Synchronization / Control Rule

This project does not use real threads, locks, or semaphores because it is a user-space simulation. Instead, it demonstrates synchronization using resource ownership rules and queues.

The control rule is:

> A process cannot run unless it has completed payment, received memory, and acquired the stove.

This prevents chaos because:

- only one process can use the `PaymentTerminal` at a time
- only one process can use the `Stove` at a time
- memory cannot be double-allocated
- blocked processes wait in the correct queue
- resources and memory are released when a process finishes

---

## Input Format

The input file is `input.txt`.

Example:

```text
5
101 08:00 2 Pho 3 MilkTea 1
102 08:02 1 BanhMi 2
103 08:02 2 Pho 1 BanhMi 1
104 08:15 3 BunBo 2 SpringRoll 1 MilkTea 2
105 08:20 2 Pho 2 SpringRoll 3
```

Format:

```text
number_of_orders
PID arrival_time number_of_distinct_items item_name quantity item_name quantity ...
```

Example row:

```text
101 08:00 2 Pho 3 MilkTea 1
```

This means:

- PID = `101`
- arrival time = `08:00`
- 2 distinct item types
- `Pho 3`
- `MilkTea 1`

---

## Menu Format

The menu file is `menu.txt`.

Example:

```text
Pho 3
BanhMi 2
MilkTea 1
SpringRoll 2
BunBo 3
Eggroll 2
Coffee 1
Pancit 2
MangoSmoothie 1
```

Each line contains:

```text
item_name cook_time
```

The burst time of an order is calculated by multiplying each item quantity by its cook time.

Example:

```text
Pho 3 + MilkTea 1
Pho cook time = 3, quantity = 3 → 9
MilkTea cook time = 1, quantity = 1 → 1
Total burst time = 10
```

---

## How to Compile

Use a C++17 compiler.

Recommended command:

```bash
g++ -std=c++17 main.cpp input.cpp logger.cpp memory.cpp resource.cpp scheduler.cpp simulation.cpp -o sim
```

---

## How to Run

Run the executable:

```bash
./sim
```

The program asks:

```text
Choose run mode:
1 = Run all policies sequentially (FCFS, SJF, RR)
2 = Run a single policy
```

### Option 1: Run All Policies

This runs:

1. FCFS
2. SJF
3. RR with default quantum `3`

It creates:

```text
fcfs_log.csv
sjf_log.csv
rr_log.csv
gantt_fcfs.csv
gantt_sjf.csv
gantt_rr.csv
```

### Option 2: Run One Policy

The program asks:

```text
Select scheduling policy: 1=FCFS, 2=SJF, 3=RR:
```

If RR is selected, the program asks for the quantum:

```text
Enter RR time quantum:
```

It creates:

```text
log.csv
gantt.csv
```

---

## Output and Logs

The console output shows:

- process arrivals
- resource blocking
- resource acquisition
- resource release
- memory allocation
- memory blocking
- memory release
- scheduler selections
- Round Robin time-slice expiration
- process termination
- memory layout at each time step
- final Gantt chart

The CSV log contains columns:

```text
time,pid,order,state,resource,memory,queue
```

Example events:

```text
ARRIVAL
ACQUIRE_RESOURCE
BLOCKED_RESOURCE
RELEASE_RESOURCE
READY
SCHEDULE
RUN
TIME_SLICE_EXPIRED
BLOCKED_MEMORY
TERMINATED
```

---

## Requirement Checklist

| Final Requirement | How This Project Meets It |
|---|---|
| Program runs from clean compile | Compiles with `g++ -std=c++17`. |
| At least 5 jobs/processes | `input.txt` contains 5 process orders. |
| Processes move through states | Uses `NEW`, `READY`, `RUNNING`, `BLOCKED_MEMORY`, `BLOCKED_RESOURCE`, `TERMINATED`. |
| FCFS scheduling | Implemented in `scheduler.cpp`. |
| Round Robin scheduling | Implemented with configurable quantum. |
| SJF / additional policy | SJF is implemented. |
| Policy selection | User selects run-all mode or single-policy mode in `main.cpp`. |
| Fixed memory size | Uses 1024 memory units. |
| Memory required before running | A process cannot enter `READY` until memory allocation succeeds. |
| Memory blocking | Processes enter `BLOCKED_MEMORY` when First-Fit cannot find enough contiguous memory. |
| Memory release | Memory is released when a process terminates. |
| At least 2 shared resources | `PaymentTerminal` and `Stove`. |
| Resource blocking | A process enters `BLOCKED_RESOURCE` if the payment terminal or stove is busy. |
| Synchronization/control | Resource ownership and waiting queues prevent multiple processes from using one resource at the same time. |
| Observable evidence | Console output and CSV logs show scheduling, blocking, state transitions, memory allocation, and resource release. |
| Architecture/code organization | Separate files for input, scheduler, memory, resource, simulation, and logging. |

