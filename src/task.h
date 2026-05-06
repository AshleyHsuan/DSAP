#pragma once
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

struct Task {
    int id;
    std::string name;
    time_t deadline;
    double duration; // estimated hours

    double remainingTime() const {
        time_t now = time(nullptr);
        return difftime(deadline, now) / 3600.0;
    }

    double stressValue() const {
        return remainingTime() - duration;
    }

    double weightedScore(double w_deadline = 0.5, double w_stress = 0.5) const {
        return w_deadline * remainingTime() + w_stress * stressValue();
    }

    std::string deadlineStr() const {
        char buf[32];
        struct tm* t = localtime(&deadline);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", t);
        return std::string(buf);
    }
};

inline time_t parseDateTime(const std::string& s) {
    struct tm t = {};
    std::istringstream ss(s);
    ss >> std::get_time(&t, "%Y-%m-%d %H:%M");
    return mktime(&t);
}
