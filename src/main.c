#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <panel.h>

#include <stdlib.h>
#include <string.h>

#include "manga-file.h"
#include "record-entry.h"
#include "primary-index-entry.h"

#define MAX_STRING_SIZE 20
#define ISBN_LENGHT 13

void print_in_middle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color);
int load_record(char* db_name, manga_file** mangas);
int main_menu(int win_w, int win_h);
int entry_search(int win_w, int win_h, manga_file* mangas, manga_record** manga);
int title_search(int win_w, int win_h, manga_file* mangas, char** title, int* amount, char*** isbns);
int entry_selector(int win_w, int win_h, manga_file* mangas, char* title, int amount, char**isbns, manga_record** manga);
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
    if(operation == 0 || operation == 1){
      if(operation == 0){ //Direct mode (ISBNS)
        result = entry_search(COLS, LINES, mangas, &manga);
      } else { //Indirect mode (Titles)
        char* title;
        int amount;
        char** isbns;
        result = title_search(COLS, LINES, mangas, &title, &amount, &isbns);
        if(result == -1)
          continue;
        result = entry_selector(COLS, LINES, mangas, title, amount, isbns, &manga);
        free(title);
        for(int i = 0; i < amount; i++) free(isbns[i]);
      }
      if(result == -1) //return to menu
        continue;
      result = entry_visualization(COLS, LINES, manga);
      if(result == 0) //doesnt want to edit, return to menu
        continue;
      if(result == 1){
        char* title = malloc(sizeof(char) * (strlen(manga->title) + 1));
        strcpy(title, manga->title);
        char* isbn = malloc(sizeof(char) * (strlen(manga->isbn) + 1));
        strcpy(isbn, manga->isbn);
        result = entry_editing(COLS, LINES, manga);
        if(result == 0){
          if(strcmp(isbn, manga->isbn) == 0)
            update_manga(mangas, title, manga);
          else
            update_manga_isbn(mangas, isbn, title, manga);
        }
        free(isbn);
        free(title);
      } else { //result == 2, remove entry
        remove_manga(mangas, manga);
      }
      free_record_entry(manga);
    }
    else if(operation == 2){ //create new entry
      manga = calloc(1, sizeof(*manga));
      manga->isbn = calloc(1, sizeof(char));
      manga->title = calloc(1, sizeof(char));
      manga->authors = calloc(1, sizeof(char));
      manga->genre = calloc(1, sizeof(char));
      manga->magazine = calloc(1, sizeof(char));
      manga->publisher = calloc(1, sizeof(char));

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
  const char secondary_keys_isbns_file_sufix[] = "-keys.sikeys";
  const char secondary_keys_titles_file_sufix[] = "-keys.stkeys";

  //Creating the file names -----
  char *secondary_keys_titles_file_name, *secondary_keys_isbns_file_name;
  char *primary_keys_file_name, *record_file_name;
  int db_name_size = strlen(db_name);

  record_file_name = calloc(db_name_size + strlen(record_file_sufix) + 1, sizeof(char));
  memcpy(record_file_name, db_name, db_name_size);
  memcpy(&record_file_name[db_name_size], record_file_sufix, strlen(record_file_sufix));

  primary_keys_file_name = calloc(db_name_size + strlen(primary_keys_file_sufix) + 1, sizeof(char));
  memcpy(primary_keys_file_name, db_name, db_name_size);
  memcpy(&primary_keys_file_name[db_name_size], primary_keys_file_sufix, strlen(primary_keys_file_sufix));

  secondary_keys_isbns_file_name = calloc(db_name_size + strlen(secondary_keys_isbns_file_sufix) + 1, sizeof(char));
  memcpy(secondary_keys_isbns_file_name, db_name, db_name_size);
  memcpy(&secondary_keys_isbns_file_name[db_name_size], secondary_keys_isbns_file_sufix, strlen(secondary_keys_isbns_file_sufix));

  secondary_keys_titles_file_name = calloc(db_name_size + strlen(secondary_keys_titles_file_sufix) + 1, sizeof(char));
  memcpy(secondary_keys_titles_file_name, db_name, db_name_size);
  memcpy(&secondary_keys_titles_file_name[db_name_size], secondary_keys_titles_file_sufix, strlen(secondary_keys_titles_file_sufix));
  // ---------------
  open_manga_file(record_file_name, secondary_keys_isbns_file_name, secondary_keys_titles_file_name, primary_keys_file_name, mangas);

  free(record_file_name);
  free(primary_keys_file_name);
  free(secondary_keys_isbns_file_name);
  free(secondary_keys_titles_file_name);
  return 0;
}
int entry_search(int win_w, int win_h, manga_file* mangas, manga_record** manga){
  clear();
  //Pesquisar com ISBN a entrada
  //-1 se quiser cancelar e voltar para o menu
  FORM  *isbn_form;
  FIELD *isbn_field[2];
  isbn_field[0] = new_field(1, ISBN_LENGHT, win_h/2 - 3, win_w/2 - 15/2 + 15%2, 0, 0);
  isbn_field[1] = NULL;

  set_field_back(isbn_field[0], A_UNDERLINE);
  set_field_type(isbn_field[0], TYPE_REGEXP, "[0-9]{13}$");
	field_opts_off(isbn_field[0], O_AUTOSKIP);

  isbn_form = new_form(isbn_field);
	post_form(isbn_form);
	refresh();
	
	mvprintw(win_h/2 - 3, win_w/2 - 11, "ISBN:");
  mvprintw(LINES - 3, 0, "Press <ENTER> to confirm");
  mvprintw(LINES - 2, 0, "Press F1 to exit");
	refresh();
  int c, run, found_manga;
  found_manga = 0;
  run = 1;
	while(run && (c = getch()) != KEY_F(1))
	{	switch(c)
		{	case KEY_DOWN:
				form_driver(isbn_form, REQ_NEXT_FIELD);
				form_driver(isbn_form, REQ_END_LINE);
				break;
			case KEY_UP:
				form_driver(isbn_form, REQ_PREV_FIELD);
				form_driver(isbn_form, REQ_END_LINE);
				break;
      case KEY_LEFT:
        form_driver(isbn_form, REQ_PREV_CHAR);
        break;
      case KEY_RIGHT:
        form_driver(isbn_form, REQ_NEXT_CHAR);
        break;
      case KEY_BACKSPACE:
				form_driver(isbn_form, REQ_PREV_CHAR);
        form_driver(isbn_form, REQ_DEL_CHAR);
        break;
      case '\n':
      {
        int is_valid = form_driver(isbn_form, REQ_VALIDATION);
        if(is_valid == E_OK){
          char* isbn = field_buffer(isbn_field[0], 0);
          if(search_manga_isbn(mangas, isbn, manga) == 0){
            run = 0;
            found_manga = 1;
          }
          else{
            mvprintw(win_h/2 - 2, win_w/2 - 23, "Manga with ISBN '%s' doesnt exist.", isbn);
				    form_driver(isbn_form, REQ_END_LINE);
          }
        }
      }
        break;
			default:
				form_driver(isbn_form, c);
				break;
		}
	}

	unpost_form(isbn_form);
	free_form(isbn_form);
	free_field(isbn_field[0]);

  if(found_manga == 1) return 0;
  return -1;
}
int title_search(int win_w, int win_h, manga_file* mangas, char** title, int* amount, char*** isbns){
  clear();
  //Pesquisar com titles os isbns
  //-1 se quiser cancelar e voltar para o menu
  FORM  *title_form;
  FIELD *title_field[2];
  title_field[0] = new_field(1, MAX_STRING_SIZE, win_h/2 - 3, win_w/2 - MAX_STRING_SIZE/2 - MAX_STRING_SIZE%2, 0, 0);
  title_field[1] = NULL;

  set_field_back(title_field[0], A_UNDERLINE);
	field_opts_off(title_field[0], O_AUTOSKIP);

  title_form = new_form(title_field);
	post_form(title_form);
	refresh();
	
	mvprintw(win_h/2 - 3, win_w/2 - MAX_STRING_SIZE/2 - MAX_STRING_SIZE%2 - 6, "TITLE:");
  mvprintw(LINES - 3, 0, "Press <ENTER> to confirm");
  mvprintw(LINES - 2, 0, "Press F1 to exit");
	refresh();
  int c, run, found_title;
  found_title = 0;
  run = 1;
	while(run && (c = getch()) != KEY_F(1))
	{	switch(c)
		{	case KEY_DOWN:
				form_driver(title_form, REQ_NEXT_FIELD);
				form_driver(title_form, REQ_END_LINE);
				break;
			case KEY_UP:
				form_driver(title_form, REQ_PREV_FIELD);
				form_driver(title_form, REQ_END_LINE);
				break;
      case KEY_LEFT:
        form_driver(title_form, REQ_PREV_CHAR);
        break;
      case KEY_RIGHT:
        form_driver(title_form, REQ_NEXT_CHAR);
        break;
      case KEY_BACKSPACE:
				form_driver(title_form, REQ_PREV_CHAR);
        form_driver(title_form, REQ_DEL_CHAR);
        break;
      case '\n':
      {
        int is_valid = form_driver(title_form, REQ_VALIDATION);
        if(is_valid == E_OK){
          char* _title = field_buffer(title_field[0], 0);
          int found = search_manga_title(mangas, _title, amount, isbns);
          if(found == 0 && *amount > 0){
            run = 0;
            found_title = 1;
            *title = malloc(sizeof(char) * (strlen(_title) + 1));
            strcpy(*title, _title);
          } else {
            mvprintw(win_h/2 - 2, win_w/2 - MAX_STRING_SIZE/2 - MAX_STRING_SIZE%2 - 7, "Title '%s' wasnt found.", _title);
				    form_driver(title_form, REQ_END_LINE);
          }
        }
      }
        break;
			default:
				form_driver(title_form, c);
				break;
		}
	}

	unpost_form(title_form);
	free_form(title_form);
	free_field(title_field[0]);

  if(found_title == 1) return 0;
  return -1;

}
int entry_selector(int win_w, int win_h, manga_file* mangas, char* title, int amount, char**isbns, manga_record** manga){
  clear();
  int H,W;

  ITEM** options = calloc(amount + 1, sizeof(ITEM*));
  MENU *selection;
  WINDOW *selection_win;
  for(int i = 0; i < amount; i++)
    options[i] = new_item(isbns[i], title);

  W = 45;
  H = 4 + 1 * amount;
  
  selection = new_menu(options);

  selection_win = newwin(H, W, win_h/2 - H/2, win_w/2 - W/2);
  keypad(selection_win, TRUE);

  set_menu_win(selection, selection_win);
  set_menu_sub(selection, derwin(selection_win, H-4, W-2, 3, 1));

  set_menu_mark(selection, " * ");

  box(selection_win, 0, 0);
  print_in_middle(selection_win, 1, 0, W, title, COLOR_PAIR (1));
  mvwaddch(selection_win, win_h/2 - 5 - 6, 0, ACS_LTEE);
  mvwhline(selection_win, win_h/2 - 5 - 6, 1, ACS_HLINE, W-2);
  mvwaddch(selection_win, win_h/2 - 5 - 6, W-1, ACS_RTEE);
  mvprintw(LINES - 2, 0, "Press F1 to exit");
  refresh();

  post_menu(selection);
  wrefresh(selection_win);
  int c;
  int run = 1;
  int found_manga = 0;
  while(run){
    c = wgetch(selection_win);
    switch(c){
      case KEY_DOWN:
        menu_driver(selection, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(selection, REQ_UP_ITEM);
        break;
      case '\n':
        {
            ITEM *current_option;
	          current_option = current_item(selection);
	          char* isbn = (char *)item_name(current_option);
            if(search_manga_isbn(mangas, isbn, manga) == 0){
              run = 0;
              found_manga = 1;
            }
            else{
              mvprintw(win_h/2 + H/2 + 2, win_w/2 + W/2, "Manga with ISBN '%s' not found.", isbn);
            }
        }
        break;
      case KEY_F(1):
        run = 0;
        break;
    }
    wrefresh(selection_win);
  } 
	
  pos_menu_cursor(selection);

  // Desmarca e libera toda a memória ocupada
  unpost_menu(selection);
  free_menu(selection);

  for(int i = 0; i < amount; ++i){
    free_item(options[i]);
  }
  free(options);

  if(found_manga) return 0;
  return -1;
}
int entry_visualization(int win_w, int win_h, manga_record* manga){
  //Apenas printar o manga
  //E ter opcoes para editar (1) e remover (2) a entrada
  //0 se ele quiser sair e voltar para o menu
  clear();
  mvprintw(win_h/2 - 10, win_w/2 - 20, "isbn=%s\n", manga->isbn);
  mvprintw(win_h/2 - 9, win_w/2 - 20,"title=%s\n", manga->title);
  mvprintw(win_h/2 - 8, win_w/2 - 20,"authors=%s\n", manga->authors);
  mvprintw(win_h/2 - 7, win_w/2 - 20,"genre=%s\n", manga->genre);
  mvprintw(win_h/2 - 6, win_w/2 - 20,"magazine=%s\n", manga->magazine);
  mvprintw(win_h/2 - 5, win_w/2 - 20,"publisher=%s\n", manga->publisher);
  mvprintw(win_h/2 - 4, win_w/2 - 20,"start_year=%d\n", manga->start_year);
  mvprintw(win_h/2 - 3, win_w/2 - 20,"end_year=%d\n", manga->end_year);
  mvprintw(win_h/2 - 2, win_w/2 - 20,"edition_year=%d\n", manga->edition_year);
  mvprintw(win_h/2 - 1, win_w/2 - 20,"volumes=[");
  for(int i = 0; i < manga->volumes_amount; i++) printw(" %d", manga->volumes[i]);
  printw(" ]\n");
  
  mvprintw(LINES - 4, 0, "Press e to edit entry");
  mvprintw(LINES - 3, 0, "Press r to remove entry");
  mvprintw(LINES - 2, 0, "Press F1 to exit");

  int option, c;       
  int run = 1;
  do{
    c = getch();
    switch(c){
      case KEY_F(1):
        option = 0;
        run = 0;
        break;
      case 'e':
        option = 1;
        run = 0;
        break;
      case 'r':
        option = 2;
        run = 0;
        break;
    }
  } while(run);
  return option;
}
int entry_editing(int win_w, int win_h, manga_record* manga){
  //Editar a entrada
  //Retornar 0 se o usuario confirmar, -1 se ele cancelar
  FORM  *editing_form;
  FIELD *editing_field[11];

  //isbn
  editing_field[0] = new_field(1, ISBN_LENGHT, win_h/2 - 5, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[0], A_UNDERLINE);
  set_field_type(editing_field[0], TYPE_REGEXP, "[0-9]{13}$");
	field_opts_off(editing_field[0], O_AUTOSKIP);
  set_field_buffer(editing_field[0], 0, manga->isbn);
  //title
  editing_field[1] = new_field(1, MAX_STRING_SIZE, win_h/2 - 4, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[1], A_UNDERLINE);
	field_opts_off(editing_field[1], O_AUTOSKIP);
  set_field_buffer(editing_field[1], 0, manga->title);
  //authors
  editing_field[2] = new_field(1, MAX_STRING_SIZE, win_h/2 - 3, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[2], A_UNDERLINE);
	field_opts_off(editing_field[2], O_AUTOSKIP);
  set_field_buffer(editing_field[2], 0, manga->authors);
  //genre
  editing_field[3] = new_field(1, MAX_STRING_SIZE, win_h/2 - 2, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[3], A_UNDERLINE);
	field_opts_off(editing_field[3], O_AUTOSKIP);
  set_field_buffer(editing_field[3], 0, manga->genre);
  //magazine
  editing_field[4] = new_field(1, MAX_STRING_SIZE, win_h/2 - 1, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[4], A_UNDERLINE);
	field_opts_off(editing_field[4], O_AUTOSKIP);
  set_field_buffer(editing_field[4], 0, manga->magazine);
  //publisher
  editing_field[5] = new_field(1, MAX_STRING_SIZE, win_h/2, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[5], A_UNDERLINE);
	field_opts_off(editing_field[5], O_AUTOSKIP);
  set_field_buffer(editing_field[5], 0, manga->publisher);
  //start_year
  editing_field[6] = new_field(1, 4, win_h/2 + 1, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[6], A_UNDERLINE);
  set_field_type(editing_field[6], TYPE_INTEGER, 0, 0, 9999);
	field_opts_off(editing_field[6], O_AUTOSKIP);
  char start_year_str[5];
  sprintf(start_year_str, "%d", manga->start_year);
  set_field_buffer(editing_field[6], 0, start_year_str);
  //end_year
  editing_field[7] = new_field(1, 4, win_h/2 + 2, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[7], A_UNDERLINE);
  set_field_type(editing_field[7], TYPE_INTEGER, 0, 0, 9999);
	field_opts_off(editing_field[7], O_AUTOSKIP);
  char end_year_str[5];
  sprintf(end_year_str, "%d", manga->end_year);
  set_field_buffer(editing_field[7], 0, end_year_str);
  //edition_year
  editing_field[8] = new_field(1, 4, win_h/2 + 3, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[8], A_UNDERLINE);
  set_field_type(editing_field[8], TYPE_INTEGER, 0, 0, 9999);
	field_opts_off(editing_field[8], O_AUTOSKIP);
  char edition_year_str[5];
  sprintf(edition_year_str, "%d", manga->edition_year);
  set_field_buffer(editing_field[8], 0, edition_year_str);
  //volumes
  editing_field[9] = new_field(1, MAX_STRING_SIZE*2, win_h/2 + 4, win_w/2 - MAX_STRING_SIZE, 0, 0);
  set_field_back(editing_field[9], A_UNDERLINE);
  set_field_type(editing_field[9], TYPE_REGEXP, "[0-1]{0,1}[0-9]{0,2}( *, *[0-1]{0,1}[0-9]{0,2})* *$");
	field_opts_off(editing_field[9], O_AUTOSKIP);

  char *volumes_str;
  size_t size;
  FILE *volumes_str_stream;

  volumes_str_stream = open_memstream (&volumes_str, &size);
  for(int i = 0; i < manga->volumes_amount; i++){
    if(i > 0) fprintf(volumes_str_stream, ", ");
    fprintf(volumes_str_stream, "%d", manga->volumes[i]);
  }
  fflush (volumes_str_stream);
  fclose (volumes_str_stream);
  
  set_field_buffer(editing_field[9], 0, volumes_str);
 
  editing_field[10] = NULL;

  editing_form = new_form(editing_field);
	post_form(editing_form);
	refresh();


	mvprintw(win_h/2 - 5, win_w/2 - 5 - MAX_STRING_SIZE, "ISBN:");
	mvprintw(win_h/2 - 4, win_w/2 - 6 - MAX_STRING_SIZE, "TITLE:");
	mvprintw(win_h/2 - 3, win_w/2 - 8 - MAX_STRING_SIZE, "AUTHORS:");
	mvprintw(win_h/2 - 2, win_w/2 - 6 - MAX_STRING_SIZE, "GENRE:");
	mvprintw(win_h/2 - 1, win_w/2 - 9 - MAX_STRING_SIZE, "MAGAZINE:");
	mvprintw(win_h/2 - 0, win_w/2 -10 - MAX_STRING_SIZE, "PUBLISHER:");
	mvprintw(win_h/2 + 1, win_w/2 -11 - MAX_STRING_SIZE, "START YEAR:");
	mvprintw(win_h/2 + 2, win_w/2 - 9 - MAX_STRING_SIZE, "END YEAR:");
	mvprintw(win_h/2 + 3, win_w/2 -13 - MAX_STRING_SIZE, "EDITION YEAR:");
	mvprintw(win_h/2 + 4, win_w/2 -11 - MAX_STRING_SIZE, "VOLUMES: [ ");
	mvprintw(win_h/2 + 4, win_w/2 + MAX_STRING_SIZE, " ]");
  
  mvprintw(LINES - 4, 0, "Press <ENTER> to confirm");
  mvprintw(LINES - 3, 0, "Press F2 to save");
  mvprintw(LINES - 2, 0, "Press F1 to exit");
	refresh();
  int c, run, current, is_valid;
  is_valid = 0;
  current = 0;
  run = 1;
	while(run)
	{
    c = getch();
    switch(c){
      case KEY_DOWN:
				form_driver(editing_form, REQ_NEXT_FIELD);
				form_driver(editing_form, REQ_END_LINE);
        current = (current - 1 + 10) % 10;
				break;
			case KEY_UP:
				form_driver(editing_form, REQ_PREV_FIELD);
				form_driver(editing_form, REQ_END_LINE);
        current = (current + 1 + 10) % 10;
				break;
      case KEY_LEFT:
        form_driver(editing_form, REQ_PREV_CHAR);
        break;
      case KEY_RIGHT:
        form_driver(editing_form, REQ_NEXT_CHAR);
        break;
      case KEY_BACKSPACE:
				form_driver(editing_form, REQ_PREV_CHAR);
        form_driver(editing_form, REQ_DEL_CHAR);
        break;
      case '\n':
      {
        form_driver(editing_form, REQ_VALIDATION);
        is_valid = 1;
        for(int i = 0; i < 10; i++){
          int status = field_status(editing_field[i]);
          mvprintw(win_h/2 + (i - 5), win_w/2 + MAX_STRING_SIZE + 6,
          (status? "O": "X"));
          is_valid &= status;
        }
      }
        break;
      case KEY_F(1):
        run = 0;
        is_valid = 0;
        break;
      case KEY_F(2):
        if(is_valid){
          run = 0;
        }
        else{
	        mvprintw(win_h/2 + 5, win_w/2 - MAX_STRING_SIZE, "Entry Cant Be Saved Until Valid");
        }
        break;
			default:
				form_driver(editing_form, c);
				break;
		}
	}

  if(is_valid){
    //isbn
    char* isbn = field_buffer(editing_field[0], 0);
    free(manga->isbn);
    manga->isbn = malloc(sizeof(char) * (strlen(isbn) + 1));
    strcpy(manga->isbn, isbn);
    //title
    char* title = field_buffer(editing_field[1], 0);
    free(manga->title);
    manga->title = malloc(sizeof(char) * (strlen(title) + 1));
    strcpy(manga->title, title);
    //authors
    char* authors = field_buffer(editing_field[2], 0);
    free(manga->authors);
    manga->authors = malloc(sizeof(char) * (strlen(authors) + 1));
    strcpy(manga->authors, authors);
    //genre
    char* genre = field_buffer(editing_field[3], 0);
    free(manga->genre);
    manga->genre = malloc(sizeof(char) * (strlen(genre) + 1));
    strcpy(manga->genre, genre);
    //magazine
    char* magazine = field_buffer(editing_field[4], 0);
    free(manga->magazine);
    manga->magazine = malloc(sizeof(char) * (strlen(magazine) + 1));
    strcpy(manga->magazine, magazine);
    //publisher
    char* publisher = field_buffer(editing_field[5], 0);
    free(manga->publisher);
    manga->publisher = malloc(sizeof(char) * (strlen(publisher) + 1));
    strcpy(manga->publisher, publisher);
    //start year
    char* start_year = field_buffer(editing_field[6], 0);
    manga->start_year = atoi(start_year);
    //end year
    char* end_year = field_buffer(editing_field[7], 0);
    manga->end_year = atoi(end_year);
    //edition year
    char* edition_year = field_buffer(editing_field[8], 0);
    manga->edition_year = atoi(edition_year);
    //volumes
    char* volumes = field_buffer(editing_field[9], 0);
    int str_size = strlen(volumes);
    int* buffer = malloc(sizeof(int) * MAX_STRING_SIZE);
    int amount_of_volumes = 0;
    char* pch = strtok(volumes, " ,");
    while (pch != NULL){
      buffer[amount_of_volumes] = atoi(pch);
      amount_of_volumes++;
      pch = strtok (NULL, " ,");
    }
    free(manga->volumes);
    manga->volumes = malloc(sizeof(char) * amount_of_volumes);
    for(int i = 0; i < amount_of_volumes; i++) 
      manga->volumes[i] = buffer[i];
    manga->volumes_amount = amount_of_volumes;
    
    free(buffer);
    free(volumes_str);
  }


	unpost_form(editing_form);
	free_form(editing_form);
  for(int i = 0; i < 10; i++) free_field(editing_field[i]);
  
  if(is_valid) return 0;
  return -1;
}
//modes
//0 - search for entry
//1 - search by title
//2 - add entry
//-1 - exit/save
#define MAIN_MENU_OPTIONS 3
int main_menu(int win_w, int win_h){
  clear();
  int chosen_mode = 0;
  int H,W;
  W = 45;
  H = 9;

  const char main_menu_title[] = "Mangas Record Visualizer";

  ITEM* options[MAIN_MENU_OPTIONS + 1];
  MENU *menu;
  WINDOW *menu_win;
  
  options[0] = new_item("Search Manga", "Search Manga By ISBN");
  options[1] = new_item("Search Title", "Search Manga By Title");
  options[2] = new_item("Add Manga", "Add Manga To Record");
  options[3] = NULL;
  
  menu = new_menu(options);

  menu_win = newwin(H, W, win_h/2 - H/2, win_w/2 - W/2);
  keypad(menu_win, TRUE);

  set_menu_win(menu, menu_win);
  set_menu_sub(menu, derwin(menu_win, H-4, W-2, 3, 1));

  set_menu_mark(menu, " * ");

  box(menu_win, 0, 0);
  print_in_middle(menu_win, 1, 0, W, main_menu_title, COLOR_PAIR (1));
  mvwaddch(menu_win, win_h/2 - H/2 + 4, 0, ACS_LTEE);
  mvwhline(menu_win, win_h/2 - H/2 + 4, 1, ACS_HLINE, W-2);
  mvwaddch(menu_win, win_h/2 - H/2 + 4, W-1, ACS_RTEE);
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
    else if(strcmp("Search Title", mode) == 0) chosen_mode = 1;
    else if(strcmp("Add Manga", mode) == 0) chosen_mode = 2;
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

