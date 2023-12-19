//
// Created by vladimir on 08.12.23.
//

#ifndef VM_HW2_INTERPRETER
#define VM_HW2_INTERPRETER

#include "bytefile.h"

#define MAX_STACK_SIZE 1024


class interpreter {

    int32_t *&stack_top;
    int32_t *&stack_bottom;
    int32_t *fp;

    bytefile *bf;

    char *ip;

    void push(int32_t);
    int32_t pop();
    int32_t *get_stack_bottom();

    char get_byte();
    int32_t get_int();
    char* get_string();



    void eval_binop(std::string op);
    void eval_const();
    void eval_string();
    void eval_sexp();
    void eval_sti();
    void eval_sta();
    void eval_jmp();
    void eval_end();
    void eval_ret();
    void eval_drop();
    void eval_dup();
    void eval_swap();
    void eval_elem();
    void eval_g();
    void eval_l();
    void eval_a();
    void eval_c();
    void eval_cjmpz();
    void eval_cjmpnz();
    void eval_begin();
    void eval_cbegin();
    void eval_closure();
    void eval_call();
    void eval_callc();
    void eval_tag();
    void eval_array();
    void eval_fail();
    void eval_line();
    void eval_patt();
    void eval_lread();
    void eval_lwrite();
    void eval_llength();
    void eval_lstring();
    void eval_larray();

public:
    interpreter(bytefile* bf) : bf(bf) {
        ip = bf->code_ptr;
        fp = stack_bottom = stack_top = (new int32_t[MAX_STACK_SIZE]) + MAX_STACK_SIZE;
    }

    void intepretate();
};


#endif //VM_HW2_INTERPRETER
