#ifndef PROCESS_H
#define PROCESS_H

enum class ProcessState
{
    New,
    Ready,
    Running,
    Blocked,
    Finished
};

enum class ProcessType
{
    RealTime,
    User
};

class Process
{
private:
    int pid;
    int startTime;
    int priority;
    int processorTime;
    int remainingTime;
    int memoryBlocks;
    int printerRequest;
    int scannerRequest;
    int modemRequest;
    int sataDiskRequest;
    ProcessState state;
    int waitingCycles;

public:
    static constexpr int REAL_TIME_PRIORITY = 0;
    static constexpr int USER_PRIORITY_HIGH = 1;
    static constexpr int USER_PRIORITY_MEDIUM = 2;
    static constexpr int USER_PRIORITY_LOW = 3;

    Process(int pid,
            int startTime,
            int priority,
            int processorTime,
            int memoryBlocks,
            int printerRequest,
            int scannerRequest,
            int modemRequest,
            int sataDiskRequest);

    static bool isValidPriority(int priority);

    int getPid() const;
    int getStartTime() const;
    int getPriority() const;
    int getProcessorTime() const;
    int getRemainingTime() const;
    int getMemoryBlocks() const;
    int getPrinterRequest() const;
    int getScannerRequest() const;
    int getModemRequest() const;
    int getSataDiskRequest() const;
    ProcessState getState() const;
    int getWaitingCycles() const;

    bool isRealTime() const;
    bool isUserProcess() const;
    ProcessType getType() const;
    bool isFinished() const;

    void setState(ProcessState newState);
    void markReady();
    void markRunning();
    void markFinished();
    void markBlocked();
    int executeFor(int timeUnits);
    int executeUntilFinished();

    void incrementWaitingCycles();
    void resetWaitingCycles();
    bool canPromotePriority() const;
    bool promotePriority();
};

#endif
