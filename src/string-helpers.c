#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string-helpers.h"
int write_string(char* string, FILE* file_pointer){
  int string_size = strlen(string);
  fwrite(&string_size, sizeof(int), 1, file_pointer);
  fwrite(string, sizeof(char), strlen(string), file_pointer);
  return 0;
}

int read_string(FILE* file_pointer, char** string){
  int string_size; 
  fread(&string_size, sizeof(int), 1, file_pointer);
  *string = malloc(sizeof(char) * (string_size + 1));
  fread(*string, sizeof(char), string_size, file_pointer);
  (*string)[string_size] = '\0';
  return 0;
}
