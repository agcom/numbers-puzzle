#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>
#include <stdbool.h>

#ifdef WINDOWS
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else

#include <unistd.h>

#define SLEEP_MS(x) usleep((x) * 1000)
#endif

// Dimensions of whole board
#define ROWS 5
#define COLUMNS 5

// Dimensions of each room
#define ROOM_WIDTH 7
#define ROOM_HEIGHT 3

#define SHUFFLE_MOVES 1000 // How many times to simulate actual moves in the shuffle method
#define INFINITE_LOOP_SLEEP_TIME 100 // Time in milliseconds: how much to wait for each loop of while(true)
#define TIME_LIMIT 3600 // Game's time limit in seconds; set to <= 0 for unlimited time.
#define EXTRAS_SIZE 150 // Max size of the string that will always be printed after the board

// Dimensions of graphics (what is printed)
#define ROW_CHARS ((ROOM_HEIGHT + 1) * ROWS + 1)
#define COLUMN_CHARS ((ROOM_WIDTH + 1) * COLUMNS + 1)

// Masks for ease of condition checking and optimization
#define ZERO_MASK 0b01
#define END_MASK 0b10

typedef enum {
	LEFT, UP, RIGHT, DOWN
} command_t;

// Fills and sorts the numbers array
void init_nums(int nums[ROWS][COLUMNS]) {
	for (int i = 0; i < ROWS; i++) {
		int j;
		for (j = 0; j < COLUMNS; j++) {
			nums[i][j] = COLUMNS * i + j + 1;
		}
	}
	
	// The empty room marked with a zero
	nums[ROWS - 1][COLUMNS - 1] = 0;
}

// Returns count of n's digits
int digits_count(int n) {
	n = abs(n);
	
	int digits = 0;
	do {
		n /= 10;
		digits++;
	} while (n != 0);
	
	return digits;
}

void clear_screen() {
#ifdef WINDOWS
	system("cls");
#else
	system("clear");
#endif
}

// Draws the whole board (graphics + numbers)
void draw_board(const int nums[ROWS][COLUMNS], const char *graphics[ROW_CHARS][COLUMN_CHARS], const char extras[]) {
	clear_screen(); // Clear the past drawn board
	
	// Iteration over graphics
	for (int i = 0; i < ROW_CHARS; i++) {
		for (int j = 0; j < COLUMN_CHARS; j++) {
			if (*graphics[i][j] == '\0') { //number place
				int num = nums[(i - 1) / (ROOM_HEIGHT + 1)][(j - 1) / (ROOM_WIDTH + 1)], digits = digits_count(num);
				
				if (num == 0) printf(" "); // The empty room
				else {
					for (int o = 0; o < ROOM_WIDTH / 2 - digits / 2; o++) { // Print enough spaces to reach middle
						printf(" ");
						j++; // Skip filled columns by spaces
					}
					
					printf("%d", num);
					j += digits - 1; // Skip filled columns by the number
				}
			} else printf("%s", graphics[i][j]); // Graphics place
		}
		
		printf("\r\n");
	}
	
	printf("%s", extras);
}

// Initializes graphics array which contains borders
void init_graphics(char *graphics[ROW_CHARS][COLUMN_CHARS], int nums[ROWS][COLUMNS]) {
	int o = 0, p = 0; // Graphics array counters
	
	for (int i = 0; i < ROWS; i++) { // Iterates over board rows
		// Iterates over each room's rows; has +1 row at first board's row.
		for (int j = 0; j < (i == 0 ? ROOM_HEIGHT + 2 : ROOM_HEIGHT + 1); j++) {
			for (int k = 0; k < COLUMNS; k++) { // Iterates over board columns
				// Iterates over each room's columns; has +1 column at first board's column.
				for (int l = 0; l < (k == 0 ? ROOM_WIDTH + 2 : ROOM_WIDTH + 1); l++) {
					// Refer to ZERO_MASK, END_MASK
					int _i = (i == 0 ? ZERO_MASK : 0) | (i == ROWS - 1 ? END_MASK : 0);
					int _j = (j == 0 ? ZERO_MASK : 0) |
					         (j == (i == 0 ? ROOM_HEIGHT + 2 : ROOM_HEIGHT + 1) - 1 ? END_MASK : 0);
					int _k = (k == 0 ? ZERO_MASK : 0) | (k == COLUMNS - 1 ? END_MASK : 0);
					int _l = (l == 0 ? ZERO_MASK : 0) |
					         (l == (k == 0 ? ROOM_WIDTH + 2 : ROOM_WIDTH + 1) - 1 ? END_MASK : 0);
					
					char *c;
					if (_i & ZERO_MASK && _j & ZERO_MASK && _k & ZERO_MASK && _l & ZERO_MASK)
						c = "╔"; // Top left corner
					else if (_i & ZERO_MASK && _j & ZERO_MASK && _k & END_MASK && _l & END_MASK)
						c = "╗"; // Top right corner
					else if (_i & END_MASK && _j & END_MASK && _k & ZERO_MASK && _l & ZERO_MASK)
						c = "╚"; // Bottom left corner
					else if (_i & END_MASK && _j & END_MASK && _k & END_MASK && _l & END_MASK)
						c = "╝"; // Bottom right corner
					else if (_i & ZERO_MASK && _j & ZERO_MASK && !(_k & END_MASK) && _l & END_MASK)
						c = "╦"; // Top connector
					else if (!(_i & END_MASK) && _j & END_MASK && _k & ZERO_MASK && _l & ZERO_MASK)
						c = "╠"; // Left connector
					else if (!(_i & END_MASK) && _j & END_MASK && !(_k & END_MASK) && _l & END_MASK)
						c = "╬"; // Mid connector
					else if (!(_i & END_MASK) && _j & END_MASK && _k & END_MASK && _l & END_MASK)
						c = "╣"; // Right connector
					else if (_i & END_MASK && _j & END_MASK && !(_k & END_MASK) && _l & END_MASK)
						c = "╩"; // Bottom connector
					else if (_i & ZERO_MASK && _j & ZERO_MASK && !(_l & END_MASK))
						c = "═"; // First column's horizontal line
					else if (_j & END_MASK && !(_l & END_MASK)) c = "═"; // Not first column's horizontal line
					else if (!(_j & END_MASK) && _k & ZERO_MASK && _l & ZERO_MASK) c = "║"; // First row's vertical line
					else if (!(_j & END_MASK) && _l & END_MASK) c = "║"; // Not first row's vertical line
					else if (l == (k == 0 ? 1 : 0) && j == (ROOM_HEIGHT / 2 + (i == 0 ? 1 : 0)))
						c = "\0"; // Start of where number can be written
					else c = " "; // Within rooms but not middle; empty space.
					
					// Add to graphics array
					graphics[o][p] = c;
					p++;
					if (p == COLUMN_CHARS) {
						o++;
						p = 0;
					}
				}
			}
		}
	}
}

// Determines whether the move is executable or not (checks for out of bound moves)
bool can_move(const command_t command, const int x, const int y) {
	switch (command) {
		case 0 : // Left
			return y + 1 < COLUMNS;
		case 1 : // Up
			return x + 1 < ROWS;
		case 2 : // Right
			return y - 1 >= 0;
		case 3 : // Down
			return x - 1 >= 0;
		default:
			return false; // Should not reach here.
	}
}

// Executes the given command; checks for out of bound indexes.
// Command: 0 = left, 1 = up, 2 = right, 3 = down.
// Returns true if done the move, otherwise false (impossible to move).
bool movee(int nums[ROWS][COLUMNS], const command_t command, int *x, int *y) {
	if (can_move(command, *x, *y)) {
		switch (command) {
			case 0 : // Left: swap (x, y) with (x, y+1)
				nums[*x][*y] = nums[*x][*y + 1];
				nums[*x][*y + 1] = 0;
				*y += 1;
				break;
			case 1 : // Up: swap (x, y) with (x+1, y)
				nums[*x][*y] = nums[*x + 1][*y];
				nums[*x + 1][*y] = 0;
				*x += 1;
				break;
			case 2 : // Right: swap (x, y) with (x, y-1)
				nums[*x][*y] = nums[*x][*y - 1];
				nums[*x][*y - 1] = 0;
				*y -= 1;
				break;
			case 3 : // Down: swap (x, y) with (x-1, y)
				nums[*x][*y] = nums[*x - 1][*y];
				nums[*x - 1][*y] = 0;
				*x -= 1;
				break;
			default:
				break;
		}
		
		return true;
	}
	
	return false;
}

// Checks the board for winning condition
// Returns true if the board is sorted, otherwise false.
bool is_sorted(const int nums[ROWS][COLUMNS]) {
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			// Jump over the empty room
			if (i == ROWS - 1 && j == COLUMNS - 1) continue;
			
			if (nums[i][j] != COLUMNS * i + j + 1) {
				return false;
			}
		}
	}
	
	return true;
}

// Shuffles the board numbers by simulating actual moves
void shuffle_nums(int nums[ROWS][COLUMNS], int *x, int *y) {
	do {
		for (int i = 0; i < SHUFFLE_MOVES; i++) {
			while (!movee(nums, rand() % 4, x, y)); // Guarantees the move
		}
	} while (is_sorted(nums)); // Make sure board is shuffled
}

command_t key_to_command(const int c) {
	switch (c) {
		case KEY_LEFT :
			return LEFT;
		case KEY_UP :
			return UP;
		case KEY_RIGHT :
			return RIGHT;
		case KEY_DOWN :
			return DOWN;
		default:
			return -1; // Not an arrow key command
	}
}

// On win condition
void win(const long timer, const int moves) {
	clear_screen();
	printf("\r\n\nCongratulations!\r\n\nWon in %ld seconds by %d moves!\r\n", timer, moves);
}

// On lose condition
void lose() {
	clear_screen();
	printf("\r\n\nLet's find your limits!\r\n\n");
}

// Returns current system time in seconds from 1900
long current_time_seconds() {
	return time(NULL);
}

// Updates the extras
void update_extras(const long timer, const int moves, char extras[]) {
	if (TIME_LIMIT > 0) {
		sprintf(extras, "\r\nTime remaining = %ld\r\n\nMoves done = %d\r\n", TIME_LIMIT - timer, moves);
	} else {
		sprintf(extras, "\r\nMoves done = %d\r\n", moves);
	}
}

void sleep_ms(const long ms) {
	SLEEP_MS(ms);
}

int kbhit() {
	const int c = getch();
	if (c != ERR) {
		ungetch(c);
		return 1;
	} else return 0;
}

int main() {
	initscr();
	noecho();
	nl();
	keypad(stdscr, TRUE);
	cbreak();
	nodelay(stdscr, TRUE);
	
	srand(time(NULL));
	
	int nums[ROWS][COLUMNS]; // Numbers array
	init_nums(nums);
	
	int x = ROWS - 1, y = COLUMNS - 1; // Position of the empty room
	shuffle_nums(nums, &x, &y);
	
	char *graphics[ROW_CHARS][COLUMN_CHARS]; // Graphics array
	init_graphics(graphics, nums);
	
	char extras[EXTRAS_SIZE] = ""; // Extra strings to be always printed after the board
	
	int moves = 0; // Moves done
	long timer = 0; // Timer
	
	// Early board draw
	update_extras(timer, moves, extras);
	draw_board(nums, graphics, extras);
	
	while (true) {
		// Save the start time
		const long start_time = current_time_seconds();
		
		while (kbhit()) { // A key was pressed
			int key = getch();
			
			switch (key) { // Key press handler
				case KEY_LEFT:
				case KEY_RIGHT:
				case KEY_DOWN:
				case KEY_UP: // An arrow key
					if (movee(nums, key_to_command(key), &x, &y)) {
						moves++;
						
						update_extras(timer, moves, extras);
						draw_board(nums, graphics, extras);
						
						if (is_sorted(nums)) { // Win condition
							win(timer, moves);
							return 0;
						}
					}
					break;
				case '\n' : // The "Enter" key; reset the game.
					timer = 0;
					moves = 0;
					
					update_extras(timer, moves, extras);
					shuffle_nums(nums, &x, &y);
					draw_board(nums, graphics, extras);
					
					break;
				default:
					break; // NOP
			}
		}
		
		sleep_ms(INFINITE_LOOP_SLEEP_TIME);
		
		// Add to the timer
		const long passed_time = current_time_seconds() - start_time;
		if (passed_time != 0) {
			timer += passed_time;
			if (TIME_LIMIT > 0) {
				update_extras(timer, moves, extras);
				draw_board(nums, graphics, extras);
				
				if (timer > TIME_LIMIT) { // Lose condition
					lose();
					return 0;
				}
			}
		}
	}
	
	endwin();
	return 0;
}
