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

Compile code: g++ main.cpp input.cpp memory.cpp resource.cpp scheduler.cpp simulation.cpp logger.cpp -o sim
Then: .\sim.exe


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

---
## File list

===============
Header files
===============
- input.h
- logger.h
- memory.h
- resource.h
- scheduler.h
- simulation.h

---

==============
Cpp files
==============
- input.cpp
- logger.cpp
- memory.cpp
- resource.cpp
- scheduler.cpp
- simulation.cpp
- main.cpp

---

===================
Other project files
===================
- description.txt
- FIX_SUMMARY.txt
- Readme.md
- sim.exe

  

