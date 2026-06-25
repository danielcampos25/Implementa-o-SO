#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../ProcessScheduler/ProcessScheduler.h"

#include <iosfwd>
#include <string>
#include <vector>

struct ProcessWorkloadEntry
{
    int startTime;
    int priority;
    int processorTime;
    int memoryBlocks;
    int printerRequest;
    int scannerRequest;
    int modemRequest;
    int sataDiskRequest;
    int inputOrder;
};

enum class DispatcherEventType
{
    Admission,
    Rejection,
    IdleAdvance,
    Dispatch,
    Quantum,
    Aging,
    Finish,
    Completion
};

struct DispatcherEvent
{
    int cycle;
    DispatcherEventType type;
    int pid;
    int priority;
    int remainingTime;
    std::string message;
};

class Dispatcher
{
private:
    struct PendingProcess
    {
        ProcessWorkloadEntry entry;
        bool admitted;
        bool rejected;
    };

    std::vector<PendingProcess> workload;
    std::vector<DispatcherEvent> events;
    ProcessScheduler scheduler;
    int currentCycle;
    int admittedProcesses;
    int rejectedProcesses;
    bool completionRecorded;
    std::size_t schedulerEventCursor;

    void recordEvent(DispatcherEventType type,
                     int pid,
                     int priority,
                     int remainingTime,
                     const std::string &message);
    bool hasPendingProcess() const;
    int nextPendingStartTime() const;
    void forwardSchedulerEvents();
    void recordCompletionIfNeeded();

public:
    Dispatcher();
    explicit Dispatcher(const std::vector<ProcessWorkloadEntry> &entries);

    void setWorkload(const std::vector<ProcessWorkloadEntry> &entries);
    void admitEligibleProcesses();
    bool runNext();
    void runUntilComplete();

    int getCurrentCycle() const;
    std::size_t pendingCount() const;
    int admittedCount() const;
    int rejectedCount() const;
    bool isComplete() const;
    const std::vector<DispatcherEvent> &getEvents() const;
    const ProcessScheduler &getScheduler() const;

    static std::string eventTypeName(DispatcherEventType type);
    static std::string formatEvent(const DispatcherEvent &event);
    void printEvents(std::ostream &output) const;
};

#endif
