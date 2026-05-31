#pragma once
#include "task.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <queue>

class TaskManager {
public:
    void addTask(const std::string& name, time_t deadline, double duration) {
        Task t;
        t.id       = nextId_++;
        t.name     = name;
        t.deadline = deadline;
        t.duration = duration;
        tasks_.push_back(t);
        std::cout << "[OK] Task #" << t.id << " \"" << name << "\" added.\n";
    }

    void deleteTask(int id) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(),
                               [id](const Task& t){ return t.id == id; });
        if (it == tasks_.end()) {
            std::cout << "[Error] Task ID=" << id << " not found.\n";
            return;
        }
        std::cout << "[OK] Task #" << id << " \"" << it->name << "\" deleted.\n";
        tasks_.erase(it);
    }

    void listTasks() const {
        if (tasks_.empty()) { std::cout << "No tasks.\n"; return; }
        printHeader();
        for (const auto& t : tasks_) printRow(t);
        std::cout << "\n";
    }

    std::vector<Task> sortByDeadline() const {
        auto v = tasks_;
        std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
            return a.deadline < b.deadline;
        });
        return v;
    }

    std::vector<Task> sortByStress() const {
        auto v = tasks_;
        std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
            return a.stressValue() < b.stressValue();
        });
        return v;
    }

    std::vector<Task> sortByDuration() const {
        auto v = tasks_;
        std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
            return a.duration < b.duration;
        });
        return v;
    }

    std::vector<Task> sortByWeighted(double w_deadline = 0.5, double w_stress = 0.5) const {
        auto v = tasks_;
        std::sort(v.begin(), v.end(),
                  [w_deadline, w_stress](const Task& a, const Task& b){
                      return a.weightedScore(w_deadline, w_stress)
                           < b.weightedScore(w_deadline, w_stress);
                  });
        return v;
    }

    void showSorted(const std::vector<Task>& sorted, const std::string& method) const {
        std::cout << "\n=== Sort Method: " << method << " ===\n";
        printHeader(true);
        int rank = 1;
        for (const auto& t : sorted) printRow(t, rank++, true);
        std::cout << "\n";
    }

    // --- Simulation ---
    struct SimResult {
        int         id;
        std::string name;
        double      finishTime;   // hours from now when this task finishes
        double      deadlineLeft; // hours before deadline after finishing (negative = missed)
        bool        missed;
    };

    std::vector<SimResult> simulate(const std::vector<Task>& order) const {
        time_t now = time(nullptr);
        double cursor = 0.0;
        std::vector<SimResult> results;
        for (const auto& t : order) {
            cursor += t.duration;
            double deadlineHrs = difftime(t.deadline, now) / 3600.0;
            double margin = deadlineHrs - cursor;
            results.push_back({t.id, t.name, cursor, margin, margin < 0});
        }
        return results;
    }

    void showSimulation(const std::vector<SimResult>& res,
                        const std::string& method) const {
        std::cout << "\n  Method: " << method << "\n";
        std::cout << "  " << std::string(72, '-') << "\n";
        std::cout << "  " << std::setw(5)  << "Rank"
                          << std::setw(24) << "Task Name"
                          << std::setw(12) << "Done(hr)"
                          << std::setw(18) << "Before DL(hr)"
                          << std::setw(8)  << "Status" << "\n";
        std::cout << "  " << std::string(72, '-') << "\n";
        for (int i = 0; i < (int)res.size(); ++i) {
            const auto& r = res[i];
            std::string status = r.missed ? "MISSED" : "OK";
            std::cout << "  " << std::setw(5)  << (i+1)
                              << std::setw(24) << r.name.substr(0, 22)
                              << std::setw(12) << std::fixed << std::setprecision(1) << r.finishTime
                              << std::setw(18) << std::fixed << std::setprecision(1) << r.deadlineLeft
                              << std::setw(8)  << status << "\n";
        }
        int missed = 0;
        for (const auto& r : res) if (r.missed) missed++;
        std::cout << "  => Missed: " << missed << " / " << res.size() << "\n";
    }

    void compareSimulation() const {
        if (tasks_.empty()) { std::cout << "No tasks.\n"; return; }

        auto edf      = sortByDeadline();
        auto dur      = sortByDuration();
        auto weighted = sortByWeighted(0.5, 0.5);

        auto resEdf = simulate(edf);
        auto resDur = simulate(dur);
        auto resW   = simulate(weighted);

        std::cout << "\n";
        std::cout << "============================================================\n";
        std::cout << "  Execution Simulation: Time Left Before Each Deadline\n";
        std::cout << "  (Tasks done sequentially starting from now)\n";
        std::cout << "  'Before DL' = hours left before deadline after finishing\n";
        std::cout << "  Negative value = deadline missed!\n";
        std::cout << "============================================================\n";

        showSimulation(resEdf, "Method 1: Earliest Deadline First (EDF)");
        showSimulation(resDur, "Method 2: Shortest Duration First");
        showSimulation(resW,   "Method 3: Weighted Score (0.5 deadline / 0.5 stress)");

        int m1 = 0, m2 = 0, m3 = 0;
        for (const auto& r : resEdf) if (r.missed) m1++;
        for (const auto& r : resDur) if (r.missed) m2++;
        for (const auto& r : resW)   if (r.missed) m3++;
        int best = std::min({m1, m2, m3});

        std::cout << "\n  >> Recommendation: ";
        bool first = true;
        auto printRec = [&](int missed, const std::string& name) {
            if (missed == best) {
                if (!first) std::cout << " / ";
                std::cout << name;
                first = false;
            }
        };
        printRec(m1, "Method 1 (EDF)");
        printRec(m2, "Method 2 (Shortest Duration)");
        printRec(m3, "Method 3 (Weighted)");
        std::cout << " misses fewest deadlines (" << best << ").\n";
        std::cout << "============================================================\n\n";
    }

    // Generate n random tasks for benchmarking
    static std::vector<Task> genTasks(int n) {
        std::vector<Task> tmp;
        tmp.reserve(n);
        time_t base = time(nullptr);
        for (int i = 0; i < n; ++i) {
            Task t;
            t.id       = i;
            t.name     = "Task_" + std::to_string(i);
            t.deadline = base + (rand() % 720 + 1) * 3600LL;
            t.duration = (rand() % 20 + 1) * 0.5;
            tmp.push_back(t);
        }
        return tmp;
    }

    // Measure execution time of a sort function in microseconds
    template<typename Fn>
    static double measureUs(std::vector<Task> v, Fn sortFn) {
        auto start = std::chrono::high_resolution_clock::now();
        sortFn(v);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::micro>(end - start).count();
    }

    // Benchmark A: compare 3 different ALGORITHMS sorting by deadline
    void benchmarkAlgorithms() const {
        std::cout << "\n";
        std::cout << "================================================================\n";
        std::cout << "  Algorithm Comparison: Sorting by Deadline\n";
        std::cout << "  3 implementations: std::sort / Bubble Sort / Priority Queue\n";
        std::cout << "================================================================\n";
        std::cout << std::setw(20) << std::left  << "Algorithm"
                  << std::setw(12) << std::right << "n=10"
                  << std::setw(12) << "n=100"
                  << std::setw(12) << "n=1,000"
                  << "  (unit: us)\n";
        std::cout << std::string(56, '-') << "\n";

        auto printRow = [&](const std::string& label, auto sortFn) {
            std::cout << std::setw(20) << std::left << label;
            for (int n : {10, 100, 1000}) {
                auto tmp = genTasks(n);
                double us = measureUs(tmp, sortFn);
                std::cout << std::setw(12) << std::right << std::fixed << std::setprecision(2) << us;
            }
            std::cout << "\n";
        };

        printRow("std::sort", [](std::vector<Task> v){
            std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
                return a.deadline < b.deadline;
            });
        });

        printRow("Bubble Sort", [](std::vector<Task> v){
            int sz = (int)v.size();
            for (int i = 0; i < sz - 1; ++i)
                for (int j = 0; j < sz - 1 - i; ++j)
                    if (v[j].deadline > v[j+1].deadline)
                        std::swap(v[j], v[j+1]);
        });

        printRow("Priority Queue", [](std::vector<Task> v){
            auto cmp = [](const Task& a, const Task& b){
                return a.deadline > b.deadline;
            };
            std::priority_queue<Task, std::vector<Task>, decltype(cmp)> pq(cmp);
            for (const auto& t : v) pq.push(t);
            std::vector<Task> result;
            result.reserve(v.size());
            while (!pq.empty()) { result.push_back(pq.top()); pq.pop(); }
        });

        std::cout << std::string(68, '-') << "\n";
        std::cout << "  std::sort    : O(n log n) - introsort (quicksort + heapsort)\n";
        std::cout << "  Bubble Sort  : O(n^2)     - simple but slow for large n\n";
        std::cout << "  Priority Queue: O(n log n) - heap-based, good for streaming\n";
        std::cout << "================================================================\n\n";
    }


    bool empty() const { return tasks_.empty(); }

private:
    std::vector<Task> tasks_;
    int nextId_ = 1;

    void printHeader(bool showRank = false) const {
        std::cout << std::string(80, '-') << "\n";
        std::cout << std::setw(4)  << "ID"
                  << std::setw(22) << "Name"
                  << std::setw(18) << "Deadline"
                  << std::setw(10) << "Est(hr)"
                  << std::setw(12) << "Left(hr)"
                  << std::setw(12) << "Stress";
        if (showRank) std::cout << std::setw(6) << "Rank";
        std::cout << "\n" << std::string(80, '-') << "\n";
    }

    void printRow(const Task& t, int rank = -1, bool showRank = false) const {
        std::cout << std::setw(4)  << t.id
                  << std::setw(22) << t.name.substr(0, 20)
                  << std::setw(18) << t.deadlineStr()
                  << std::setw(10) << std::fixed << std::setprecision(1) << t.duration
                  << std::setw(12) << std::fixed << std::setprecision(1) << t.remainingTime()
                  << std::setw(12) << std::fixed << std::setprecision(1) << t.stressValue();
        if (showRank && rank > 0) std::cout << std::setw(6) << rank;
        std::cout << "\n";
    }
};
