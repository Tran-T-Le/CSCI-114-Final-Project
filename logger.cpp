#include "logger.h"

Logger::Logger(const string &filename)
{
    file.open(filename);

    if (file.is_open())
    {
        file << "time,pid,order,state,resource,memory_used,ready_queue\n";
    }
}

Logger::~Logger()
{
    if (file.is_open())
        file.close();
}

void Logger::log(int time, int pid, string order, string state, string resource, int memory, string queue)
{
    if (!file.is_open())
        return;

    file << time << "," << pid << "," << order << "," << state << "," << resource << "," << memory << "," << queue << " " << endl;
}