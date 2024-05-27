#include <stdio.h>

#ifndef RECORD_ENTRY_H
#define RECORD_ENTRY_H
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
  int* volumes;
} typedef manga_record;

int write_record(manga_record* record, FILE* file_pointer);
int read_record(FILE* file_pointer, manga_record** record);
int read_record_in_position(FILE* file_pointer, long position, manga_record** record);
int print_record(manga_record* record);
#endif