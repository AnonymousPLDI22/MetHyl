//
// Created by pro on 2021/10/23.
//

#include "time_guard.h"
#include "glog/logging.h"
#include <sys/time.h>

void TimeGuard::set(double _time_limit) {
    gettimeofday(&start_time, NULL);
    time_limit = _time_limit;
}

TimeGuard::TimeGuard(double time_limit) {
    set(time_limit);
}

bool TimeGuard::isTimeOut() const {
    timeval now;
    gettimeofday(&now, NULL);
    double period = (now.tv_sec - start_time.tv_sec) + (now.tv_usec - start_time.tv_usec) / 1000000.0;
    // LOG(INFO) << "check " << period << " " << time_limit << std::endl;
    return period > time_limit;
}

TimeRecorder::TimeRecorder() {
    gettimeofday(&start_time, NULL);
}

double TimeRecorder::getTime() {
    timeval now; gettimeofday(&now, NULL);
    double period = (now.tv_sec - start_time.tv_sec) + (now.tv_usec - start_time.tv_usec) / 1000000.0;
    start_time = now;
    return period;
}