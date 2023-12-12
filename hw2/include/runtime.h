//
// Created by vladimir on 08.12.23.
//

#ifndef VM_HW2_RUNTIME
#define VM_HW2_RUNTIME

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#define WORD_SIZE (CHAR_BIT * sizeof(int))

void failure (char *s, ...);

#endif //VM_HW2_RUNTIME
