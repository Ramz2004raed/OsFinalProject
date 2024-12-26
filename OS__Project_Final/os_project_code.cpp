#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>
using namespace std;

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int finishTime;
    int waitingTime;
    int turnaroundTime;
};

void readProcesses(string filename, vector<Process>& processes, int& timeQuantum, int& contextSwitchTime) {
    ifstream file(filename);
    if (!file) {
        cout << "Error opening file." << endl;
        exit(1);
    }

    string line;
    int numProcesses;

    getline(file, line);
    numProcesses = stoi(line.substr(line.find(":") + 1));

    vector<int> arrivalTimes, cpuBursts;

    getline(file, line);
    stringstream ss1(line.substr(line.find(":") + 1));
    int value;
    while (ss1 >> value) {
        arrivalTimes.push_back(value);
    }

    getline(file, line);
    stringstream ss2(line.substr(line.find(":") + 1));
    while (ss2 >> value) {
        cpuBursts.push_back(value);
    }

    getline(file, line);
    contextSwitchTime = stoi(line.substr(line.find(":") + 1));

    getline(file, line);
    timeQuantum = stoi(line.substr(line.find(":") + 1));

    for (int i = 0; i < numProcesses; i++) {
        Process p;
        p.id = i + 1;
        p.arrivalTime = arrivalTimes[i];
        p.burstTime = cpuBursts[i];
        p.remainingTime = p.burstTime;
        processes.push_back(p);
    }
}

void calculateMetrics(vector<Process>& processes) {
    for (int i = 0; i < processes.size(); i++) {
        processes[i].turnaroundTime = processes[i].finishTime - processes[i].arrivalTime;
        processes[i].waitingTime = processes[i].turnaroundTime - processes[i].burstTime;
    }
}

void printResults(string title, vector<Process>& processes, vector<pair<int, int>>& ganttChart) {
    cout << "\n" << title << "\n";
    cout << "Gantt Chart:\n";
    for (int i = 0; i < ganttChart.size(); i++) {
        cout << "| P" << ganttChart[i].second << " ";
    }
    cout << "|\n";
    for (int i = 0; i < ganttChart.size(); i++) {
        cout << ganttChart[i].first << "    ";
    }
    cout << ganttChart.back().first + 1 << "\n";

    cout << "Process\tArrival\tBurst\tFinish\tWaiting\tTurnaround\n";
    for (int i = 0; i < processes.size(); i++) {
        cout << "P" << processes[i].id << "\t\t" << processes[i].arrivalTime << "\t\t" << processes[i].burstTime << "\t" << processes[i].finishTime
            << "\t\t" << processes[i].waitingTime << "\t\t" << processes[i].turnaroundTime << "\n";
    }

    double totalTurnaround = 0, totalWaiting = 0;
    for (int i = 0; i < processes.size(); i++) {
        totalTurnaround += processes[i].turnaroundTime;
        totalWaiting += processes[i].waitingTime;
    }

    double cpuUtilization = ganttChart.back().first + 1;
    cout << "\nAverage Turnaround Time: " << totalTurnaround / processes.size() << "\n";
    cout << "Average Waiting Time: " << totalWaiting / processes.size() << "\n";
    cout << "CPU Utilization: " << (cpuUtilization / (cpuUtilization + ganttChart.front().first)) * 100 << "%\n";
}

void fcfs(vector<Process> processes) {
    vector<pair<int, int>> ganttChart;
    int currentTime = 0;

    for (int i = 0; i < processes.size(); i++) {
        if (currentTime < processes[i].arrivalTime)
            currentTime = processes[i].arrivalTime;

        ganttChart.push_back({ currentTime, processes[i].id });
        currentTime += processes[i].burstTime;
        processes[i].finishTime = currentTime;
    }

    calculateMetrics(processes);
    printResults("First-Come First-Served", processes, ganttChart);
}

void srt(vector<Process> processes) {
    vector<pair<int, int>> ganttChart;
    int currentTime = 0, completed = 0;

    while (completed < processes.size()) {
        Process* currentProcess = NULL;
        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0) {
                if (currentProcess == NULL || processes[i].remainingTime < currentProcess->remainingTime) {
                    currentProcess = &processes[i];
                }
            }
        }

        if (currentProcess == NULL) {
            currentTime++;
            continue;
        }

        ganttChart.push_back({ currentTime, currentProcess->id });
        currentProcess->remainingTime--;
        currentTime++;

        if (currentProcess->remainingTime == 0) {
            currentProcess->finishTime = currentTime;
            completed++;
        }
    }

    calculateMetrics(processes);
    printResults("Shortest Remaining Time", processes, ganttChart);
}

void roundRobin(vector<Process> processes, int timeQuantum) {
    vector<pair<int, int>> ganttChart;
    queue<Process*> readyQueue;
    int currentTime = 0;
    int completed = 0;

    for (int i = 0; i < processes.size(); i++) {
        if (processes[i].arrivalTime <= currentTime) {
            readyQueue.push(&processes[i]);
        }
    }

    while (completed < processes.size()) {
        if (readyQueue.empty()) {
            currentTime++;
            continue;
        }

        Process* currentProcess = readyQueue.front();
        readyQueue.pop();

        int executeTime = min(timeQuantum, currentProcess->remainingTime);
        ganttChart.push_back({ currentTime, currentProcess->id });
        currentProcess->remainingTime -= executeTime;
        currentTime += executeTime;

        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0 && processes[i].id != currentProcess->id) {
                readyQueue.push(&processes[i]);
            }
        }

        if (currentProcess->remainingTime > 0) {
            readyQueue.push(currentProcess);
        }
        else {
            currentProcess->finishTime = currentTime;
            completed++;
        }
    }

    calculateMetrics(processes);
    printResults("Round Robin", processes, ganttChart);
}

int main() {
    string filename = "C:\\Users\\ramzd\\Desktop\\ramzinput.txt";

    vector<Process> processes;
    int timeQuantum;
    int contextSwitchTime;

    readProcesses(filename, processes, timeQuantum, contextSwitchTime);

    cout << "Context switch time: " << contextSwitchTime << " milliseconds" << endl;

    fcfs(processes);
    srt(processes);
    roundRobin(processes, timeQuantum);

    return 0;
}
/*
Number of processes:4
Arrival times:0 1 3 5
CPU bursts:7 5 3 1
Context switch time (milliseconds):2
Time quantum for Round Robin:2
*/

/*Context switch time: 2 milliseconds

First-Come First-Served
Gantt Chart:
| P1 | P2 | P3 | P4 |
0    7    12    15    16
Process Arrival Burst   Finish  Waiting Turnaround
P1              0               7       7               0               7
P2              1               5       12              6               11
P3              3               3       15              9               12
P4              5               1       16              10              11

Average Turnaround Time: 10.25
Average Waiting Time: 6.25
CPU Utilization: 100%

Shortest Remaining Time
Gantt Chart:
| P1 | P2 | P2 | P2 | P2 | P2 | P4 | P3 | P3 | P3 | P1 | P1 | P1 | P1 | P1 | P1 |
0    1    2    3    4    5    6    7    8    9    10    11    12    13    14    15    16
Process Arrival Burst   Finish  Waiting Turnaround
P1              0               7       16              9               16
P2              1               5       6               0               5
P3              3               3       10              4               7
P4              5               1       7               1               2

Average Turnaround Time: 7.5
Average Waiting Time: 3.5
CPU Utilization: 100%

Round Robin
Gantt Chart:
| P1 | P2 | P1 | P1 | P3 | P2 | P2 | P3 | P4 | P1 |
0    2    4    6    8    10    12    13    14    15    16
Process Arrival Burst   Finish  Waiting Turnaround
P1              0               7       16              9               16
P2              1               5       13              7               12
P3              3               3       14              8               11
P4              5               1       15              9               10

Average Turnaround Time: 12.25
Average Waiting Time: 8.25
CPU Utilization: 100%*/