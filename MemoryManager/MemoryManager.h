#ifndef MEMORY_MANAGER
#define MEMORY_MANAGER

#include <vector>
#include <unordered_map>
#include <utility>

//*Feito na mão, com consulta no Gemini para correção de erros

typedef std::pair<int, int> int_pair;

struct FrameEntry {
    int pid = -1;           // Processo dono do frame
    int page_number = -1;   // Página alocada no frame
};


struct WorkingSet {

    // Páginas alocadas na memória {nº da pagina, counter_LRU}
    std::vector<int_pair> pages_in_mem;

    // Zera o contador de uma página
    void reset_counter(int page_number) {
        for (int_pair& page : pages_in_mem) {
            if (page.first == page_number) {
                page.second = 0;
                break;
            }
        }
    }
    
    // Incrementa os contadores de todas as páginas no Working Set
    void inc_all_counters() {
        for (int_pair& page : pages_in_mem) {
            page.second++;
        }
    }

    // Remove a página page_number do WorkingSet
    void remove_page(int page_number) {
        for (int i = 0; i < pages_in_mem.size(); i++) {
            if (pages_in_mem[i].first == page_number) {
                pages_in_mem.erase(pages_in_mem.begin() + i);
                break;
            }
        }
    }

    // Retorna quantas páginas tem no working set
    int size() {
        return pages_in_mem.size();
    }
};


class MemoryManager {
    private:
        const int N_FRAMES_RT = 8;
        const int N_FRAMES_USR = 12;

        std::vector<FrameEntry> memory_table_rt = std::vector<FrameEntry>(N_FRAMES_RT);
        std::vector<FrameEntry> memory_table_usr = std::vector<FrameEntry>(N_FRAMES_USR);

        // std::vector<WorkingSet> working_sets_usr;
        std::unordered_map<int, WorkingSet> working_sets_usr;

        // Aloca uma página na memória de processos de usuário no index solicitado, independente de estar vazia ou não
        void alloc_page_usr(int pid, int page_number, int index);

        // Tenta alocar uma página na memória de processos de usuário no primeiro frame disponível
        bool alloc_firstfit_usr(int pid, int page_number);

        // Aloca uma página no working set do processo, substituindo a página local referenciada há mais tempo (LRU)
        void substitute_local_usr(int pid, int page_number);
        
        // Lida com falta de página. Aloca na memória:
        // no primeiro espaço livre, se possível; no lugar da página referenciada há mais tempo, se não
        void page_fault_usr(int pid, int page_number);

    public:
        MemoryManager() {}

        /**
         * @brief Verifica se a memória de processos de usuário está cheia.
         * @return True se a memória de processos de usuário está cheia; False se há pelo menos um Frame disponível.
         */
        bool is_usr_memory_full();

        /**
         * @brief Referencia uma página na memória virtual.
         * @param pid PID do processo que está referenciando a página.
         * @param page_number Número da página referenciada.
         * @return 0 se a página estava na memória; 1 se houve Page Fault.
         */
        int ref_page_usr(int pid, int page_number);
        

        /**
         * @brief Libera a memória ocupada por um processo.
         * @param pid ID do processo cuja memória será liberada.
         */
        void free_process_memory(int pid);
};

#endif