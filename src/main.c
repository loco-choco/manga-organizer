#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <panel.h>

#include <stdlib.h>
#include <string.h>

#include "manga-file.h"
#include "record-entry.h"
#include "primary-index-entry.h"

void print_in_middle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color);

#define MAIN_MENU_OPTIONS 2
int main_menu(int win_w, int win_h){
  int choosen_mode = 0;

  const char main_menu_title[] = "Manga Database";

  ITEM* options[MAIN_MENU_OPTIONS];
  MENU *menu;
  WINDOW *menu_win;
  int c;
  
  options[0] = new_item("New File", "Load New File");
  options[1] = new_item("Existing File", "Load Existing File");
  
  menu = new_menu(options);

  menu_win = newwin(10, 40, win_h/2 - 5, win_w/2 - 20);
  keypad(menu_win, TRUE);

  set_menu_win(menu, menu_win);
  set_menu_sub(menu, derwin(menu_win, 6, 38, 3, 1));

  set_menu_mark(menu, " * ");

  box(menu_win, 0, 0);
  print_in_middle(menu_win, 1, 0, 40, main_menu_title, COLOR_PAIR (1));
  mvwaddch(menu_win, win_h/2 - 5 - 6, 0, ACS_LTEE);
  mvwhline(menu_win, win_h/2 - 5 - 6, 1, ACS_HLINE, 38);
  mvwaddch(menu_win, win_h/2 - 5 - 6, 39, ACS_RTEE);
  mvprintw(LINES - 2, 0, "Press F1 to exit");
  refresh();

  // Posta o menu
  post_menu(menu);
  wrefresh(menu_win);

  do{
    switch(c){
      case KEY_DOWN:
        menu_driver(menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(menu, REQ_UP_ITEM);
        break;
    }
    wrefresh(menu_win);
    c = wgetch(menu_win);
  } while(c != KEY_F(1) && c != '\n');

  if(c == '\n'){
    ITEM *current_option;
	  current_option = current_item(menu);
	  char* mode = (char *)item_name(current_option);
    if(strcmp("Existing File", mode) == 0) choosen_mode = 0;
    else if(strcmp("Existing File", mode) == 0) choosen_mode = 1;
  }
  else
    choosen_mode = -1; //Exit program
	
  pos_menu_cursor(menu);

  // Desmarca e libera toda a memória ocupada
  unpost_menu(menu);
  free_menu(menu);

  for(int i = 0; i < MAIN_MENU_OPTIONS; ++i){
    free_item(options[i]);
  }


  return 0;
}

int database_visualization_options(int win_w, int win_h){
 /* 
  const char main_menu_title[] = "Manga Database";

  ITEM* options[MAIN_MENU_OPTIONS];
  MENU *menu;
  WINDOW *menu_win;
  int c;
  
  options[0] = new_item("New File", "Load New File");
  options[1] = new_item("Existing File", "Load Existing File");
  
  menu = new_menu(options);

  menu_win = newwin(10, 40, win_h/2 - 5, win_w/2 - 20);
  keypad(menu_win, TRUE);

  set_menu_win(menu, menu_win);
  set_menu_sub(menu, derwin(menu_win, 6, 38, 3, 1));

  set_menu_mark(menu, " * ");

  box(menu_win, 0, 0);
  print_in_middle(menu_win, 1, 0, 40, main_menu_title, COLOR_PAIR (1));
  mvwaddch(menu_win, win_h/2 - 5 - 6, 0, ACS_LTEE);
  mvwhline(menu_win, win_h/2 - 5 - 6, 1, ACS_HLINE, 38);
  mvwaddch(menu_win, win_h/2 - 5 - 6, 39, ACS_RTEE);
  mvprintw(LINES - 2, 0, "Press F1 to exit");
  refresh();

  // Posta o menu
  post_menu(menu);
  wrefresh(menu_win);

  do{
    switch(c){
      case KEY_DOWN:
        menu_driver(menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(menu, REQ_UP_ITEM);
        break;
    }
    wrefresh(menu_win);
    c = wgetch(menu_win);
  } while(c != KEY_F(1) && c != '\n');

  if(c == '\n'){
    ITEM *current_option;
	  current_option = current_item(menu);
	  char* mode = (char *)item_name(current_option);
    if(strcmp("Existing File", mode) == 0) choosen_mode = 0;
    else if(strcmp("Existing File", mode) == 0) choosen_mode = 1;
  }
  else
    choosen_mode = -1; //Exit program
	
  pos_menu_cursor(menu);

  // Desmarca e libera toda a memória ocupada
  unpost_menu(menu);
  free_menu(menu);

  for(int i = 0; i < MAIN_MENU_OPTIONS; ++i){
    free_item(options[i]);
  }
*/
  return 0;
}
int main(int argc, char ** argv)
{
  if(argc < 2){
    printf("DB name missing!\n");
    exit(-1);
  }
  char* db_name = argv[1];
  //Open/Create Magas record file
  const char record_file_sufix[] = "-db.mango";
  const char primary_keys_file_sufix[] = "-keys.pkeys";
  char *primary_keys_file_name, *record_file_name;
  int db_name_size = strlen(db_name);
  record_file_name = calloc(db_name_size + strlen(record_file_sufix) + 1, sizeof(char));
  primary_keys_file_name = calloc(db_name_size + strlen(primary_keys_file_sufix) + 1, sizeof(char));
  memcpy(record_file_name, db_name, db_name_size);
  memcpy(&record_file_name[db_name_size], record_file_sufix, strlen(record_file_sufix));
  memcpy(primary_keys_file_name, db_name, db_name_size);
  memcpy(&primary_keys_file_name[db_name_size], primary_keys_file_sufix, strlen(primary_keys_file_sufix));

  manga_file* mangas;
  open_manga_file(record_file_name, primary_keys_file_name, &mangas);
  free(record_file_name);
  free(primary_keys_file_name);

  //Initialize ncurses
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  init_pair(1, COLOR_RED, COLOR_BLACK);
   
  main_menu(COLS, LINES);
  main_menu(COLS, LINES);

  endwin();
  close_manga_file(mangas);
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
    


  manga_file* mangas;
  manga_record *manga_in, *manga_out;
  printf("Reading the files...\n");
  open_manga_file("manga-2-db.mango", "manga-2-keys.keys", &mangas);
  printf("Files read!\n");
  printf("Searching file!\n");
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

  printf("Trying to add to the file!\n");
  if(add_new_manga(mangas, manga_out) == -1){
    printf("A manga with isbn %s already exists!\n", manga_out->isbn);
  }

  printf("Closing file!\n");

  close_manga_file(mangas);

  return 0;
  */
}

void database_viewer(char* mode){
  move(4, 4);
	clrtoeol();
	mvprintw(4, 4, "Item selected is : %s", mode);
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color){
  int length, x, y;
  float temp;

  if(win == NULL){
    win = stdscr;
  }

  getyx(win, y, x);
  if(startx != 0){
    x = startx;
  }

  if(starty != 0){
    y = starty;
  }

  if(width == 0){
    width = 80;
  }

  length = strlen(string);
  temp = (width - length)/ 2;
  x = startx + (int)temp;
  wattron(win, color);
  mvwprintw(win, y, x, "%s", string);
  wattroff(win, color);
  refresh();
}

