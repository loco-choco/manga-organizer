#include <stdio.h>

#ifndef RECORD_ENTRY_H
#define RECORD_ENTRY_H

#define DELETED_RECORD '\x18'

struct manga_record {
  char* isbn;
  char* title;
  char* authors;
  char* genre;
  char* magazine;
  char* publisher;
  int start_year;
  int end_year;
  int edition_year;
  int volumes_amount;
  int acquired_volumes_amount;
  char* acquired_volumes;
  int original_size;
} typedef manga_record;

int write_record(manga_record* record, FILE* file_pointer);
int read_record(FILE* file_pointer, manga_record** record);
int write_new_record(manga_record* record, FILE* file_pointer, long* position);
int update_record(long position, manga_record* record, FILE* file_pointer); //returns -1 if the record sizes dont match
int mark_record_as_deleted(long position, FILE* file_pointer);
int read_record_in_position(FILE* file_pointer, long position, manga_record** record);
int print_record(manga_record* record);
int record_size(manga_record* record, int* size);
int free_record_entry(manga_record* record);
#endif
