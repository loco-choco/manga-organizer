#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "primary-index-entry.h"
#include "record-entry.h"
#include "string-helpers.h"

#define ENTRY_START '|'

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

int write_all_primary_keys(primary_index_list* primary_keys, FILE* file_pointer){
  primary_index_list* current = primary_keys;
  while(current != NULL){
    char entry_start = ENTRY_START;
    fwrite(&entry_start, sizeof(char), 1, file_pointer);
    write_primary_key(current->entry, file_pointer);
    current = current->next;
  }
  return 0;
}
int read_all_primary_keys(FILE* file_pointer, primary_index_list** primary_keys){
  *primary_keys = NULL;
  primary_index_list* current;

  char entry_start;
  fread(&entry_start, sizeof(char), 1, file_pointer);
  while(entry_start != EOF){
    primary_index_entry* new_entry;
    read_primary_key(file_pointer, &new_entry);

    current = calloc(1, sizeof(*current));

    if(*primary_keys == NULL)
      *primary_keys = current;

    current->entry = new_entry;
    current = current->next;
  }
  return 0;
}

int free_primary_index_list(primary_index_list* primary_keys){
  primary_index_list* current = primary_keys;
  primary_index_list* old_current;
  while(current != NULL){
    if(current->entry != NULL) free_primary_index(current->entry);
    current = current->next;
    old_current = current;
    free(old_current);
  }
  return 0;
}

int free_primary_index(primary_index_entry* entry){
  if(entry->isbn != NULL) free(entry->isbn);
  free(entry);
  return 0;
}
