#include "bytefile.h"

# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <malloc.h>

//extern "C" {
#include "runtime.h"
//}

static void vfailure (char *s, va_list args) {
    fflush   (stdout);
    fprintf  (stderr, "*** FAILURE: ");
    vfprintf (stderr, s, args); // vprintf (char *, va_list) <-> printf (char *, ...)
    exit     (255);
}

void failure(char *s, ...) {
    va_list args;

    va_start (args, s);
    vfailure (s, args);
}

void *__start_custom_data;
void *__stop_custom_data;

char* get_string(bytefile *f, int pos) {
    return &f->string_ptr[pos];
}

char* get_public_name(bytefile *f, int i) {
    return get_string (f, f->public_ptr[i*2]);
}

int get_public_offset(bytefile *f, int i) {
    return f->public_ptr[i*2+1];
}

bytefile* read_file(char *fname) {
    FILE *f = fopen(fname, "rb");
    long size;
    bytefile *file;

    if (f == 0) {
        failure("%s\n", strerror (errno));
    }

    if (fseek (f, 0, SEEK_END) == -1) {
        failure("%s\n", strerror (errno));
    }

    file = (bytefile*) malloc (sizeof(int)*4 + (size = ftell (f)));

    if (file == 0) {
        failure("*** FAILURE: unable to allocate memory.\n");
    }

    rewind (f);

    if (size != fread (&file->stringtab_size, 1, size, f)) {
        failure("%s\n", strerror (errno));
    }

    fclose (f);

    file->string_ptr  = &file->buffer [file->public_symbols_number * 2 * sizeof(int)];
    file->public_ptr  = (int*) file->buffer;
    file->code_ptr    = &file->string_ptr [file->stringtab_size];
    file->global_ptr  = (int*) malloc (file->global_area_size * sizeof (int));

    return file;
}