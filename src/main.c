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
int load_record(char* db_name, manga_file** mangas);
int main_menu(int win_w, int win_h);
int entry_search(int win_w, int win_h, manga_record** manga);
int entry_visualization(int win_w, int win_h, manga_record* manga);
int entry_editing(int win_w, int win_h, manga_record* manga);

int main(int argc, char ** argv)
{
  if(argc < 2){
    printf("DB name missing!\n");
    exit(-1);
  }
  char* db_name = argv[1];
  manga_file* mangas;
  load_record(db_name, &mangas);

  //Initialize ncurses
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  
  int run = 1;
  int operation, result;
  manga_record* manga;
  while(run){
    operation = main_menu(COLS, LINES);
    if(operation == -1){
      run = 0;
      continue;
    }
    if(operation == 0){
      result = entry_search(COLS, LINES, &manga);
      if(result == -1) //return to menu
        continue;
      result = entry_visualization(COLS, LINES, manga);
      if(result == 0) //doesnt want to edit, return to menu
        continue;
      if(result == 1){
        char* isbn = malloc(sizeof(char) * (strlen(manga->isbn) + 1));
        strcpy(isbn, manga->isbn);
        result = entry_editing(COLS, LINES, manga);
        if(result == 0){
          if(strcmp(isbn, manga->isbn) == 0)
            update_manga(mangas, manga);
          else
            update_manga_isbn(mangas, isbn, manga);
        }
        free(isbn);
      }
      else{ //result == 2, remove entry
        remove_manga(mangas, manga);
      }
      free_record_entry(manga);
    }
    else{ //create new entry
      manga = malloc(sizeof(*manga));
      manga->isbn = "";
      manga->title = "";
      manga->authors = "";
      manga->genre = "";
      manga->magazine = "";
      manga->publisher = "";
      manga->volumes_amount = 0;
      manga->volumes = "";

      result = entry_editing(COLS, LINES, manga);
      if(result == 0)
        add_new_manga(mangas, manga);

      free_record_entry(manga);
    }
  }
  endwin();
  close_manga_file(mangas);
}

int load_record(char* db_name, manga_file** mangas){
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

  open_manga_file(record_file_name, primary_keys_file_name, mangas);
  free(record_file_name);
  free(primary_keys_file_name);
  return 0;
}
int entry_search(int win_w, int win_h, manga_record** manga){
  //TODO
  //Pesquisar com ISBN a entrada
  //-1 se quiser cancelar e voltar para o menu
}
int entry_visualization(int win_w, int win_h, manga_record* manga){

  //TODO
  //Apenas printar o manga
  //E ter opcoes para editar (1) e remover (2) a entrada
  //0 se ele quiser sair e voltar para o menu
}
int entry_editing(int win_w, int win_h, manga_record* manga){
  //TODO
  //Editar a entrada
  //Retornar 0 se o usuario confirmar, -1 se ele cancelar
}
//modes
//0 - search for entry
//1 - add entry
//-1 - exit/save
#define MAIN_MENU_OPTIONS 2
int main_menu(int win_w, int win_h){
  int chosen_mode = 0;
  int H,W;
  W = 45;
  H = 8;

  const char main_menu_title[] = "Mangas Record Visualizer";

  ITEM* options[MAIN_MENU_OPTIONS + 1];
  MENU *menu;
  WINDOW *menu_win;
  
  options[0] = new_item("Search Manga", "Search Manga By ISBN");
  options[1] = new_item("Add Manga", "Add Manga To Record");
  options[2] = NULL;
  
  menu = new_menu(options);

  menu_win = newwin(H, W, win_h/2 - H/2, win_w/2 - W/2);
  keypad(menu_win, TRUE);

  set_menu_win(menu, menu_win);
  set_menu_sub(menu, derwin(menu_win, H-4, W-2, 3, 1));

  set_menu_mark(menu, " * ");

  box(menu_win, 0, 0);
  print_in_middle(menu_win, 1, 0, W, main_menu_title, COLOR_PAIR (1));
  mvwaddch(menu_win, win_h/2 - 5 - 6, 0, ACS_LTEE);
  mvwhline(menu_win, win_h/2 - 5 - 6, 1, ACS_HLINE, W-2);
  mvwaddch(menu_win, win_h/2 - 5 - 6, W-1, ACS_RTEE);
  mvprintw(LINES - 2, 0, "Press F1 to exit");
  refresh();

  post_menu(menu);
  wrefresh(menu_win);
  int c;
  do{
    c = wgetch(menu_win);
    switch(c){
      case KEY_DOWN:
        menu_driver(menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(menu, REQ_UP_ITEM);
        break;
    }
    wrefresh(menu_win);
  } while(c != KEY_F(1) && c != '\n');

  if(c == '\n'){
    ITEM *current_option;
	  current_option = current_item(menu);
	  char* mode = (char *)item_name(current_option);
    if(strcmp("Search Manga", mode) == 0) chosen_mode = 0;
    else if(strcmp("Add Manga", mode) == 0) chosen_mode = 1;
  }
  else
    chosen_mode = -1; //Exit program
	
  pos_menu_cursor(menu);

  // Desmarca e libera toda a memória ocupada
  unpost_menu(menu);
  free_menu(menu);

  for(int i = 0; i < MAIN_MENU_OPTIONS; ++i){
    free_item(options[i]);
  }

  return chosen_mode;
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

