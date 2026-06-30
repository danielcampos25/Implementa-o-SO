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
    // TESTE 1: Alocação básica e Cache Hit / Cache Miss (USER)
    // =================================================================
    std::cout << ">> Teste 1: Falta de pagina e acerto na memoria (USER)\n";
    
    fault = mm.ref_page(0, 10, USER); // Page 10, PID 0
    print_test_result("Falta de pagina (Miss) na primeira referencia", fault == 1);
    
    fault = mm.ref_page(0, 20, USER); // Page 20, PID 0
    fault = mm.ref_page(0, 30, USER); // Page 30, PID 0
    
    fault = mm.ref_page(0, 10, USER); // Page 10 de novo
    print_test_result("Acerto (Hit) em pagina ja alocada", fault == 0);

    // =================================================================
    // TESTE 2: Substituição LRU Local (Estouro de Working Set) (USER)
    // =================================================================
    std::cout << "\n>> Teste 2: Substituicao LRU Local (USER)\n";
    
    // PID 0 tem as páginas 10, 20 e 30.
    mm.ref_page(0, 40, USER); // 4ª pagina. WS cheio: [20(antiga), 30, 10, 40(nova)]
    
    fault = mm.ref_page(0, 50, USER); // 5ª pagina. Deve ejetar a pagina 20.
    print_test_result("Falta de pagina ao exceder Working Set", fault == 1);
    
    fault = mm.ref_page(0, 20, USER); 
    print_test_result("Pagina 20 foi ejetada corretamente (LRU funcionou)", fault == 1);

    fault = mm.ref_page(0, 10, USER); 
    print_test_result("Pagina 10 foi mantida no Working Set", fault == 0);

    // =================================================================
    // TESTE 3: Memória Global Cheia (USER)
    // =================================================================
    std::cout << "\n>> Teste 3: Memoria Global Cheia (USER)\n";
    
    // Lota os 12 frames: PID 0 (4), PID 1 (4), PID 2 (4)
    for (int i = 1; i <= 4; i++) mm.ref_page(1, i, USER); 
    for (int i = 1; i <= 4; i++) mm.ref_page(2, i, USER); 

    print_test_result("Memoria USER acusa estar cheia (12/12)", mm.is_memory_full(USER) == true);

    fault = mm.ref_page(1, 5, USER); // PID 1 canibaliza a própria página 1
    print_test_result("Substituicao feita com memoria lotada sem crash", fault == 1);
    
    fault = mm.ref_page(1, 1, USER); 
    print_test_result("LRU local respeitou o isolamento entre os processos", fault == 1);

    // =================================================================
    // TESTE 4: Liberação de Memória (USER)
    // =================================================================
    std::cout << "\n>> Teste 4: Liberacao de Memoria do Processo (USER)\n";
    
    mm.free_process_memory(2, USER); // Libera 4 frames
    
    print_test_result("Memoria USER acusa ter espaco apos liberar processo", mm.is_memory_full(USER) == false);
    
    fault = mm.ref_page(3, 100, USER);
    print_test_result("Novo processo conseguiu alocar pagina nos frames liberados", fault == 1);

    // =================================================================
    // TESTE 5: Alocação Inicial (REAL_TIME)
    // =================================================================
    std::cout << "\n>> Teste 5: Alocacao e Hit/Miss (REAL_TIME)\n";
    
    fault = mm.ref_page(4, 100, REAL_TIME); // PID 4 (RT)
    print_test_result("Falta de pagina (Miss) inicial para RT", fault == 1);
    
    fault = mm.ref_page(4, 100, REAL_TIME);
    print_test_result("Acerto (Hit) em pagina RT ja alocada", fault == 0);

    print_test_result("Memoria RT acusa ter espaco", mm.is_memory_full(REAL_TIME) == false);

    // =================================================================
    // TESTE 6: Memória Global RT Cheia (8 Frames) e Isolamento
    // =================================================================
    std::cout << "\n>> Teste 6: Memoria Global RT Cheia e Isolamento\n";
    
    // Preenche o resto do WS do PID 4 (total 4 frames)
    mm.ref_page(4, 101, REAL_TIME);
    mm.ref_page(4, 102, REAL_TIME);
    mm.ref_page(4, 103, REAL_TIME);

    // Preenche os 4 frames restantes da RT com o PID 5
    for(int i = 200; i < 204; i++) {
        mm.ref_page(5, i, REAL_TIME);
    }

    print_test_result("Memoria RT acusa estar cheia (8/8)", mm.is_memory_full(REAL_TIME) == true);
    
    // Garante que encher a memória RT não sinaliza erroneamente que a memória USER está cheia
    print_test_result("Isolamento: Memoria USER segue inalterada", mm.is_memory_full(USER) == false);

    fault = mm.ref_page(5, 204, REAL_TIME); // PID 5 substitui sua página 200
    print_test_result("Substituicao local RT operando corretamente", fault == 1);

    // =================================================================
    // TESTE 7: Intercalação de Processos e Liberação (REAL_TIME)
    // =================================================================
    std::cout << "\n>> Teste 7: Intercalacao e Liberacao Cruzada\n";
    
    mm.free_process_memory(4, REAL_TIME);
    print_test_result("Memoria RT acusa ter espaco apos liberar PID 4", mm.is_memory_full(REAL_TIME) == false);

    // Confirma que a tabela e os ponteiros se mantêm estáveis em referências intercaladas
    fault = mm.ref_page(3, 101, USER);
    print_test_result("Processo USER continua funcionando apos liberacao na tabela RT", fault == 1);

    std::cout << "\n--- TESTES FINALIZADOS ---\n";
    return 0;
}