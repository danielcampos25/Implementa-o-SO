#include "MemoryManager.h"

#include <vector>
#include <unordered_map>
#include <utility>
#include <stdexcept>

//*Feito na mão, com consulta no Gemini para sugetões e correção de erros

// | --------------------------------------------
// | Geral
// | --------------------------------------------

std::vector<FrameEntry>& MemoryManager::get_memory_table(ProcessType process_type) {
    if (process_type == USER) {
        return this->memory_table_usr;
    } else { // process_type == REAL_TIME
        return this->memory_table_rt;
    }
}


bool MemoryManager::is_memory_full(ProcessType process_type) {

    const std::vector<FrameEntry>& memory_table = this->get_memory_table(process_type);

    for (FrameEntry frame : memory_table) {
        if (frame.pid == -1 && frame.page_number == -1)
            return false;
    }
    return true;
}


void MemoryManager::free_process_memory(int pid, ProcessType process_type) {

    std::vector<FrameEntry>& memory_table = this->get_memory_table(process_type);

    // Remover da memória as páginas do processo
    for (int i = 0; i < memory_table.size(); i++) {
        if (memory_table[i].pid == pid) {
            memory_table[i] = FrameEntry{-1, -1};
        }
    }

    // Remover WorkingSet do processo
    this->working_sets.erase(pid);
    this->max_working_set_by_pid.erase(pid);
}

// \ --------------------------------------------

void MemoryManager::register_process_working_set_limit(int pid, int max_working_set)
{
    if (max_working_set <= 0)
    {
        max_working_set = 1;
    }

    this->max_working_set_by_pid[pid] = max_working_set;
}

// | --------------------------------------------
// | Referência e Falta de Página
// | --------------------------------------------

int MemoryManager::ref_page(int pid, int page_number, ProcessType process_type) {

    const std::vector<FrameEntry>& memory_table = this->get_memory_table(process_type);

    int fault = 1;
    
    // Procura a página na memória
    for (const FrameEntry& frame : memory_table) {
        if (frame.pid == pid && frame.page_number == page_number) {
            fault = 0;
            break;
        }
    }
    
    // Falta de página
    if (fault) {
        this->page_fault(pid, page_number, process_type);
    }
    
    // Incrementar contador LRU de todas as páginas do working set
    this->working_sets[pid].inc_all_counters();
    
    // Zerar contador LRU da página referenciada
    this->working_sets[pid].reset_counter(page_number);

    return fault;
}


void MemoryManager::page_fault(int pid, int page_number, ProcessType process_type) {
    
    int max_working_set = 4;

    const auto maxIt = this->max_working_set_by_pid.find(pid);
    if (maxIt != this->max_working_set_by_pid.end())
    {
        max_working_set = maxIt->second;
    }
    
    
    // Verificar se o processo está no mapa de Working Sets
    auto it = this->working_sets.find(pid);
    
    // Primeira referência do processo
    if (it == this->working_sets.end()) {
        this->working_sets.emplace(pid, WorkingSet());
        this->alloc_firstfit(pid, page_number, process_type);
        return;
    }

    
    int working_set_size = this->working_sets[pid].size();
    
    if (working_set_size < max_working_set && !this->is_memory_full(process_type)) {
        // Working Set não cheio E tem espaço na memória
        // Aloca no primeiro frame disponível
        this->alloc_firstfit(pid, page_number, process_type);
    } else {
        // Sem espaço na memória OU working set com tamanho máximo
        // Substitui a página referenciada há mais tempo
        this->substitute_local(pid, page_number, process_type);
    }

    return;
}

// \ --------------------------------------------



// | --------------------------------------------
// | Alocação
// | --------------------------------------------

void MemoryManager::alloc_page(int pid, int page_number, int index, ProcessType process_type) {

    std::vector<FrameEntry>& memory_table = this->get_memory_table(process_type);
    
    memory_table[index] = FrameEntry{pid, page_number};
}


bool MemoryManager::alloc_firstfit(int pid, int page_number, ProcessType process_type) {

    const std::vector<FrameEntry>& memory_table = this->get_memory_table(process_type);
    
    for (int i = 0; i < (memory_table.size()); i++) {
        if (
            memory_table[i].pid == -1
            && memory_table[i].page_number == -1
        ) {
            this->alloc_page(pid, page_number, i, process_type);
            this->working_sets[pid].pages_in_mem.push_back(int_pair(page_number, 0));
            return true;
        }
    }
    return false;
}


void MemoryManager::substitute_local(int pid, int page_number, ProcessType process_type) {

    const std::vector<FrameEntry>& memory_table = this->get_memory_table(process_type);

    int max = 0;
    int replaced_page = -1;
    int replaced_page_index = -1;

    // Encontrar página referenciada há mais tempo
    for (const int_pair& page : this->working_sets[pid].pages_in_mem) {
        if (page.second >= max) {
            max = page.second;
            replaced_page = page.first;
        }
    }

    if (replaced_page == -1)
        throw std::runtime_error("Erro ao substituir página: falha ao encontrar página referenciada há mais tempo");
        
        
    // Encontrar a posição na memória da página a ser substituída
    for (int i = 0; i < memory_table.size(); i++) {
        if (memory_table[i].pid == pid && memory_table[i].page_number == replaced_page) {
            replaced_page_index = i;
            break;
        }
    }

    if (replaced_page_index == -1)
        throw std::runtime_error("Erro ao substituir página: falha ao encontrar posição na memória da página a ser substituída");


    // Remover do WorkingSet a página a ser substituída
    this->working_sets[pid].remove_page(replaced_page);

    // Alocar nova página
    this->alloc_page(pid, page_number, replaced_page_index, process_type);

    // Inserir nova página no WorkingSet do processo
    this->working_sets[pid].pages_in_mem.push_back(int_pair(page_number, 0));
}


// \ --------------------------------------------