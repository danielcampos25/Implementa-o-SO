#include "ResourceManager/ResourceManager.h"
#include "ProcessScheduler/ProcessScheduler.h"
#include <cassert>
#include <iostream>

/*
 * Teste básico de alocação e bloqueio.
 *
 * Cenário:
 * 1) P1 solicita todos os recursos e deve ser alocado.
 * 2) P2 solicita impressora e modem; deve ficar bloqueado.
 * 3) P1 libera recursos e o ProcessScheduler deve reavaliar P2.
 * 4) P3 tenta alocar apenas impressora.
 */
int main()
{
    ProcessScheduler scheduler;
    ResourceManager rm(&scheduler);
    scheduler.setResourceManager(&rm);
    rm.setScheduler(&scheduler);

    const int pid1 = scheduler.createProcess(0, 1, 3, 64, 1, 1, 1, 1);
    const int pid2 = scheduler.createProcess(0, 1, 3, 64, 1, 0, 1, 0);
    const int pid3 = scheduler.createProcess(0, 1, 3, 64, 1, 0, 0, 0);

    ResourceRequest p1{pid1, true, true, true, true};
    ResourceRequest p2{pid2, true, false, true, false};
    ResourceRequest p3{pid3, true, false, false, false};

    std::cout << "=== Teste 1: alocacao inicial de P1 ===\n";
    bool allocated1 = rm.allocate(p1);
    std::cout << "P1 alocado: " << (allocated1 ? "sim" : "nao") << "\n";
    rm.printStatus();

    std::cout << "=== Teste 2: P2 tenta alocar (deve bloquear) ===\n";
    bool allocated2 = rm.allocate(p2);
    std::cout << "P2 alocado: " << (allocated2 ? "sim" : "nao") << "\n";
    std::cout << "Bloqueados: " << scheduler.getBlockedQueue().size() << "\n";
    assert(!allocated2);
    assert(scheduler.getBlockedQueue().size() == 1);
    assert(scheduler.getProcessState(pid2) == ProcessState::Blocked);
    assert(scheduler.selectNextProcessId().value() == pid1);

    std::cout << "=== Teste 3: P1 libera recursos e dispara rechecagem ===\n";
    rm.release(pid1);
    std::cout << "Bloqueados apos release: " << scheduler.getBlockedQueue().size() << "\n";
    rm.printStatus();
    assert(scheduler.getBlockedQueue().empty());
    assert(scheduler.getProcessState(pid2) == ProcessState::Ready);

    std::cout << "=== Teste 4: P3 tenta alocar uma impressora ===\n";
    bool allocated3 = rm.allocate(p3);
    std::cout << "P3 alocado: " << (allocated3 ? "sim" : "nao") << "\n";
    rm.printStatus();

    std::cout << "=== Teste 5: status final ===\n";
    std::cout << "Fila bloqueados final: " << scheduler.getBlockedQueue().size() << "\n";
    assert(allocated3);
    assert(scheduler.getBlockedQueue().empty());

    return 0;
}
