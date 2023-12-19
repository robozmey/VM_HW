//
// Created by vladimir on 08.12.23.
//

#include <string>
#include <functional>
#include <stdexcept>
#include "interpreter.h"
#include "bytefile.h"

extern "C" {
    extern int Lread();
    extern int Lwrite(int);
    extern int Llength(void*);
    extern int LtagHash(char *s);
    extern int Btag(void*, int, int);
    extern int Bstring_patt(void*, void*);
    extern int Bstring_tag_patt(void*);
    extern int Barray_tag_patt(void*);
    extern int Bsexp_tag_patt(void*);
    extern int Bunboxed_patt(void*);
    extern int Bboxed_patt(void*);
    extern int Bclosure_tag_patt (void*);
    extern int Barray_patt (void*, int);

    extern void* Lstring (void*);
    extern void* Bstring(void*);
    extern void* Bsta (void*, int, void*);
    extern void* Belem(void*,int);
    extern void* Belem_ptr(void*, int);
    extern void* Barray(int, void*);
    extern void* Bsexp_(int, void*);
    extern void* Bclosure(int, void*, void*);
}

variable::variable(int type, int32_t val): type {type}, val {val} { }

std::string variable::str() {
    switch (type)
    {
        case 0:
            return "G(" + std::to_string(val) + ")";
        case 1:
            return "L(" + std::to_string(val) + ")";
        case 2:
            return "A(" + std::to_string(val) + ")";
        case 3:
            return "C(" + std::to_string(val) + ")";
        default:
            throw std::runtime_error("unknown var type");
    }
}

char *ops [] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
char *lds [] = {"LD", "LDA", "ST"};
const static std::vector<std::function<int32_t(int32_t, int32_t)>> ops_table {
        [](int x, int y) { return x + y; },
        [](int x, int y) { return x - y; },
        [](int x, int y) { return x * y; },
        [](int x, int y) { return x / y; },
        [](int x, int y) { return x % y; },
        [](int x, int y) { return x < y; },
        [](int x, int y) { return x <= y; },
        [](int x, int y) { return x > y; },
        [](int x, int y) { return x >= y; },
        [](int x, int y) { return x == y; },
        [](int x, int y) { return x != y; },
        [](int x, int y) { return x && y; },
        [](int x, int y) { return x || y; },
};

int32_t box(int32_t value) {
    return (value << 1) | 1;
}

int32_t unbox(int32_t value) {
    return value >> 1;
}

bool boxed(int32_t value) {
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
    return ::get_string(bf, get_int());
}

int32_t& interpreter::get_var(variable& var) {
    switch(var.type) {
        case 0: return bf->global_ptr[var.val];
        case 1: return get_local(var.val);
        case 2: return get_arg(var.val);
        case 3: {
            void* closure = reinterpret_cast<void*>(get_closure());
            return *(reinterpret_cast<int32_t*>(Belem_ptr(closure, box(var.val + 1))));
        }
        default: throw std::runtime_error("unknown var type");
    }
}

# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   failure ("ERROR: invalid opcode %d-%d\n", h, l)

void interpreter::eval_binop(std::string op) {
    int32_t x = unbox(pop());
    int32_t y = unbox(pop());

    int32_t res = ops_table[op](y, x);
    push(box(res));
}
void interpreter::eval_const(int32_t value) {
    push(box(value));
}
void interpreter::eval_string(int32_t offset) {
    push(reinterpret_cast<int32_t>(Bstring(bf->string_ptr + offset)));
}
void interpreter::eval_sexp(char* name, int n) {
    int32_t* arr = new int32_t[n + 1];
    for (int i = 0; i < n; i++) {
        arr[n - 1 - i] = pop();
    }
    arr[n] = LtagHash(name);
    push(reinterpret_cast<int32_t>(Bsexp_(box(n + 1), reinterpret_cast<void*>(arr))));
    delete[] arr;
}
//void interpreter::eval_sti() {}
void interpreter::eval_sta() {}
void interpreter::eval_jmp(int32_t addr) {
    ip = bf->code_ptr + addr;
}
void interpreter::eval_end() {
    ip = reinterpret_cast<char*>(epilogue());
}
//void interpreter::eval_ret() {}
void interpreter::eval_drop() {
    drop(1);
}
void interpreter::eval_dup() {
    push(top());
}
//void interpreter::eval_swap() {}
void interpreter::eval_elem() {
    int32_t elem = pop();
    void* sexp = reinterpret_cast<void*>(pop());
    push(reinterpret_cast<int32_t>(Belem(sexp, elem)));
}
void interpreter::eval_ld(variable& var) {
    int32_t& val = get_var(var);
    push(val);
}
void interpreter::eval_lda(variable& var) {
    push(reinterpret_cast<int32_t>(&(get_var(var))));
}
void interpreter::eval_st(variable& var) {
    int32_t rval = top();
    int32_t& lval = get_var(var);
    lval = rval;
}
void interpreter::eval_cjmpz(int32_t addr) {
    int32_t ifval = unbox(pop());
    if (ifval == 0)
        ip = bf->code_ptr + addr;
}
void interpreter::eval_cjmpnz(int32_t addr) {
    int32_t ifval = unbox(pop());
    if (ifval != 0)
        ip = bf->code_ptr + addr;
}
void interpreter::eval_begin(int32_t nargs, int32_t nlocals) {
    prologue(nlocals);
}
void interpreter::eval_cbegin(int32_t nargs, int32_t nlocals) {
    prologue(nlocals);
}
void interpreter::eval_closure(int addr, std::vector<variable> vars) {
    int32_t* arr = new int32_t[vars.size()];
    for (int i = 0; i < vars.size(); i++) {
        arr[i] = get_var(vars[i]);
    }
    push(reinterpret_cast<int32_t>(Bclosure(box(vars.size()), reinterpret_cast<void*>(bf->code_ptr + addr), arr)));
    delete[] arr;
}
void interpreter::eval_call(int32_t addr, int32_t nargs) {
    reverse(nargs);
    push(reinterpret_cast<int32_t>(ip));
    push(nargs);
    ip = bf->code_ptr + addr;
}
void interpreter::eval_callc(int32_t nargs) {
    int32_t closure = top(nargs);

    reverse(nargs);
    push(reinterpret_cast<int32_t>(ip));
    push(nargs + 1);
    ip = reinterpret_cast<char*>(Belem(reinterpret_cast<void*>(closure), box(0)));
}
void interpreter::eval_tag(int32_t* name, int32_t n) {
    int32_t tag = LtagHash(name);
    int32_t sexp = pop();
    push(Btag(reinterpret_cast<void*>(sexp), tag, box(n)));
}
void interpreter::eval_array(int32_t n) {
    push(reinterpret_cast<int32_t>(Barray_patt(reinterpret_cast<void*>(pop()), box(n))));
}
void interpreter::eval_fail(char h, char l) {
    failure("ERROR: invalid opcode %d-%d\n", h, l);
}
void interpreter::eval_line() {
    int line = get_int();
}
void interpreter::eval_patt(int32_t patt) {
    switch(patt) {
        case 0: {
            int32_t x = pop();
            int32_t y = pop();
            push(reinterpret_cast<int32_t>(Bstring_patt(reinterpret_cast<void*>(y), reinterpret_cast<void*>(x))));
            break;
        }
        case 1: {
            push(reinterpret_cast<int32_t>(Bstring_tag_patt(reinterpret_cast<void*>(pop()))));
            break;
        }
        case 2: {
            push(reinterpret_cast<int32_t>(Barray_tag_patt(reinterpret_cast<void*>(pop()))));
            break;
        }
        case 3: {
            push(reinterpret_cast<int32_t>(Bsexp_tag_patt(reinterpret_cast<void*>(pop()))));
            break;
        }
        case 4: {
            push(reinterpret_cast<int32_t>(Bunboxed_patt(reinterpret_cast<void*>(pop()))));
            break;
        }
        case 5: {
            push(reinterpret_cast<int32_t>(Bboxed_patt(reinterpret_cast<void*>(pop()))));
            break;
        }
        case 6: {
            push(reinterpret_cast<int32_t>(Bclosure_tag_patt(reinterpret_cast<void*>(pop()))));
            break;
        }
        default:
            throw std::runtime_error("unknown pattern");
    }
}
void interpreter::eval_lread() {
    push(Lread());
}
void interpreter::eval_lwrite() {
    int32_t val = pop();
    push(Lwrite(val));
}
void interpreter::eval_llength() {
    int32_t length = Llength(reinterpret_cast<void*>(pop()));
    push(length);
}
void interpreter::eval_lstring() {
    push(reinterpret_cast<int32_t>(Lstring(reinterpret_cast<void*>(pop()))));
}
void interpreter::eval_barray() {
    int32_t n = INT;
    int32_t* arr = new int32_t[n];
    for (int i = 0; i < n; i++) {
        arr[n - 1 - i] = pop();
    }

    push(reinterpret_cast<int32_t>(Barray(box(n), arr)));
    delete[] arr;
}
static void not_impl() {
    throw std::runtime_error("Not implemented");
}

void interpreter::intepretate() {
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
//                fprintf (f, "BINOP\t%s", ops[l-1]);
                eval_binop(ops[l-1]);
                break;

            case 1:
                switch (l) {
                    case  0:
                        eval_const(get_int());
                        break;

                    case  1:
                        eval_string(get_string());
                        break;

                    case  2:
                        eval_sexp(get_string(), INT);
                        break;

                    case  3: // notimpl
                        not_impl();
                        break;

                    case  4:
                        eval_sta();
                        break;

                    case  5:
                        eval_jmp(INT);
                        break;

                    case  6:
                        eval_end();
                        break;

                    case  7: // notimpl
                        not_impl();
                        break;

                    case  8:
                        eval_drop();
                        break;

                    case  9:
                        eval_dup();
                        break;

                    case 10: // notimpl
                        not_impl();
                        break;

                    case 11:
                        eval_elem();
                        break;

                    default:
                        eval_fail();
                }
                break;

            case 2:
            case 3:
            case 4:
                variable var(l, get_int());
                switch (h - 2) {
                    case 0:
                        eval_ld(var);
                        break;
                    case 1:
                        eval_lda(var);
                        break;
                    case 2:
                        eval_st(var);
                        break;
                    default:
                        eval_fail(h, l);
                }
                break;

            case 5:
                switch (l) {
                    case  0:
                        eval_cjmpz(get_int());
                        break;

                    case  1:
                        eval_cjmpnz(get_int());
                        break;

                    case  2:
                        int nargs = get_int();
                        int nlocals = get_int();
                        eval_begin(nargs, nlocals);
                        break;

                    case  3:
                        int nargs = get_int();
                        int nlocals = get_int();
                        eval_cbegin(nargs, nlocals);
                        break;

                    case  4:
                        int addr = get_int();
                        int n = get_int();
                        std::vector<variable> vars;
                        for (int i = 0; i < n; i++) {
                            int tp = get_byte();
                            int val = get_int();
                            vars.emplace_back(tp, val);
                        }
                        eval_closure(addr, vars);
                        break;

                    case  5:
                        eval_callc(get_int());
                        break;

                    case  6:
                        int addr = get_int();
                        int nargs = get_int();
                        eval_call(addr, nargs);
                        break;

                    case  7:
                        char* name = get_string();
                        int n = get_int();
                        eval_tag(name, n);
                        break;

                    case  8:
                        eval_array(get_int());
                        break;

                    case  9:
                        int x = get_int(), y = get_int();
//                        debug(printf("FAIL %d %d\n", x, y););
                        eval_fail(x, y);
                        break;

                    case 10:
                        eval_line();
                        break;

                    default:
                        eval_fail(h, l);
                }
                break;

            case 6:
                eval_patt(l);
                break;

            case 7: {
                switch (l) {
                    case 0:
                        eval_lread();
                        break;

                    case 1:
                        eval_lwrite();
                        break;

                    case 2:
                        eval_llength();
                        break;

                    case 3:
                        eval_lstring();
                        break;

                    case 4:
                        eval_barray();
                        break;

                    default:
                        eval_fail(h, l);
                }
            }
                break;

            default:
                eval_fail(h, l);
        }

//        fprintf (f, "\n");
    }
    while (1);
//    stop: fprintf (f, "<end>\n");
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

int32_t interpreter::top() {
    return top(0);
}

int32_t interpreter::top(int i) {
    if (cur_size() <= i) [[unlikely]]
                throw new std::runtime_error("get top " + std::to_string(i) + " element when stack size " + std::to_string(cur_size()));

    return *(stack_bottom + i);
}

void interpreter::allocate(int n) {
    if (stack_size - cur_size() < n) [[unlikely]]
                throw new std::runtime_error("allocate " + std::to_string(n) + " elements when free stack space is " + std::to_string(stack_size - cur_size()));

    stack_bottom -= n;
}

void interpreter::drop(int n) {
    if (cur_size() < n) [[unlikely]]
                throw new std::runtime_error("drop " + std::to_string(n) + " elements when stack size is " + std::to_string(cur_size()));

    stack_bottom += n;
}

void interpreter::prologue(int32_t nlocals) {
    push(reinterpret_cast<int32_t>(fp));
    fp = stack_bottom;
    allocate(nlocals);
}

int32_t interpreter::epilogue() {
    int32_t ret_val = pop();
    stack_bottom = fp;
    fp = reinterpret_cast<int32_t*>(pop());
    int32_t nargs = pop();
    int32_t ip = pop();
    drop(nargs);
    push(ret_val);
    return ip;
}

void interpreter::reverse(int nargs) {
    if (cur_size() < nargs) [[unlikely]]
                throw new std::runtime_error("reverse " + std::to_string(nargs) + " elements when stack size is " + std::to_string(cur_size()));

    int32_t* st = stack_bottom + nargs - 1;
    int32_t* fn = stack_bottom;

    while (st > fn) {
        std::swap(*(st--), *(fn++));
    }
}

int32_t& interpreter::get_arg(int arg) {
    if (fp + 3 + arg >= stack_top) [[unlikely]]
                throw new std::runtime_error("fail to take argument " + std::to_string(arg));

    return *(fp + 3 + arg);
}

int32_t& interpreter::get_local(int local) {
    if (fp - local - 1 < (stack_top - stack_size)) [[unlikely]]
                throw new std::runtime_error("fail to get local " + std::to_string(local));

    return *(fp - local - 1);
}

int32_t& interpreter::get_closure() {
    if (fp + 1 >= stack_top || fp + 2 + *(fp + 1) >= stack_top) [[unlikely]]
                throw new std::runtime_error("faile to get closure");

    return *(fp + 2 + *(fp + 1));
}

int32_t interpreter::cur_size() {
    return stack_top - stack_bottom;
}

