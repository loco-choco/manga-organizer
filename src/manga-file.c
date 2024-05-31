#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "manga-file.h"
#include "record-entry.h"
#include "primary-index-entry.h"

int open_manga_file(char* record_file_name, char* primary_keys_file_name, manga_file** mangas){
  FILE* keys_file = fopen(primary_keys_file_name, "r+");
  FILE* record_file = fopen(record_file_name, "r+");
  primary_index_list* keys;
  if(keys_file == NULL || read_primary_keys_file(keys_file, &keys) == -1){
    keys_file = fopen(primary_keys_file_name,"w");
    if(record_file != NULL){
      create_primary_keys_from_record_file(record_file, &keys);
    }
    else{
      record_file = fopen(record_file_name, "w");
      keys = NULL;
    }
  }
  //as we are now editing it, mark the file as inconsistent 
  mark_primary_keys_file_as_inconsistent(keys_file);

  manga_file* mangas_ = malloc(sizeof(*mangas_));
  mangas_->record_file = record_file;
  mangas_->primary_keys_file = keys_file;
  mangas_->keys = keys;

  *mangas = mangas_;
  return 0;
}

int close_manga_file(manga_file* mangas){
  fclose(mangas->record_file);
  write_primary_keys_file(mangas->keys, mangas->primary_keys_file);
  fclose(mangas->primary_keys_file);
  free_primary_index_list(mangas->keys);
  free(mangas);
  return 0;
}

int search_manga_isbn(manga_file* mangas, char* isbn, manga_record** manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, isbn, &entry) != 0) return -1;
  return read_record_in_position(mangas->record_file, entry->position, manga);
}

int add_new_manga(manga_file* mangas, manga_record* manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, manga->isbn, &entry) == 0) return -1; //cant add if with same isbn exists
  long position;
  write_new_record(manga, mangas->record_file, &position);

  primary_index_entry* new_key_entry = malloc(sizeof(*new_key_entry));
  new_key_entry->isbn = malloc(sizeof(char)*(strlen(manga->isbn) + 1));
  strcpy(new_key_entry->isbn, manga->isbn);
  new_key_entry->position = position;

  sorted_insert_primary_keys(&mangas->keys, new_key_entry);
  return 0;
}

int update_manga(manga_file* mangas, manga_record* manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, manga->isbn, &entry) != 0) return -1; //cant update if the isbn doesnt exists
  if(update_record(entry->position, manga, mangas->record_file) == 0) return 0; //was able to update without an issue
  //no space, so mark the current pos for deletion and add append record
  mark_record_as_deleted(entry->position, mangas->record_file);
  write_new_record(manga, mangas->record_file, &entry->position);
  return 0;
}

int update_manga_isbn(manga_file* mangas, char* old_isbn, manga_record* manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, old_isbn, &entry) != 0) return -1; //cant update if the old isbn doesnt exists
  long position_of_record = entry->position;
  remove_primary_keys(&mangas->keys, old_isbn);
  if(update_record(position_of_record, manga, mangas->record_file) != 0){ //we need to remove and write a new one
    mark_record_as_deleted(position_of_record, mangas->record_file);
    write_new_record(manga, mangas->record_file, &position_of_record);
  }
  //add updated pos and isbn to keys
  entry = malloc(sizeof(*entry));
  entry->isbn = malloc(sizeof(char)*strlen(manga->isbn));
  strcpy(entry->isbn, manga->isbn);
  entry->position = position_of_record;

  sorted_insert_primary_keys(&mangas->keys, entry);
  return 0;
}

int remove_manga(manga_file* mangas, manga_record* manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, manga->isbn, &entry) != 0) return -1; //cant remove if the isbn doesnt exists
  mark_record_as_deleted(entry->position, mangas->record_file);
  remove_primary_keys(&mangas->keys, manga->isbn);
  return 0;
}
