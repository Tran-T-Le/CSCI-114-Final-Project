#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
using namespace std;

class Logger
{
private:
    ofstream file;

public:
    Logger(const string &filename);
    ~Logger();

    void log(int time, int pid,
             string event, string state, string stove_status1, string stove_status2, string payment_status, int memory, string queue, string blocked_memory, string blocked_payment, string blocked_stove);
};

#endif
