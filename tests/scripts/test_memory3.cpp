#include <iostream>
#include "./../../MemoryManager/MemoryManager.h"

int main() {

    MemoryManager mm = MemoryManager();
    int fault = 0;

    std::string refs = "1,2,3,4,1,2,5,1,2,3,4,5";

    std::cout << "Teste 1:\n";

    // Referências
    mm.register_process_working_set_limit(1, 4);
    mm.ref_page(1, 1, USER); //pré-carga
    int i = 2;
    std::string str_num = "";
    while (i < refs.size()) {
        
        if (refs[i] == ',') {
            fault += mm.ref_page(1, std::stoi(str_num), USER);
            str_num = "";
        } else {
            str_num += refs[i];
        }
        
        if (i == refs.size()-1) {
            fault += mm.ref_page(1, std::stoi(str_num), USER);
        }
        
        i++;
    }
    std::cout << "Numero de faltas:" << fault << "\n";
    
    if (fault == 7) printf("PASSOU!"); else printf("FALHOU!");
    
    mm.show_memory_table(USER);
    
    
    std::cout << "Teste 2:\n";
    
    fault = 0;
    refs = "7,0,1,2,0,3,0,4,2,3,0,3,1,0,2,8,9,10,11,12,9,7,8,3,0,1";
    str_num = "";
    
    // Referências
    mm.register_process_working_set_limit(2, 8);
    mm.ref_page(2, 7, USER); //pré-carga
    i = 2;
    while (i < refs.size()) {
        
        if (refs[i] == ',') {
            fault += mm.ref_page(2, std::stoi(str_num), USER);
            str_num = "";
        } else {
            str_num += refs[i];
        }
        
        if (i == refs.size()-1) {
            fault += mm.ref_page(2, std::stoi(str_num), USER);
        }

        i++;
    }
    std::cout << "Numero de faltas:" << fault << "\n";

    if (fault == 14) printf("PASSOU!"); else printf("FALHOU!");


    mm.show_memory_table(USER);

    return 0;
}