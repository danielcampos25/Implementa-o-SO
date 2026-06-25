#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include "../ResourceManager/ResourceManager.h"

struct blockedBy;

/*
 * BlockedProcess guarda o PID do processo bloqueado, a requisição
 * original e o motivo do bloqueio.
 */
struct BlockedProcess
{
    int pid;                 // PID do processo bloqueado
    ResourceRequest request; // Requisição original de recursos
    blockedBy reason;        // Recursos que estavam indisponíveis
};

/*
 * Scheduler gerencia a fila de processos bloqueados por I/O.
 * Ele mantém a fila blockedIO e tenta acordar processos quando
 * recursos ficam disponíveis.
 */
class Scheduler
{
private:
    std::queue<BlockedProcess> blockedIO; // Fila de bloqueados
    ResourceManager *resourceManager = nullptr;

public:
    Scheduler();
    void setResourceManager(ResourceManager *rm);

    void blockProcess(const ResourceRequest &request, const blockedBy &reason);
    void unblockProcess(int pid);

    void checkBlockedProcesses();

    std::queue<BlockedProcess> &getBlockedQueue();
    bool isBlocked(int pid) const;
};

#endif