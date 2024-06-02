#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "manga-file.h"
#include "record-entry.h"
#include "primary-index-entry.h"
#include "secondary-index-entry.h"

int open_manga_file(char* record_file_name, char* secondary_keys_isbns_file_name, char* secondary_keys_titles_file_name, char* primary_keys_file_name, manga_file** mangas){
  FILE* secondary_keys_isbns_file = fopen(secondary_keys_isbns_file_name, "r+");
  FILE* secondary_keys_titles_file = fopen(secondary_keys_titles_file_name, "r+");
  FILE* keys_file = fopen(primary_keys_file_name, "r+");
  FILE* record_file = fopen(record_file_name, "r+");

  primary_index_list* keys;
  secondary_index_file* secondary_keys = calloc(1, sizeof(*secondary_keys));
  //setting what files are for the secondary keys

  if(keys_file == NULL || read_primary_keys_file(keys_file, &keys) == -1){
    keys_file = fopen(primary_keys_file_name,"w");
    if(record_file != NULL){ //Remake primary and secondary keys 
      create_primary_keys_from_record_file(record_file, &keys);
      create_secondary_keys_from_record_file(record_file, secondary_keys);
    } else { //New record
      record_file = fopen(record_file_name, "w+");
      keys = NULL;

      if(secondary_keys_isbns_file != NULL) fclose(secondary_keys_isbns_file);
      if(secondary_keys_titles_file != NULL) fclose(secondary_keys_titles_file);
      secondary_keys_isbns_file = fopen(secondary_keys_isbns_file_name, "w+");
      secondary_keys_titles_file = fopen(secondary_keys_titles_file_name, "w");
      secondary_keys->index_list = NULL;
    }
  } else if(secondary_keys_titles_file == NULL || secondary_keys_isbns_file == NULL){
    //if either of the files for the secondary keys are missing, remake them
      if(secondary_keys_isbns_file != NULL) fclose(secondary_keys_isbns_file);
      if(secondary_keys_titles_file != NULL) fclose(secondary_keys_titles_file);
      secondary_keys_isbns_file = fopen(secondary_keys_isbns_file_name, "w+");
      secondary_keys_titles_file = fopen(secondary_keys_titles_file_name, "w");
      
      secondary_keys->isbns_file_pointer = secondary_keys_isbns_file;
      secondary_keys->titles_file_pointer = secondary_keys_titles_file;
      secondary_keys->index_list = NULL;
      
      create_secondary_keys_from_record_file(record_file, secondary_keys);
  } else { //Both exist, and no inconsistency, read
    read_all_secondary_keys(secondary_keys_titles_file, &secondary_keys->index_list);
  }
  //save the file pointers for the secondary key
  secondary_keys->isbns_file_pointer = secondary_keys_isbns_file;
  secondary_keys->titles_file_pointer = secondary_keys_titles_file;

  //as we are now editing it, mark the file as inconsistent 
  mark_primary_keys_file_as_inconsistent(keys_file);

  manga_file* mangas_ = malloc(sizeof(*mangas_));
  mangas_->record_file = record_file;
  mangas_->primary_keys_file = keys_file;
  mangas_->keys = keys;
  mangas_->secondary_keys = secondary_keys;

  *mangas = mangas_;
  return 0;
}

int close_manga_file(manga_file* mangas){
  fclose(mangas->record_file);
  close_secondary_keys(mangas->secondary_keys); //close this before, so the primary keys being inconsistent
                                                //show that we also shouldnt trust this close
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

int search_manga_title(manga_file* mangas, char* title, int* amount, char*** isbns){
  return search_secondary_keys(mangas->secondary_keys, title, amount, isbns);
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
  sorted_insert_secondary_keys(mangas->secondary_keys, manga->title, manga->isbn);
  return 0;
}

int update_manga(manga_file* mangas, char* old_title, manga_record* manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, manga->isbn, &entry) != 0) return -1; //cant update if the isbn doesnt exists
  //update secondary keys
  if(strcmp(old_title, manga->title) != 0)
    move_secondary_keys(mangas->secondary_keys, old_title, manga->title, manga->isbn);
  if(update_record(entry->position, manga, mangas->record_file) == 0) return 0; //was able to update without an issue
  //no space, so mark the current pos for deletion and add append record
  mark_record_as_deleted(entry->position, mangas->record_file);
  write_new_record(manga, mangas->record_file, &entry->position);
  return 0;
}

int update_manga_isbn(manga_file* mangas, char* old_isbn, char* old_title, manga_record* manga){
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
  //Update secondary keys
  update_secondary_keys(mangas->secondary_keys, old_title, manga->title, old_isbn, manga->isbn);
  return 0;
}

int remove_manga(manga_file* mangas, manga_record* manga){
  primary_index_entry* entry;
  if(search_primary_keys(mangas->keys, manga->isbn, &entry) != 0) return -1; //cant remove if the isbn doesnt exists
  mark_record_as_deleted(entry->position, mangas->record_file);
  remove_primary_keys(&mangas->keys, manga->isbn);
  remove_secondary_keys(mangas->secondary_keys, manga->title, manga->isbn);
  return 0;
}
