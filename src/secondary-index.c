#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "secondary-index-entry.h"
#include "string-helpers.h"

int write_secondary_key(secondary_index_entry* entry, FILE* file_pointer){
  write_string(entry->title, file_pointer);
  write_string(entry->isbn, file_pointer);
  return 0;
}

int read_secondary_key(FILE* file_pointer, secondary_index_entry** entry){
  *entry = malloc(sizeof(**entry));
  read_string(file_pointer, &(*entry)->title);
  read_string(file_pointer, &(*entry)->isbn);
  return 0;
}

int write_all_secondary_keys(secondary_index_entry** secondary_keys, int secondary_keys_amount, FILE* file_pointer){
  fwrite(&secondary_keys_amount, sizeof(int), 1, file_pointer);
  for(int i = 0; i < secondary_keys_amount; i++)
    write_secondary_key(secondary_keys[i], file_pointer);
}
int read_all_secondary_keys(FILE* file_pointer, secondary_index_entry*** secondary_keys){
  int amount;
  fread(&amount, sizeof(int), 1, file_pointer);
  *secondary_keys = malloc(sizeof(**secondary_keys) * amount);
  for(int i = 0; i < amount; i++)
    read_secondary_key(file_pointer, secondary_keys[i]);
  return 0;
}
