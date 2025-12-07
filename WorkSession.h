#ifndef WORKSESSION_H
#define WORKSESSION_H

#include <string>
#include <chrono>

struct WorkSession {
    std::string name;
    std::string description;
    std::string dateString;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    double durationMinutes {0.0};

    // Helper to calculate duration in minutes
    double getDurationInMinutes() const {
        if (durationMinutes > 0.0) return durationMinutes;
        std::chrono::duration<double> elapsed_seconds = endTime - startTime;
        return elapsed_seconds.count() / 60.0;
    }
};

#endif // WORKSESSION_H
