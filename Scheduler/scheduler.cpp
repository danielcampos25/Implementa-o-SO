#include "scheduler.h"
#include "../ResourceManager/ResourceManager.h"

void Scheduler::setResourceManager(ResourceManager *rm)
{
    this->resourceManager = rm;
}

Scheduler::Scheduler()
{
    // Construtor vazio: fila de bloqueados começa vazia automaticamente
}

/*
 * Adiciona um processo na fila de bloqueio (BLOCKED_IO)
 * junto com o motivo do bloqueio (quais recursos não estavam disponíveis)
 */
void Scheduler::blockProcess(const ResourceRequest &request, const blockedBy &reason)
{
    // Evita adicionar o mesmo processo múltiplas vezes na fila de bloqueio
    if (!isBlocked(request.pid))
    {
        blockedIO.push({request.pid, request, reason});
    }
}

/*
 * Remove um processo específico da fila de bloqueio
 * Usado quando o processo consegue obter os recursos e volta a READY
 */
void Scheduler::unblockProcess(int pid)
{
    std::queue<BlockedProcess> temp;

    // Percorre toda a fila atual de bloqueados
    while (!blockedIO.empty())
    {
        BlockedProcess current = blockedIO.front();
        blockedIO.pop();

        // Mantém todos os processos que NÃO são o que queremos remover
        if (current.pid != pid)
        {
            temp.push(current);
        }
    }

    // Substitui a fila antiga pela nova (sem o processo removido)
    blockedIO.swap(temp);
}

/*
 * Retorna referência direta para a fila de bloqueados
 * Permite ao escalonador ou sistema reavaliar processos bloqueados
 * quando algum recurso for liberado
 */
std::queue<BlockedProcess> &Scheduler::getBlockedQueue()
{
    return blockedIO;
}

/*
 * Verifica se um processo já está na fila de bloqueio
 * Evita duplicação de entradas na blockedIO
 */
bool Scheduler::isBlocked(int pid) const
{
    std::queue<BlockedProcess> temp = blockedIO;

    // Percorre cópia da fila para não alterar a original
    while (!temp.empty())
    {
        if (temp.front().pid == pid)
        {
            return true; // processo já está bloqueado
        }
        temp.pop();
    }

    return false; // processo não está bloqueado
}

void Scheduler::checkBlockedProcesses()
{
    if (!resourceManager)
        return;

    std::queue<BlockedProcess> nextBlockedIO;

    // Percorremos a fila de bloqueados
    while (!blockedIO.empty())
    {
        BlockedProcess current = blockedIO.front();
        blockedIO.pop();

        // Tentamos alocar novamente com os mesmos requisitos originais
        // A função allocate do ResourceManager deve ser chamada aqui
        // NOTA: Como você já está dentro de um lock_guard no release,
        // cuidado com deadlocks recursivos se o allocate também travar o mutex.

        if (resourceManager->allocate(current.request, false))
        {
            // Sucesso! O processo pode voltar para a fila de prontos
            // readyQueue.push(current.pid);
            // std::cout << "Processo " << current.pid << " desbloqueado.\n";
        }
        else
        {
            // Ainda não conseguiu, mantém na fila de bloqueados
            nextBlockedIO.push(current);
        }
    }

    // Atualiza a fila original com os que continuam bloqueados
    blockedIO = nextBlockedIO;
}