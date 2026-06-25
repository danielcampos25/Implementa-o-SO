#include "ResourceManager/ResourceManager.h"
#include "Scheduler/scheduler.h"
#include <iostream>

/*
 * Teste básico de alocação e bloqueio.
 *
 * Cenário:
 * 1) P1 solicita todos os recursos e deve ser alocado.
 * 2) P2 solicita scanner e impressora; deve ficar bloqueado.
 * 3) P1 libera recursos e o Scheduler deve reavaliar P2.
 * 4) P3 tenta alocar apenas impressora.
 */
int main()
{
    Scheduler scheduler;
    ResourceManager rm(&scheduler);
    scheduler.setResourceManager(&rm);
    rm.setScheduler(&scheduler);

    ResourceRequest p1{1, true, true, true, true};
    ResourceRequest p2{2, true, false, true, false};
    ResourceRequest p3{3, false, false, true, false};

    std::cout << "=== Teste 1: alocacao inicial de P1 ===\n";
    bool allocated1 = rm.allocate(p1);
    std::cout << "P1 alocado: " << (allocated1 ? "sim" : "nao") << "\n";
    rm.printStatus();

    std::cout << "=== Teste 2: P2 tenta alocar (deve bloquear) ===\n";
    bool allocated2 = rm.allocate(p2);
    std::cout << "P2 alocado: " << (allocated2 ? "sim" : "nao") << "\n";
    std::cout << "Bloqueados: " << scheduler.getBlockedQueue().size() << "\n";

    std::cout << "=== Teste 3: P1 libera recursos e dispara rechecagem ===\n";
    rm.release(1);
    std::cout << "Bloqueados apos release: " << scheduler.getBlockedQueue().size() << "\n";
    rm.printStatus();

    std::cout << "=== Teste 4: P3 tenta alocar uma impressora ===\n";
    bool allocated3 = rm.allocate(p3);
    std::cout << "P3 alocado: " << (allocated3 ? "sim" : "nao") << "\n";
    rm.printStatus();

    std::cout << "=== Teste 5: status final ===\n";
    std::cout << "Fila bloqueados final: " << scheduler.getBlockedQueue().size() << "\n";

    return 0;
}
