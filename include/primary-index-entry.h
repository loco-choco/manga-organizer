#include <stdio.h>
#ifndef PRIMARY_INDEX_ENTRY_H
#define PRIMARY_INDEX_ENTRY_H
struct primary_index_entry {
  char* isbn;
  long position;
} typedef primary_index_entry;

struct primary_index_list {
  primary_index_entry* entry;
  struct primary_index_list* next;
} typedef primary_index_list;

int write_primary_key(primary_index_entry* entry, FILE* file_pointer);
int read_primary_key(FILE* file_pointer, primary_index_entry** entry);
int write_all_primary_keys(primary_index_list* primary_keys, FILE* file_pointer);
int read_all_primary_keys(FILE* file_pointer, primary_index_list** primary_keys);
int free_primary_index_list(primary_index_list* primary_keys);
int free_primary_index(primary_index_entry* entry);

int write_primary_keys_file(primary_index_list* primary_keys, FILE* file_pointer);
int mark_primary_keys_file_as_inconsistent(FILE* file_pointer);
int read_primary_keys_file(FILE* file_pointer, primary_index_list** primary_keys);
int create_primary_keys_from_record_file(FILE* file_pointer, primary_index_list** primary_keys);
int sorted_insert_primary_keys(primary_index_list** primary_keys, primary_index_entry* entry);
int remove_primary_keys(primary_index_list** primary_keys, char* isbn);
int search_primary_keys(primary_index_list* primary_keys, char* isbn, primary_index_entry** entry);
#endif
