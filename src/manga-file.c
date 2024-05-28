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

}

int add_new_manga(manga_file* mangas, manga_record* manga){

}

int update_manga(manga_file* mangas, manga_record* manga){

}

int remove_manga(manga_file* mangas, manga_record* manga){

}
