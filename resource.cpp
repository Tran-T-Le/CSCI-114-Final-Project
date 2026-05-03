#include "resource.h"
#include <algorithm>

ResourceManager::ResourceManager(string resourceName)
{
    name = resourceName;
    busy = false;
    holder = -1;
}

bool ResourceManager::request(int pid)
{
    if (!busy)
    {
        busy = true;
        holder = pid;
        removeWaiting(pid);
        return true;
    }

    if (holder == pid)
    {
        removeWaiting(pid);
        return true;
    }

    addWaiting(pid);
    return false;
}

void ResourceManager::release()
{
    busy = false;
    holder = -1;
}

void ResourceManager::release(int pid)
{
    if (holder == pid)
    {
        release();
    }
}

bool ResourceManager::isBusy() const
{
    return busy;
}

bool ResourceManager::isAvailableFor(int pid) const
{
    return !busy || holder == pid;
}

int ResourceManager::getHolder() const
{
    return holder;
}

void ResourceManager::addWaiting(int pid)
{
    if (find(waitingOrders.begin(), waitingOrders.end(), pid) == waitingOrders.end())
    {
        waitingOrders.push_back(pid);
    }
}

void ResourceManager::removeWaiting(int pid)
{
    waitingOrders.erase(remove(waitingOrders.begin(), waitingOrders.end(), pid), waitingOrders.end());
}

string ResourceManager::getName() const
{
    return name;
}

string ResourceManager::getQueueString() const
{
    if (waitingOrders.empty())
    {
        return "EMPTY";
    }

    string s;
    for (int pid : waitingOrders)
    {
        s += "P" + to_string(pid) + " ";
    }

    if (!s.empty() && s.back() == ' ')
    {
        s.pop_back();
    }

    return s;
}
