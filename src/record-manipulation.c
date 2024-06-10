#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "string-helpers.h"
#include "record-entry.h"


#define END_OF_VOLUMES '\x7f'
#define MAX_VOLUME_AMOUNT 100

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
  fwrite(record->acquired_volumes, sizeof(char), record->acquired_volumes_amount, file_pointer);
  char end_of_record = END_OF_VOLUMES;
  fwrite(&end_of_record, sizeof(char), 1, file_pointer);
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

  int acquired_volumes_amount = 0;
  char* buffer = malloc(MAX_VOLUME_AMOUNT * sizeof(char));
  char volume;
  volume = getc(file_pointer);
  while(volume != END_OF_VOLUMES){
    if(acquired_volumes_amount < MAX_VOLUME_AMOUNT){
      buffer[acquired_volumes_amount] = volume;
      acquired_volumes_amount++;
    }
    volume = getc(file_pointer);
  }
  (*record)->acquired_volumes_amount = acquired_volumes_amount;
  (*record)->acquired_volumes = malloc(sizeof(char) * (*record)->acquired_volumes_amount);
  memcpy((*record)->acquired_volumes, buffer, sizeof(char) * acquired_volumes_amount);
  free(buffer);

  int og_size;
  record_size(*record, &og_size);
  (*record)->original_size = og_size;
  return 0;
}

int write_new_record(manga_record* record, FILE* file_pointer, long* position){
  fseek(file_pointer, 0, SEEK_END);
  *position = ftell(file_pointer);
  write_record(record, file_pointer);
  fflush(file_pointer);
  return 0;
}

int update_record(long position, manga_record* record, FILE* file_pointer){
  int size_of_updated_record;
  record_size(record, &size_of_updated_record);
  if(size_of_updated_record != record->original_size)
    return -1;

  fseek(file_pointer, position, SEEK_SET);
  write_record(record, file_pointer);
  fflush(file_pointer);
  return 0;
}

int mark_record_as_deleted(long position, FILE* file_pointer){
  fseek(file_pointer, position, SEEK_SET);
  char deleted_record = DELETED_RECORD;
  fwrite(&deleted_record, sizeof(char), 1, file_pointer);
  fflush(file_pointer);
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
  printf("volumes_amount=%d\n", record->volumes_amount);
  printf("acquired_volumes=[");
  for(int i = 0; i < record->acquired_volumes_amount; i++) printf(" %d", record->acquired_volumes[i]);
  printf(" ]\n");
  return 0;
}

int record_size(manga_record* record, int* size){
  *size = strlen(record->isbn) * sizeof(char) + sizeof(char) +
    strlen(record->title) * sizeof(char) + sizeof(char) +
    strlen(record->authors) * sizeof(char) + sizeof(char) +
    strlen(record->genre) * sizeof(char) + sizeof(char) +
    strlen(record->magazine) * sizeof(char) + sizeof(char) +
    strlen(record->publisher) * sizeof(char) + sizeof(char) +
    sizeof(int) +
    sizeof(int) +
    sizeof(int) +
    sizeof(int) +
    sizeof(char) * record->volumes_amount +
    sizeof(char);
  return 0;
}

int free_record_entry(manga_record* record){
  if(record->isbn != NULL) free(record->isbn);
  if(record->title != NULL) free(record->title);
  if(record->authors != NULL) free(record->authors);
  if(record->genre != NULL) free(record->genre);
  if(record->magazine != NULL) free(record->magazine);
  if(record->publisher != NULL) free(record->publisher);
  if(record->acquired_volumes != NULL) free(record->acquired_volumes);
  free(record);
  return 0;
}
