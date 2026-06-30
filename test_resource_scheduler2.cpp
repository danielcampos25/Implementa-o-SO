#include "ResourceManager/ResourceManager.h"
#include "ProcessScheduler/ProcessScheduler.h"
#include <cassert>
#include <iostream>

/*
 * Teste 2: estresse da fila de bloqueio.
 *
 * Objetivos:
 * - P1 consome vários recursos.
 * - P2 e P3 ficam bloqueados por disputas de recursos.
 * - Quando P1 libera seus recursos, todos os bloqueados devem ser reavaliados.
 */
int main()
{
    ProcessScheduler scheduler;
    ResourceManager rm(&scheduler);
    scheduler.setResourceManager(&rm);
    rm.setScheduler(&scheduler);

    const int pid1 = scheduler.createProcess(0, 1, 3, 64, 1, 1, 1, 1);
    const int pid2 = scheduler.createProcess(0, 1, 3, 64, 0, 1, 0, 0);
    const int pid3 = scheduler.createProcess(0, 1, 3, 64, 0, 0, 1, 0);

    // P1: Precisa de quase tudo (Scanner, Modem, Printer, SATA)
    ResourceRequest p1{pid1, true, true, true, true};
    // P2: Precisa de Scanner
    ResourceRequest p2{pid2, false, true, false, false};
    // P3: Precisa de Modem
    ResourceRequest p3{pid3, false, false, true, false};

    std::cout << "--- TESTE DE ESTRESSE: ALOCACAO EM CADEIA ---\n";

    // 1. P1 pega quase tudo
    rm.allocate(p1);

    // 2. P2 tenta pegar Scanner (deve bloquear)
    std::cout << "P2 tentando alocar...\n";
    assert(!rm.allocate(p2));

    // 3. P3 tenta pegar Modem (deve bloquear)
    std::cout << "P3 tentando alocar...\n";
    assert(!rm.allocate(p3));

    rm.printStatus();
    std::cout << "Fila de bloqueados: " << scheduler.getBlockedQueue().size() << " (Esperado: 2)\n";
    assert(scheduler.getBlockedQueue().size() == 2);
    assert(scheduler.getProcessState(pid2) == ProcessState::Blocked);
    assert(scheduler.getProcessState(pid3) == ProcessState::Blocked);

    // 4. P1 libera tudo
    std::cout << "\n--- P1 LIBERANDO TUDO ---\n";
    rm.release(pid1);

    // O que esperar aqui?
    // Como P1 liberou tudo, o checkBlockedProcesses deve rodar.
    // P2 deve pegar Scanner
    // P3 deve pegar Modem

    rm.printStatus();
    std::cout << "Fila de bloqueados apos liberacao: " << scheduler.getBlockedQueue().size() << " (Esperado: 0)\n";
    assert(scheduler.getBlockedQueue().empty());
    assert(scheduler.getProcessState(pid2) == ProcessState::Ready);
    assert(scheduler.getProcessState(pid3) == ProcessState::Ready);

    return 0;
}
