#include <iostream>
#include <algorithm>
#include <vector>

extern "C" {
    #include "byterun.h"
    extern void __init(void);
}
extern int32_t *__gc_stack_top, *__gc_stack_bottom;

int main (int argc, char* argv[]) {
    __init();

    bytefile *f = read_file (argv[1]);

    std::vector<std::pair<char, int>> statistics_1;

    char* ip = f->code_ptr;
    while (ip != nullptr) {
        char* new_ip = disassemble_instruction(stdout, f, ip);
        if (new_ip == nullptr) break;
    }

    std::vector<std::pair<int, char>> statistics_2;
    for (auto [bytecode, cnt]: statistics_1) {
        statistics_2.push_back({-cnt, bytecode});
    }
    std::sort(statistics_2.begin(), statistics_2.end());
    for (auto [cnt, bytecode]: statistics_2) {
        std::cout << bytecode << " " << -cnt << std::endl;
    }
    return 0;
}