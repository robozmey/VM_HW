//
// Created by vladimir on 08.12.23.
//

#include <string>
#include "interpreter.h"
#include "bytefile.h"

int32_t box(int32_t value) {
    return (value << 1) | 1;
}

int32_t unbox(int32_t value) {
    return value >> 1;
}

bool is_boxed(int32_t value) {
    return value & 1;
}

// bf
char interpreter::get_byte() {
    return *ip++;
}

int32_t interpreter::get_int() {
    return (ip += sizeof (int32_t), *(int32_t*)(ip - sizeof (int32_t)));
}

char* interpreter::get_string() {
    return get_string(bf, get_int());
}

// stack
void interpreter::push(int32_t value) {
    if (stack_bottom == stack_top - MAX_STACK_SIZE) {
        failure("Stack limit exceeded");
    }
    *(--stack_bottom) = value;
}

int32_t interpreter::pop() {
    return *(stack_bottom++);
}

int32_t* interpreter::get_stack_bottom() {
    return stack_bottom;
}



void interpreter::eval_binop(std::string op) {

}
void interpreter::eval_const() {
    push()
}
void interpreter::eval_string() {}
void interpreter::eval_sexp() {}
void interpreter::eval_sti() {}
void interpreter::eval_sta() {}
void interpreter::eval_jmp() {}
void interpreter::eval_end() {}
void interpreter::eval_ret() {}
void interpreter::eval_drop() {}
void interpreter::eval_dup() {}
void interpreter::eval_swap() {}
void interpreter::eval_elem() {}
void interpreter::eval_g() {}
void interpreter::eval_l() {}
void interpreter::eval_a() {}
void interpreter::eval_c() {}
void interpreter::eval_cjmpz() {}
void interpreter::eval_cjmpnz() {}
void interpreter::eval_begin() {}
void interpreter::eval_cbegin() {}
void interpreter::eval_closure() {}
void interpreter::eval_call() {}
void interpreter::eval_callc() {}
void interpreter::eval_tag() {}
void interpreter::eval_array() {}
void interpreter::eval_fail() {}
void interpreter::eval_line() {}
void interpreter::eval_patt() {}
void interpreter::eval_lread() {}
void interpreter::eval_lwrite() {}
void interpreter::eval_llength() {}
void interpreter::eval_lstring() {}
void interpreter::eval_larray() {}
void interpreter::intepretate() {

# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   failure ("ERROR: invalid opcode %d-%d\n", h, l)

    char *ops [] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
    char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
    char *lds [] = {"LD", "LDA", "ST"};
    do {
        char x = BYTE,
                h = (x & 0xF0) >> 4,
                l = x & 0x0F;

        fprintf (f, "0x%.8x:\t", ip-bf->code_ptr-1);

        switch (h) {
            case 15:
                goto stop;

                /* BINOP */
            case 0:
                fprintf (f, "BINOP\t%s", ops[l-1]);
                eval_binop(ops[l-1]);
                break;

            case 1:
                switch (l) {
                    case  0:
                        eval_const();
                        break;

                    case  1:
                        eval_string();
                        break;

                    case  2:
                        eval_sexp();
                        break;

                    case  3:
                        eval_sti();
                        break;

                    case  4:
                        eval_sta();
                        break;

                    case  5:
                        eval_jmp();
                        break;

                    case  6:
                        eval_end();
                        break;

                    case  7:
                        eval_ret();
                        break;

                    case  8:
                        eval_drop()
                        break;

                    case  9:
                        eval_dup();
                        break;

                    case 10:
                        eval_swap();
                        break;

                    case 11:
                        eval_elem();
                        break;

                    default:
                        fail();
                }
                break;

            case 2:
            case 3:
            case 4:
                fprintf (f, "%s\t", lds[h-2]);
                switch (l) {
                    case 0: fprintf (f, "G(%d)", INT); break;
                    case 1: fprintf (f, "L(%d)", INT); break;
                    case 2: fprintf (f, "A(%d)", INT); break;
                    case 3: fprintf (f, "C(%d)", INT); break;
                    default: FAIL;
                }
                break;

            case 5:
                switch (l) {
                    case  0:
                        eval_cjmpz();
                        break;

                    case  1:
                        eval_cjmpz();
                        break;

                    case  2:
                        eval_begin();
                        break;

                    case  3:
                        eval_cbegin();
                        break;

                    case  4:
                        eval_closure();
                        {int n = INT;
                            for (int i = 0; i<n; i++) {
                                switch (BYTE) {
                                    case 0: fprintf (f, "G(%d)", INT); break;
                                    case 1: fprintf (f, "L(%d)", INT); break;
                                    case 2: fprintf (f, "A(%d)", INT); break;
                                    case 3: fprintf (f, "C(%d)", INT); break;
                                    default: FAIL;
                                }
                            }
                        };
                        break;

                    case  5:
                        eval_callc();
                        break;

                    case  6:
                        eval_call();
                        break;

                    case  7:
                        eval_tag();
                        break;

                    case  8:
                        eval_array();
                        break;

                    case  9:
                        eval_fail();
                        break;

                    case 10:
                        eval_line();
                        break;

                    default:
                        fail();
                }
                break;

            case 6:
                eval_patt(l);
                break;

            case 7: {
                switch (l) {
                    case 0:
                        eval_read();
                        break;

                    case 1:
                        eval_write();
                        break;

                    case 2:
                        eval_length();
                        break;

                    case 3:
                        eval_lstring();
                        break;

                    case 4:
                        eval_barray();
                        break;

                    default:
                        FAIL;
                }
            }
                break;

            default:
                FAIL;
        }

        fprintf (f, "\n");
    }
    while (1);
    stop: fprintf (f, "<end>\n");
}