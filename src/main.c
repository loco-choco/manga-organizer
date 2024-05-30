#include <ncurses.h>
#include <stdlib.h>

#include "manga-file.h"
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
  manga_file* mangas;
  manga_record *manga_in, *manga_out;
  printf("Reading the files...\n");
  open_manga_file("manga-db.mango", "manga-keys.keys", &mangas);
  printf("Files read!\n");
  if(search_manga_isbn(mangas, "1", &manga_in) == 0){
    printf("Found manga with isbn %s\n", manga_in->isbn);
    free_record_entry(manga_in);
  }
  manga_out = calloc(1, sizeof(*manga_out));
  manga_out->isbn = "1";
  manga_out->title = "A";
  manga_out->authors = "Eu";
  manga_out->genre = "Todos";
  manga_out->magazine = "nenhuma";
  manga_out->publisher = "eu, denovo";
  manga_out->volumes_amount = 2;
  manga_out->volumes = "\x01";

  if(add_new_manga(mangas, manga_out) == -1){
    printf("A manga with isbn %s already exists!\n", manga_out->isbn);
  }

  close_manga_file(mangas);

  return 0;
}

