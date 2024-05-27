#include <stdio.h>
#ifndef SECONDARY_INDEX_ENTRY_H
#define SECONDARY_INDEX_ENTRY_H
struct secondary_index_entry {
  char* title;
  char* isbn;
} typedef secondary_index_entry;

int write_secondary_key(secondary_index_entry* entry, FILE* file_pointer);
int read_secondary_key(FILE* file_pointer, secondary_index_entry** entry);
int write_all_secondary_keys(secondary_index_entry** secondary_keys, int secondary_keys_amount, FILE* file_pointer);
int read_all_secondary_keys(FILE* file_pointer, secondary_index_entry*** secondary_keys);
#endif
