//
// Created by vladimir on 08.12.23.
//

#ifndef VM_HW2_BYTEFILE_H
#define VM_HW2_BYTEFILE_H


/* Lama SM Bytecode interpreter */

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
char* get_string (bytefile *f, int pos) {
    return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char* bytefile_get_public_name (bytefile *f, int i) {
    return get_string (f, f->public_ptr[i*2]);
}

/* Gets an offset for a publie symbol */
int get_public_offset (bytefile *f, int i) {
    return f->public_ptr[i*2+1];
}

/* Reads a binary bytecode file by name and unpacks it */
bytefile* read_file (char *fname) {
    FILE *f = fopen (fname, "rb");
    long size;
    bytefile *file;

    if (f == 0) {
        failure ("%s\n", strerror (errno));
    }

    if (fseek (f, 0, SEEK_END) == -1) {
        failure ("%s\n", strerror (errno));
    }

    file = (bytefile*) malloc (sizeof(int)*4 + (size = ftell (f)));

    if (file == 0) {
        failure ("*** FAILURE: unable to allocate memory.\n");
    }

    rewind (f);

    if (size != fread (&file->stringtab_size, 1, size, f)) {
        failure ("%s\n", strerror (errno));
    }

    fclose (f);

    file->string_ptr  = &file->buffer [file->public_symbols_number * 2 * sizeof(int)];
    file->public_ptr  = (int*) file->buffer;
    file->code_ptr    = &file->string_ptr [file->stringtab_size];
    file->global_ptr  = (int*) malloc (file->global_area_size * sizeof (int));

    return file;
}


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


#endif //VM_HW2_BYTEFILE_H
