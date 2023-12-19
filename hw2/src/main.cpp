#include "interpretator.h"
#include "bytefile.h"
extern "C" {
    extern void __init(void);
}
extern int32_t *__gc_stack_top, *__gc_stack_bottom;

extern const int stack_size = MAX_STACK_SIZE; // 4 MB
int32_t*& stack_top = __gc_stack_top;
int32_t*& stack_bottom = __gc_stack_bottom;

int main (int argc, char* argv[]) {
    __init();
    stack_bottom = stack_top = (int32_t*)malloc(stack_size * sizeof(int32_t)) + stack_size;

    bytefile *f = read_file (argv[1]);
    auto i = interpretator (f);
    i.intepretate();
    return 0;
}