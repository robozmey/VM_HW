/* Lama SM Bytecode interpreter */
#pragma once

# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <malloc.h>

# include "runtime.h"

void *__start_custom_data;
void *__stop_custom_data;

/* The unpacked representation of bytecode file */
typedef struct {
    char *string_ptr;              /* A pointer to the beginning of the string table */
    int  *public_ptr;              /* A pointer to the beginning of publics table    */
    char *code_ptr;                /* A pointer to the bytecode itself               */
    int  *global_ptr;              /* A pointer to the global area                   */
    int   stringtab_size;          /* The size (in bytes) of the string table        */
    int   global_area_size;        /* The size (in words) of global area             */
    int   public_symbols_number;   /* The number of public symbols                   */
    char  buffer[0];
} bytefile;

/* Gets a string from a string table by an index */
char* get_string (bytefile *f, int pos);

/* Gets a name for a public symbol */
char* get_public_name (bytefile *f, int i);

/* Gets an offset for a publie symbol */
int get_public_offset (bytefile *f, int i);

/* Reads a binary bytecode file by name and unpacks it */
bytefile* read_file (char *fname);


///* Dumps the contents of the file */
//void dump_file (FILE *f, bytefile *bf) {
//    int i;
//
//    fprintf (f, "String table size       : %d\n", bf->stringtab_size);
//    fprintf (f, "Global area size        : %d\n", bf->global_area_size);
//    fprintf (f, "Number of public symbols: %d\n", bf->public_symbols_number);
//    fprintf (f, "Public symbols          :\n");
//
//    for (i=0; i < bf->public_symbols_number; i++)
//        fprintf (f, "   0x%.8x: %s\n", bytefile_get_public_offset (bf, i), bytefile_get_public_name (bf, i));
//
//    fprintf (f, "Code:\n");
//    disassemble (f, bf);
//}
