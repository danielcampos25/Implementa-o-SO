#include <iostream>
#include <cassert>
#include "./../../MemoryManager/MemoryManager.h"

//*Feito pelo Gemini

// Função auxiliar para imprimir resultados dos testes
void print_test_result(const std::string& test_name, bool passed) {
    if (passed) {
        std::cout << "[PASSOU] " << test_name << "\n";
    } else {
        std::cout << "[FALHOU] " << test_name << "\n";
    }
}

int main() {
    std::cout << "--- INICIANDO TESTES DO GERENCIADOR DE MEMORIA ---\n\n";

    MemoryManager mm;
    int fault;

    // =================================================================
    // TESTE 1: Alocação básica e Cache Hit / Cache Miss
    // =================================================================
    std::cout << ">> Teste 1: Falta de pagina e acerto na memoria\n";
    
    fault = mm.ref_page(0, 10, USER); // Page 10, PID 0
    print_test_result("Falta de pagina (Miss) na primeira referencia", fault == 1);
    
    fault = mm.ref_page(0, 20, USER); // Page 20, PID 0
    fault = mm.ref_page(0, 30, USER); // Page 30, PID 0
    
    fault = mm.ref_page(0, 10, USER); // Page 10 de novo
    print_test_result("Acerto (Hit) em pagina ja alocada", fault == 0);

    // =================================================================
    // TESTE 2: Substituição LRU Local (Limite de Working Set = 4)
    // =================================================================
    std::cout << "\n>> Teste 2: Substituicao LRU Local (Estouro de Working Set)\n";
    
    // Até agora o PID 0 tem as páginas 10, 20 e 30. (10 foi a referenciada mais recentemente).
    mm.ref_page(0, 40, USER); // 4ª pagina. WS cheio: [20(mais antiga), 30, 10, 40(mais nova)]
    
    fault = mm.ref_page(0, 50, USER); // 5ª pagina. Deve ejetar a pagina 20 (LRU).
    print_test_result("Falta de pagina ao exceder Working Set", fault == 1);
    
    fault = mm.ref_page(0, 20, USER); // Como a 20 foi ejetada, pedir ela de novo DEVE dar page fault
    print_test_result("Pagina 20 foi ejetada corretamente (LRU funcionou)", fault == 1);

    fault = mm.ref_page(0, 10, USER); // A pagina 10 não foi ejetada, DEVE ser um Hit.
    print_test_result("Pagina 10 foi mantida no Working Set", fault == 0);


    // =================================================================
    // TESTE 3: Memória Global Cheia e Concorrência entre Processos
    // =================================================================
    std::cout << "\n>> Teste 3: Memoria Global Cheia\n";
    
    // Memória tem 12 frames. O PID 0 já ocupa 4.
    // Vamos lotar a memória com PID 1 e PID 2.
    for (int i = 1; i <= 4; i++) mm.ref_page(1, i, USER); // PID 1 usa 4 frames
    for (int i = 1; i <= 4; i++) mm.ref_page(2, i, USER); // PID 2 usa 4 frames

    print_test_result("Memoria acusa estar cheia (12/12 frames ocupados)", mm.is_memory_full(USER) == true);

    // O PID 1 vai pedir uma nova página (5). A memória global está cheia,
    // e o PID 1 está no seu limite. Ele DEVE canibalizar a si próprio (LRU local).
    fault = mm.ref_page(1, 5, USER); // Ejeta a página 1 do PID 1.
    print_test_result("Substituicao feita com memoria global lotada sem crash", fault == 1);
    
    fault = mm.ref_page(1, 1, USER); // Confirma que a página 1 do PID 1 foi a ejetada
    print_test_result("LRU local respeitou o isolamento entre os processos", fault == 1);


    // =================================================================
    // TESTE 4: Liberação de Memória
    // =================================================================
    std::cout << "\n>> Teste 4: Liberacao de Memoria do Processo\n";
    
    mm.free_process_memory(2, USER); // Mata o processo 2 (libera 4 frames)
    
    print_test_result("Memoria acusa ter espaco apos liberar processo", mm.is_memory_full(USER) == false);
    
    // Cria um novo processo PID 3, que deve conseguir usar os frames recém-liberados
    fault = mm.ref_page(3, 100, USER);
    print_test_result("Novo processo conseguiu alocar pagina nos frames liberados", fault == 1);


    std::cout << "\n--- TESTES FINALIZADOS ---\n";
    return 0;
}