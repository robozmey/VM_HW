#include <iostream>
#include <algorithm>
#include "interpretator.h"
extern "C" {
    #include "runtime.h"
    #include "bytefile.h"
    extern void __init(void);
}
extern int32_t *__gc_stack_top, *__gc_stack_bottom;

extern const int stack_size = MAX_STACK_SIZE; // 4 MB

int main (int argc, char* argv[]) {
    __init();

    bytefile *f = read_file (argv[1]);
    auto i = interpretator (f);
    i.intepretate();

#ifdef STATISTICS
    std::vector<std::pair<int, std::string>> statistics;
    for (auto [bytecode, cnt]: i.statistics) {
        statistics.push_back({-cnt, bytecode});
    }
    std::sort(statistics.begin(), statistics.end());
    for (auto [cnt, bytecode]: statistics) {
        std::cout << bytecode << " " << -cnt << std::endl;
    }
#endif
    return 0;
}