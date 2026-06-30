#ifndef PROCESS_SCHEDULER_H
#define PROCESS_SCHEDULER_H

#include "../Process/Process.h"
#include "../ResourceManager/Resource.h"

#include <array>
#include <deque>
#include <optional>
#include <queue>
#include <string>
#include <vector>

// Forward declaration para evitar dependência circular
class ResourceManager;

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

/*
 * Processo bloqueado por recursos de E/S
 */
struct BlockedProcess
{
    int pid;
    ResourceRequest request;
    blockedBy reason;
};

class ProcessScheduler
{
private:
    static constexpr int QUEUE_COUNT = 4;

    // Filas Ready
    std::array<std::deque<int>, QUEUE_COUNT> readyQueues;

    // Fila BLOCKED por E/S
    std::queue<BlockedProcess> blockedIO;

    // Gerenciador de recursos
    ResourceManager *resourceManager = nullptr;

    // Tabela de processos
    std::vector<Process> processes;

    // Eventos
    std::vector<SchedulerEvent> events;

    int currentCycle;
    int lastConsumedTime;
    int nextPid;
    int maxProcesses;
    int agingThreshold;

    void recordEvent(SchedulerEventType type,
                     int pid,
                     const std::string &message);

    bool hasProcess(int pid) const;

    Process &getMutableProcess(int pid);

    const Process &getExistingProcess(int pid) const;

    void enqueueReadyProcess(int pid);

    void removeFromReadyQueues(int pid);

    std::optional<int> popNextReadyPid();

    void advanceWaitingCycles(int elapsedCycles,
                              int runningPid);

    void applyAging();

    void removeFinishedFromReadyQueues();

    bool hasUnfinishedProcess() const;

public:
    static constexpr int MAX_PROCESSES = 1000;
    static constexpr int USER_QUANTUM = 1;
    static constexpr int DEFAULT_AGING_THRESHOLD = 5;

    explicit ProcessScheduler(
        int maxProcesses = MAX_PROCESSES,
        int agingThreshold = DEFAULT_AGING_THRESHOLD);

    //--------------------------------------------------
    // Criação de processos
    //--------------------------------------------------

    int createProcess(int startTime,
                      int priority,
                      int processorTime,
                      int memoryBlocks,
                      int printerRequest,
                      int scannerRequest,
                      int modemRequest,
                      int sataDiskRequest);

    //--------------------------------------------------
    // Escalonamento
    //--------------------------------------------------

    bool hasReadyProcess() const;

    std::optional<int> selectNextProcessId() const;

    bool runNext();

    void runUntilComplete();

    //--------------------------------------------------
    // Integração com ResourceManager
    //--------------------------------------------------

    void setResourceManager(ResourceManager *rm);

    void blockProcess(const ResourceRequest &request,
                      const blockedBy &reason);

    void unblockProcess(int pid);

    void checkBlockedProcesses();

    bool isBlocked(int pid) const;

    std::queue<BlockedProcess> &getBlockedQueue();

    //--------------------------------------------------
    // Consultas
    //--------------------------------------------------

    std::size_t queueSize(int priority) const;

    bool empty() const;

    std::size_t processCount() const;

    int getCurrentCycle() const;

    int getLastConsumedTime() const;

    const Process &getProcess(int pid) const;

    ProcessState getProcessState(int pid) const;

    int getProcessPriority(int pid) const;

    int getProcessRemainingTime(int pid) const;

    int getProcessWaitingCycles(int pid) const;

    const std::vector<SchedulerEvent> &getEvents() const;
};

#endif
