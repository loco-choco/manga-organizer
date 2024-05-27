#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "primary-index-entry.h"
#include "string-helpers.h"

int write_primary_key(primary_index_entry* entry, FILE* file_pointer){
  write_string(entry->isbn, file_pointer);
  fwrite(&entry->position, sizeof(int), 1, file_pointer);
  return 0;
}

int read_primary_key(FILE* file_pointer, primary_index_entry** entry){
  *entry = malloc(sizeof(**entry));
  read_string(file_pointer, &(*entry)->isbn);
  fread(&(*entry)->position, sizeof(int), 1, file_pointer);
  return 0;
}

int write_all_primary_keys(primary_index_entry** primary_keys, int primary_keys_amount, FILE* file_pointer){
  fwrite(&primary_keys_amount, sizeof(int), 1, file_pointer);
  for(int i = 0; i < primary_keys_amount; i++)
    write_primary_key(primary_keys[i], file_pointer);
}
int read_all_primary_keys(FILE* file_pointer, primary_index_entry*** primary_keys){
  int amount;
  fread(&amount, sizeof(int), 1, file_pointer);
  *primary_keys = malloc(sizeof(**primary_keys) * amount);
  for(int i = 0; i < amount; i++)
    read_primary_key(file_pointer, primary_keys[i]);
  return 0;
}
