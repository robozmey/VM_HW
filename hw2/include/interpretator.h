//
// Created by vladimir on 08.12.23.
//
#pragma once

#ifndef VM_HW2_INTERPRETER
#define VM_HW2_INTERPRETER

#include "bytefile.h"
#include <string>
#include <vector>

#define MAX_STACK_SIZE (1 << 20)

class variable {
public:
    variable(int type, int32_t val);
    std::string str();

    int type;
    int32_t val;
};


class interpretator {

    int32_t *fp;

    int32_t *&stack_top;
    int32_t *&stack_bottom;

    int32_t* stack_end;

    bytefile *bf;

    char *ip;

    void push(int32_t);
    int32_t pop();
    int32_t top();
    int32_t top(int i);
    void allocate(int n);
    void drop(int n);
    void prologue(int32_t nlocals, int32_t nargs);
    int32_t epilogue();
    void reverse(int nargs);
    int32_t& get_arg(int arg);
    int32_t& get_local(int local);
    int32_t& get_closure();
    int32_t cur_size();

    char get_byte();
    int32_t get_int();
    char* get_string();
    int32_t& get_var(variable& var);


    void eval_binop(int op);
    void eval_const(int32_t value);
    void eval_string(int32_t offset);
    void eval_sexp(char* name, int n);
//    void eval_sti();
    void eval_sta();
    void eval_jmp(int32_t addr);
    void eval_end();
//    void eval_ret();
    void eval_drop();
    void eval_dup();
//    void eval_swap();
    void eval_elem() ;
    void eval_ld(variable& var);
    void eval_lda(variable& var);
    void eval_st(variable& var);
    void eval_cjmpz(int32_t addr);
    void eval_cjmpnz(int32_t addr);
    void eval_begin(int32_t nargs, int32_t nlocals);
    void eval_cbegin(int32_t nargs, int32_t nlocals);
    void eval_closure();
    void eval_call(int32_t addr, int32_t nargs);
    void eval_callc(int32_t nargs);
    void eval_tag(char* name, int32_t n);
    void eval_array(int32_t n);
    void eval_fail(char h, char l);
    void eval_line();
    void eval_patt(int32_t patt);
    void eval_lread();
    void eval_lwrite();
    void eval_llength();
    void eval_lstring();
    void eval_barray();

public:
    interpretator(bytefile* bf, int32_t *&stack_top, int32_t *&stack_bottom);

    void intepretate();

    int32_t *get_stack_bottom();

    int32_t *get_stack_top();
};


#endif //VM_HW2_INTERPRETER
