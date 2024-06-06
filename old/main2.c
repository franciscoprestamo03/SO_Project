#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curses.h>
#include <time.h>

#include "memory_handling.h"

#define WIDTH 75
#define HEIGHT 45
#define MAX_BULLETS 5
#define MAX_ALIENS 10
#define RESPAWN_TIME 30 // time units for an alien to respawn
#define SCORE_FILE "highscore.txt"

#define WHITE_CHAR_COLOR_PAIR 1
#define RED_CHAR_COLOR_PAIR 2
#define GREEN_CHAR_COLOR_PAIR 3


// 0 - continue
// 1 - new game
// 2 - load game
// 3 - change user
// 4 - exit

#define MENU_MAX 4 // maximum possible cursor position in the menu
#define MENU_CONTINUE 0
#define MENU_NEW_GAME 1
#define MENU_LOAD 2
#define MENU_CHANGE_USER 3
#define MENU_EXIT 4
char* menu_choices[] = {"continue", "new game", "load game", "change user", "exit"};



#define MENU_STATE 0
#define IN_GAME_STATE 1
#define GAME_OVER_SCREEN_STATE 2


pthread_mutex_t alienMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;

void initMemory();

void loadHighScore();
void saveHighScore();
void drawPlayer();
void drawBullets();
void drawAliens();
void drawScore();

void display_menu();
void* menuInput(void *arg);

int main()
{
    srand(time(NULL));
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    INT_MEMORY[statePtr] = MENU_STATE;

    WINDOW *menu_win;
    int end_y, end_x;
    getmaxyx(stdscr, end_y, end_x);
    menu_win = newwin(end_y, end_x, 0, 0);
    keypad(menu_win, TRUE);

    pthread_t menuInputThread;
    pthread_create(&menuInputThread, NULL, menuInput, NULL);

    while(state == MENU_STATE)
    {
        clear();
        display_menu();
        refresh();
        usleep(100000);
    }

    while (state == IN_GAME_STATE)
    {
        
    }
    

    endwin();
    free(MEMORY);
`
    return 0;
}


#pragma region fs
void loadHighScore() {
    FILE *file = fopen(SCORE_FILE, "r");
    if (file != NULL) {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
}

void saveHighScore() {
    FILE *file = fopen(SCORE_FILE, "w");
    if (file != NULL) {
        fprintf(file, "%d", highScore);
        fclose(file);
    }
}

#pragma endregion

#pragma region drawing

void drawPlayer() {
    mvprintw(playerY, playerX + 2, " ");
    mvprintw(playerY + 1, playerX + 1, "   ");
    mvprintw(playerY + 2, playerX, "     ");
}

void drawBullets() {
    for (int i = 0; i < bulletCount; i++) {
        mvprintw(bullets[i][1], bullets[i][0], "*");
    }
}

void drawAliens() {
    pthread_mutex_lock(&alienMutex);
    for (int i = 0; i < alienCount; i++) {
        if (aliens[i][3] == 1) { // Life 1
            mvprintw(aliens[i][1], aliens[i][0], "   ");
            mvprintw(aliens[i][1] + 1, aliens[i][0] + 1, " ");
            mvprintw(aliens[i][1] + 2, aliens[i][0] + 1, " ");
        } else if (aliens[i][3] == 2) { // Life 2
            mvprintw(aliens[i][1], aliens[i][0], "   ");
            mvprintw(aliens[i][1] + 1, aliens[i][0], "   ");
            mvprintw(aliens[i][1] + 2, aliens[i][0] + 1, " ");
        } else if (aliens[i][3] == 3) { // Life 3
            mvprintw(aliens[i][1], aliens[i][0], "   ");
            mvprintw(aliens[i][1] + 1, aliens[i][0], "   ");
            mvprintw(aliens[i][1] + 2, aliens[i][0], "   ");
        }
    }
    pthread_mutex_unlock(&alienMutex);
}

void drawScore() {
    int infoX = WIDTH + 2; // X position for the info
    mvprintw(0, infoX, "Score: %d", score);
    mvprintw(2, infoX, "High Score: %d", highScore);
    mvprintw(4, infoX, "Respawn Times:");
    for (int i = 0; i < MAX_ALIENS; i++) {
        mvprintw(6 + i, infoX, "Alien %d: %d", i + 1, alienRespawnTimes[i]);
    }
}


#pragma endregion


#pragma region Menu

void display_menu()
{
    int top = HEIGHT/2 - MENU_MAX/2;
    int left = WIDTH/2 - 5;
    for (int i = 0; i <= MENU_MAX; i++)
    {
        if (menuChoice == i)
        {
            mvprintw(top + i + 1, left + 1, ">");
        }
        mvprintw(top + i + 1, left + 3, "%s", menu_choices[i]);
    }

}

void* menuInput(void *arg)
{
    while (state == MENU_STATE)
    {
        int ch = getch();

        switch(ch)
        {
        case 'w':
        case 'W':
        case KEY_UP:
            if(menuChoice == 0)
                menuChoice = MENU_MAX;
            else menuChoice--;
            break;
        case 's':
        case 'S':
        case KEY_DOWN:
            if(menuChoice == MENU_MAX)
                menuChoice = 0;
            else menuChoice++;
            break;
        case '\n':
            if (menuChoice == MENU_EXIT)
                state = GAME_OVER_SCREEN_STATE;
            if (menuChoice == MENU_NEW_GAME)
                state = IN_GAME_STATE;
        }
    }
    
}

#pragma endregion