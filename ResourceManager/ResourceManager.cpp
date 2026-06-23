#include "ResourceManager.h"
#include "../Scheduler/scheduler.h"
#include <iostream>
#include <mutex>
#include <array>

using namespace std;

/*
 * Construtor do ResourceManager
 * Inicializa todos os recursos como livres (-1 indica "sem dono")
 * e associa opcionalmente o Scheduler responsável pelo bloqueio de processos
 */
ResourceManager::ResourceManager(Scheduler *scheduler)
    : scannerOwner(-1), modemOwner(-1), printerOwners(), sataOwners(), scheduler(scheduler)
{
    printerOwners.fill(-1);
    sataOwners.fill(-1);
}

/*
 * Permite alterar o Scheduler associado ao ResourceManager
 * Útil caso o sistema seja inicializado em etapas separadas
 */
void ResourceManager::setScheduler(Scheduler *scheduler)
{
    std::lock_guard<std::recursive_mutex> guard(mtx);
    this->scheduler = scheduler;
}

/*
 * Verifica quais recursos estão bloqueando a requisição atual
 * Retorna um struct (blockedBy) indicando individualmente cada recurso
 *
 * OBS:
 * true  -> recurso está bloqueando
 * false -> recurso está disponível para uso
 */
blockedBy ResourceManager::canAllocate(const ResourceRequest &req)
{
    std::lock_guard<std::recursive_mutex> guard(mtx);
    blockedBy blockeds{};
    blockeds.pid = req.pid;

    // Bloqueia apenas se o recurso estiver ocupado por OUTRO processo
    blockeds.scanner = (req.scanner && scannerOwner != -1 && scannerOwner != req.pid);
    blockeds.modem = (req.modem && modemOwner != -1 && modemOwner != req.pid);

    int printerAvailableCount = 0;
    for (int i = 0; i < 2; ++i)
    {
        if (printerOwners[i] == -1 || printerOwners[i] == req.pid)
            printerAvailableCount++;
    }
    blockeds.printer = (req.printer && printerAvailableCount == 0);

    int sataAvailableCount = 0;
    for (int i = 0; i < 2; ++i)
    {
        if (sataOwners[i] == -1 || sataOwners[i] == req.pid)
            sataAvailableCount++;
    }
    blockeds.sata = (req.sata && sataAvailableCount == 0);

    return blockeds;
}

/*
 * Tenta alocar todos os recursos solicitados pelo processo
 *
 * Estratégia usada:
 * - Verifica todos os recursos primeiro (alocação atômica)
 * - Se qualquer recurso falhar:
 *      -> processo é bloqueado no Scheduler
 *      -> não aloca nenhum recurso (evita estado parcial)
 * - Se todos estiverem disponíveis:
 *      -> realiza a alocação completa
 */
bool ResourceManager::allocate(const ResourceRequest &req, bool canBlock)
{
    std::lock_guard<std::recursive_mutex> guard(mtx);

    blockedBy blocked_by = canAllocate(req);

    // Verifica se algum recurso solicitado está bloqueado
    bool isBlocked = (req.scanner && blocked_by.scanner) ||
                     (req.modem && blocked_by.modem) ||
                     (req.printer && blocked_by.printer) ||
                     (req.sata && blocked_by.sata);

    if (isBlocked)
    {
        // Só bloqueia no scheduler se permitido (evita duplicar em rechecagens)
        if (canBlock && scheduler)
        {
            scheduler->blockProcess(req, blocked_by);
        }
        return false;
    }

    // Alocação (se chegou aqui, o caminho está livre)
    if (req.scanner)
        scannerOwner = req.pid;
    if (req.modem)
        modemOwner = req.pid;

    if (req.printer)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (printerOwners[i] == -1)
            {
                printerOwners[i] = req.pid;
                break; // Aloca apenas uma instância por chamada conforme sua lógica
            }
        }
    }

    if (req.sata)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (sataOwners[i] == -1)
            {
                sataOwners[i] = req.pid;
                break;
            }
        }
    }

    return true;
}

/*
 * Libera todos os recursos que estavam sendo usados por um processo
 * chamado pelo PID quando o processo termina ou perde a CPU
 */
void ResourceManager::release(int pid)
{
    std::lock_guard<std::recursive_mutex> guard(mtx);

    if (scannerOwner == pid)
        scannerOwner = -1;

    if (modemOwner == pid)
        modemOwner = -1;

    for (int i = 0; i < 2; ++i)
    {
        if (printerOwners[i] == pid)
            printerOwners[i] = -1;

        if (sataOwners[i] == pid)
            sataOwners[i] = -1;
    }
    // NOTIFICAÇÃO: Assim que algo for liberado,
    // pedimos ao scheduler para tentar acordar processos bloqueados
    if (scheduler)
    {
        scheduler->checkBlockedProcesses();
    }
}

/*
 * Exibe o estado atual de todos os recursos do sistema
 * Útil para debug e demonstração do trabalho
 */
void ResourceManager::printStatus() const
{
    std::lock_guard<std::recursive_mutex> guard(mtx);

    cout << "\n===== RESOURCE STATUS =====\n";

    cout << "Scanner: ";
    if (scannerOwner == -1)
        cout << "FREE\n";
    else
        cout << "PID " << scannerOwner << "\n";

    cout << "Modem: ";
    if (modemOwner == -1)
        cout << "FREE\n";
    else
        cout << "PID " << modemOwner << "\n";

    cout << "Printer 0: ";
    if (printerOwners[0] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << printerOwners[0] << "\n";

    cout << "Printer 1: ";
    if (printerOwners[1] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << printerOwners[1] << "\n";

    cout << "SATA 0: ";
    if (sataOwners[0] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << sataOwners[0] << "\n";

    cout << "SATA 1: ";
    if (sataOwners[1] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << sataOwners[1] << "\n";

    cout << "===========================\n";
}