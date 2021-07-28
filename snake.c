////////////////////////////////////////////////////////////////////////////////
///                                  snake.c                                 ///
////////////////////////////////////////////////////////////////////////////////
//
// You are tasked with implementing a simplified version of the timeless snake
// video game. In this version, the game operates as follows:
// 	
// 	1. The snake is initially five blocks long, and is positioned somewhere
// 	   in the center of the game box.
// 	2. The snake does not move unless a directional key is pressed by the
// 	   player (big difference from the original snake game).
// 	3. The snake may not move over its own body. So, at any time, the snake
// 	   should only be able to move in at most three directions (less if near
// 	   a border).
// 	4. It doesn't matter what character you use to represent the snake's
// 	   body.
// 	3. The snake must stay within the boxed window. The player does not
// 	   loose the game by hitting a border of that box though. There is no
// 	   way for the player to loose the game in this version of the game in
// 	   fact.
// 	4. Exactly one cherry is present inside the box at all times, at a 
// 	   random location. The snake can pick it up and grow by one block
// 	   as a result. A new cherry should then by spawned at another location
// 	   (that does not overlap with the snake's body).
// 	5. The player can press 'q' at any time to exit out of the game.
//
// The game should be implemented using the curses library, which is an
// extremely simple yet versatile tool for character window manipulations.
// The template code in this file implements most of the setup and 
// initialization for that library. You will need to scour the library and 
// identify the additional functions you will need to use to implement your
// game. I recommend starting off with a tutorial (e.g. 
// https://invisible-island.net/ncurses/ncurses-intro.html), making sure you
// understand the code already present in this file, before moving to using
// the manual pages for documentation.
//
// As for the game itself, well, everything is up to you. For example, you will
// need to decide how you will implement your snake. It could be an array of 
// coordinate points for example, where each entry stores the row and column
// of each block in the snake's body. However, since the snake should grow each
// time it eats a cherry, maybe a linked list would be better. Regardless of 
// your choice, you should initialize the snake's position inside the game
// initialization function. The game loop skeleton is provided inside of main().
// You will have to implement the code that determines what should happen when
// one of the directional keys is pressed.
//
// Some of the curses functions you may find useful: getch(), addch(), move(),
// addstr(), mvaddch(), mvaddstr().
//


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

/*
 * The game window's dimensions.
 */
#define WIN_H	24
#define WIN_W	80

typedef struct sn_cell{
	int x;
	int y;
	struct sn_cell * next;
	struct sn_cell * prev;
}SNcell;

SNcell * snakeHead = NULL;
int cherry_x, cherry_y;
int num_cherries = 0;
SNcell * get_tail();
void add_to_head();
void move_snake(char);
void free_snake(SNcell *);
void disp();
void new_cherry();
bool snake_legal();

WINDOW *initialize_window(void);
void cleanup(void);


/*
 * Almost all functions from the curses library return either ERR on error, or
 * OK on success. This enables us to create the following short routine for 
 * general error checking. Simply call this right after any such function call.
 */
void check(int status, char *function, int line)
{
	if(status == ERR) {
		cleanup();
		fprintf(stderr, "[ERROR] on line %d: %s() failed.\n", line, function);
		exit(EXIT_FAILURE);
	}
}


/*
 * Initialization and paramterization of the game window is done in this
 * function. initscr() must be called first. Use the manual pages to find out
 * what the other functions do. This function calls refresh().
 */
WINDOW *initialize_window(void)
{
	int status;

	WINDOW *window = initscr();
	if(window == NULL) {
		fprintf(stderr, "[ERROR] on line %d: %s() failed.\n", 28, "initscr");
		exit(EXIT_FAILURE);
	}

	/*
 	 * The window is parametrized below, but feel free to modify and/or
 	 * expand on this.
 	 */

	status = resizeterm(WIN_H, WIN_W);
	check(status, "resizeterm", __LINE__);

	status = cbreak();
	check(status, "cbreak", __LINE__);

	status = noecho();
	check(status, "noecho", __LINE__);

	status = keypad(window, TRUE);
	check(status, "keypad", __LINE__);

	status = refresh();
	check(status, "refresh", __LINE__);

	return window;
}


/*
 * Use this function to clean up any messes before exiting the program, lest
 * the terminal becomes unusable. If you are allocating memory on the heap,
 * you may want to use this function to free it.
 */
void cleanup(void)
{
	free_snake(snakeHead);
	endwin();
}


/*
 * Use this function to initialize the game elements (e.g. draw a box around
 * the window, initialize the snake's position, etc.).
 */
void initialize_game(WINDOW *window)
{
	int status;

	status = box(window, 0, 0);
	check(status, "box", __LINE__);

	/* Your code here. */
	new_cherry();
	add_to_head(WIN_W / 2 - 2, WIN_H / 2);
	add_to_head(WIN_W / 2 - 1, WIN_H / 2);
	add_to_head(WIN_W / 2, WIN_H / 2);
	add_to_head(WIN_W / 2 + 1, WIN_H / 2);
	add_to_head(WIN_W / 2 + 2, WIN_H / 2);
	disp();

	status = refresh();
	check(status, "refresh", __LINE__);
}


int main()
{
	int status;
	(void)status;
	WINDOW *window = initialize_window();
	initialize_game(window);
	
	/*
 	 * The game loop. The player can press 'q' to exit out of it and end
 	 * the game. To keep things simple, you need only worry about 
 	 * implementing the snake's movement using the arrow keys. Of course,
 	 * if you wish, feel free to add some functionality (e.g. a key to reset
 	 * the game, a key to pause the game, etc.).
 	 */
	while(1) {
		if(!snake_legal()){
			break;
		}

		int key = getch();

		if(key == KEY_LEFT) {
			move_snake('l');
		}

		else if(key == KEY_RIGHT) {
			move_snake('r');
		}

		else if(key == KEY_UP) {
			move_snake('u');
		}

		else if(key == KEY_DOWN) {
			move_snake('d');
		}

		else if(key == 'q') {
			break;
		}
		disp();
	}
	cleanup();
	printf("\nGame over!!!\n");
	printf("cherries eaten: %d\n\n", num_cherries);
	return EXIT_SUCCESS;
}

void free_snake(SNcell * head)
{
	if(head->next){	
		free_snake(head->next);
	}
	free(head);
}

SNcell * get_tail()
{
        SNcell * temp = snakeHead;
        while(temp->next){
                temp = temp->next;
        }
        return temp;
}

void add_to_head(int x, int y)
{
	SNcell * newCell = malloc(sizeof(SNcell));
	if(newCell){
		newCell->x = x;
		newCell->y = y;
		newCell->prev = NULL;
		newCell->next = snakeHead;
		if(snakeHead){
			snakeHead->prev = newCell;
		}
		snakeHead = newCell;
	}
}

void new_cherry(){
	cherry_x = (rand() % (WIN_W - 2)) + 1;
	cherry_y = (rand() % (WIN_H - 2)) + 1;
	SNcell * temp = snakeHead;
	while(temp){
		if((temp->x == cherry_x) && (temp->y == cherry_y)){
			new_cherry();
			break;
		}
		temp = temp->next;
	}
}

void move_snake(char dir)
{
	int target_x = -1, target_y = -1;
	switch(dir){
		case 'u': target_x = snakeHead->x;
			  target_y = snakeHead->y - 1;
			  break;
		case 'l': target_x = snakeHead->x - 1;
                          target_y = snakeHead->y;
			  break;
		case 'd': target_x = snakeHead->x;
                          target_y = snakeHead->y + 1;
			  break;
		case 'r': target_x = snakeHead->x + 1;
                          target_y = snakeHead->y;
			  break;
	}
	if((target_x == snakeHead->next->x) && (target_y == snakeHead->next->y)){
		return;
	}
	add_to_head(target_x, target_y);
	if((cherry_x == target_x) && (cherry_y == target_y)){
		num_cherries++;
		new_cherry();
	}
	else{
		SNcell * temp = get_tail();
		//mvaddch(temp->y, temp->x, ' ');
		temp = temp->prev;
		free(temp->next);
		temp->next = NULL;
	}
}

void disp()
{
	int i, j;
        for(i = 1; i < WIN_H - 1; i++){
                for(j = 1; j < WIN_W - 1; j++){
                        mvaddch(i, j, ' ');
                }
        }

	SNcell * temp = snakeHead;
	mvaddch(temp->y, temp->x, '0');
	while(temp->next){
		temp = temp->next;
		mvaddch(temp->y, temp->x, 'o');
	}
	mvaddch(cherry_y, cherry_x, 'X');
}

bool snake_legal()
{
	SNcell * temp = snakeHead;
	SNcell * temp2;
	while(temp){
		if(((temp->x) < 1) || ((temp->y) < 1) || ((temp->x) > WIN_W - 2) || ((temp->y) > WIN_H - 2)){
			return false;
		}
		temp2 = temp->prev;
		while(temp2){
			if(((temp->x) == (temp2->x)) && ((temp->y) == (temp2->y))){
				return false;
			}
			temp2 = temp2->prev;
		}
		temp = temp->next;
	}
	return true;
}
