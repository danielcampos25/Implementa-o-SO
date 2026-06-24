#include "MemoryManager.h"

#include <vector>
#include <unordered_map>
#include <utility>
#include <exception>

//*Feito na mão, com consulta no Gemini para correção de erros

// | --------------------------------------------
// | Geral
// | --------------------------------------------

bool MemoryManager::is_usr_memory_full() {
    for (FrameEntry frame : this->memory_table_usr) {
        if (frame.pid == -1 && frame.page_number == -1)
            return false;
    }
    return true;
}


void MemoryManager::free_process_memory(int pid) {

    // Remover da memória as páginas do processo
    for (int i = 0; i < this->memory_table_usr.size(); i++) {
        if (this->memory_table_usr[i].pid == pid) {
            memory_table_usr[i] = FrameEntry{-1, -1};
        }
    }

    // Remover WorkingSet do processo
    this->working_sets_usr.erase(pid);
}

// \ --------------------------------------------



// | --------------------------------------------
// | Referência e Falta de Página
// | --------------------------------------------

int MemoryManager::ref_page_usr(int pid, int page_number) {

    int fault = 1;
    
    // Procura a página na memória
    for (const FrameEntry& frame : this->memory_table_usr) {
        if (frame.pid == pid && frame.page_number == page_number) {
            fault = 0;
            break;
        }
    }
    
    // Falta de página
    if (fault) {
        this->page_fault_usr(pid, page_number);
    }
    
    // Incrementar contador LRU de todas as páginas do working set
    this->working_sets_usr[pid].inc_all_counters();
    
    // Zerar contador LRU da página referenciada
    this->working_sets_usr[pid].reset_counter(page_number);

    return fault;
}


void MemoryManager::page_fault_usr(int pid, int page_number) {
    
    //>TO-DO: obter esse valor da tabela de processos OU colocar como parâmetro OU manter um vetor de max_working_sets na classe
    // Algo como:
    // int max_working_set = ProcessManager.get_process_info(pid).max_working_set
    const int max_working_set = 4;
    
    
    // Verificar se o processo está no mapa de Working Sets
    auto it = this->working_sets_usr.find(pid);
    
    // Primeira referência do processo
    if (it == this->working_sets_usr.end()) {
        this->working_sets_usr.emplace(pid, WorkingSet());
        this->alloc_firstfit_usr(pid, page_number);
        return;
    }

    
    int working_set_size = this->working_sets_usr[pid].size();
    
    if (working_set_size < max_working_set && !this->is_usr_memory_full()) {
        // Working Set não cheio E tem espaço na memória
        // Aloca no primeiro frame disponível
        this->alloc_firstfit_usr(pid, page_number);
    } else {
        // Sem espaço na memória OU working set com tamanho máximo
        // Substitui a página referenciada há mais tempo
        this->substitute_local_usr(pid, page_number);
    }

    return;
}

// \ --------------------------------------------



// | --------------------------------------------
// | Alocação
// | --------------------------------------------

void MemoryManager::alloc_page_usr(int pid, int page_number, int index) {
    this->memory_table_usr[index] = FrameEntry{pid, page_number};
}


bool MemoryManager::alloc_firstfit_usr(int pid, int page_number) {
    for (int i = 0; i < (this->memory_table_usr.size()); i++) {
        if (
            this->memory_table_usr[i].pid == -1
            && this->memory_table_usr[i].page_number == -1
        ) {
            this->alloc_page_usr(pid, page_number, i);
            this->working_sets_usr[pid].pages_in_mem.push_back(int_pair(page_number, 0));
            return true;
        }
    }
    return false;
}


void MemoryManager::substitute_local_usr(int pid, int page_number) {

    int max = 0;
    int replaced_page = -1;
    int replaced_page_index = -1;

    // Encontrar página referenciada há mais tempo
    for (const int_pair& page : this->working_sets_usr[pid].pages_in_mem) {
        if (page.second >= max) {
            max = page.second;
            replaced_page = page.first;
        }
    }

    if (replaced_page == -1)
        throw std::runtime_error("Erro ao substituir página: falha ao encontrar página referenciada há mais tempo");
        
        
    // Encontrar a posição na memória da página a ser substituída
    for (int i = 0; i < this->memory_table_usr.size(); i++) {
        if (this->memory_table_usr[i].page_number == replaced_page) {
            replaced_page_index = i;
            break;
        }
    }

    if (replaced_page_index == -1)
        throw std::runtime_error("Erro ao substituir página: falha ao encontrar posição na memória da página a ser substituída");


    // Remover do WorkingSet a página a ser substituída
    this->working_sets_usr[pid].remove_page(replaced_page);

    // Alocar nova página
    this->alloc_page_usr(pid, page_number, replaced_page_index);

    // Inserir nova página no WorkingSet do processo
    this->working_sets_usr[pid].pages_in_mem.push_back(int_pair(page_number, 0));
}


// \ --------------------------------------------