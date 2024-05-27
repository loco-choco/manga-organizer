#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "string-helpers.h"
#include "record-entry.h"

int write_record(manga_record* record, FILE* file_pointer){
  write_string(record->isbn, file_pointer);
  write_string(record->title, file_pointer);
  write_string(record->authors, file_pointer);
  write_string(record->genre, file_pointer);
  write_string(record->magazine, file_pointer);
  write_string(record->publisher, file_pointer);
  fwrite(&record->start_year, sizeof(int), 1, file_pointer);
  fwrite(&record->end_year, sizeof(int), 1, file_pointer);
  fwrite(&record->edition_year, sizeof(int), 1, file_pointer);
  fwrite(&record->volumes_amount, sizeof(int), 1, file_pointer);
  fwrite(record->volumes, sizeof(int), record->volumes_amount, file_pointer);
  return 0;
}

int read_record(FILE* file_pointer, manga_record** record){
  *record = malloc(sizeof(**record));
  read_string(file_pointer, &(*record)->isbn);
  read_string(file_pointer, &(*record)->title);
  read_string(file_pointer, &(*record)->authors);
  read_string(file_pointer, &(*record)->genre);
  read_string(file_pointer, &(*record)->magazine);
  read_string(file_pointer, &(*record)->publisher);
  fread(&(*record)->start_year, sizeof(int), 1, file_pointer);
  fread(&(*record)->end_year, sizeof(int), 1, file_pointer);
  fread(&(*record)->edition_year, sizeof(int), 1, file_pointer);
  fread(&(*record)->volumes_amount, sizeof(int), 1, file_pointer);
  (*record)->volumes = malloc(sizeof(int) * (*record)->volumes_amount);
  fread((*record)->volumes, sizeof(int), (*record)->volumes_amount, file_pointer);
  return 0;
}

int read_record_in_position(FILE* file_pointer, long position, manga_record** record){
  fseek(file_pointer, position, SEEK_SET);
  return read_record(file_pointer, record);
}

int print_record(manga_record* record){
  printf("isbn=%s\n", record->isbn);
  printf("title=%s\n", record->title);
  printf("authors=%s\n", record->authors);
  printf("genre=%s\n", record->genre);
  printf("magazine=%s\n", record->magazine);
  printf("publisher=%s\n", record->publisher);
  printf("start_year=%d\n", record->start_year);
  printf("end_year=%d\n", record->end_year);
  printf("edition_year=%d\n", record->edition_year);
  printf("volumes_year=[");
  for(int i = 0; i < record->volumes_amount; i++) printf(" %d", record->volumes[i]);
  printf("]\n");
  return 0;
}
