#include "ResourceManager/ResourceManager.h"
#include "ProcessScheduler/ProcessScheduler.h"
#include <cassert>
#include <iostream>

/*
 * Teste 3: verifica desbloqueios parciais em cadeia.
 *
 * Objetivos:
 * - P1, P2 e P3 alocam recursos distintos.
 * - P4 e P5 ficam bloqueados por recursos já ocupados.
 * - Quando P1 libera recursos, apenas um bloqueado deve ser atendido.
 * - Quando P2 libera recursos, o último bloqueado deve ser atendido.
 */
int main()
{
    ProcessScheduler scheduler;
    ResourceManager rm(&scheduler);
    scheduler.setResourceManager(&rm);
    rm.setScheduler(&scheduler);

    const int pid1 = scheduler.createProcess(0, 1, 3, 64, 1, 1, 0, 0);
    const int pid2 = scheduler.createProcess(0, 1, 3, 64, 0, 0, 1, 0);
    const int pid3 = scheduler.createProcess(0, 1, 3, 64, 0, 0, 0, 1);
    const int pid4 = scheduler.createProcess(0, 1, 3, 64, 1, 0, 1, 0);
    const int pid5 = scheduler.createProcess(0, 1, 3, 64, 0, 1, 0, 0);

    ResourceRequest p1{pid1, true, true, false, false};  // printer + scanner
    ResourceRequest p2{pid2, false, false, true, false}; // modem only
    ResourceRequest p3{pid3, false, false, false, true}; // sata only
    ResourceRequest p4{pid4, true, false, true, false};  // printer + modem
    ResourceRequest p5{pid5, false, true, false, false}; // scanner only

    std::cout << "=== TESTE 3: bloqueios em fila e desbloqueio parcial ===\n";
    std::cout << "P1, P2, P3 alocam recursos iniciais\n";
    bool allocated1 = rm.allocate(p1);
    bool allocated2 = rm.allocate(p2);
    bool allocated3 = rm.allocate(p3);
    std::cout << "P1: " << (allocated1 ? "sim" : "nao") << ", ";
    std::cout << "P2: " << (allocated2 ? "sim" : "nao") << ", ";
    std::cout << "P3: " << (allocated3 ? "sim" : "nao") << "\n";

    std::cout << "P4 e P5 tentam alocar e devem bloquear\n";
    bool allocated4 = rm.allocate(p4);
    bool allocated5 = rm.allocate(p5);
    std::cout << "P4: " << (allocated4 ? "sim" : "nao") << ", ";
    std::cout << "P5: " << (allocated5 ? "sim" : "nao") << "\n";
    std::cout << "Bloqueados apos tentativas: " << scheduler.getBlockedQueue().size() << " (esperado: 2)\n";
    assert(allocated1);
    assert(allocated2);
    assert(allocated3);
    assert(!allocated4);
    assert(!allocated5);
    assert(scheduler.getBlockedQueue().size() == 2);
    assert(scheduler.getProcessState(pid4) == ProcessState::Blocked);
    assert(scheduler.getProcessState(pid5) == ProcessState::Blocked);

    rm.printStatus();

    std::cout << "\nLiberando P1 (printer + scanner) e rechecando bloqueados\n";
    rm.release(pid1);
    rm.printStatus();
    std::cout << "Bloqueados apos release de P1: " << scheduler.getBlockedQueue().size() << " (esperado: 1)\n";
    assert(scheduler.getBlockedQueue().size() == 1);
    assert(scheduler.getProcessState(pid4) == ProcessState::Blocked);
    assert(scheduler.getProcessState(pid5) == ProcessState::Ready);

    std::cout << "\nLiberando P2 (modem) e rechecando bloqueados\n";
    rm.release(pid2);
    rm.printStatus();
    std::cout << "Bloqueados apos release de P2: " << scheduler.getBlockedQueue().size() << " (esperado: 0)\n";
    assert(scheduler.getBlockedQueue().empty());
    assert(scheduler.getProcessState(pid4) == ProcessState::Ready);

    std::cout << "=== FIM TESTE 3 ===\n";
    return 0;
}
