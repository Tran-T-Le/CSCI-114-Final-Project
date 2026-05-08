#include "logger.h"

Logger::Logger(const string &filename)
{
    file.open(filename);

    if (file.is_open())
    {
        file << "Time,PID,Event,State,Stove1,Stove2,Payment_terminal,Memory_Used,Ready_Queue,Blocked_Memory,Blocked_Payment,Blocked_Stove\n";
    }
}

Logger::~Logger()
{
    if (file.is_open())
        file.close();
}

void Logger::log(int time, int pid, string event, string state, string stove_status1, string stove_status2, string payment_status, int memory, string queue, string blocked_memory, string blocked_payment, string blocked_stove)
{
    if (!file.is_open())
        return;

    file << time << "," << pid << "," << event << "," << state << "," << stove_status1 << "," << stove_status2 << "," << payment_status << "," << memory << "," << queue << "," << blocked_memory << "," << blocked_payment << "," << blocked_stove << endl;
}
