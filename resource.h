#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>
using namespace std;

class ResourceManager
{
private:
    string name;
    bool busy;
    int holder;
    vector<int> waitingOrders;

public:
    ResourceManager(string n);

    bool request(int pid);
    void release();
    void release(int pid);

    bool isBusy() const;
    bool isAvailableFor(int pid) const;
    int getHolder() const;

    void addWaiting(int pid);
    void removeWaiting(int pid);

    string getName() const;
    string getQueueString() const;
};

#endif
