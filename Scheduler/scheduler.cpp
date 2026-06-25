#include "scheduler.h"
#include "../ResourceManager/ResourceManager.h"

/*
 * Associa um ResourceManager ao Scheduler.
 *
 * O Scheduler precisa conhecer o ResourceManager para poder reaplicar
 * requisições quando recursos são liberados.
 */
void Scheduler::setResourceManager(ResourceManager *rm)
{
    this->resourceManager = rm;
}

/*
 * Construtor do Scheduler.
 * A fila de processos bloqueados começa vazia por padrão.
 */
Scheduler::Scheduler()
{
}

/*
 * Adiciona um processo à fila de bloqueio.
 *
 * A fila armazena tanto o pedido original quanto o motivo do bloqueio.
 * Isso permite que o Scheduler tente reavaliar a requisição completa
 * mais tarde, quando recursos forem liberados.
 */
void Scheduler::blockProcess(const ResourceRequest &request, const blockedBy &reason)
{
    if (!isBlocked(request.pid))
    {
        blockedIO.push({request.pid, request, reason});
    }
}

/*
 * Remove um processo específico da fila de bloqueados.
 *
 * Essa operação recria a fila sem o processo informado.
 */
void Scheduler::unblockProcess(int pid)
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

/*
 * Retorna a fila de processos bloqueados.
 *
 * Essa função é usada principalmente para inspeção ou testes.
 */
std::queue<BlockedProcess> &Scheduler::getBlockedQueue()
{
    return blockedIO;
}

/*
 * Verifica se um processo já está na fila de bloqueio.
 *
 * Para não adicionar o mesmo PID duas vezes, essa função percorre a fila
 * e verifica se o PID já existe.
 */
bool Scheduler::isBlocked(int pid) const
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

/*
 * Reavalia todos os processos bloqueados.
 *
 * Quando algum recurso é liberado pelo ResourceManager, o Scheduler
 * chama essa função para tentar realocar os pedidos dos processos
 * que estavam bloqueados.
 */
void Scheduler::checkBlockedProcesses()
{
    if (!resourceManager)
        return;

    std::queue<BlockedProcess> nextBlockedIO;

    while (!blockedIO.empty())
    {
        BlockedProcess current = blockedIO.front();
        blockedIO.pop();

        if (resourceManager->allocate(current.request, false))
        {
            // Se a alocação foi bem-sucedida, o processo sai da fila de bloqueados.
        }
        else
        {
            nextBlockedIO.push(current);
        }
    }

    blockedIO = nextBlockedIO;
}