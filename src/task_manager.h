#pragma once
#include "task.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

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

    void benchmarkSort(int n) const {
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

        auto measure = [&](const std::string& name, auto sortFn) {
            auto v = tmp;
            auto start = std::chrono::high_resolution_clock::now();
            sortFn(v);
            auto end   = std::chrono::high_resolution_clock::now();
            double us  = std::chrono::duration<double, std::micro>(end - start).count();
            std::cout << std::setw(22) << std::left << name
                      << " | n=" << std::setw(8) << n
                      << " | " << std::fixed << std::setprecision(3)
                      << us << " us\n";
        };

        std::cout << "\n=== Benchmark (n=" << n << ") ===\n";
        std::cout << std::string(55, '-') << "\n";
        measure("EDF (deadline)", [](std::vector<Task>& v){
            std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
                return a.deadline < b.deadline; });
        });
        measure("Stress Value", [](std::vector<Task>& v){
            std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
                return a.stressValue() < b.stressValue(); });
        });
        measure("Weighted(0.5/0.5)", [](std::vector<Task>& v){
            std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
                return a.weightedScore() < b.weightedScore(); });
        });
        measure("Weighted(0.7/0.3)", [](std::vector<Task>& v){
            std::sort(v.begin(), v.end(), [](const Task& a, const Task& b){
                return a.weightedScore(0.7, 0.3) < b.weightedScore(0.7, 0.3); });
        });
        std::cout << std::string(55, '-') << "\n";
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
