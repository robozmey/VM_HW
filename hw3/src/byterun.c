/* Lama SM Bytecode interpreter */

# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <malloc.h>
# include <stdarg.h>
# include <stdlib.h>
# include "byterun.h"


void *__start_custom_data;
void *__stop_custom_data;


static void vfailure (char *s, va_list args) {
    fprintf(stderr, "*** FAILURE: ");
    vfprintf(stderr, s, args);   // vprintf (char *, va_list) <-> printf (char *, ...)
    exit(255);
}

void failure (char *s, ...) {
    va_list args;

    va_start(args, s);
    vfailure(s, args);
}

/* Gets a string from a string table by an index */
char* get_string (bytefile *f, int pos) {
    return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char* get_public_name (bytefile *f, int i) {
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
    file->bytecode_size = (char*) &file->stringtab_size + size - file->code_ptr;

    return file;
}

char *ops [] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
char *lds [] = {"LD", "LDA", "ST"};

void flogf(FILE *f, const char *format, ...) {
    if (f == NULL) return;

    va_list args;
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
}

/* Disassembles the bytecode pool */
char * disassemble_instruction (FILE *f, bytefile *bf, char *ip) {

# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   failure ("ERROR: invalid opcode %d-%d\n", h, l)

    char x = BYTE,
            h = (x & 0xF0) >> 4,
            l = x & 0x0F;

    switch (h) {
        case 15:
            flogf (f, "STOP");
            break;

            /* BINOP */
        case 0:
            flogf (f, "BINOP\t%s", ops[l-1]);
            break;

        case 1:
            switch (l) {
                case  0:
                    flogf (f, "CONST\t%d", INT);
                    break;

                case  1:
                    flogf (f, "STRING\t%s", STRING);
                    break;

                case  2:
                    flogf (f, "SEXP\t%s ", STRING);
                    flogf (f, "%d", INT);
                    break;

                case  3:
                    flogf (f, "STI");
                    break;

                case  4:
                    flogf (f, "STA");
                    break;

                case  5:
                    flogf (f, "JMP\t0x%.8x", INT);
                    break;

                case  6:
                    flogf (f, "END");
                    break;

                case  7:
                    flogf (f, "RET");
                    break;

                case  8:
                    flogf (f, "DROP");
                    break;

                case  9:
                    flogf (f, "DUP");
                    break;

                case 10:
                    flogf (f, "SWAP");
                    break;

                case 11:
                    flogf (f, "ELEM");
                    break;

                default:
                    FAIL;
            }
            break;

        case 2:
        case 3:
        case 4:
            flogf (f, "%s\t", lds[h-2]);
            switch (l) {
                case 0: flogf (f, "G(%d)", INT); break;
                case 1: flogf (f, "L(%d)", INT); break;
                case 2: flogf (f, "A(%d)", INT); break;
                case 3: flogf (f, "C(%d)", INT); break;
                default: FAIL;
            }
            break;

        case 5:
            switch (l) {
                case  0:
                    flogf (f, "CJMPz\t0x%.8x", INT);
                    break;

                case  1:
                    flogf (f, "CJMPnz\t0x%.8x", INT);
                    break;

                case  2:
                    flogf (f, "BEGIN\t%d ", INT);
                    flogf (f, "%d", INT);
                    break;

                case  3:
                    flogf (f, "CBEGIN\t%d ", INT);
                    flogf (f, "%d", INT);
                    break;

                case  4:
                    flogf (f, "CLOSURE\t0x%.8x", INT);
                    {int n = INT;
                        for (int i = 0; i<n; i++) {
                            switch (BYTE) {
                                case 0: flogf (f, "G(%d)", INT); break;
                                case 1: flogf (f, "L(%d)", INT); break;
                                case 2: flogf (f, "A(%d)", INT); break;
                                case 3: flogf (f, "C(%d)", INT); break;
                                default: FAIL;
                            }
                        }
                    };
                    break;

                case  5:
                    flogf (f, "CALLC\t%d", INT);
                    break;

                case  6:
                    flogf (f, "CALL\t0x%.8x ", INT);
                    flogf (f, "%d", INT);
                    break;

                case  7:
                    flogf (f, "TAG\t%s ", STRING);
                    flogf (f, "%d", INT);
                    break;

                case  8:
                    flogf (f, "ARRAY\t%d", INT);
                    break;

                case  9:
                    flogf (f, "FAIL\t%d", INT);
                    flogf (f, "%d", INT);
                    break;

                case 10:
                    flogf (f, "LINE\t%d", INT);
                    break;

                default:
                    FAIL;
            }
            break;

        case 6:
            flogf (f, "PATT\t%s", pats[l]);
            break;

        case 7: {
            switch (l) {
                case 0:
                    flogf (f, "CALL\tLread");
                    break;

                case 1:
                    flogf (f, "CALL\tLwrite");
                    break;

                case 2:
                    flogf (f, "CALL\tLlength");
                    break;

                case 3:
                    flogf (f, "CALL\tLstring");
                    break;

                case 4:
                    flogf (f, "CALL\tBarray\t%d", INT);
                    break;

                default:
                    FAIL;
            }
        }
            break;

        default:
            FAIL;
    }

    flogf (f, "\n");
    return ip;
}

