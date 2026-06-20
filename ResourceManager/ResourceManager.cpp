#include "ResourceManager.h"
#include "../Scheduler/scheduler.h"
#include <iostream>

using namespace std;

/*
 * Construtor do ResourceManager
 * Inicializa todos os recursos como livres (-1 indica "sem dono")
 * e associa opcionalmente o Scheduler responsável pelo bloqueio de processos
 */
ResourceManager::ResourceManager(Scheduler* scheduler)
    : scheduler(scheduler)
{
    scannerOwner = -1;
    modemOwner = -1;

    // inicializa arrays de recursos com 2 instâncias cada
    for (int i = 0; i < 2; i++)
    {
        printerOwners[i] = -1;
        sataOwners[i] = -1;
    }
}

/*
 * Permite alterar o Scheduler associado ao ResourceManager
 * Útil caso o sistema seja inicializado em etapas separadas
 */
void ResourceManager::setScheduler(Scheduler* scheduler)
{
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
blockedBy ResourceManager::canAllocate(const ResourceRequest& req)
{
    blockedBy blockeds;

    // Scanner bloqueado se foi solicitado e já está ocupado
    blockeds.scanner = !(req.scanner && scannerOwner == -1);

    // Modem bloqueado se foi solicitado e já está ocupado
    blockeds.modem   = !(req.modem && modemOwner == -1);

    // Conta quantas impressoras estão livres
    int printerFree = 0;
    for (int i = 0; i < 2; i++)
        if (printerOwners[i] == -1)
            printerFree++;

    // Bloqueado se solicitado e não há impressoras livres
    blockeds.printer = !(req.printer && printerFree <= 0);

    // Conta quantos SATA estão livres
    int sataFree = 0;
    for (int i = 0; i < 2; i++)
        if (sataOwners[i] == -1)
            sataFree++;

    // Bloqueado se solicitado e não há SATA livres
    blockeds.sata = !(req.sata && sataFree <= 0);

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
bool ResourceManager::allocate(const ResourceRequest& req)
{
    blockedBy blocked_by = canAllocate(req);

    bool ok = true;

    // Verifica se algum recurso solicitado está bloqueado
    if (req.scanner && blocked_by.scanner) ok = false;
    if (req.modem   && blocked_by.modem)   ok = false;
    if (req.printer && blocked_by.printer) ok = false;
    if (req.sata    && blocked_by.sata)    ok = false;

    /*
     * Caso algum recurso esteja indisponível:
     * - envia o processo para a fila BLOCKED_IO
     * - informa quais recursos causaram o bloqueio
     */
    if (!ok)
    {
        if (scheduler)
        {
            scheduler->blockProcess(req.pid, blocked_by);
        }
        return false;
    }

    /*
     * Se chegou aqui, todos os recursos necessários estão livres
     * Realiza a alocação definitiva
     */

    if (req.scanner)
    {
        scannerOwner = req.pid;
    }

    if (req.modem)
    {
        modemOwner = req.pid;
    }

    if (req.printer)
    {
        for (int i = 0; i < 2; i++)
        {
            if (printerOwners[i] == -1)
            {
                printerOwners[i] = req.pid;
                break;
            }
        }
    }

    if (req.sata)
    {
        for (int i = 0; i < 2; i++)
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
    if (scannerOwner == pid)
    {
        scannerOwner = -1;
    }

    if (modemOwner == pid)
    {
        modemOwner = -1;
    }

    for (int i = 0; i < 2; i++)
    {
        if (printerOwners[i] == pid)
        {
            printerOwners[i] = -1;
        }

        if (sataOwners[i] == pid)
        {
            sataOwners[i] = -1;
        }
    }
}

/*
 * Exibe o estado atual de todos os recursos do sistema
 * Útil para debug e demonstração do trabalho
 */
void ResourceManager::printStatus() const
{
    cout << "\n===== RESOURCE STATUS =====\n";

    cout << "Scanner: ";
    if(scannerOwner == -1)
        cout << "FREE\n";
    else
        cout << "PID " << scannerOwner << "\n";

    cout << "Modem: ";
    if(modemOwner == -1)
        cout << "FREE\n";
    else
        cout << "PID " << modemOwner << "\n";

    cout << "Printer 0: ";
    if(printerOwners[0] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << printerOwners[0] << "\n";

    cout << "Printer 1: ";
    if(printerOwners[1] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << printerOwners[1] << "\n";

    cout << "SATA 0: ";
    if(sataOwners[0] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << sataOwners[0] << "\n";

    cout << "SATA 1: ";
    if(sataOwners[1] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << sataOwners[1] << "\n";

    cout << "===========================\n";
}