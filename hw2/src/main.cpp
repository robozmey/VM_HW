#include "interpreter.h"
#include "bytefile.h"

int main (int argc, char* argv[]) {
    bytefile *f = read_file (argv[1]);
    auto i = interpreter (f);
    i.intepretate();
    return 0;
}