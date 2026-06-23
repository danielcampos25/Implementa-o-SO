#include "ResourceManager/ResourceManager.h"
#include "Scheduler/scheduler.h"
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
    Scheduler scheduler;
    ResourceManager rm(&scheduler);
    scheduler.setResourceManager(&rm);
    rm.setScheduler(&scheduler);

    // P1: Precisa de quase tudo (Scanner, Modem, Printer, SATA)
    ResourceRequest p1{1, true, true, true, true};
    // P2: Precisa de Printer e SATA
    ResourceRequest p2{2, false, false, true, true};
    // P3: Precisa de Scanner e Modem
    ResourceRequest p3{3, true, true, false, false};

    std::cout << "--- TESTE DE ESTRESSE: ALOCACAO EM CADEIA ---\n";

    // 1. P1 pega quase tudo
    rm.allocate(p1);

    // 2. P2 tenta pegar Printer/SATA (deve bloquear)
    std::cout << "P2 tentando alocar...\n";
    rm.allocate(p2);

    // 3. P3 tenta pegar Scanner/Modem (deve bloquear)
    std::cout << "P3 tentando alocar...\n";
    rm.allocate(p3);

    rm.printStatus();
    std::cout << "Fila de bloqueados: " << scheduler.getBlockedQueue().size() << " (Esperado: 2)\n";

    // 4. P1 libera tudo
    std::cout << "\n--- P1 LIBERANDO TUDO ---\n";
    rm.release(1);

    // O que esperar aqui?
    // Como P1 liberou tudo, o checkBlockedProcesses deve rodar.
    // P2 deve pegar os recursos de Printer/SATA
    // P3 deve pegar os recursos de Scanner/Modem

    rm.printStatus();
    std::cout << "Fila de bloqueados apos liberacao: " << scheduler.getBlockedQueue().size() << " (Esperado: 0)\n";

    return 0;
}