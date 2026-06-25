#ifndef PROCESS_SCHEDULER_H
#define PROCESS_SCHEDULER_H

#include "../Process/Process.h"

#include <array>
#include <deque>
#include <optional>
#include <string>
#include <vector>

enum class SchedulerEventType
{
    Created,
    Rejected,
    Dispatch,
    Quantum,
    Aging,
    Finish,
    Idle
};

struct SchedulerEvent
{
    int cycle;
    SchedulerEventType type;
    int pid;
    int priority;
    int remainingTime;
    std::string message;
};

class ProcessScheduler
{
private:
    static constexpr int QUEUE_COUNT = 4;

    std::array<std::deque<int>, QUEUE_COUNT> readyQueues;
    std::vector<Process> processes;
    std::vector<SchedulerEvent> events;
    int currentCycle;
    int nextPid;
    int maxProcesses;
    int agingThreshold;

    void recordEvent(SchedulerEventType type, int pid, const std::string &message);
    bool hasProcess(int pid) const;
    Process &getMutableProcess(int pid);
    const Process &getExistingProcess(int pid) const;
    void enqueueReadyProcess(int pid);
    std::optional<int> popNextReadyPid();
    void advanceWaitingCycles(int elapsedCycles, int runningPid);
    void applyAging();
    void removeFinishedFromReadyQueues();
    bool hasUnfinishedProcess() const;

public:
    static constexpr int MAX_PROCESSES = 1000;
    static constexpr int USER_QUANTUM = 1;
    static constexpr int DEFAULT_AGING_THRESHOLD = 5;

    explicit ProcessScheduler(int maxProcesses = MAX_PROCESSES,
                              int agingThreshold = DEFAULT_AGING_THRESHOLD);

    int createProcess(int startTime,
                      int priority,
                      int processorTime,
                      int memoryBlocks,
                      int printerRequest,
                      int scannerRequest,
                      int modemRequest,
                      int sataDiskRequest);

    bool hasReadyProcess() const;
    std::optional<int> selectNextProcessId() const;
    bool runNext();
    void runUntilComplete();

    std::size_t queueSize(int priority) const;
    bool empty() const;
    std::size_t processCount() const;
    int getCurrentCycle() const;

    const Process &getProcess(int pid) const;
    ProcessState getProcessState(int pid) const;
    int getProcessPriority(int pid) const;
    int getProcessRemainingTime(int pid) const;
    int getProcessWaitingCycles(int pid) const;
    const std::vector<SchedulerEvent> &getEvents() const;
};

#endif
