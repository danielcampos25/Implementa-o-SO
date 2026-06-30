#include "ResourceManager.h"
#include "../ProcessScheduler/ProcessScheduler.h"
#include <iostream>
#include <mutex>
#include <array>

using namespace std;

/*
 * Construtor do ResourceManager
 *
 * Inicializa todos os recursos como livres:
 * - scannerOwner = -1
 * - modemOwner = -1
 * - printerOwners[i] = -1
 * - sataOwners[i] = -1
 *
 * O valor -1 indica que o recurso não possui nenhum processo
 * associado no momento.
 *
 * Também armazena uma referência para o ProcessScheduler, que será
 * utilizado para bloquear e desbloquear processos quando
 * houver disputa por recursos de E/S.
 */
ResourceManager::ResourceManager(ProcessScheduler *scheduler)
    : scannerOwner(-1), modemOwner(-1), printerOwners(), sataOwners(), scheduler(scheduler)
{
    printerOwners.fill(-1);
    sataOwners.fill(-1);
}

/*
 * Atualiza a referência para o ProcessScheduler utilizado pelo sistema.
 *
 * Essa função existe para permitir que o ResourceManager e o
 * ProcessScheduler sejam criados separadamente e conectados posteriormente.
 */
void ResourceManager::setScheduler(ProcessScheduler *scheduler)
{
    std::lock_guard<std::recursive_mutex> guard(mtx);
    this->scheduler = scheduler;
}

/*
 * Verifica se os recursos solicitados pelo processo estão disponíveis.
 *
 * O retorno é uma estrutura blockedBy contendo, para cada recurso:
 *
 * true  -> o recurso está bloqueando a execução do processo
 * false -> o recurso está disponível para uso
 *
 * A verificação considera que um recurso já pertencente ao próprio
 * processo NÃO deve ser tratado como bloqueado.
 *
 * Nenhum estado interno é modificado nesta função; ela apenas
 * consulta a situação atual dos recursos.
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
 * Tenta realizar a alocação dos recursos solicitados por um processo.
 *
 * Fluxo:
 *
 * 1) Verifica a disponibilidade de todos os recursos solicitados.
 * 2) Caso algum recurso esteja indisponível:
 *      - o processo pode ser enviado para a fila BLOCKED_IO
 *        do ProcessScheduler (quando canBlock == true);
 *      - nenhum recurso é alocado;
 *      - a função retorna false.
 * 3) Caso todos os recursos estejam disponíveis:
 *      - a alocação é realizada de forma atômica;
 *      - o processo passa a ser o proprietário dos recursos;
 *      - a função retorna true.
 *
 * O parâmetro canBlock é utilizado principalmente durante as
 * reavaliações de processos bloqueados, evitando que o mesmo
 * processo seja inserido repetidamente na fila BLOCKED_IO.
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
 * Libera todos os recursos atualmente associados ao processo.
 *
 * Essa função normalmente é chamada quando o processo termina
 * sua execução ou deixa de utilizar os dispositivos de E/S.
 *
 * Após a liberação, o ProcessScheduler é notificado para verificar
 * se algum processo da fila BLOCKED_IO pode voltar para READY.
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
 * Exibe o estado atual de todos os recursos gerenciados.
 *
 * Para cada dispositivo é mostrado:
 * - FREE, quando não existe processo utilizando o recurso;
 * - PID X, quando o recurso está associado a um processo.
 *
 * Função utilizada principalmente para depuração e demonstração
 * do funcionamento do gerenciador de recursos.
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
