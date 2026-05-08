#include "resource.h"

ResourceManager::ResourceManager(string resourceName)
{
    name = resourceName;
    busy = false;
    holder = -1;
}

bool ResourceManager::request(int orderId)
{
    if (!busy)
    {
        busy = true;
        holder = orderId;
        return true;
    }
    return false;
}

void ResourceManager::release()
{

    if (!waitingOrders.empty())
    {
        holder = waitingOrders.front();
        waitingOrders.pop();
    }
    else
    {
        busy = false;
        holder = -1;
    }
}

bool ResourceManager::isBusy() const
{
    return busy;
}

int ResourceManager::getHolder() const
{
    return holder;
}

string ResourceManager::getName() const
{
    return name;
}

string ResourceManager::getQueueString() const
{
    queue<int> temp = waitingOrders;
    string s;

    while (!temp.empty())
    {
        s += "Order" + to_string(temp.front()) + " ";
        temp.pop();
    }

    return s.empty() ? "EMPTY" : s;
}