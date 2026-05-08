#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
using namespace std;

class Logger {
private:
    ofstream file;

public:
    Logger(const string &filename);
    ~Logger();

    void log(int time, int pid, string event, string state, string resource, int memory, string queue);
};

#endif
