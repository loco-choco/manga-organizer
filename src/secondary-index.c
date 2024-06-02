#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "secondary-index-entry.h"
#include "string-helpers.h"
#include "record-entry.h"


// ISBNS FILE
int write_new_secondary_key_isbn_list(char* isbn, long next, FILE* isbns_file_pointer, long* position){
  fseek(isbns_file_pointer, 0, SEEK_END);
  *position = ftell(isbns_file_pointer);
  write_string(isbn, isbns_file_pointer);
  fwrite(&next, sizeof(long), 1, isbns_file_pointer);
  fflush(isbns_file_pointer);
  return 0;
}

int read_secondary_key_isbn_list(FILE* isbns_file_pointer, long position, char** isbn, long* next){
  fseek(isbns_file_pointer, position, SEEK_SET);
  read_string(isbns_file_pointer, isbn);
  fread(next, sizeof(long), 1, isbns_file_pointer);
  return 0;
}

int update_secondary_key_title_list(long position, long new_next, FILE* isbns_file_pointer){
  fseek(isbns_file_pointer, position, SEEK_SET);
  char string_end;
  do {
  string_end = fgetc(isbns_file_pointer);
  } while(string_end == STRING_END);

  fwrite(&new_next, sizeof(long), 1, isbns_file_pointer);
  fflush(isbns_file_pointer);
  return 0;
}

#define ENTRY_START '>'

//TITLES FILE
int write_secondary_key(secondary_index_entry* entry, FILE* titles_file_pointer){
  write_string(entry->title, titles_file_pointer);
  fwrite(&entry->first_position_of_isbn_list, sizeof(long), 1, titles_file_pointer);
  return 0;
}

int read_secondary_key(FILE* titles_file_pointer, secondary_index_entry** entry){
  *entry = malloc(sizeof(**entry));
  read_string(titles_file_pointer, &(*entry)->title);
  fread(&(*entry)->first_position_of_isbn_list, sizeof(long), 1, titles_file_pointer);
  return 0;
}

int write_all_secondary_keys(FILE* titles_file_pointer, secondary_index_list* secondary_keys){
  secondary_index_list* current = secondary_keys;
  while(current != NULL){
    fputc(ENTRY_START, titles_file_pointer);
    write_secondary_key(current->entry, titles_file_pointer);
    current = current->next;
  }
  return 0;
}
int read_all_secondary_keys(FILE* titles_file_pointer, secondary_index_list** secondary_keys){
 *secondary_keys = NULL;
 secondary_index_list* current = NULL;
 char entry_start = fgetc(titles_file_pointer);
 while(entry_start != EOF){
  secondary_index_entry* new_entry;
  read_secondary_key(titles_file_pointer, &new_entry);

  secondary_index_list* new_current = calloc(1, sizeof(*new_current));
  new_current->entry = new_entry;

  if(current == NULL){
    *secondary_keys = new_current;
  }
  else{
    current->next = new_current;
  }

  current = new_current;
 }
 return 0;
}
int create_secondary_keys_from_record_file(FILE* file_pointer, secondary_index_file* secondary_keys_file){
  int current_char;
  current_char = fgetc(file_pointer);
  while(current_char != EOF){
    ungetc(current_char, file_pointer);

    manga_record* manga;
    read_record(file_pointer, &manga);

    if(manga->isbn[0] == DELETED_RECORD){
      free_record_entry(manga);
      continue;
    }
    sorted_insert_secondary_keys(secondary_keys_file, manga->title, manga->isbn);
    free_record_entry(manga);
    current_char = fgetc(file_pointer);
  }
  return 0;
}

int free_secondary_index(secondary_index_entry* entry){
  if(entry->title != NULL) free(entry->title);
  free(entry);
  return 0;
}
int free_secondary_index_list(secondary_index_list* secondary_keys){
  secondary_index_list* current = secondary_keys;
  while(current != NULL){
    free_secondary_index(current->entry);
    secondary_index_list* next = current->next;
    free(current);
    current = next;
  }
  return 0;
}
//SECONDARY INDEX INTERFACE

int sorted_insert_secondary_keys(secondary_index_file* secondary_keys_file, char* title, char* isbn){
  long position;
  secondary_index_list* current, *new_entry;
  current = secondary_keys->index_list;

  //title in the start
  if(current == NULL || strcmp(title, current->entry->title) <= 0){
    //title is the same
    if(strcmp(title, current->entry->title) == 0){
      write_new_secondary_key_isbn_list(isbn, current->entry->first_position_of_isbn_list, secondary_keys->isbns_file_pointer, &position);
      current->entry->first_position_of_isbn_list = position;
      return 0;
    }
    //title isnt the same
    write_new_secondary_key_isbn_list(isbn, -1, secondary_keys->isbns_file_pointer, &position);

    new_entry = malloc(sizeof(*new_entry));
    new_entry->entry->first_position_of_isbn_list = position; 
    new_entry->entry->title = malloc(sizeof(char) * (strlen(title) + 1));
    strcpy(new_entry->entry->title, title);
    new_entry->next = current;
    secondary_keys->index_list = new_entry;
    return 0;
  }
 
  while(current->next != NULL && strcmp(title, current->next->entry->title) > 0){
    current = current->next;
  }

  // add isbn to existing title
  if(strcmp(title, current->next->entry->title) == 0){
    write_new_secondary_key_isbn_list(isbn, current->next->entry->first_position_of_isbn_list, secondary_keys->isbns_file_pointer, &position);
    current->next->entry->first_position_of_isbn_list = position;
    return 0;
  }

  //new title in middle or end
  write_new_secondary_key_isbn_list(isbn, -1, secondary_keys->isbns_file_pointer, &position);

  new_entry = malloc(sizeof(*new_entry));
  new_entry->entry->first_position_of_isbn_list = position; 
  new_entry->entry->title = malloc(sizeof(char) * (strlen(title) + 1));
  strcpy(new_entry->entry->title, title);
  
  new_entry->next = current->next;
  current->next = new_entry;
  return 0;
}

int remove_secondary_keys(secondary_index_file* secondary_index_file, char* title, char* isbn){
  secondary_index_list* current = secondary_index_file->index_list;
  while(current != NULL && strcmp(title, current->entry->title) > 0){
    current = current->next;
  }

  if(current == NULL || strcmp(title, current->entry->title) != 0) 
    return -1; //said title doesnt exist
  
  int found_isbn = 0;
  long current_position = current->entry->first_position_of_isbn_list;
  long previous_position, next_position;
  char* read_isbn;
   
  while(current_position >= 0 && found_isbn == 0){
    previous_position = current_position;
    read_secondary_key_isbn_list(secondary_index_file->isbns_file_pointer, current_position, &read_isbn, &next_position);

    if(strcmp(read_isbn, isbn) == 0)
      found_isbn = 1;  
    else
      current_position = next_position;
  }

  if(found_isbn == 0) return -2; //couldnt find isbn in this title

  //mark as removed (-2)
  update_secondary_key_title_list(current_position, -2, secondary_index_file->isbns_file_pointer);
  //if it was at the start, point to the next
  if(previous_position == current_position){
    current->entry->first_position_of_isbn_list = next_position;
  } else { //if not, move the linked array
    update_secondary_key_title_list(previous_position, next_position, secondary_index_file->isbns_file_pointer);
  }
  return 0;
}

int move_secondary_keys(secondary_index_file* secondary_index_file, char* old_title, char* title, char* isbn){
  secondary_index_list* current = secondary_index_file->index_list;
  while(current != NULL && strcmp(old_title, current->entry->title) > 0){
    current = current->next;
  }

  if(current == NULL || strcmp(old_title, current->entry->title) != 0) 
    return -1; //said title doesnt exist
  
  int found_isbn = 0;
  long current_position = current->entry->first_position_of_isbn_list;
  long previous_position, next_position;
  char* read_isbn;
   
  while(current_position >= 0 && found_isbn == 0){
    previous_position = current_position;
    read_secondary_key_isbn_list(secondary_index_file->isbns_file_pointer, current_position, &read_isbn, &next_position);

    if(strcmp(read_isbn, isbn) == 0)
      found_isbn = 1;  
    else
      current_position = next_position;
  }

  if(found_isbn == 0) return -2; //couldnt find isbn in this title

  secondary_index_list *new_entry = secondary_index_file->index_list;
  while(new_entry != NULL && strcmp(title, new_entry->entry->title) > 0){
    new_entry = new_entry->next;
  }

  if(new_entry == NULL || strcmp(title, new_entry->entry->title) != 0) 
    return -3; //no title to move to

  //Adding new isbn to title
  update_secondary_key_title_list(current_position, new_entry->entry->first_position_of_isbn_list, secondary_index_file->isbns_file_pointer);
  new_entry->entry->first_position_of_isbn_list = current_position;

  //Removing old isbn from title
  //if it was at the start, point to the next
  if(previous_position == current_position){
    current->entry->first_position_of_isbn_list = next_position;
  } else { //if not, move the linked array
    update_secondary_key_title_list(previous_position, next_position, secondary_index_file->isbns_file_pointer);
  }
  return 0;
}

int update_secondary_keys(secondary_index_file* secondary_index_file, char* old_title, char* title, char* old_isbn, char* isbn){
  //if(old_isbn != isbn) -> remove from old title and add to new title
  //else if(old_title != title) -> remove from old title and add to new title but dont mark as removed (move from one title to another)
  //else -> do nothing
  if(strcmp(old_isbn, isbn) != 0){
    remove_secondary_keys(secondary_index_file, old_title, old_isbn);
    sorted_insert_secondary_keys(secondary_index_file, title, isbn);
  } else if(strcmp(old_title, title) != 0){
    move_secondary_keys(secondary_index_file, old_title, title, isbn);
  }
  return 0;
}
int search_secondary_keys(secondary_index_file* secondary_index_file, char* title, int* amount_of_isbns, char*** isbns){
  int amount = 0;
  secondary_index_list *current = secondary_index_file->index_list;
  while(current != NULL && strcmp(title, current->entry->title) > 0){
    current = current->next;
  }

  if(current == NULL || strcmp(title, current->entry->title) != 0) 
    return -1; //no title

  char* read_isbn;
  long current_position, next_position;
  current_position = current->entry->first_position_of_isbn_list;
  while(current_position >= 0){
    read_secondary_key_isbn_list(secondary_index_file->isbns_file_pointer, current_position, &read_isbn, &next_position);
    free(read_isbn);    
    amount++;
    current_position = next_position;
  }

  char** _isbns = calloc(amount, sizeof(char*));

  current_position = current->entry->first_position_of_isbn_list;
  while(current_position >= 0){
    read_secondary_key_isbn_list(secondary_index_file->isbns_file_pointer, current_position, &read_isbn, &next_position);
    current_position = next_position;
    amount--;
    _isbns[amount] = read_isbn;
  }
  
  *amount_of_isbns = amount;
  *isbns = _isbns;

  return 0;
}

int close_secondary_keys(secondary_index_file* secondary_index_file){
  write_all_secondary_keys(secondary_index_file->titles_file_pointer, secondary_index_file->index_list);
  
  fclose(secondary_index_file->titles_file_pointer);
  fclose(secondary_index_file->isbns_file_pointer);
  free_secondary_index_list(secondary_index_file->index_list);

  free(secondary_index_file);
  return 0;
}


