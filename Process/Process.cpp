#include "Process.h"

#include <algorithm>
#include <stdexcept>

Process::Process(int pid,
                 int startTime,
                 int priority,
                 int processorTime,
                 int memoryBlocks,
                 int printerRequest,
                 int scannerRequest,
                 int modemRequest,
                 int sataDiskRequest)
    : pid(pid),
      startTime(startTime),
      priority(priority),
      processorTime(processorTime),
      remainingTime(processorTime),
      memoryBlocks(memoryBlocks),
      printerRequest(printerRequest),
      scannerRequest(scannerRequest),
      modemRequest(modemRequest),
      sataDiskRequest(sataDiskRequest),
      state(ProcessState::New),
      waitingCycles(0)
{
    if (pid < 0)
    {
        throw std::invalid_argument("PID nao pode ser negativo");
    }
    if (!isValidPriority(priority))
    {
        throw std::invalid_argument("Prioridade invalida");
    }
    if (processorTime < 0)
    {
        throw std::invalid_argument("Tempo de processador nao pode ser negativo");
    }
    if (memoryBlocks < 0)
    {
        throw std::invalid_argument("Tamanho do conjunto de trabalho nao pode ser negativo");
    }
}

bool Process::isValidPriority(int priority)
{
    return priority >= REAL_TIME_PRIORITY && priority <= USER_PRIORITY_LOW;
}

int Process::getPid() const
{
    return pid;
}

int Process::getStartTime() const
{
    return startTime;
}

int Process::getPriority() const
{
    return priority;
}

int Process::getProcessorTime() const
{
    return processorTime;
}

int Process::getRemainingTime() const
{
    return remainingTime;
}

int Process::getMemoryBlocks() const
{
    return memoryBlocks;
}

int Process::getPrinterRequest() const
{
    return printerRequest;
}

int Process::getScannerRequest() const
{
    return scannerRequest;
}

int Process::getModemRequest() const
{
    return modemRequest;
}

int Process::getSataDiskRequest() const
{
    return sataDiskRequest;
}

ProcessState Process::getState() const
{
    return state;
}

int Process::getWaitingCycles() const
{
    return waitingCycles;
}

bool Process::isRealTime() const
{
    return priority == REAL_TIME_PRIORITY;
}

bool Process::isUserProcess() const
{
    return priority >= USER_PRIORITY_HIGH && priority <= USER_PRIORITY_LOW;
}

ProcessType Process::getType() const
{
    return isRealTime() ? ProcessType::RealTime : ProcessType::User;
}

bool Process::isFinished() const
{
    return state == ProcessState::Finished || remainingTime == 0;
}

void Process::setState(ProcessState newState)
{
    state = newState;
}

void Process::markReady()
{
    state = ProcessState::Ready;
}

void Process::markRunning()
{
    state = ProcessState::Running;
}

void Process::markFinished()
{
    remainingTime = 0;
    state = ProcessState::Finished;
}

int Process::executeFor(int timeUnits)
{
    if (timeUnits <= 0 || isFinished())
    {
        return 0;
    }

    const int consumed = std::min(timeUnits, remainingTime);
    remainingTime -= consumed;

    if (remainingTime == 0)
    {
        state = ProcessState::Finished;
    }

    return consumed;
}

int Process::executeUntilFinished()
{
    return executeFor(remainingTime);
}

void Process::incrementWaitingCycles()
{
    if (isUserProcess() && state == ProcessState::Ready)
    {
        ++waitingCycles;
    }
}

void Process::resetWaitingCycles()
{
    waitingCycles = 0;
}

bool Process::canPromotePriority() const
{
    return priority == USER_PRIORITY_LOW || priority == USER_PRIORITY_MEDIUM;
}

bool Process::promotePriority()
{
    if (!canPromotePriority())
    {
        return false;
    }

    --priority;
    resetWaitingCycles();
    return true;
}
