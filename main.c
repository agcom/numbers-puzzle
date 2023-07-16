#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <string.h>

//dimensions of whole board
#define ROWS 5
#define COLUMNS 5

//dimensions of each room
#define ROOM_WIDTH 7
#define ROOM_HEIGTH 3

#define SHUFFLE_MOVES 1000 //how many times to simulate actual moves in the shuffle method
#define INFINITE_LOOP_SLEEP_TIME 100 //time in milliseconds, how much to wait for each loop of while(1)
#define TIME_LIMIT 10 //game's time limit in seconds, set to <= 0 for unlimited time
#define EXTRAS_SIZE 150 //max size of the string that always will be printed after the board

#define AKCCM -32 //Arrow Key Char Code Modifier
#define EKCC 13 //Enter Key Char Code

//------------------------------------------------------------//

//dimensions of graphics (what should be printed)
#define ROW_CHARS (ROOM_HEIGTH + 1) * ROWS + 1
#define COLUMN_CHARS (ROOM_WIDTH + 1) * COLUMNS + 1

//masks for ease of condition checking and optimization
#define ZERO_MASK 0b01
#define END_MASK 0b10

//fills and sorts the numbers array
void initNums(int nums[ROWS][COLUMNS]) {

    int i;
    for(i = 0; i < ROWS; i++) {

        int j;
        for(j = 0; j < COLUMNS; j++) {

            nums[i][j] = COLUMNS * i + j + 1;

        }

    }

    //the empty room, marked with a zero
    nums[ROWS-1][COLUMNS-1] = 0;

}

//draws the whole board (graphics + numbers)
void drawBoard(int nums[ROWS][COLUMNS], char graphics[ROW_CHARS][COLUMN_CHARS], char extras[]) {

    system("cls"); //clear the past drawn board

    //iteration over graphics
    int i;
    for(i = 0; i < ROW_CHARS; i++) {

        int j;
        for(j = 0; j < COLUMN_CHARS; j++) {

            if(graphics[i][j] == '\0') { //number place

                int num = nums[(i - 1) / (ROOM_HEIGTH + 1)][(j - 1) / (ROOM_WIDTH + 1)], digits = digitsCount(num);

                if(num == 0) printf(" "); //the empty room
                else {

                    int o;
                    for(o = 0; o < ROOM_WIDTH/2 - digits/2; o++) { //print enough spaces to reach middle

                        printf(" ");
                        j++; //skip filled columns by spaces

                    }

                    printf("%d", num);
                    j += digits-1; //skip filled columns by the number

                }

            } else printf("%c", graphics[i][j]); //graphics place

        }

        printf("\n");

    }

    printf(extras);

}

//initializes graphics array which contains borders
void initGraphics(char graphics[ROW_CHARS][COLUMN_CHARS], int nums[ROWS][COLUMNS]) {

    int o = 0, p = 0; //graphics array counters
    int i;
    for(i = 0; i < ROWS; i++) { //iterates over board rows

        int j;
        for(j = 0; j < (i == 0 ? ROOM_HEIGTH + 2 : ROOM_HEIGTH + 1); j++) { //iterates over each room rows; has +1 row at first board's row

            int k;
            for(k = 0; k < COLUMNS; k++) { //iterates over board columns

                int l;
                for(l = 0; l < (k == 0 ? ROOM_WIDTH + 2 : ROOM_WIDTH + 1); l++) { //iterates over each room columns; has +1 column at first board's column

                    //refer to ZERO_MASK, END_MASK
                    int _i = (i == 0 ? ZERO_MASK : 0) | (i == ROWS - 1 ? END_MASK : 0);
                    int _j = (j == 0 ? ZERO_MASK : 0) | (j == (i == 0 ? ROOM_HEIGTH + 2 : ROOM_HEIGTH + 1) - 1 ? END_MASK : 0);
                    int _k = (k == 0 ? ZERO_MASK : 0) | (k == COLUMNS - 1 ? END_MASK : 0);
                    int _l = (l == 0 ? ZERO_MASK : 0) | (l == (k == 0 ? ROOM_WIDTH + 2 : ROOM_WIDTH + 1) - 1 ? END_MASK : 0);

                    char c;

                    if(_i & ZERO_MASK && _j & ZERO_MASK && _k & ZERO_MASK && _l & ZERO_MASK) c = 201; //top left corner

                    else if(_i & ZERO_MASK && _j & ZERO_MASK && _k & END_MASK && _l & END_MASK) c = 187; //top right corner

                    else if(_i & END_MASK && _j & END_MASK && _k & ZERO_MASK && _l & ZERO_MASK) c = 200; //bottom left corner

                    else if(_i & END_MASK && _j & END_MASK && _k & END_MASK && _l & END_MASK) c = 188; //bottom right corner

                    else if(_i & ZERO_MASK && _j & ZERO_MASK && !(_k & END_MASK) && _l & END_MASK) c = 203; //top connector

                    else if(!(_i & END_MASK) && _j & END_MASK && _k & ZERO_MASK && _l & ZERO_MASK) c = 204; //left connector

                    else if(!(_i & END_MASK) && _j & END_MASK && !(_k & END_MASK) && _l & END_MASK) c = 206; //mid connector

                    else if(!(_i & END_MASK) && _j & END_MASK && _k & END_MASK && _l & END_MASK) c = 185; //right connector

                    else if(_i & END_MASK && _j & END_MASK && !(_k & END_MASK) && _l & END_MASK) c = 202; //bottom connector

                    else if(_i & ZERO_MASK && _j & ZERO_MASK && !(_l & END_MASK)) c = 205; //first column's horizontal line

                    else if(_j & END_MASK && !(_l & END_MASK)) c = 205; //not first column's horizontal line

                    else if(!(_j & END_MASK) && _k & ZERO_MASK && _l & ZERO_MASK) c = 186; //first row's vertical line

                    else if(!(_j & END_MASK) && _l & END_MASK) c = 186; //not first row's vertical line

                    else if(l == (k == 0 ? 1 : 0) && j == (ROOM_HEIGTH/2 + (i == 0 ? 1 : 0))) c = '\0'; //start of where number can be written

                    else c = ' '; //within rooms but not middle; empty space

                    //add to graphics array
                    graphics[o][p] = c;
                    p++;
                    if(p == COLUMN_CHARS) {

                        o++;
                        p = 0;

                    }

                }

            }

        }

    }

}

//returns count of n's digits
int digitsCount(int n) {

    n = abs(n);

    int digits = 0;
    do {

        n/=10;
        digits++;

    } while(n != 0);

    return digits;

}

//executes the given command, checks for out of bound indexes
//command : 0 = left, 1 = up, 2 = right, 3 = down
//returns 1 if done the move, otherwise 0 (impossible to move)
int move(int nums[ROWS][COLUMNS], int command, int *x, int *y) {

    if(canMove(command, *x, *y)) {

        switch(command) {

            case 0 : //left, swap (x, y) with (x, y+1)
                nums[*x][*y] = nums[*x][*y+1];
                nums[*x][*y+1] = 0;
                *y += 1;
                break;

            case 1 : //up, swap (x, y) with (x+1, y)
                nums[*x][*y] = nums[*x+1][*y];
                nums[*x+1][*y] = 0;
                *x += 1;
                break;

            case 2 : //right, swap (x, y) with (x, y-1)
                nums[*x][*y] = nums[*x][*y-1];
                nums[*x][*y-1] = 0;
                *y -= 1;
                break;

            case 3 : //down, swap (x, y) with (x-1, y)
                nums[*x][*y] = nums[*x-1][*y];
                nums[*x-1][*y] = 0;
                *x -= 1;
                break;

        }

        return 1;

    }

    return 0;

}

//determines whether the move is executable or not (checks for out of bound moves)
int canMove(int command, int x, int y) {

    switch(command) {

        case 0 : //left
            return y + 1 < COLUMNS;

        case 1 : //up
            return x + 1 < ROWS;

        case 2 : //right
            return y - 1 >= 0;

        case 3 : //down
            return x - 1 >= 0;

    }

    return 0; //should not reach here

}

//checks the board for winning condition
//returns 1 if the board is sorted, otherwise 0
int isSorted(int nums[ROWS][COLUMNS]) {

    int i;
    for(i = 0; i < ROWS; i++) {

        int j;
        for(j = 0; j < COLUMNS; j++) {

            //jump over the empty room
            if(i == ROWS - 1 && j == COLUMNS - 1) continue; //break;

            if(nums[i][j] != COLUMNS * i + j + 1) {

                return 0;

            }

        }

    }

    return 1;

}

//shuffles the board numbers by simulating actual moves
void shuffleNums(int nums[ROWS][COLUMNS], int *x, int *y) {

    do {

        int i;
        for(i = 0; i < SHUFFLE_MOVES; i++) {

            while(!move(nums, rand()%4, x, y)); //guarantees the move

        }

    } while(isSorted(nums)); //make sure board is shuffled

}

//returns : 75 = 0 = left, 72 = 1 = up, 77 = 2 = right, 80 = 3 = down, -1 = non arrow key
int charToCommand(char c) {

    switch(c) {

        case 75 : return 0;
        case 72 : return 1;
        case 77 : return 2;
        case 80 : return 3;

    }

    return -1; //not an arrow key command
}

//on win condition
void win(long timer, int moves) {

    system("cls");
    printf("\n\nCongratulations!\n\nWon in %d seconds by %d moves!\n", timer, moves);

}

//on lose condition
void lose() {

    system("cls");
    printf("\n\nLet's find your limits!\n\n");

}

//returns current system time in seconds from 1900
long currentTimeSeconds() {

    return time(NULL);

}

//updates the extras
void updateExtras(long timer, int moves, char extras[]) {

    if(TIME_LIMIT > 0) {

        sprintf(extras, "\nTime remaining = %d\n\nMoves done = %d", TIME_LIMIT - timer, moves);

    } else {

        sprintf(extras, "\nMoves done = %d", moves);

    }

}

int main() {

    system("chcp 850"); //support for ASCII characters
    srand(time(NULL)); //for absolute random

    int nums[ROWS][COLUMNS]; //numbers array
    initNums(nums);
    int x = ROWS-1, y = COLUMNS-1; //position of the empty room
    shuffleNums(nums, &x, &y);

    char graphics[ROW_CHARS][COLUMN_CHARS]; //graphics array
    initGraphics(graphics, nums);

    char extras[EXTRAS_SIZE] = ""; //extra strings to be always printed after the board
    int moves = 0; //moves done

    long timer = 0; //timer

    //early board draw
    updateExtras(timer, moves, extras);
    drawBoard(nums, graphics, extras);

    while(1) {

        //save the start time
        long start_time = currentTimeSeconds();

        while(_kbhit()) { //a key was pressed

            char key = _getch();

            switch(key) { //key press handler

                case AKCCM : //an arrow key

                    if(move(nums, charToCommand(_getch()), &x, &y)) {

                        moves++;

                        updateExtras(timer, moves, extras);
                        drawBoard(nums, graphics, extras);

                        if(isSorted(nums)) { //win condition

                            win(timer, moves);
                            return 0;

                        }

                    }

                    break;

                case EKCC : //the "Enter" key, reset the game

                    timer = 0;
                    moves = 0;

                    updateExtras(timer, moves, extras);
                    shuffleNums(nums, &x, &y);
                    drawBoard(nums, graphics, extras);

                    break;

            }

        }

        Sleep(INFINITE_LOOP_SLEEP_TIME);

        //add to the timer
        long passed_time = currentTimeSeconds() - start_time;

        if(passed_time != 0) {

            timer += passed_time;

            if(TIME_LIMIT > 0) {

                updateExtras(timer, moves, extras);
                drawBoard(nums, graphics, extras);

                if(timer > TIME_LIMIT) { //lose condition

                    lose();
                    return 0;

                }

            }

        }

    }

    return 0;

}