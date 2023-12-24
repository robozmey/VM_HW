//
// Created by vladimir on 08.12.23.
//

#include <string>
#include <functional>
#include <stdexcept>
#include "interpretator.h"

extern "C" {
    #include "runtime.h"
}

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
    extern void* Barray_arr(int, int*);
    extern void* Bsexp_arr (int, int, int*);
    extern void* Bclosure_arr(int, void*, int*);
}

extern int32_t*& stack_top;
extern int32_t*& stack_bottom;
extern const int stack_size;

variable::variable(int type, int32_t val): type {type}, val {val} { }

std::string ops [13] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
std::string pats[7] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
std::string lds [3] = {"LD", "LDA", "ST"};
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

inline int32_t box(int32_t value) {
    return (value << 1) | 1;
}

inline int32_t unbox(int32_t value) {
    return value >> 1;
}

inline bool boxed(int32_t value) {
    return value & 1;
}

// bf
inline char interpretator::get_byte() {
    return *ip++;
}

inline int32_t interpretator::get_int() {
    return (ip += sizeof (int32_t), *(int32_t*)(ip - sizeof (int32_t)));
}

inline char* interpretator::get_string() {
    return ::get_string(bf, get_int());
}

int32_t& interpretator::get_var(variable& var) {
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

inline void interpretator::eval_binop(int op) {
    int32_t y = unbox(pop());
    int32_t x = unbox(pop());
    int32_t res = ops_table[op](x, y);

    switch (op+1) {
        case 1:
            res = x + y;
            break;
        case 2:
            res = x - y;
            break;
        case 3:
            res = x * y;
            break;
        case 4:
            res = x / y;
            break;
        case 5:
            res = x % y;
            break;
        case 6:
            res = x < y;
            break;
        case 7:
            res = x <= y;
            break;
        case 8:
            res = x > y;
            break;
        case 9:
            res = x >= y;
            break;
        case 10:
            res = x == y;
            break;
        case 11:
            res = x != y;
            break;
        case 12:
            res = x && y;
            break;
        case 13:
            res = x || y;
            break;
        default:
            failure("ERROR: invalid opcode %d\n", op);
    }

    push(box(res));
}
void interpretator::eval_const(int32_t value) {
    push(box(value));
}
void interpretator::eval_string(int32_t offset) {
    push(reinterpret_cast<int64_t>(Bstring(bf->string_ptr + offset)));
}
void interpretator::eval_sexp(char* name, int n) {
    int32_t tag = LtagHash(name);
    reverse(n);
    int32_t res = reinterpret_cast<int32_t>(Bsexp_arr(box(n+1), tag, get_stack_bottom()));
    drop(n);
    push(res);
}
//void interpreter::eval_sti() {
// }
void interpretator::eval_sta() {
    int32_t v = pop();
    int32_t i = pop();

    if (!boxed(i)) {
        *(void**)i = reinterpret_cast<void*>(v);
        push(v);
        return;
    }

    int32_t x = pop();
    push(reinterpret_cast<int32_t>(
            Bsta(
                reinterpret_cast<void*>(v),
                i,
                reinterpret_cast<void*>(x)
            )
    ));
}
void interpretator::eval_jmp(int32_t addr) {
    ip = bf->code_ptr + addr;
}
void interpretator::eval_end() {
    ip = reinterpret_cast<char*>(epilogue());
}
//void interpreter::eval_ret() {}
void interpretator::eval_drop() {
    drop(1);
}
void interpretator::eval_dup() {
    push(top());
}
//void interpreter::eval_swap() {}
void interpretator::eval_elem() {
    int32_t elem = pop();
    void* sexp = reinterpret_cast<void*>(pop());
    push(reinterpret_cast<int64_t>(Belem(sexp, elem)));
}
void interpretator::eval_ld(variable& var) {
    int32_t& val = get_var(var);
    push(val);
}
void interpretator::eval_lda(variable& var) {
    push(reinterpret_cast<int64_t>(&(get_var(var))));
}
void interpretator::eval_st(variable& var) {
    int32_t rval = top();
    int32_t& lval = get_var(var);
    lval = rval;
}
void interpretator::eval_cjmpz(int32_t addr) {
    int32_t ifval = unbox(pop());
    if (ifval == 0)
        ip = bf->code_ptr + addr;
}
void interpretator::eval_cjmpnz(int32_t addr) {
    int32_t ifval = unbox(pop());
    if (ifval != 0)
        ip = bf->code_ptr + addr;
}
void interpretator::eval_begin(int32_t nargs, int32_t nlocals) {
    prologue(nlocals, nargs);
}
void interpretator::eval_cbegin(int32_t nargs, int32_t nlocals) {
    prologue(nlocals, nargs);
}
void interpretator::eval_closure() {
    int32_t shift = get_int();
    int32_t n_binded = get_int();
    int32_t binds[n_binded];
    for (int i = 0; i < n_binded; i++) {
        char l = get_byte();
        int value = get_int();
        variable var = variable(l, value);
        binds[i] = get_var(var);
    }
    push(reinterpret_cast<int32_t>(Bclosure_arr(box(n_binded), bf->code_ptr + shift, binds)));
}
void interpretator::eval_call(int32_t addr, int32_t nargs) {
    reverse(nargs);
    push(reinterpret_cast<int64_t>(ip));
    push(nargs);
    ip = bf->code_ptr + addr;
}
void interpretator::eval_callc(int32_t nargs) {
    int32_t closure = top(nargs);

    reverse(nargs);
    push(reinterpret_cast<int64_t>(ip));
    push(nargs + 1);
    ip = reinterpret_cast<char*>(Belem(reinterpret_cast<void*>(closure), box(0)));
}
void interpretator::eval_tag(char* name, int32_t n) {
    int32_t tag = LtagHash(name);
    int32_t sexp = pop();
    push(Btag(reinterpret_cast<void*>(sexp), tag, box(n)));
}
void interpretator::eval_array(int32_t n) {
    push(reinterpret_cast<int32_t>(Barray_patt(reinterpret_cast<void*>(pop()), box(n))));
}
void interpretator::eval_fail(char h, char l) {
    failure("ERROR: invalid opcode %d-%d\n", h, l);
}
void interpretator::eval_line() {
    int line = get_int();
}
void interpretator::eval_patt(int32_t patt) {
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
void interpretator::eval_lread() {
    push(Lread());
}
void interpretator::eval_lwrite() {
    int32_t val = pop();
    push(Lwrite(val));
}
void interpretator::eval_llength() {
    int32_t length = Llength(reinterpret_cast<void*>(pop()));
    push(length);
}
void interpretator::eval_lstring() {
    push(reinterpret_cast<int64_t>(Lstring(reinterpret_cast<void*>(pop()))));
}
void interpretator::eval_barray() {
    int32_t len = get_int();
    reverse(len);
    int32_t res = reinterpret_cast<int32_t>(Barray_arr(box(len), get_stack_bottom()));
    drop(len);
    push(res);
}
static void not_impl() {
    throw std::runtime_error("Not implemented");
}

void interpretator::intepretate() {
    int pos = 0;
    do {
        pos++;
        char x = get_byte(),
                h = (x & 0xF0) >> 4,
                l = x & 0x0F;

        switch (h) {
            case 15:
                goto stop;

                /* BINOP */
            case 0:
//                fprintf (f, "BINOP\t%s", ops[l-1]);
                eval_binop(l-1);
                break;

            case 1:
                switch (l) {
                    case  0:
                        eval_const(get_int());
                        break;

                    case  1:
                        eval_string(get_int());
                        break;

                    case  2: {
                        char *name = get_string();
                        int n = get_int();
                        eval_sexp(name, n);
                        break;
                    }

                    case  3: // notimpl
                        not_impl();
                        break;

                    case  4:
                        eval_sta();
                        break;

                    case  5:
                        eval_jmp(get_int());
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
                        eval_fail(h, l);
                }
                break;

            case 2:
            case 3:
            case 4: {
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
            }
            case 5:
                switch (l) {
                    case  0:
                        eval_cjmpz(get_int());
                        break;

                    case  1:
                        eval_cjmpnz(get_int());
                        break;

                    case  2: {
                        int nargs = get_int();
                        int nlocals = get_int();
                        eval_begin(nargs, nlocals);
                        break;
                    }
                    case  3: {
                        int nargs = get_int();
                        int nlocals = get_int();
                        eval_cbegin(nargs, nlocals);
                        break;
                    }
                    case  4: {
                        eval_closure();
                        break;
                    }
                    case  5:
                        eval_callc(get_int());
                        break;

                    case  6: {
                        int addr = get_int();
                        int nargs = get_int();
                        eval_call(addr, nargs);
                        break;
                    }
                    case  7: {
                        char *name = get_string();
                        int n = get_int();
                        eval_tag(name, n);
                        break;
                    }
                    case  8:
                        eval_array(get_int());
                        break;

                    case  9: {
                        int x = get_int(), y = get_int();
//                        debug(printf("FAIL %d %d\n", x, y););
                        eval_fail(x, y);
                        break;
                    }
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
    }
    while (ip != nullptr);

    stop: return;
}

// stack
inline int32_t* interpretator::get_stack_bottom() {
    return stack_bottom;
}

inline int32_t* interpretator::get_stack_top() {
    return stack_top;
}

void interpretator::push(int32_t value) {
    if (stack_bottom == stack_top - stack_size) {
        throw new std::runtime_error("Pushing on full stack exceeded");
    }
    *(--stack_bottom) = value;
}

int32_t interpretator::pop() {
    if (stack_bottom == stack_top) {
        throw new std::runtime_error("Popping empty stack exceeded");
    }
    return *(stack_bottom++);
}

int32_t interpretator::top() {
    return top(0);
}

int32_t interpretator::top(int i) {
    if (cur_size() <= i) {
        throw new std::runtime_error(
                "get top " + std::to_string(i) + " element when stack size " + std::to_string(cur_size()));
    }

    return *(stack_bottom + i);
}

void interpretator::allocate(int n) {
    if (stack_size - cur_size() < n) {
        throw new std::runtime_error("allocate " + std::to_string(n) + " elements when free stack space is " +
                                     std::to_string(stack_size - cur_size()));
    }

    stack_bottom -= n;
}

void interpretator::drop(int n) {
    if (cur_size() < n) [[unlikely]]
                throw new std::runtime_error("drop " + std::to_string(n) + " elements when stack size is " + std::to_string(cur_size()));

    stack_bottom += n;
}

void interpretator::prologue(int32_t nlocals, int32_t nargs) {
    push(reinterpret_cast<int64_t>(fp));
    fp = stack_bottom;
    allocate(nlocals);
}

int32_t interpretator::epilogue() {
    int32_t ret_val = pop();
    stack_bottom = fp;
    fp = reinterpret_cast<int32_t*>(pop());
    int32_t nargs = pop();
    int32_t ip = pop();
    drop(nargs);
    push(ret_val);
    return ip;
}

void interpretator::reverse(int nargs) {
    if (cur_size() < nargs) [[unlikely]]
                throw new std::runtime_error("reverse " + std::to_string(nargs) + " elements when stack size is " + std::to_string(cur_size()));

    int32_t* st = stack_bottom + nargs - 1;
    int32_t* fn = stack_bottom;

    while (st > fn) {
        std::swap(*(st--), *(fn++));
    }
}

int32_t& interpretator::get_arg(int arg) {
    if (fp + 3 + arg >= stack_top) [[unlikely]]
                throw new std::runtime_error("fail to take argument " + std::to_string(arg));

    return *(fp + 3 + arg);
}

int32_t& interpretator::get_local(int local) {
    if (fp - local - 1 < (stack_top - stack_size)) [[unlikely]]
                throw new std::runtime_error("fail to get local " + std::to_string(local));

    return *(fp - local - 1);
}

int32_t& interpretator::get_closure() {
    if (fp + 1 >= stack_top || fp + 2 + *(fp + 1) >= stack_top) [[unlikely]]
                throw new std::runtime_error("faile to get closure");

    return *(fp + 2 + *(fp + 1));
}

int32_t interpretator::cur_size() {
    return stack_top - stack_bottom;
}

interpretator::interpretator(bytefile* bf) : bf(bf) {
    push(reinterpret_cast<int32_t>((char *)nullptr));
    push(0);
    ip = bf->code_ptr;
}
