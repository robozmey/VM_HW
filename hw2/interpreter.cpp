//
// Created by vladimir on 08.12.23.
//

#include "interpreter.h"
#include "bytefile.h"

void interpreter::eval_binop() {}

void interpreter::eval_const() {}
void interpreter::eval_string() {}
void interpreter::eval_sexp() {}
void interpreter::eval_sti() {}
void interpreter::eval_sta() {}
void interpreter::eval_jump() {}
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
void interpreter::eval() {

# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   failure ("ERROR: invalid opcode %d-%d\n", h, l)

    char *ip     = bf->code_ptr;
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
                break;

            case 1:
                switch (l) {
                    case  0:
                        fprintf (f, "CONST\t%d", INT);
                        break;

                    case  1:
                        fprintf (f, "STRING\t%s", STRING);
                        break;

                    case  2:
                        fprintf (f, "SEXP\t%s ", STRING);
                        fprintf (f, "%d", INT);
                        break;

                    case  3:
                        fprintf (f, "STI");
                        break;

                    case  4:
                        fprintf (f, "STA");
                        break;

                    case  5:
                        fprintf (f, "JMP\t0x%.8x", INT);
                        break;

                    case  6:
                        fprintf (f, "END");
                        break;

                    case  7:
                        fprintf (f, "RET");
                        break;

                    case  8:
                        fprintf (f, "DROP");
                        break;

                    case  9:
                        fprintf (f, "DUP");
                        break;

                    case 10:
                        fprintf (f, "SWAP");
                        break;

                    case 11:
                        fprintf (f, "ELEM");
                        break;

                    default:
                        FAIL;
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
                        fprintf (f, "CJMPz\t0x%.8x", INT);
                        break;

                    case  1:
                        fprintf (f, "CJMPnz\t0x%.8x", INT);
                        break;

                    case  2:
                        fprintf (f, "BEGIN\t%d ", INT);
                        fprintf (f, "%d", INT);
                        break;

                    case  3:
                        fprintf (f, "CBEGIN\t%d ", INT);
                        fprintf (f, "%d", INT);
                        break;

                    case  4:
                        fprintf (f, "CLOSURE\t0x%.8x", INT);
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
                        fprintf (f, "CALLC\t%d", INT);
                        break;

                    case  6:
                        fprintf (f, "CALL\t0x%.8x ", INT);
                        fprintf (f, "%d", INT);
                        break;

                    case  7:
                        fprintf (f, "TAG\t%s ", STRING);
                        fprintf (f, "%d", INT);
                        break;

                    case  8:
                        fprintf (f, "ARRAY\t%d", INT);
                        break;

                    case  9:
                        fprintf (f, "FAIL\t%d", INT);
                        fprintf (f, "%d", INT);
                        break;

                    case 10:
                        fprintf (f, "LINE\t%d", INT);
                        break;

                    default:
                        FAIL;
                }
                break;

            case 6:
                fprintf (f, "PATT\t%s", pats[l]);
                break;

            case 7: {
                switch (l) {
                    case 0:
                        fprintf (f, "CALL\tLread");
                        break;

                    case 1:
                        fprintf (f, "CALL\tLwrite");
                        break;

                    case 2:
                        fprintf (f, "CALL\tLlength");
                        break;

                    case 3:
                        fprintf (f, "CALL\tLstring");
                        break;

                    case 4:
                        fprintf (f, "CALL\tBarray\t%d", INT);
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