#include <iostream>
#include "input.h"

using namespace std;

int main()
{
    string openTime = "08:00";
    auto menu = loadMenu("menu.txt");
    vector<Process> processes = loadOrders("orders.txt", menu, openTime);

    return 0;
}