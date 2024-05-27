#include <stdio.h>
#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H
int write_string(char* string, FILE* file_pointer);
int read_string(FILE* file_pointer, char** string);
#endif
