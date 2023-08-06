/**
 * @file main.c
 * @brief Interactive text-based adventure game.
 *
 * This file contains the implementation for a simple interactive fiction game.
 * The game presents the user with a series of passages from a JSON file.
 *
 * @license MIT License
 * @author Isaak
 * @date 2023-08-06
 */

#include <jansson.h>
#include <ncurses.h>
#include <string.h>
#include <menu.h>
#include <mimalloc.h>
#include <locale.h>

typedef struct {
  char *Option;
  char *Target;
} Link;

typedef struct {
  char *PassageName;
  char *Content;
  size_t numLinks;
  Link **Links;
} Passage;

typedef struct {
  size_t numPassages;
  Passage **Passages;
} GameState;

GameState *loadGame(const char *Filename) {
  json_t *Root;
  json_error_t Error;
  Root = json_load_file(Filename, 0, &Error);
  if(!Root) {
	return NULL;
  }
  if(!json_is_array(Root)) {
	return NULL;
  }
  GameState *GameState = mi_malloc(sizeof(*GameState));
  GameState->numPassages = (int)json_array_size(Root);
  GameState->Passages = mi_malloc(GameState->numPassages * sizeof(Passage *));
  for(size_t I = 0; I < GameState->numPassages; I++) {
	json_t *JsonPassage = json_array_get(Root, I);
	Passage *Passage = mi_malloc(sizeof(*Passage));
	Passage->PassageName = strdup(json_string_value(json_object_get(JsonPassage, "PassageName")));
	Passage->Content = strdup(json_string_value(json_object_get(JsonPassage, "Content")));
	json_t *JsonLinks = json_object_get(JsonPassage, "Links");
	Passage->numLinks = json_array_size(JsonLinks);
	Passage->Links = mi_malloc(Passage->numLinks * sizeof(Link *));
	for(size_t J = 0; J < Passage->numLinks; J++) {
	  json_t *JsonLink = json_array_get(JsonLinks, J);
	  Link *Link = mi_malloc(sizeof(*Link));
	  Link->Option = strdup(json_string_value(json_object_get(JsonLink, "Option")));
	  Link->Target = strdup(json_string_value(json_object_get(JsonLink, "Target")));
	  Passage->Links[J] = Link;
	}
	GameState->Passages[I] = Passage;
  }
  json_decref(Root);
  return GameState;
}

void printCentered(WINDOW *Win, int Starty, int Startx, int Width, const char *String) {
  size_t Length = strlen(String);
  int X = Startx + (Width - (int)Length) / 2;
  mvwprintw(Win, Starty, X, "%s", String);
  refresh();
}

#define MAX_LINE_LENGTH 50

void displayGameState(const Passage *P) {
  clear();
  const char *Start = P->Content;
  const char *End;
  int Y = LINES / 10;
  int X = COLS / 2 - MAX_LINE_LENGTH / 2;

  while(*Start != '\0') {
	if(strlen(Start) > MAX_LINE_LENGTH) {
	  End = Start + MAX_LINE_LENGTH;
	  while(*End != ' ' && End != Start) {
		--End;
	  }
	  if(End == Start) {
		End = Start + MAX_LINE_LENGTH;
	  }
	} else {
	  End = Start + strlen(Start);
	}

	char Line[MAX_LINE_LENGTH + 1];
	memcpy(Line, Start, End - Start);
	Line[End - Start] = '\0';
	mvwprintw(stdscr, Y, X, "%s", Line);

	++Y;
	Start = End + 1;
  }

  refresh();
}

void freeGameState(GameState *State) {
  if (!State) return;
  for (size_t I = 0; I < State->numPassages; I++) {
	Passage *Passage = State->Passages[I];
	for (size_t J = 0; J < Passage->numLinks; J++) {
	  Link *Link = Passage->Links[J];
	  mi_free(Link->Option);
	  mi_free(Link->Target);
	  mi_free(Link);
	}
	mi_free(Passage->Links);
	mi_free(Passage->PassageName);
	mi_free(Passage->Content);
	mi_free(Passage);
  }
  mi_free(State->Passages);
  mi_free(State);
}

void displayEscapeMenu(void) {
  ITEM **MyItems;
  int C;
  MENU *MyMenu;
  int NChoices, I;
  ITEM *CurItem;
  char *Choices[] = {
	  "Continue",
	  "Exit",
	  (char *)NULL,
  };
  NChoices = sizeof(Choices) / sizeof(Choices[0]) - 1;
  MyItems = (ITEM **)calloc(NChoices + 1, sizeof(ITEM *));
  for (I = 0; I < NChoices; ++I) {
	MyItems[I] = new_item(Choices[I], "");
  }
  MyItems[NChoices] = (ITEM *)NULL;
  MyMenu = new_menu((ITEM **)MyItems);
  int Startx, Starty, Width, Height;
  Height = 8;
  Width = 50;
  Starty = (LINES - Height) / 2;
  Startx = (COLS - Width) / 2;
  WINDOW *MenuWin = newwin(Height, Width, Starty, Startx);
  box(MenuWin, 0, 0);
  mvwprintw(MenuWin, 2, (Width - (int)strlen("Escape Menu")) / 2, "Escape Menu");
  set_menu_win(MyMenu, MenuWin);
  set_menu_sub(MyMenu, derwin(MenuWin, 4, 38, 4, 6));
  set_menu_mark(MyMenu, " > ");
  post_menu(MyMenu);
  wrefresh(MenuWin);
  keypad(MenuWin, TRUE);
  while ((C = wgetch(MenuWin)) != KEY_F(1)) {
	switch (C) {
	case KEY_DOWN:
	  menu_driver(MyMenu, REQ_DOWN_ITEM);
	  break;
	case KEY_UP:
	  menu_driver(MyMenu, REQ_UP_ITEM);
	  break;
	case 27:
	  unpost_menu(MyMenu);
	  free_menu(MyMenu);
	  for (I = 0; I < NChoices; ++I) {
		free_item(MyItems[I]);
	  }
	  delwin(MenuWin);
	  return;
	case 10:
	  CurItem = current_item(MyMenu);
	  if (strcmp(item_name(CurItem), "Exit") == 0) {
		endwin();
		exit(0);
	  } else {
		unpost_menu(MyMenu);
		free_menu(MyMenu);
		for (I = 0; I < NChoices; ++I) {
		  free_item(MyItems[I]);
		}
		delwin(MenuWin);
		return;
	  }
	default:
	  break;
	}
	wrefresh(MenuWin);
  }
  unpost_menu(MyMenu);
  free_menu(MyMenu);
  for (I = 0; I < NChoices; ++I) {
	free_item(MyItems[I]);
  }
  mi_free(MyItems);
  delwin(MenuWin);
}

Passage* findPassageByName(const GameState *GameState, const char *Name) {
  for (size_t I = 0; I < GameState->numPassages; ++I) {
	if (strcmp(GameState->Passages[I]->PassageName, Name) == 0) {
	  return GameState->Passages[I];
	}
  }
  return NULL;
}

size_t handleUserInput(Passage *CurrentPassage, WINDOW *MenuWin) {
  int Ch;
  size_t I = 0;
  size_t MaxLength = 0;

  if(CurrentPassage->numLinks > 0) {
	for(size_t J = 0; J < CurrentPassage->numLinks; J++) {
	  size_t Length = strlen(CurrentPassage->Links[J]->Option);
	  if(Length > MaxLength) {
		MaxLength = Length;
	  }
	}

	int Offset = (COLS - (int)MaxLength) / 2;

	wattron(MenuWin, A_STANDOUT);
	mvwprintw(MenuWin, 2, Offset, "%s", CurrentPassage->Links[0]->Option);
	wattroff(MenuWin, A_STANDOUT);
	for(size_t J = 1; J < CurrentPassage->numLinks; J++) {
	  mvwprintw(MenuWin, 2 + (int)J, Offset, "%s", CurrentPassage->Links[J]->Option);
	}
	wrefresh(MenuWin);
	while((Ch = wgetch(MenuWin)) != '\n') {
	  if(Ch == KEY_DOWN && I < CurrentPassage->numLinks - 1) {
		I++;
	  } else if(Ch == KEY_UP && I > 0) {
		I--;
	  } else if (Ch == 27) {
		displayEscapeMenu();
		clear();
		displayGameState(CurrentPassage);
		for(size_t J = 0; J < CurrentPassage->numLinks; J++) {
		  if(J == I) {
			wattron(MenuWin, A_STANDOUT);
			mvwprintw(MenuWin, 2 + (int)J, Offset, "%s", CurrentPassage->Links[J]->Option);
			wattroff(MenuWin, A_STANDOUT);
		  } else {
			mvwprintw(MenuWin, 2 + (int)J, Offset, "%s", CurrentPassage->Links[J]->Option);
		  }
		}
	  }
	  for(size_t J = 0; J < CurrentPassage->numLinks; J++) {
		if(J == I) {
		  wattron(MenuWin, A_STANDOUT);
		  mvwprintw(MenuWin, 2 + (int)J, Offset, "%s", CurrentPassage->Links[J]->Option);
		  wattroff(MenuWin, A_STANDOUT);
		} else {
		  mvwprintw(MenuWin, 2 + (int)J, Offset, "%s", CurrentPassage->Links[J]->Option);
		}
	  }
	}
  } else {
	printCentered(MenuWin, 2, 0, COLS, "End Game");
	wrefresh(MenuWin);
	wgetch(MenuWin);
  }
  return I;
}

int main(int Argc, char **Argv) {
  setenv("ESCDELAY", "1", 1);
  setlocale(LC_ALL, "");
  if(Argc < 2) {
	printf("Usage: %s <Game JSON>\n", Argv[0]);
	return 1;
  }
  GameState *GameState = loadGame(Argv[1]);
  if(GameState == NULL) {
	printf("Failed to load game from %s\n", Argv[1]);
	return 1;
  }
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  Passage *CurrentPassage = GameState->Passages[0];
  while(1) {
	displayGameState(CurrentPassage);
	WINDOW *MenuWindow = newwin((int)(CurrentPassage->numLinks + 4), COLS, LINES / 2 + 2, 0);
	keypad(MenuWindow, TRUE);

	size_t Choice = handleUserInput(CurrentPassage, MenuWindow);
	delwin(MenuWindow);

	CurrentPassage = findPassageByName(GameState, CurrentPassage->Links[Choice]->Target);
	if (CurrentPassage == NULL) {
	  break;
	}
  }
  freeGameState(GameState);
  endwin();
  return 0;
}
