#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curses.h>
#include <time.h>

#define WIDTH 75
#define HEIGHT 45
#define MAX_BULLETS 5
#define MAX_ALIENS 10
#define RESPAWN_TIME 30 // time units for an alien to respawn
#define SCORE_FILE "highscore.txt"

#define WHITE_CHAR_COLOR_PAIR 1
#define RED_CHAR_COLOR_PAIR 2
#define GREEN_CHAR_COLOR_PAIR 3

int MEMORY[BUFSIZ];

int playerX, playerY;
int bullets[MAX_BULLETS][2]; // [bullet index][x, y]
int bulletCount = 0;
int gameOver = 0;

int aliens[MAX_ALIENS][4]; // [alien index][x, y, direction, life]
int alienRespawnTimes[MAX_ALIENS]; // [alien index][respawn time]
int alienCount = 0; // Number of active aliens

int score = 0;
int highScore = 0;

pthread_mutex_t alienMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;


void loadHighScore();
void saveHighScore();
void drawPlayer();
void drawBullets();
void drawAliens();
void drawScore();

int main()
{


    return 0;
}

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

int getMemory(int size)
{

}
