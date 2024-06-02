#include <stdio.h>
#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H

#define STRING_END '|'
#define MAX_STRING_SIZE 100

int write_string(char* string, FILE* file_pointer);
int read_string(FILE* file_pointer, char** string);
#endif
