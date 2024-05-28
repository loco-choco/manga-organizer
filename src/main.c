#include <ncurses.h>
#include <stdlib.h>

#include "record-entry.h"
#include "primary-index-entry.h"

int main(int argc, char ** argv)
{
    /*
    // init screen and sets up screen
    initscr();

    // print to screen
    printw("Hello World");

    // refreshes the screen
    refresh();

    // pause the screen output
    getch();

    // deallocates memory and ends ncurses
    endwin();
    */

    FILE *file_pointer = fopen("manga-db.mango", "w");
    manga_record* manga1 = calloc(1, sizeof(*manga1));

    manga1->isbn = "1";
    manga1->title = "A";
    manga1->authors = "Eu";
    manga1->genre = "Todos";
    manga1->magazine = "nenhuma";
    manga1->publisher = "eu, denovo";
    manga1->volumes_amount = 2;
    manga1->volumes = "\x01";
    record_size(manga1, &manga1->original_size);
    long position;
    write_new_record(manga1, file_pointer, &position);
    write_new_record(manga1, file_pointer, &position);
    write_new_record(manga1, file_pointer, &position);
    write_new_record(manga1, file_pointer, &position);
    manga1->title = "B";
    update_record(position, manga1, file_pointer);
    fclose(file_pointer);
        

    manga_record* manga2;
    file_pointer = fopen("manga-db.mango", "r");
    
    read_record(file_pointer, &manga2);
    print_record(manga2);
    
    free_record_entry(manga2);
    
    read_record(file_pointer, &manga2);
    print_record(manga2);
    
    free_record_entry(manga2);
    
    read_record(file_pointer, &manga2);
    print_record(manga2);
    
    free_record_entry(manga2);
    
    read_record(file_pointer, &manga2);
    
    print_record(manga2);
    free_record_entry(manga2);
    
    primary_index_list* keys;
    create_primary_keys_from_record_file(file_pointer, &keys);

    FILE *keys_file_pointer = fopen("manga-keys.keys", "w");

    write_primary_keys_file(keys, keys_file_pointer);


    fclose(file_pointer);

    return 0;
}

