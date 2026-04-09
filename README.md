So Pho so Good
=================

<p> For this project we are simulating a operating system in the context of running a resturant. The sumulaton was programmed usign C++ on Visual Studio and Visual Studio Code. There is a total of 1024 bytes of memory. Three sheduling practices, which are First Come First Serve (FCFS), Shortest Job First (SJF), and Round Robin (RR). </p>

___

<p> As we are simulating a operating system with the context of running a resturant a menu is given. Each item on the given menu is a process available for the user to choose. The price of each menu is the turnaround time for each process. </p>

#### The given menu is listed below. 

```
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
___

<p> In order for the simulation to work a input is needed. The input must be based off the menu items. </p>
<p> When "ordering" off the menu the order must be done in a specific way. </p>

#### An example input would be:

```
5
101 08:00 2 Pho 3 MilkTea 1
102 08:02 1 BanhMi 2
103 08:02 2 Pho 1 BanhMi 1
104 08:15 3 BunBo 2 SpringRoll 1 MilkTea 2
105 08:20 2 Pho 2 SpringRoll 3
```

<p> The number listed in the first row is the number of "order" being inserted. </p>
<p> For the following rows. The first column resporesent the Process ID (PID). The next column is the time the "order" or jobs are inserted and processed. The following are the number of "items" followed by the specific name of the item being ordered. </p>
