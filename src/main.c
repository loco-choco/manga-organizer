#include <ncurses.h>
#include <stdlib.h>

#include "record-entry.h"

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

    write_record(manga1, file_pointer);
    write_record(manga1, file_pointer);
    write_record(manga1, file_pointer);
    write_record(manga1, file_pointer);
    fclose(file_pointer);
    manga_record* manga2;
    file_pointer = fopen("manga-db.mango", "r");
    read_record(file_pointer, &manga2);
    print_record(manga2);
    free(manga2);
    read_record(file_pointer, &manga2);
    print_record(manga2);
    free(manga2);
    read_record(file_pointer, &manga2);
    print_record(manga2);
    free(manga2);
    read_record(file_pointer, &manga2);
    print_record(manga2);
    free(manga2);

    return 0;
}

