CC := gcc
STDFLAG := -std=c11
FLAGS := -Wall -Werror -Wextra -Wvla -g -lncurses
snake: snake.c
	$(CC) $(STDFLAG) $(FLAGS) -o snake snake.c
