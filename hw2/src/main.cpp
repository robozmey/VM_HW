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
    auto i = interpretator (f, __gc_stack_top, __gc_stack_bottom);
    i.intepretate();
    return 0;
}