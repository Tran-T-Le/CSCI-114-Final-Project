#include "logger.h"

Logger::Logger(const string &filename) {
    file.open(filename);

    if (file.is_open()) {
        file << "Time,PID,Event,State,Resource,Memory_Used,Ready_Queue\n";
    }
}

Logger::~Logger() {
    if (file.is_open()) {
        file.close();
    }
}

void Logger::log(int time, int pid, string event, string state, string resource, int memory, string queue) {
    if (!file.is_open())
        return;

    file << time << "," << pid << "," << event << "," << state << "," << resource << "," << memory << "," << queue << " " << endl;
}
