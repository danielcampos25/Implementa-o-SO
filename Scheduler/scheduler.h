#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>

struct blockedBy;

struct BlockedProcess
{
    int pid;
    blockedBy reason;
};

class Scheduler
{
private:
    // fila global de processos bloqueados por I/O (recursos)
    std::queue<BlockedProcess> blockedIO;

public:

    Scheduler();

    // adiciona processo na fila de bloqueio
    void blockProcess(int pid, const blockedBy& reason);

    // remove processo da fila de bloqueio (quando puder voltar a READY)
    void unblockProcess(int pid);

    // acesso à fila (para rechecagem pelo sistema)
    std::queue<BlockedProcess>& getBlockedQueue();

    // utilitário
    bool isBlocked(int pid) const;
};

#endif