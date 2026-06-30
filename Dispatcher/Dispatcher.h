#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../ProcessScheduler/ProcessScheduler.h"

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class MemoryManager;

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
    std::vector<std::vector<int>> referenceStrings;
    bool referenceStringsConfigured;
    std::unordered_map<int, std::size_t> pidReferenceIndexes;
    std::unordered_map<int, std::size_t> pidReferenceCursors;
    std::unordered_map<int, int> pageFaultsByPid;
    std::unordered_set<int> releasedPids;
    std::unique_ptr<MemoryManager> memoryManager;
    bool simulationError;
    std::string simulationErrorMessage;

    void recordEvent(DispatcherEventType type,
                     int pid,
                     int priority,
                     int remainingTime,
                     const std::string &message);
    void recordEventAtCycle(int cycle,
                            DispatcherEventType type,
                            int pid,
                            int priority,
                            int remainingTime,
                            const std::string &message);
    bool hasPendingProcess() const;
    int nextPendingStartTime() const;
    void clearMemoryAccountingState();
    void initializeProcessMemoryAccounting(int pid, int inputOrder);
    bool consumeReferencesForLastRun();
    void releaseProcessMemoryIfFinished(int pid);
    void forwardSchedulerEvents(int cycleOffset);
    void recordCompletionIfNeeded();

public:
    Dispatcher();
    ~Dispatcher();
    explicit Dispatcher(const std::vector<ProcessWorkloadEntry> &entries);
    Dispatcher(const std::vector<ProcessWorkloadEntry> &entries,
               const std::vector<std::vector<int>> &referenceStrings);

    void setWorkload(const std::vector<ProcessWorkloadEntry> &entries);
    void setReferenceStrings(const std::vector<std::vector<int>> &referenceStrings);
    void admitEligibleProcesses();
    bool runNext();
    void runUntilComplete();

    int getCurrentCycle() const;
    std::size_t pendingCount() const;
    int admittedCount() const;
    int rejectedCount() const;
    bool isComplete() const;
    std::size_t getReferenceStringCount() const;
    bool hasPageFaultTotal(int pid) const;
    int getPageFaultsForPid(int pid) const;
    bool hasSimulationError() const;
    const std::string &getSimulationErrorMessage() const;
    const std::vector<DispatcherEvent> &getEvents() const;
    const ProcessScheduler &getScheduler() const;

    static std::string eventTypeName(DispatcherEventType type);
    static std::string formatEvent(const DispatcherEvent &event);
    void printEvents(std::ostream &output) const;
};

#endif
