    #include "ProcessScheduler.h"
    #include "../ResourceManager/ResourceManager.h"
    #include <stdexcept>

    namespace
    {
    bool isBinaryResourceRequest(int request)
    {
        return request == 0 || request == 1;
    }

    bool hasInvalidResourceRequest(int printerRequest,
                                int scannerRequest,
                                int modemRequest,
                                int sataDiskRequest)
    {
        return !isBinaryResourceRequest(printerRequest) ||
            !isBinaryResourceRequest(scannerRequest) ||
            !isBinaryResourceRequest(modemRequest) ||
            !isBinaryResourceRequest(sataDiskRequest);
    }

    bool hasIoRequest(int printerRequest,
                    int scannerRequest,
                    int modemRequest,
                    int sataDiskRequest)
    {
        return printerRequest != 0 || scannerRequest != 0 ||
            modemRequest != 0 || sataDiskRequest != 0;
    }
    }

    ProcessScheduler::ProcessScheduler(int maxProcesses, int agingThreshold)
        : readyQueues(),
        processes(),
        events(),
        currentCycle(0),
        lastConsumedTime(0),
        nextPid(0),
        maxProcesses(maxProcesses),
        agingThreshold(agingThreshold)
    {
    }

    void ProcessScheduler::recordEvent(SchedulerEventType type, int pid, const std::string &message)
    {
        int priority = -1;
        int remainingTime = -1;

        if (hasProcess(pid))
        {
            const Process &process = processes[pid];
            priority = process.getPriority();
            remainingTime = process.getRemainingTime();
        }

        events.push_back({currentCycle, type, pid, priority, remainingTime, message});
    }

    bool ProcessScheduler::hasProcess(int pid) const
    {
        return pid >= 0 && pid < static_cast<int>(processes.size());
    }

    Process &ProcessScheduler::getMutableProcess(int pid)
    {
        if (!hasProcess(pid))
        {
            throw std::out_of_range("Processo inexistente");
        }
        return processes[pid];
    }

    const Process &ProcessScheduler::getExistingProcess(int pid) const
    {
        if (!hasProcess(pid))
        {
            throw std::out_of_range("Processo inexistente");
        }
        return processes[pid];
    }

    void ProcessScheduler::enqueueReadyProcess(int pid)
    {
        Process &process = getMutableProcess(pid);

        if (process.isFinished())
        {
            return;
        }

        process.markReady();
        readyQueues[process.getPriority()].push_back(pid);
    }

    std::optional<int> ProcessScheduler::popNextReadyPid()
    {
        removeFinishedFromReadyQueues();

        for (std::deque<int> &queue : readyQueues)
        {
            while (!queue.empty())
            {
                const int pid = queue.front();
                queue.pop_front();

                if (hasProcess(pid) && !processes[pid].isFinished())
                {
                    return pid;
                }
            }
        }

        return std::nullopt;
    }

    void ProcessScheduler::advanceWaitingCycles(int elapsedCycles, int runningPid)
    {
        for (int cycle = 0; cycle < elapsedCycles; ++cycle)
        {
            ++currentCycle;

            for (const std::deque<int> &queue : readyQueues)
            {
                for (int pid : queue)
                {
                    if (pid != runningPid && hasProcess(pid))
                    {
                        processes[pid].incrementWaitingCycles();
                    }
                }
            }

            applyAging();
        }
    }

    void ProcessScheduler::applyAging()
    {
        for (int priority = Process::USER_PRIORITY_LOW; priority >= Process::USER_PRIORITY_MEDIUM; --priority)
        {
            std::deque<int> remaining;

            while (!readyQueues[priority].empty())
            {
                const int pid = readyQueues[priority].front();
                readyQueues[priority].pop_front();

                if (!hasProcess(pid) || processes[pid].isFinished())
                {
                    continue;
                }

                Process &process = processes[pid];
                if (process.getWaitingCycles() >= agingThreshold && process.promotePriority())
                {
                    readyQueues[process.getPriority()].push_back(pid);
                    recordEvent(SchedulerEventType::Aging, pid, "Processo promovido por aging");
                }
                else
                {
                    remaining.push_back(pid);
                }
            }

            readyQueues[priority].swap(remaining);
        }
    }

    void ProcessScheduler::removeFinishedFromReadyQueues()
    {
        for (std::deque<int> &queue : readyQueues)
        {
            std::deque<int> filtered;
            while (!queue.empty())
            {
                const int pid = queue.front();
                queue.pop_front();

                if (hasProcess(pid) && !processes[pid].isFinished())
                {
                    filtered.push_back(pid);
                }
            }
            queue.swap(filtered);
        }
    }

    bool ProcessScheduler::hasUnfinishedProcess() const
    {
        for (const Process &process : processes)
        {
            if (!process.isFinished())
            {
                return true;
            }
        }

        return false;
    }

    int ProcessScheduler::createProcess(int startTime,
                                        int priority,
                                        int processorTime,
                                        int memoryBlocks,
                                        int printerRequest,
                                        int scannerRequest,
                                        int modemRequest,
                                        int sataDiskRequest)
    {
        if (!Process::isValidPriority(priority))
        {
            recordEvent(SchedulerEventType::Rejected, -1, "Prioridade invalida");
            return -1;
        }

        if (startTime < 0 || processorTime < 0 || memoryBlocks < 0)
        {
            recordEvent(SchedulerEventType::Rejected, -1, "Campos numericos invalidos");
            return -1;
        }

        if (hasInvalidResourceRequest(printerRequest, scannerRequest, modemRequest, sataDiskRequest))
        {
            recordEvent(SchedulerEventType::Rejected, -1, "Requisicoes de recursos invalidas");
            return -1;
        }

        if (priority == Process::REAL_TIME_PRIORITY &&
            hasIoRequest(printerRequest, scannerRequest, modemRequest, sataDiskRequest))
        {
            recordEvent(SchedulerEventType::Rejected, -1, "Processo de tempo real nao pode requisitar E/S");
            return -1;
        }

        if (static_cast<int>(processes.size()) >= maxProcesses)
        {
            recordEvent(SchedulerEventType::Rejected, -1, "Limite de processos excedido");
            return -1;
        }

        const int pid = nextPid++;
        processes.emplace_back(pid, startTime, priority, processorTime, memoryBlocks,
                            printerRequest, scannerRequest, modemRequest, sataDiskRequest);
        recordEvent(SchedulerEventType::Created, pid, "Processo criado");

        if (processorTime == 0)
        {
            processes[pid].markFinished();
            recordEvent(SchedulerEventType::Finish, pid, "Processo finalizado sem despacho de CPU");
            return pid;
        }

        enqueueReadyProcess(pid);
        return pid;
    }

    bool ProcessScheduler::hasReadyProcess() const
    {
        for (const std::deque<int> &queue : readyQueues)
        {
            if (!queue.empty())
            {
                return true;
            }
        }

        return false;
    }

    std::optional<int> ProcessScheduler::selectNextProcessId() const
    {
        for (const std::deque<int> &queue : readyQueues)
        {
            for (int pid : queue)
            {
                if (hasProcess(pid) && !processes[pid].isFinished())
                {
                    return pid;
                }
            }
        }

        return std::nullopt;
    }

    bool ProcessScheduler::runNext()
    {
        lastConsumedTime = 0;
        std::optional<int> maybePid = popNextReadyPid();

        if (!maybePid.has_value())
        {
            recordEvent(SchedulerEventType::Idle, -1, "Nenhum processo pronto");
            return false;
        }

        const int pid = maybePid.value();
        Process &process = getMutableProcess(pid);
        process.markRunning();
        process.resetWaitingCycles();
        recordEvent(SchedulerEventType::Dispatch, pid, "Processo despachado");

        if (process.isRealTime())
        {
            const int consumed = process.executeUntilFinished();
            lastConsumedTime = consumed;
            advanceWaitingCycles(consumed, pid);
            recordEvent(SchedulerEventType::Finish, pid, "Processo de tempo real finalizado");
            return true;
        }

        const int consumed = process.executeFor(USER_QUANTUM);
        lastConsumedTime = consumed;
        advanceWaitingCycles(consumed, pid);
        recordEvent(SchedulerEventType::Quantum, pid, "Processo de usuario consumiu quantum");

        if (process.isFinished())
        {
            recordEvent(SchedulerEventType::Finish, pid, "Processo de usuario finalizado");
        }
        else
        {
            process.resetWaitingCycles();
            enqueueReadyProcess(pid);
        }

        return true;
    }

    void ProcessScheduler::runUntilComplete()
    {
        while (hasUnfinishedProcess() && hasReadyProcess())
        {
            runNext();
        }

        if (!hasReadyProcess() && hasUnfinishedProcess())
        {
            recordEvent(SchedulerEventType::Idle, -1, "Simulacao encerrada sem processos prontos");
        }
    }

    std::size_t ProcessScheduler::queueSize(int priority) const
    {
        if (!Process::isValidPriority(priority))
        {
            return 0;
        }

        std::size_t count = 0;
        for (int pid : readyQueues[priority])
        {
            if (hasProcess(pid) && !processes[pid].isFinished())
            {
                ++count;
            }
        }

        return count;
    }

    bool ProcessScheduler::empty() const
    {
        for (int priority = 0; priority < QUEUE_COUNT; ++priority)
        {
            if (queueSize(priority) > 0)
            {
                return false;
            }
        }

        return true;
    }

    std::size_t ProcessScheduler::processCount() const
    {
        return processes.size();
    }

    int ProcessScheduler::getCurrentCycle() const
    {
        return currentCycle;
    }

    int ProcessScheduler::getLastConsumedTime() const
    {
        return lastConsumedTime;
    }

    const Process &ProcessScheduler::getProcess(int pid) const
    {
        return getExistingProcess(pid);
    }

    ProcessState ProcessScheduler::getProcessState(int pid) const
    {
        return getExistingProcess(pid).getState();
    }

    int ProcessScheduler::getProcessPriority(int pid) const
    {
        return getExistingProcess(pid).getPriority();
    }

    int ProcessScheduler::getProcessRemainingTime(int pid) const
    {
        return getExistingProcess(pid).getRemainingTime();
    }

    int ProcessScheduler::getProcessWaitingCycles(int pid) const
    {
        return getExistingProcess(pid).getWaitingCycles();
    }

    const std::vector<SchedulerEvent> &ProcessScheduler::getEvents() const
    {
        return events;
    }

    void ProcessScheduler::setResourceManager(ResourceManager *rm)
{
    resourceManager = rm;
}

void ProcessScheduler::blockProcess(const ResourceRequest &request,
                                    const blockedBy &reason)
{
    if (!isBlocked(request.pid))
    {
        blockedIO.push({request.pid, request, reason});
    }
}

void ProcessScheduler::unblockProcess(int pid)
{
    std::queue<BlockedProcess> temp;

    while (!blockedIO.empty())
    {
        BlockedProcess current = blockedIO.front();
        blockedIO.pop();

        if (current.pid != pid)
        {
            temp.push(current);
        }
    }

    blockedIO.swap(temp);
}

bool ProcessScheduler::isBlocked(int pid) const
{
    std::queue<BlockedProcess> temp = blockedIO;

    while (!temp.empty())
    {
        if (temp.front().pid == pid)
        {
            return true;
        }
        temp.pop();
    }

    return false;
}

std::queue<BlockedProcess> &ProcessScheduler::getBlockedQueue()
{
    return blockedIO;
}

void ProcessScheduler::checkBlockedProcesses()
{
    if (!resourceManager)
        return;

    std::queue<BlockedProcess> nextBlocked;

    while (!blockedIO.empty())
    {
        BlockedProcess current = blockedIO.front();
        blockedIO.pop();

        if (resourceManager->allocate(current.request, false))
        {
            // Processo desbloqueado.
            // Futuramente aqui ele pode voltar para READY.
        }
        else
        {
            nextBlocked.push(current);
        }
    }

    blockedIO.swap(nextBlocked);
}
