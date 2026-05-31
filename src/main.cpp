#include "task_manager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <limits>

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void printMenu() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "   Task Priority Scheduling System\n";
    std::cout << "========================================\n";
    std::cout << "  1. Add Task\n";
    std::cout << "  2. Delete Task\n";
    std::cout << "  3. List All Tasks\n";
    std::cout << "  4. Sort by EDF (Earliest Deadline First)\n";
    std::cout << "  5. Sort by Stress Value\n";
    std::cout << "  6. Sort by Weighted Score (custom weights)\n";
    std::cout << "  7. Compare All 3 Sort Methods\n";
    std::cout << "  8. Algorithm Comparison (std::sort vs Bubble vs PriorityQueue)\n";
    std::cout << "  9. Simulate Execution & Compare Methods\n";
    std::cout << "  0. Exit\n";
    std::cout << "========================================\n";
    std::cout << "Enter option: ";
}

time_t inputDeadline() {
    std::string s;
    while (true) {
        std::cout << "Deadline (format: YYYY-MM-DD HH:MM): ";
        std::getline(std::cin, s);
        time_t t = parseDateTime(s);
        if (t != -1 && t > 0) return t;
        std::cout << "[Error] Invalid format. Try again.\n";
    }
}

int main() {
    TaskManager mgr;
    srand(static_cast<unsigned>(time(nullptr)));

    // Sample tasks
    time_t now = time(nullptr);
    mgr.addTask("DS Homework 3",       now + 24*3600, 3.0);
    mgr.addTask("Algorithm Midterm",   now + 48*3600, 6.0);
    mgr.addTask("OS Design Document",  now + 12*3600, 2.0);
    mgr.addTask("Project Proposal",    now + 72*3600, 4.0);
    mgr.addTask("English Presentation",now + 36*3600, 1.5);

    int choice = -1;
    while (choice != 0) {
        printMenu();
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            std::string name;
            double duration;
            std::cout << "Task name: ";
            std::getline(std::cin, name);
            time_t dl = inputDeadline();
            std::cout << "Estimated duration (hours): ";
            std::cin >> duration;
            clearInput();
            mgr.addTask(name, dl, duration);
            break;
        }
        case 2: {
            mgr.listTasks();
            int id;
            std::cout << "Enter task ID to delete: ";
            std::cin >> id;
            clearInput();
            mgr.deleteTask(id);
            break;
        }
        case 3:
            mgr.listTasks();
            break;
        case 4:
            if (mgr.empty()) { std::cout << "No tasks.\n"; break; }
            mgr.showSorted(mgr.sortByDeadline(), "Earliest Deadline First (EDF)");
            break;
        case 5:
            if (mgr.empty()) { std::cout << "No tasks.\n"; break; }
            mgr.showSorted(mgr.sortByStress(), "Stress Value (remaining - duration)");
            break;
        case 6: {
            if (mgr.empty()) { std::cout << "No tasks.\n"; break; }
            double wd, ws;
            std::cout << "Deadline weight (0~1): "; std::cin >> wd;
            std::cout << "Stress weight   (0~1): "; std::cin >> ws;
            clearInput();
            if (wd + ws <= 0) { std::cout << "[Error] Weights cannot both be 0.\n"; break; }
            double total = wd + ws;
            wd /= total; ws /= total;
            std::ostringstream label;
            label << "Weighted (deadline=" << std::fixed << std::setprecision(2)
                  << wd << ", stress=" << ws << ")";
            mgr.showSorted(mgr.sortByWeighted(wd, ws), label.str());
            break;
        }
        case 7: {
            if (mgr.empty()) { std::cout << "No tasks.\n"; break; }
            auto edf      = mgr.sortByDeadline();
            auto stress   = mgr.sortByStress();
            auto weighted = mgr.sortByWeighted();

            std::cout << "\n=== Comparison of 3 Sort Methods ===\n";

            std::cout << "\n[Method 1] EDF (Earliest Deadline First)\n";
            std::cout << " Rank | Task Name\n" << std::string(40, '-') << "\n";
            for (int i = 0; i < (int)edf.size(); ++i)
                std::cout << "  " << (i+1) << "    | " << edf[i].name << "\n";

            std::cout << "\n[Method 2] Stress Value Sort\n";
            std::cout << " Rank | Task Name\n" << std::string(40, '-') << "\n";
            for (int i = 0; i < (int)stress.size(); ++i)
                std::cout << "  " << (i+1) << "    | " << stress[i].name << "\n";

            std::cout << "\n[Method 3] Weighted Sort (0.5 / 0.5)\n";
            std::cout << " Rank | Task Name\n" << std::string(40, '-') << "\n";
            for (int i = 0; i < (int)weighted.size(); ++i)
                std::cout << "  " << (i+1) << "    | " << weighted[i].name << "\n";
            std::cout << "\n";
            break;
        }
        case 8:
            mgr.benchmarkAlgorithms();
            break;
        case 9:
            mgr.compareSimulation();
            break;
        case 0:
            std::cout << "Goodbye!\n";
            break;
        default:
            std::cout << "[Error] Invalid option.\n";
        }
    }
    return 0;
}
