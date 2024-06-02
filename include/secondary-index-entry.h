#include <stdio.h>
#ifndef SECONDARY_INDEX_ENTRY_H
#define SECONDARY_INDEX_ENTRY_H
struct secondary_index_entry {
  char* title;
  long first_position_of_isbn_list;
} typedef secondary_index_entry;

struct secondary_index_list {
  secondary_index_entry* entry;
  struct secondary_index_list* next;
} typedef secondary_index_list;

struct secondary_index_file {
  FILE* titles_file_pointer;
  FILE* isbns_file_pointer;
  secondary_index_list* index_list;
} typedef secondary_index_file;

int write_new_secondary_key_isbn_list(char* isbn, long next, FILE* isbns_file_pointer, long* position);
int read_secondary_key_isbn_list(FILE* isbns_file_pointer, long position, char** isbn, long* next);
int update_secondary_key_title_list(long position, long new_next, FILE* isbns_file_pointer);

int write_secondary_key(secondary_index_entry* entry, FILE* titles_file_pointer);
int read_secondary_key(FILE* titles_file_pointer, secondary_index_entry** entry);
int write_all_secondary_keys(FILE* titles_file_pointer, secondary_index_list* secondary_keys);
int read_all_secondary_keys(FILE* titles_file_pointer, secondary_index_list** secondary_keys);
int create_secondary_keys_from_record_file(FILE* file_pointer, secondary_index_file* secondary_keys_file);

int free_secondary_index_list(secondary_index_list* secondary_keys);
int free_secondary_index(secondary_index_entry* entry);

int sorted_insert_secondary_keys(secondary_index_file* secondary_keys, char* title, char* isbn);
int remove_secondary_keys(secondary_index_file* secondary_index_file, char* title, char* isbn);
int move_secondary_keys(secondary_index_file* secondary_index_file, char* old_title, char* title, char* isbn);
int update_secondary_keys(secondary_index_file* secondary_index_file, char* old_title, char* title, char* old_isbn, char* isbn);
int search_secondary_keys(secondary_index_file* secondary_index_file, char* title, int* amount_of_isbns, char*** isbns);

int close_secondary_keys(secondary_index_file* secondary_index_file);
#endif
