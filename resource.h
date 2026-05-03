#ifndef RESOURCE_H
#define RESOURCE_H

#include <queue>
#include <string>
using namespace std;

class ResourceManager
{
private:
    string name;
    bool busy;
    int holder;
    queue<int> waitingOrders;

public:
    ResourceManager(string n);

    bool request(int pid); // trying to use resource
    void release();        // free up resource

    bool isBusy() const;
    int getHolder() const;

    string getName() const;
    string getQueueString() const;
};

#endif