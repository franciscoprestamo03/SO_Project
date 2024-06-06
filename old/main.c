#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curses.h>
#include <time.h>

#define WIDTH 75
#define HEIGHT 20
#define MAX_BULLETS 5
#define MAX_ALIENS 10
#define RESPAWN_TIME 30 // time units for an alien to respawn
#define SCORE_FILE "highscore.txt"

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

void moveBullets() {
    for (int i = 0; i < bulletCount; i++) {
        bullets[i][1]--;
        if (bullets[i][1] < 0) {
            // Remove the bullet if it goes out of bounds
            for (int j = i; j < bulletCount - 1; j++) {
                bullets[j][0] = bullets[j + 1][0];
                bullets[j][1] = bullets[j + 1][1];
            }
            bulletCount--;
        }
    }
}

void moveAliens() {
    pthread_mutex_lock(&alienMutex);
    for (int i = 0; i < alienCount; i++) {
        aliens[i][0] += aliens[i][2]; // Move the alien according to its direction
        if (aliens[i][0] < 0) {
            // If the alien reaches the left side, move down and change direction
            aliens[i][0] = 0;
            aliens[i][1] += 3; // Move down by 3 rows
            aliens[i][2] = 1; // Change direction to right
        } else if (aliens[i][0] >= WIDTH - 2) {
            // If the alien reaches the right side, move down and change direction
            aliens[i][0] = WIDTH - 2;
            aliens[i][1] += 3; // Move down by 3 rows
            aliens[i][2] = -1; // Change direction to left
        }

        // Check if the alien is in the same row as the player
        if ((aliens[i][1] == playerY || aliens[i][1] + 1 == playerY) &&
            (aliens[i][1] == playerY + 1 || aliens[i][1] + 1 == playerY + 1) &&
            (aliens[i][1] == playerY + 2 || aliens[i][1] + 1 == playerY + 2)) {
            gameOver = 1;
        }

        // If the alien reaches the bottom, set game over
        if (aliens[i][1] >= HEIGHT - 3) {
            gameOver = 1;
        }
    }
    pthread_mutex_unlock(&alienMutex);
}

void checkCollisions() {
    pthread_mutex_lock(&alienMutex);
    for (int i = 0; i < bulletCount; i++) {
        for (int j = 0; j < alienCount; j++) {
            if (bullets[i][0] >= aliens[j][0] && bullets[i][0] <= aliens[j][0] + 2 &&
                bullets[i][1] >= aliens[j][1] && bullets[i][1] <= aliens[j][1] + 2) {
                // Remove the bullet
                for (int k = i; k < bulletCount - 1; k++) {
                    bullets[k][0] = bullets[k + 1][0];
                    bullets[k][1] = bullets[k + 1][1];
                }
                bulletCount--;

                // Reduce alien life
                aliens[j][3]--;
 
                // Update the score (hitting an alien gives 10 points, killing it gives 20)
                score += 10;
                if (score > highScore) {
                    highScore = score;
                }

                if (aliens[j][3] <= 0) {
                    // Remove the alien if life is 0
                    for (int k = j; k < alienCount - 1; k++) {
                        aliens[k][0] = aliens[k + 1][0];
                        aliens[k][1] = aliens[k + 1][1];
                        aliens[k][2] = aliens[k + 1][2];
                        aliens[k][3] = aliens[k + 1][3];
                    }
                    alienCount--;

                    // Set respawn time for the next alien
                    alienRespawnTimes[j] = rand() % 7 + 4;

                    // Update the score
                    score += 10;
                    if (score > highScore) {
                        highScore = score;
                    }
                }

                // Break out of the loops to avoid out-of-bounds access
                break;
            }
        }
    }
    pthread_mutex_unlock(&alienMutex);
}

void* alienRespawn(void* arg) {
    while (!gameOver) {
        pthread_mutex_lock(&alienMutex);
        for (int i = 0; i < MAX_ALIENS; i++) {
            if (alienRespawnTimes[i] > 0) {
                alienRespawnTimes[i]--;
                if (alienRespawnTimes[i] == 0) {
                    // Respawn the alien
                    int posX;
                    do {
                        posX = (rand() % (WIDTH / 3)) * 3; // Ensure posX is a multiple of 3
                    } while (posX >= WIDTH - 2); // Ensure posX is within bounds
                    aliens[alienCount][0] = posX;
                    aliens[alienCount][1] = rand() % (HEIGHT / 2);
                    aliens[alienCount][2] = -1; // Initial direction to left
                    aliens[alienCount][3] = rand() % 3 + 1; // Random life between 1 and 3
                    alienCount++;
                }
            }
        }

        pthread_mutex_unlock(&alienMutex);
        usleep(100000);
    }
    return NULL;
}

void* getInput(void* arg) {
    while (!gameOver) {
        int ch = getch();
        switch (ch) {
            case 'a':
                if (playerX > 0) playerX--;
            break;
            case 'd':
                if (playerX < WIDTH - 3) playerX++;
            break;
            case ' ':
                if (bulletCount < MAX_BULLETS) {
                    bullets[bulletCount][0] = playerX + 1;
                    bullets[bulletCount][1] = playerY - 1;
                    bulletCount++;
                }
            break;
            case 'q':
                gameOver = 1;
            break;
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    initscr();
    
    if (has_colors() == FALSE)
    {
        printf("NO COLORS");
        exit(1);
    }

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_GREEN);
    attron(COLOR_PAIR(1));

    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    loadHighScore();

    playerX = WIDTH / 2;
    playerY = HEIGHT - 3;

    // Initialize alien respawn times
    for (int i = 0; i < MAX_ALIENS; i++) {
        alienRespawnTimes[i] = rand() % 7 + 4; // Spread out the initial respawn times
    }

    pthread_t inputThread, alienRespawnThread;
    pthread_create(&inputThread, NULL, getInput, NULL);
    pthread_create(&alienRespawnThread, NULL, alienRespawn, NULL);

    while (!gameOver) {
        clear();
        drawPlayer();
        moveBullets();
        moveAliens();
        checkCollisions();
        drawBullets();
        drawAliens();
        drawScore();
        refresh();
        usleep(100000);
    }

    clear(); // Clear the screen before printing "GAME OVER"

    // Print each line of "GAME OVER"
    attron(A_BOLD); // Turn on bold attribute
    mvprintw(HEIGHT / 2 - 1, (WIDTH - 10) / 2, "GAME OVER");
    mvprintw(HEIGHT / 2 + 1, (WIDTH - 10) / 2, "Score: %d", score);
    mvprintw(HEIGHT / 2 + 3, (WIDTH - 10) / 2, "High Score: %d", highScore);
    attroff(A_BOLD); // Turn off bold attribute

    refresh();

    usleep(3000000);

    endwin();
    pthread_join(inputThread, NULL);
    pthread_join(alienRespawnThread, NULL);

    saveHighScore();

    printf("Game Over! Your score: %d\n", score);
    printf("High Score: %d\n", highScore);

    return 0;
}
