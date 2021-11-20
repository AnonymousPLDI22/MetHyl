//
// Created by pro on 2021/10/23.
//

#ifndef DPSYNTHESISNEW_TIME_GUARD_H
#define DPSYNTHESISNEW_TIME_GUARD_H

#include <ctime>
#include <exception>

class TimeGuard {
public:
    timeval start_time;
    double time_limit;
    void set(double time_limit);
    bool isTimeOut() const;
    TimeGuard(double time_limit);
    TimeGuard() = default;
};

class TimeRecorder {
public:
    timeval start_time;
    double getTime();
    TimeRecorder();
};

struct TimeOutError: public std::exception {
};


#endif //DPSYNTHESISNEW_TIME_GUARD_H
