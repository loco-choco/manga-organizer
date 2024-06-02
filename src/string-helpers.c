#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string-helpers.h"

int write_string(char* string, FILE* file_pointer){
  int string_size = strlen(string);
  fwrite(string, sizeof(char), strlen(string), file_pointer);
  char string_end = STRING_END;
  fwrite(&string_end, sizeof(char), 1, file_pointer);
  return 0;
}

int read_string(FILE* file_pointer, char** string){
  char* buffer = calloc(MAX_STRING_SIZE + 1, sizeof(char));
  int string_size = 0; 
  char read_char;
  fread(&read_char, sizeof(char), 1, file_pointer);
  while(read_char != STRING_END){
    if(string_size < MAX_STRING_SIZE){
      buffer[string_size] = read_char;
      string_size++;
    }
    fread(&read_char, sizeof(char), 1, file_pointer);
  }
  *string = malloc(sizeof(char) * (string_size + 1));
  strcpy(*string, buffer);
  free(buffer);
  return 0;
}
