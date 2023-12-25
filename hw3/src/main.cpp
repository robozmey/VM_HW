#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <map>

#include "byterun.h"


#define instr std::pair<char *, int>
#define len second
#define ptr first

int main (int argc, char* argv[]) {

    bytefile *f = read_file (argv[1]);

    std::map<instr, int, decltype([](instr i1, instr i2) {
        return i1.len < i2.len || i1.len == i2.len && memcmp(i1.ptr, i2.ptr, i1.len) < 0;
    }) > count;

    char* ip = f->code_ptr;
    while (ip != nullptr) {
        char* new_ip = disassemble_instruction(nullptr, f, ip);
        if (new_ip == nullptr) break;

        count[{ip, new_ip - ip}]++;

        ip = new_ip;
    }

    std::vector<std::pair<int, instr>> statistics;
    for (auto [bytecode, cnt]: count) {
        statistics.emplace_back(cnt, bytecode);
    }
    std::sort(statistics.begin(), statistics.end(), [](std::pair<int, instr> i1, std::pair<int, instr> i2) {
        if (i1.first == i2.first)
            return i1.second.len == i2.second.len && !memcmp(i1.second.ptr, i2.second.ptr, i1.second.len);

        return i1.first > i2.first;
    });

    for (auto [cnt, bytecode]: statistics) {
        fprintf(stdout, "%d: ", cnt);
        disassemble_instruction(stdout, f, bytecode.ptr);
    }

    free(f->global_ptr);
    free(f);

    return 0;
}