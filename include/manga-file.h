#include <stdio.h>
#include "record-entry.h"
#include "primary-index-entry.h"

#ifndef MANGA_FILE_H
#define MANGA_FILE_H
struct manga_file {
  FILE* record_file;
  FILE* primary_keys_file;
  primary_index_list* keys;
} typedef manga_file;

int open_manga_file(char* record_file_name, char* primary_keys_file_name, manga_file** mangas);
int close_manga_file(manga_file* mangas);
int search_manga_isbn(manga_file* mangas, char* isbn, manga_record** manga);
int add_new_manga(manga_file* mangas, manga_record* manga);
int update_manga(manga_file* mangas, manga_record* manga);
int remove_manga(manga_file* mangas, manga_record* manga);
#endif
