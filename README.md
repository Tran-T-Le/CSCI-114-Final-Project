# So Phở So Good — MiniOS Simulator

## Team Information

**Course:** CSCI 114 — Operating Systems  
**Project:** Final Project — MiniOS Simulator  
**Team:** Team 3  
**Members:** Angel Tiffany Gago Manuel, Nhu Quynh Do, Tran Thanh Dat Le

---

### Language and Environment

- Language: C++
- Recommended standard: C++17
- Compiler used: g++
- Recommend editor: Visual Studio Code or CodeBlocks
- Operating system used for testing: Windows

## Comepile instruction

Open Powershell or terminal in the project's folder location:

```bash
g++ src/*.cpp -Iinclude -o sim
.\sim.exe
```

## Project Overview

Our project purpose is to stimulate a major operating system through a restaurant theme miniOS simulator title "**So Pho So Good**".

In the project, each customer order is treated like a process/job an in operating system. The restaurant itself and the kitchen will acts like the CPU, the food preparation area acts like the memory, and shared tools act like shared resources.

This simulation demonstrate these operating system concepts:

- Process/job creation and state transitions.
- Scheduling using First Come First Served (FCFS), Shortest Job First (SJF), and Round Robin (RR).
- Fixed-size memory management using First-Fit contiguous allocation.
- Blocking when memory or shared resources are unavailable.
- Shared resource control using a Payment Terminal and Stove.
- Logs and Gantt chart output to show scheduling decisions and process behavior.

The output terminal shows the simulation running over time.
It will display the following:

- process arrivals
- ready queues
- block queues
- running process
- memory layout
- scheduling decisions
- termination events

# File list

## Header files

- input.h  
  Handles input loading and process creation declarations.
- logger.h  
  Handles CSV logging declarations for simulation events.
- memory.h  
  Contains memory manager declarations and First-Fit allocation functions.
- resource.h  
  Handles shared resource declarations such as Payment Terminal and Stove.
- scheduler.h  
  Contains scheduling policy declarations and process selection functions.
- simulation.h  
  Contains the main simulator class and simulation flow declarations.

---

## Cpp files

- input.cpp  
  Reads menu.txt and input files, then initializes processes.
- logger.cpp  
  Writes simulation logs into log.csv.
- memory.cpp  
  Implements contiguous memory allocation and memory release.
- resource.cpp  
  Implements shared resource acquire and release behavior.
- scheduler.cpp  
  Implements FCFS, SJF, and Round Robin scheduling logic.
- simulation.cpp  
  Contains the main simulation loop, process state transitions, scheduling, resource handling, and Gantt chart generation.
- main.cpp  
  Main driver program that starts the simulator and lets the user choose test cases and scheduling policies.

---

## Input files

- default_input.txt  
  Default simulation test case.
- input_memory.txt  
  Test case for blocked memory situations.
- input_payment.txt  
  Test case for blocked payment terminal situations.
- input_stove.txt  
  Test case for blocked stove/resource situations using Round Robin scheduling.

---

## Output files

- gantt.csv  
  Stores the final Gantt chart timeline in CSV format.
- log.csv  
  Stores detailed simulation logs and process state changes.

---

## Other project files

- description.txt  
  Contains project explanation and implementation details.
- menu.txt  
  Stores menu items and burst times for each food item.
- FIX_SUMMARY.txt  
  Contains notes about bug fixes and project improvements.
- README.md  
  Project documentation and usage instructions.
- outputDemo.txt  
  Example output generated from the simulator.
