#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curses.h>
#include <time.h>
#include <string.h>

#define SCREEN_WIDTH 75
#define SCREEN_HEIGHT 40
#define SCREEN_DANGER_LOWER_BAR_HEIGHT 3  
#define infoPosition_x (SCREEN_WIDTH + 2)
#define LEFT_DIR 1
#define RIGHT_DIR 0

#define GREEN_COLOR_PAIR 1
#define BLUE_COLOR_PAIR 2
#define RED_COLOR_PAIR 3
#define YELLOW_COLOR_PAIR 4


#define _lockMemory pthread_mutex_lock(&memoryMutex)
#define _unlockMemory pthread_mutex_unlock(&memoryMutex)

#pragma region Memory Allocation Declarations

#define sizeofBlockData 3
#define MEMORY_LENGTH 1000000
#define NULLPTR -1

int PROGRAM_MEMORY[MEMORY_LENGTH];

#define blockCounter PROGRAM_MEMORY[0] // stores the amount of blocks
#define curBlockPtr PROGRAM_MEMORY[1] // stores a pointer to the current block for the nextFit algorithm
#define GLOBAL_DATA_SIZE 2
#define FIRST_BLOCK_PTR 2

pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_HIGHSCORES 10
#define MAX_NAME_LENGTH 50

typedef struct {
    int score;
    char name[MAX_NAME_LENGTH];
} Highscore;

Highscore highscores[MAX_HIGHSCORES];

void loadHighscores() {
    FILE *file = fopen("highscores.dat", "rb");
    if (file != NULL) {
        fread(highscores, sizeof(Highscore), MAX_HIGHSCORES, file);
        fclose(file);
    } else {
        // Inicializar la lista de puntajes en caso de que no exista el archivo
        for (int i = 0; i < MAX_HIGHSCORES; i++) {
            highscores[i].score = 0;
            strcpy(highscores[i].name, "N/A");
        }
    }
}

void saveHighscores() {
    FILE *file = fopen("highscores.dat", "wb");
    if (file != NULL) {
        fwrite(highscores, sizeof(Highscore), MAX_HIGHSCORES, file);
        fclose(file);
    }
}

void updateHighscores(int newScore) {
    char playerName[MAX_NAME_LENGTH];
    echo();
    mvprintw(SCREEN_HEIGHT / 2 + 5, (SCREEN_WIDTH - 20) / 2, "Enter your name: ");
    getnstr(playerName, MAX_NAME_LENGTH - 1);
    noecho();

    // Insertar el nuevo puntaje en la lista de puntajes
    for (int i = 0; i < MAX_HIGHSCORES; i++) {
        if (newScore > highscores[i].score) {
            for (int j = MAX_HIGHSCORES - 1; j > i; j--) {
                highscores[j] = highscores[j - 1];
            }
            highscores[i].score = newScore;
            strncpy(highscores[i].name, playerName, MAX_NAME_LENGTH - 1);
            highscores[i].name[MAX_NAME_LENGTH - 1] = '\0';
            break;
        }
    }

    saveHighscores();
}

void initMemory();
// returns free block index in MEMORY_BLOCKS
int allocateMemory(int size);  
void deleteMem(int blockPtr);

int _getBlockSize(int blockPtr);
void _setBlockSize(int blockPtr, int newLength);

int _getBlockFree(int blockPtr);
void _setBlockFree(int blockPtr, int newFree);

int _getBlockNext(int blockPtr);
void _setBlockNext(int blockPtr, int newNext);

int getBlockMemoryPtr(int blockPtr);
int _getBlockMemoryPtr(int blockPtr);

void printBlocks();


#pragma endregion

#pragma region Memory Allocation Definitions

void initMemory()
{
    blockCounter = 1;
    curBlockPtr = FIRST_BLOCK_PTR;
    _setBlockSize(curBlockPtr, MEMORY_LENGTH - GLOBAL_DATA_SIZE - sizeofBlockData);
    _setBlockFree(curBlockPtr, 1);
    _setBlockNext(curBlockPtr, -1);
}


int _allocateMemory(int size)
{
    int firstBlock = curBlockPtr;
    int answ = -1;
    do
    {
        if (_getBlockFree(curBlockPtr) && _getBlockSize(curBlockPtr) >= size)
        {
            if (_getBlockSize(curBlockPtr) > size)
            {            
                int newBlockPtr = curBlockPtr + sizeofBlockData + size;
                blockCounter += 1;
                _setBlockFree(newBlockPtr, 1);
                _setBlockSize(newBlockPtr, _getBlockSize(curBlockPtr) - size - sizeofBlockData);
                _setBlockNext(newBlockPtr, _getBlockNext(curBlockPtr));

                _setBlockNext(curBlockPtr, newBlockPtr);
                _setBlockSize(curBlockPtr, size);                
            }

            _setBlockFree(curBlockPtr, 0);
            answ = curBlockPtr;
            break;
        }
        else
        {
            if (_getBlockNext(curBlockPtr) == -1)
                curBlockPtr = FIRST_BLOCK_PTR;
            else
                curBlockPtr = _getBlockNext(curBlockPtr);
        }
    } while (curBlockPtr != firstBlock);
    
    if (answ == -1)
    {
        printf("NOT ENOUGH MEMORY\n");
        exit(1);
    } 
    return answ;
}


int allocateMemory(int size)
{
    _lockMemory;
    int answ = _allocateMemory(size);
    _unlockMemory;

    return answ;
}

void deleteMem(int blockPtr)
{
    _lockMemory;

    if (_getBlockFree(blockPtr))
    {
        printf("FREEING ALREADY FREE MEMORY\n");
        printf("MEM SIZE: %d\n", _getBlockSize(blockPtr));
        exit(1);
    }
    _setBlockFree(blockPtr, 1);

    blockPtr = FIRST_BLOCK_PTR;

    do
    {
        if (_getBlockFree(blockPtr) &&
         _getBlockNext(blockPtr) != -1 &&
         _getBlockFree(_getBlockNext(blockPtr)))
        {
            int nextBlock = _getBlockNext(blockPtr);
            _setBlockSize(blockPtr, _getBlockSize(blockPtr) + _getBlockSize(nextBlock) + sizeofBlockData);
            _setBlockNext(blockPtr, _getBlockNext(nextBlock));
        }
        else blockPtr = _getBlockNext(blockPtr);
    }while(blockPtr != -1);


    _unlockMemory;
}

#define BLOCK_SIZE 0
#define BLOCK_FREE 1
#define BLOCK_NEXT 2

int _getBlock(int blockPtr, int offset){    
    return PROGRAM_MEMORY[blockPtr + offset];
}
void _setBlock(int blockPtr, int offset, int newBlock){    
    PROGRAM_MEMORY[blockPtr + offset] = newBlock;
}

int _getBlockSize(int blockPtr){
    return _getBlock(blockPtr, BLOCK_SIZE);
}
void _setBlockSize(int blockPtr, int newLength){
    _setBlock(blockPtr, BLOCK_SIZE, newLength);
}

int _getBlockFree(int blockPtr){
    return _getBlock(blockPtr, BLOCK_FREE);
}
void _setBlockFree(int blockPtr, int newFree){
    _setBlock(blockPtr, BLOCK_FREE, newFree);
}

int _getBlockNext(int blockPtr){
    return _getBlock(blockPtr, BLOCK_NEXT);
}
void _setBlockNext(int blockPtr, int newNext){
    _setBlock(blockPtr, BLOCK_NEXT, newNext);
}

int getBlockMemoryPtr(int blockPtr){
    pthread_mutex_lock(&memoryMutex);
    int answ = _getBlockMemoryPtr(blockPtr);
    pthread_mutex_unlock(&memoryMutex);
    return answ;
}
int _getBlockMemoryPtr(int blockPtr){
    return blockPtr + sizeofBlockData;
}



void printBlocks()
{
    pthread_mutex_lock(&memoryMutex);
    int curBlock = FIRST_BLOCK_PTR;
    do
    {
        printf(
            "BlockPtr: %d\nBlock MemoryPtr: %d\nBlock Size: %d\nBlock Free: %d\nBlock Next: %d\n\n",
            curBlock,
            _getBlockMemoryPtr(curBlock),
            _getBlockSize(curBlock),
            _getBlockFree(curBlock),
            _getBlockNext(curBlock)
        );

        curBlock = _getBlockNext(curBlock);
    }while(curBlock != -1);
    pthread_mutex_unlock(&memoryMutex);

}


#pragma endregion


#pragma region Memory Handling Declarations

    #define int_t_size 1
    #define char_t_size 1
    #define byte_size 1

    #pragma region Simple Types

        int createInteger();
        int createIntegerInit(int init);
        void writeInteger(int blockPtr, int value);
        int getInteger(int blockPtr);

        
        int createChar();
        int createCharInit(char init);
        void writeChar(int blockPtr, char value);
        char getChar(int blockPtr);


    #pragma endregion

    #pragma region Arrays

        int createIntegerArray(int size);
        void writeIntegerInArray(int blockPtr, int index, int value);
        int getIntegerInArray(int blockPtr, int index);

        int createCharArray(int size);
        int createCharArrayInit(int size, char string[]);
        void writeCharInArray(int blockPtr, int index, char value);
        char getCharInArray(int blockPtr, int index);

    #pragma endregion

    

#pragma endregion

#pragma region Memory Handling Definitions

    int createInteger()
    {
        return allocateMemory(int_t_size);
    }
    int createIntegerInit(int init)
    {
        int ptr = createInteger();
        writeInteger(ptr, init);
        return ptr;
    }
    void writeInteger(int blockPtr, int value)
    {
        _lockMemory;
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        *loc = value;
        _unlockMemory;
    }
    int getInteger(int blockPtr)
    {
        int *loc = (int *)(PROGRAM_MEMORY + getBlockMemoryPtr(blockPtr));
        return *loc;
    }

    int createChar()
    {
        return allocateMemory(char_t_size);
    }
    int createCharInit(char init)
    {
        int ptr = createChar();
        writeChar(ptr, init);
        return ptr;
    }
    void writeChar(int blockPtr, char value)
    {
        _lockMemory;
        char *loc = (char *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        *loc = value;
        _unlockMemory;
    }
    char getChar(int blockPtr)
    {
        char *loc = (char *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        return *loc;
    }

    
    int createIntegerArray(int size)
    {
        return allocateMemory(int_t_size*size);
    }
    void _writeIntegerInArray(int blockPtr, int index, int value)
    {
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
    }
    void writeIntegerInArray(int blockPtr, int index, int value)
    {
        _lockMemory;
        _writeIntegerInArray(blockPtr, index, value);
        _unlockMemory;
    }  
    int getIntegerInArray(int blockPtr, int index)
    {
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        return *loc;
    }

    int createCharArray(int size)
    {
        return allocateMemory(char_t_size*size);
    }
    int createCharArrayInit(int size, char string[])
    {
        int arr = createCharArray(size);
        for (int i = 0; i < size; i++)
        {
            writeCharInArray(arr, i, string[i]);
        }

        return arr;
    }
    void _writeCharInArray(int blockPtr, int index, char value)
    {
        char *loc = (char *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
    }
    void writeCharInArray(int blockPtr, int index, char value)
    {
        _lockMemory;
        _writeCharInArray(blockPtr, index, value);
        _unlockMemory;
    }
    

    char getCharInArray(int blockPtr, int index)
    {
        char *loc = (char* )(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        return *loc;
    }

#pragma endregion

#pragma region Application Code

    #pragma region Game Objects

        int createPlayer(int pos_x, int pos_y)
        {
            int arr = createIntegerArray(2);
            writeIntegerInArray(arr, 0, pos_x);
            writeIntegerInArray(arr, 1, pos_y);

            return arr;
        }
        void writePlayer(int blockPtr, int pos_x, int pos_y)
        {
            writeIntegerInArray(blockPtr, 0, pos_x);
            writeIntegerInArray(blockPtr, 1, pos_y);
        }
        int getPlayer_x(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 0);
        }
        int getPlayer_y(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 1);
        }

        int createBullet(int pos_x, int pos_y)
        {
            int bullet = createIntegerArray(2);
            writeIntegerInArray(bullet, 0, pos_x);
            writeIntegerInArray(bullet, 1, pos_y);
            return bullet;
        }
        void writeBullet(int blockPtr, int pos_x, int pos_y)
        {
            writeIntegerInArray(blockPtr, 0, pos_x);
            writeIntegerInArray(blockPtr, 1, pos_y);
        }
        int getBullet_x(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 0);
        }
        int getBullet_y(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 1);
        }


        void writeAlien(int blockPtr, int pos_x, int pos_y, int direction, int life)
        {
            writeIntegerInArray(blockPtr, 0, pos_x);
            writeIntegerInArray(blockPtr, 1, pos_y);
            writeIntegerInArray(blockPtr, 2, life);
            writeIntegerInArray(blockPtr, 3, direction);
        }
        int createAlien(int pos_x, int pos_y, int direction, int life)
        {
            int alien = createIntegerArray(4);
            writeAlien(alien, pos_x, pos_y, direction, life);

            return alien;
        }
        int getAlien_x(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 0);
        }
        int getAlien_y(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 1);
        }
        int getAlien_life(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 2);
        }
        int getAlien_dir(int blockPtr)
        {
            return getIntegerInArray(blockPtr, 3);
        }

        // returns blockPointer to new score track
        int createScoreTrack(char* nameStr, int score);
        void writeScoreTrack(int blockPtr, char* nameStr, int score);

        char* getScoreName(int blockPtr);
        int getScore(int blockPtr);


    #pragma endregion

    #pragma region Global Variables

int maxAliens_int;
int maxBullets_int;

int alienXSize_int;
int alienYSize_int;

int alienSpeed_int;

int fullLifeAlienBmp_char_arr;
int mediumLifeAlienBmp_char_arr;
int lowLifeAlienBmp_char_arr;

int alienBmpSize_int;

int laneSize_int;

int player;
int playerSpeed_int;

int currentScore_int;

int alienPtrs_alien_arr;
int alienCounter_int;
int alienRespawnTimes_arr;

int bulletCounter_int;
int bulletPtrs_bullet_arr;
int bulletSpeed_int;

int scoreOnHit_int;
int scoreOnKill_int;

int newAlienObject;

int respawnTimerMax_int;

bool gameOver;

pthread_mutex_t alienMutex = PTHREAD_MUTEX_INITIALIZER;
#define lockAlienMutex pthread_mutex_lock(&alienMutex)
#define unlockAlienMutex pthread_mutex_unlock(&alienMutex)

pthread_mutex_t bulletMutex = PTHREAD_MUTEX_INITIALIZER;
#define lockBulletMutex pthread_mutex_lock(&bulletMutex)
#define unlockBulletMutex pthread_mutex_unlock(&bulletMutex)


    #pragma endregion

    #pragma region Utils
void removeFromIntArray(int int_arr, int index, int size)
{
    for (int i = index; i < size - 1; i++)
    {
        writeIntegerInArray(int_arr, i, getIntegerInArray(int_arr, i + 1));
    }
}

void deleteFromPtrArray(int ptr_arr, int index, int size)
{
    int toDelete = getIntegerInArray(ptr_arr, index);
    deleteMem(toDelete);
    for (int i = index; i < size - 1; i++)
    {
        writeIntegerInArray(ptr_arr, i, getIntegerInArray(ptr_arr, i + 1));
    }   
}

int randint(int start, int endNonInclusive)
{
    if (endNonInclusive == 0)
        exit(1);
    
    return start + rand()%endNonInclusive;
}

void fillIntArrayWithNull(int int_arr, int size)
{
    for (int i = 0; i < size; i ++)
        writeIntegerInArray(int_arr, i, NULLPTR);
}

void writeIntArray(int arr, int size)
{
    for (int i = 0; i < size; i++)
    {   
        int item = getIntegerInArray(arr, i);
        printf("%d, ", item);
    }
}
void writeCharArray(int arr, int size)
{
    for (int i = 0; i < size; i++)
    {   
        int item = getCharInArray(arr, i);
        printf("%c, ", item);
    }
}



    #pragma endregion

    #pragma region Functions

void drawPlayer()
{
    attron(COLOR_PAIR(BLUE_COLOR_PAIR));
    mvprintw(
        getPlayer_y(player),
        getPlayer_x(player) + 2,
        " "
    );

    mvprintw(
        getPlayer_y(player) + 1,
        getPlayer_x(player) + 1,
        "   "
    );

    mvprintw(
        getPlayer_y(player) + 2,
        getPlayer_x(player),
        "     "
    );
    attroff(COLOR_PAIR(BLUE_COLOR_PAIR));
}

void _drawBullet(int bulletPtr);
void* drawBullets(void* arg)
{
    lockBulletMutex;
    int bulletCount = getInteger(bulletCounter_int);
    for (int i = 0; i < bulletCount; i++)
    {
        _drawBullet(getIntegerInArray(bulletPtrs_bullet_arr, i));
    }
    unlockBulletMutex;

    return NULL;
}
void _drawBullet(int bulletPtr)
{
    int x = getBullet_x(bulletPtr);
    int y = getBullet_y(bulletPtr);
    attron(COLOR_PAIR(RED_COLOR_PAIR));

    mvprintw(y, x, "*");
    attroff(COLOR_PAIR(RED_COLOR_PAIR));

}


void _drawBitmap(int bitmapPtr, int x, int y, int bmpSize);
void _drawAlien(int alien);
void* drawAliens(void* arg)
{
    lockAlienMutex;
    for (int i = 0; i < getInteger(alienCounter_int); i++)
        _drawAlien(getIntegerInArray(alienPtrs_alien_arr, i));
    unlockAlienMutex;

    return NULL;
}

void _drawAlien(int alien)
{
    int bmpSize = getInteger(alienBmpSize_int);
    int x = getAlien_x(alien);
    int y = getAlien_y(alien);

    switch(getAlien_life(alien)){
    case 3:
        attron(COLOR_PAIR(GREEN_COLOR_PAIR));
        _drawBitmap(fullLifeAlienBmp_char_arr, x, y, bmpSize);
        attroff(COLOR_PAIR(GREEN_COLOR_PAIR));
        break;
    case 2:
        attron(COLOR_PAIR(YELLOW_COLOR_PAIR));
        _drawBitmap(mediumLifeAlienBmp_char_arr, x, y, bmpSize);
        attroff(COLOR_PAIR(YELLOW_COLOR_PAIR));
        break;
    case 1:
        attron(COLOR_PAIR(RED_COLOR_PAIR));
        _drawBitmap(lowLifeAlienBmp_char_arr, x, y, bmpSize);
        attroff(COLOR_PAIR(RED_COLOR_PAIR));
        break;

    default:
        exit(2);
    }
}
void _drawBitmap(int bitmapPtr, int x, int y, int bmpSize)
{   
    move(y, x);
    for (int i = 0, yCurrPos = 0, xCurrPos = 0; i < bmpSize; i++)
    {
        char currChar = getCharInArray(bitmapPtr, i);
        if (currChar == '\n')
        {
            move(y + ++yCurrPos, x);
            xCurrPos = 0;
        }
        else if (currChar == '_')
        {
            move(y + yCurrPos, x + ++xCurrPos);
        }
        else
        {
            addch(currChar);
            xCurrPos++;
        }
    }
}

void* drawInformation(void* arg)
{
    mvprintw(
        0, 
        infoPosition_x, 
        "Score: %d", getInteger(currentScore_int)
    );

    for (int i = 0; i < MAX_HIGHSCORES; i++) {
        mvprintw(
            3 + i,
            infoPosition_x,
            "Player %d: %s - %d ",
            i + 1,
            highscores[i].name,
            highscores[i].score
        );
    }

    mvprintw(
        15,
        infoPosition_x,
        "Respawn Times:"
    );

    for (int i = 0; i < getInteger(maxAliens_int); i++) {
        mvprintw(
            17 + i,
            infoPosition_x,
            "Alien %d: %d",
                i + 1,
                getIntegerInArray(alienRespawnTimes_arr, i)
        );
    }

    return NULL;  
}


bool _bulletIsOutOfBounds(int bulletPtr);
void _moveBulletUp(int bulletPtr, int offset);
void* moveBullets(void* arg)
{
    lockBulletMutex;
    for (int i = 0; i < getInteger(bulletCounter_int); i++)
    {
        int bulletPtr = getIntegerInArray(bulletPtrs_bullet_arr, i);
        _moveBulletUp(bulletPtr, getInteger(bulletSpeed_int));   
        if (_bulletIsOutOfBounds(bulletPtr))
        {
            deleteFromPtrArray(bulletPtrs_bullet_arr, i, getInteger(bulletCounter_int));
            writeInteger(bulletCounter_int, getInteger(bulletCounter_int) - 1);
            i--;
        }
    }
    unlockBulletMutex;

    return NULL;
}

void _moveBulletUp(int bulletPtr, int offset)
{
    writeBullet(
        bulletPtr, 
        getBullet_x(bulletPtr), 
        getBullet_y(bulletPtr) - offset
    );
}

bool _bulletIsOutOfBounds(int bulletPtr)
{
    return getBullet_y(bulletPtr) < 0;
}

void* moveAliens(void *arg)
{
    lockAlienMutex;
    for (int i = 0; i < getInteger(alienCounter_int); i++)
    {
        int alien = getIntegerInArray(alienPtrs_alien_arr, i);
        int x = getAlien_x(alien);
        int y = getAlien_y(alien);
        int life = getAlien_life(alien);
        int laneSize = getInteger(laneSize_int);
        int alienSpeed = getInteger(alienSpeed_int);
        int direction = getAlien_dir(alien);

        int alienYSize = getInteger(alienYSize_int);
        
        if (y + alienYSize >= SCREEN_HEIGHT - SCREEN_DANGER_LOWER_BAR_HEIGHT)
        {
            gameOver = true;
            return NULL;
        }

        if (direction == LEFT_DIR)
        {
            if (x <= 0)
            {
                y += laneSize;
                direction = RIGHT_DIR;
            }
            else
            {
                x -= alienSpeed;
            }
        }
        else
        {
            if (x >= SCREEN_WIDTH - getInteger(alienXSize_int))
            {
                y += laneSize;
                direction = LEFT_DIR;
            }
            else
            {
                x += alienSpeed;
            }
        }

        writeAlien(alien, x, y, direction, life);
    }
    unlockAlienMutex;

    return NULL;
}


bool _areColliding(int alienPtr, int bulletPtr);
bool _reduceLifeTrueIfDead(int alienPtr);
void killAliensAndIncreaseScore()
{
    lockAlienMutex;
    lockBulletMutex;
    
    int alienCount = getInteger(alienCounter_int);
    int bulletCount = getInteger(bulletCounter_int);
    
    int score = getInteger(currentScore_int);
    int hitScore = getInteger(scoreOnHit_int);
    int killScore = getInteger(scoreOnKill_int);

    bool alienKilled;
    for (int i = 0; i < alienCount; i++)
    {
        int alien = getIntegerInArray(alienPtrs_alien_arr, i);
        alienKilled = false;
        for (int j = 0; j < bulletCount && !alienKilled; j++)
        {
            int bullet = getIntegerInArray(bulletPtrs_bullet_arr, j);
            if (_areColliding(alien, bullet))
            {
                bool dead = _reduceLifeTrueIfDead(alien);
                deleteFromPtrArray(bulletPtrs_bullet_arr, j, bulletCount);
                bulletCount--;
                j--;
                score += hitScore;
                if (dead)
                {
                    score += killScore;
                    deleteFromPtrArray(alienPtrs_alien_arr, i, alienCount);
                    alienCount--;
                    i--;
                    alienKilled = true;
                }
            }
        }
    }
    writeInteger(alienCounter_int, alienCount);
    writeInteger(bulletCounter_int, bulletCount);
    writeInteger(currentScore_int, score);
    unlockAlienMutex;
    unlockBulletMutex;
}
bool _areColliding(int alienPtr, int bulletPtr)
{
    int bX = getBullet_x(bulletPtr);
    int bY = getBullet_y(bulletPtr);

    int aX = getAlien_x(alienPtr);
    int aY = getAlien_y(alienPtr);

    int alienXSize = getInteger(alienXSize_int);
    int alienYSize = getInteger(alienYSize_int);

    return (
        (aX <= bX && bX < aX + alienXSize) &&
        (aY <= bY && bY < aY + alienYSize)
    );
}
bool _reduceLifeTrueIfDead(int alienPtr)
{
    int dir = getAlien_dir(alienPtr);
    int x = getAlien_x(alienPtr);
    int y = getAlien_y(alienPtr);
    int life = getAlien_life(alienPtr);
    writeAlien(alienPtr, x, y, dir, life - 1);
    return life - 1 == 0;
}

bool newAlienFull()
{
    return getIntegerInArray(newAlienObject, 0);
}
int popNewAlien()
{
    writeIntegerInArray(newAlienObject, 0, 0);
    return getIntegerInArray(newAlienObject, 1);
}
void pushNewAlien(int alienPtr)
{
    writeIntegerInArray(newAlienObject, 0, 1);
    writeIntegerInArray(newAlienObject, 1, alienPtr);
}

void _decreaseRespawnTimers();
bool _popRespawnTimer();
void _addAlienToNewAlienObject();
void* generateAliens(void* arg)
{
    while(!gameOver)
    {
        _decreaseRespawnTimers();
        if (!newAlienFull())
        {
            bool timerReached0 = _popRespawnTimer();
            if (timerReached0) _addAlienToNewAlienObject();
        }
        usleep(100000);
    }
    return NULL;
}
void _decreaseRespawnTimers()
{
    int maxAliens = getInteger(maxAliens_int);
    for (int i = 0; i < maxAliens; i++)
    {
        int currCtr = getIntegerInArray(alienRespawnTimes_arr, i);
        if (currCtr > 0)
            writeIntegerInArray(alienRespawnTimes_arr, i, currCtr - 1);
    }
}
bool _popRespawnTimer()
{
    int maxAliens = getInteger(maxAliens_int);
    int respawnTimerMax = getInteger(respawnTimerMax_int);
    for (int i = 0; i < maxAliens; i++)
    {
        int currCtr = getIntegerInArray(alienRespawnTimes_arr, i);
        if (currCtr == 0)
        {
            currCtr = respawnTimerMax; //randint(1, respawnTimerMax + 1);
            writeIntegerInArray(alienRespawnTimes_arr, i, currCtr);
            return true;
        }
    }

    return false;
}
void _addAlienToNewAlienObject()
{
    int x = 0;
    int y = 0;
    int life = randint(1, 4);
    int dir = RIGHT_DIR;

    int alien = createAlien(x, y, dir, life);
    pushNewAlien(alien);
}

    #pragma endregion

void saveGameState() {
    FILE *file = fopen("game_state.dat", "wb");
    if (file == NULL) {
        perror("No se pudo abrir el archivo para guardar el estado del juego");
        return;
    }
    _lockMemory;
    fwrite(PROGRAM_MEMORY, sizeof(int), MEMORY_LENGTH, file);
    _unlockMemory;
    fclose(file);
    printf("Estado del juego guardado con éxito\n");
}

void loadGameState() {



    FILE *file = fopen("game_state.dat", "rb");
    if (file == NULL) {
        perror("No se pudo abrir el archivo para cargar el estado del juego");
        return;
    }
    _lockMemory;
    fread(PROGRAM_MEMORY, sizeof(int), MEMORY_LENGTH, file);
    _unlockMemory;
    fclose(file);
    printf("Estado del juego cargado con éxito\n");

}

void _shootBulletIfPossible();
void* getGameInput(void* arg)
{

    int speed = getInteger(playerSpeed_int);
    while(!gameOver)
    {
        int ch = getch();
        
        switch(ch)
        {
        case 'a':
        case 'A':
        case KEY_LEFT:
            if (getPlayer_x(player) > 0) writePlayer(player, getPlayer_x(player) - speed, getPlayer_y(player));
            break;
        case 'd':
        case 'D':
        case KEY_RIGHT:
            if (getPlayer_x(player) < SCREEN_WIDTH) writePlayer(player, getPlayer_x(player) + speed, getPlayer_y(player));
            break;
        case ' ':
            lockBulletMutex;
            _shootBulletIfPossible();
            unlockBulletMutex;
            break;
        case 'q':
            gameOver = true;
            break;
        case 'g':
            saveGameState();
            break;
        }


    }
    return NULL;
}
void _shootBulletIfPossible()
{
    int bulletCount = getInteger(bulletCounter_int);
    int maxBullets = getInteger(maxBullets_int);

    if (bulletCount < maxBullets)
    {
        int x = getPlayer_x(player) + 2;
        int y = getPlayer_y(player);
        int newBullet = createBullet(x, y);
        writeIntegerInArray(bulletPtrs_bullet_arr, bulletCount,  newBullet);
        writeInteger(bulletCounter_int, bulletCount + 1);
    }
}

void initializeGame(
    int maxAliens, int maxBullets, int bulletSpeed, 
    int alienXSize, int alienYSize, 
    int laneSize, 
    char fullLifeAlienBmp[],
    char mediumLifeAlienBmp[],
    char lowLifeAlienBmp[],
    int alienSpeed,
    int scoreOnHit,
    int scoreOnKill,
    int respawnTimerMax,
    int playerSpeed)
{
    if (laneSize < alienYSize){
        printf("LANE SIZE MUST BE GREATER OR EQUAL TO ALIEN SIZE");
        exit(1);
    }

    maxAliens_int = createIntegerInit(maxAliens);
    maxBullets_int = createIntegerInit(maxBullets);
    
    alienXSize_int = createIntegerInit(alienXSize);
    alienYSize_int = createIntegerInit(alienYSize);

    alienSpeed_int = createIntegerInit(alienSpeed);

    laneSize_int = createIntegerInit(laneSize);

    int amountOfNewLinesInBitmap = getInteger(alienYSize_int) - 1;
    int bitMapSize = getInteger(alienXSize_int)*getInteger(alienYSize_int) + amountOfNewLinesInBitmap;

    fullLifeAlienBmp_char_arr = createCharArrayInit(
        bitMapSize,
        fullLifeAlienBmp
    );

    mediumLifeAlienBmp_char_arr = createCharArrayInit(
        bitMapSize,
        mediumLifeAlienBmp
    );

    lowLifeAlienBmp_char_arr = createCharArrayInit(
        bitMapSize,
        lowLifeAlienBmp
    );

    alienBmpSize_int = createIntegerInit(bitMapSize);

    player = createPlayer(SCREEN_WIDTH / 2, SCREEN_HEIGHT - SCREEN_DANGER_LOWER_BAR_HEIGHT);
    playerSpeed_int = createIntegerInit(playerSpeed);

    currentScore_int = createIntegerInit(0);

    alienCounter_int = createIntegerInit(0);    
    alienPtrs_alien_arr = createIntegerArray(getInteger(maxAliens_int));
    fillIntArrayWithNull(alienPtrs_alien_arr, getInteger(maxAliens_int));
    
    alienRespawnTimes_arr = createIntegerArray(getInteger(maxAliens_int));
    for (int i = 0; i < getInteger(maxAliens_int)/2; i++)
        writeIntegerInArray(alienRespawnTimes_arr, i, 0);
    for (int i = (getInteger(maxAliens_int) + 1)/2; i < getInteger(maxAliens_int); i++)
        writeIntegerInArray(alienRespawnTimes_arr, i, randint(1, 11));

    bulletCounter_int = createIntegerInit(0);
    bulletPtrs_bullet_arr = createIntegerArray(getInteger(maxBullets_int));
    fillIntArrayWithNull(bulletPtrs_bullet_arr, getInteger(maxBullets_int));
    bulletSpeed_int = createIntegerInit(bulletSpeed);

    scoreOnHit_int = createIntegerInit(scoreOnHit);
    scoreOnKill_int = createIntegerInit(scoreOnKill);

    newAlienObject = createIntegerArray(2);
    writeIntegerInArray(newAlienObject, 0, 0);

    respawnTimerMax_int = createIntegerInit(respawnTimerMax);

    loadHighscores();

    gameOver = false;
}

void initializeGame2(
    int maxAliens, int maxBullets, int bulletSpeed,
    int alienXSize, int alienYSize,
    int laneSize,
    char fullLifeAlienBmp[],
    char mediumLifeAlienBmp[],
    char lowLifeAlienBmp[],
    int alienSpeed,
    int scoreOnHit,
    int scoreOnKill,
    int respawnTimerMax,
    int playerSpeed)
{
    if (laneSize < alienYSize){
        printf("LANE SIZE MUST BE GREATER OR EQUAL TO ALIEN SIZE");
        exit(1);
    }
    gameOver = false;
    loadGameState();

    loadHighscores();

}

void freeGameMemory()
{

    deleteMem(maxAliens_int);
    deleteMem(maxBullets_int);
    deleteMem(alienXSize_int);
    deleteMem(alienYSize_int);

    deleteMem(alienSpeed_int);

    deleteMem(laneSize_int);

    deleteMem(fullLifeAlienBmp_char_arr);
    deleteMem(mediumLifeAlienBmp_char_arr);
    deleteMem(lowLifeAlienBmp_char_arr);

    deleteMem(alienBmpSize_int);
    
    deleteMem(player);
    deleteMem(playerSpeed_int);
    
    deleteMem(currentScore_int);
    
    for (int i = 0; i < getInteger(alienCounter_int); i++)
    {
        int alienPtr = getIntegerInArray(alienPtrs_alien_arr, i);
        deleteMem(alienPtr);
    }
    deleteMem(alienPtrs_alien_arr);
    deleteMem(alienCounter_int);
    deleteMem(alienRespawnTimes_arr);

    for (int i = 0; i < getInteger(bulletCounter_int); i++)
    {
        int bulletPtr = getIntegerInArray(bulletPtrs_bullet_arr, i);
        deleteMem(bulletPtr);
    }
    deleteMem(bulletCounter_int);
    deleteMem(bulletSpeed_int);

    deleteMem(scoreOnHit_int);
    deleteMem(scoreOnKill_int);

    deleteMem(respawnTimerMax_int);

    deleteMem(newAlienObject);

}

void newAlien()
{
    lockAlienMutex;
    int newAlienPtr = createAlien(0, 0, RIGHT_DIR, 3);
    writeIntegerInArray(alienPtrs_alien_arr, getInteger(alienCounter_int), newAlienPtr);
    writeInteger(alienCounter_int, getInteger(alienCounter_int) + 1);
    unlockAlienMutex;
}
bool noAlienInTopLeftCorner()
{
    lockAlienMutex;

    for (int i = 0; i < getInteger(alienCounter_int); i++)
    {
        int alienPtr = getIntegerInArray(alienPtrs_alien_arr, i);
        int x = getAlien_x(alienPtr);
        if (0 <= x && x < getInteger(alienXSize_int) + 2)
        {
            unlockAlienMutex;
            return false;
        }
    }
    unlockAlienMutex;
    return true;
}

void _moveObjects();
void _drawObjects();
void newGame(){
    char fullLifeBitMap[] = "     \n     \n     \n     \n     ";
    char mediumLifeBitMap[] = "     \n     \n     \n     \n__ __";
    char lowLifeBitMap[] = "     \n     \n__ __\n__ __\n__ __";
    initializeGame(
        20, 10, 1, 5, 5, 5,
        fullLifeBitMap, 
        mediumLifeBitMap,
        lowLifeBitMap,
        1, 10, 10, 10, 1
    );

    pthread_t inputThread, alienGenThread;
    pthread_create(&inputThread, NULL, getGameInput, NULL);
    pthread_create(&alienGenThread, NULL, generateAliens, NULL);

    while(!gameOver)
    {
        clear();
        drawPlayer();
        if (newAlienFull() && getInteger(alienCounter_int) < getInteger(maxAliens_int) && noAlienInTopLeftCorner())
        {
            newAlien();
        }
        _moveObjects();
        killAliensAndIncreaseScore();
        _drawObjects();
        refresh();
        usleep(100000);
    }

    clear();
    // Print each line of "GAME OVER"
    attron(A_BOLD); // Turn on bold attribute
    attron(COLOR_PAIR(RED_COLOR_PAIR));
    mvprintw(SCREEN_HEIGHT / 2 - 1, (SCREEN_WIDTH - 10) / 2, "GAME OVER");
    mvprintw(SCREEN_HEIGHT / 2 + 1, (SCREEN_WIDTH - 10) / 2, "Score: %d", getInteger(currentScore_int));
    mvprintw(SCREEN_HEIGHT / 2 + 3, (SCREEN_WIDTH - 10) / 2, "PRESS ANY KEY TO GO BACK", getInteger(currentScore_int));
    attroff(A_BOLD); // Turn off bold attribute
    attroff(COLOR_PAIR(RED_COLOR_PAIR));
    refresh();


    scanf("hello");

    getchar();

    updateHighscores(getInteger(currentScore_int));

    freeGameMemory();

}

void loadGame() {
    char fullLifeBitMap[] = "     \n     \n     \n     \n     ";
    char mediumLifeBitMap[] = "     \n     \n     \n     \n__ __";
    char lowLifeBitMap[] = "     \n     \n__ __\n__ __\n__ __";
    initializeGame2(
        20, 10, 1, 5, 5, 5,
        fullLifeBitMap,
        mediumLifeBitMap,
        lowLifeBitMap,
        1, 10, 10, 10, 1
    );



    pthread_t inputThread, alienGenThread;
    pthread_create(&inputThread, NULL, getGameInput, NULL);
    pthread_create(&alienGenThread, NULL, generateAliens, NULL);


    loadGameState();

    while(!gameOver) {
        clear();
        drawPlayer();
        if (newAlienFull() && getInteger(alienCounter_int) < getInteger(maxAliens_int) && noAlienInTopLeftCorner()) {
            newAlien();
        }
        _moveObjects();
        killAliensAndIncreaseScore();
        _drawObjects();
        refresh();
        usleep(100000);
    }

    clear();
    attron(A_BOLD);
    attron(COLOR_PAIR(RED_COLOR_PAIR));
    mvprintw(SCREEN_HEIGHT / 2 - 1, (SCREEN_WIDTH - 10) / 2, "GAME OVER");
    mvprintw(SCREEN_HEIGHT / 2 + 1, (SCREEN_WIDTH - 10) / 2, "Score: %d", getInteger(currentScore_int));
    mvprintw(SCREEN_HEIGHT / 2 + 3, (SCREEN_WIDTH - 10) / 2, "PRESS ANY KEY TO GO BACK");
    attroff(A_BOLD);
    attroff(COLOR_PAIR(RED_COLOR_PAIR));
    refresh();

    getchar();

    updateHighscores(getInteger(currentScore_int));

    freeGameMemory();


}

void _drawObjects()
{
    pthread_t alienDrawer, bulletDrawer, infoDrawer;
    // pthread_create(&alienDrawer, NULL, drawAliens, NULL);
    // pthread_create(&bulletDrawer, NULL, drawBullets, NULL); 
    // pthread_create(&infoDrawer, NULL, drawInformation, NULL); 

    // pthread_join(alienDrawer, NULL);
    // pthread_join(bulletDrawer, NULL);
    // pthread_join(infoDrawer, NULL);
    drawAliens(NULL);
    drawBullets(NULL);
    drawInformation(NULL);
}
void _moveObjects()
{
    pthread_t alienMover, bulletMover;
    pthread_create(&alienMover, NULL, moveAliens, NULL);
    pthread_create(&bulletMover, NULL, moveBullets, NULL); 

    pthread_join(alienMover, NULL);
    pthread_join(bulletMover, NULL);

}

    #pragma region Menus
void destroyCurses();
void mainMenu()
{
    int cursor = 0;
    attron(A_BOLD);
    attron(COLOR_PAIR(GREEN_COLOR_PAIR));
    while(true)
    {
        attron(A_BOLD);
        attron(COLOR_PAIR(GREEN_COLOR_PAIR));
        clear();
        mvprintw(SCREEN_HEIGHT / 2 - 1, (SCREEN_WIDTH - 10) / 2, "MAIN MENU");
        
        mvprintw(SCREEN_HEIGHT / 2, (SCREEN_WIDTH - 10) / 2, "1. Play");
        if (cursor == 0)
            mvprintw(SCREEN_HEIGHT / 2, (SCREEN_WIDTH - 10) / 2 - 2, ">");
        
        mvprintw(SCREEN_HEIGHT / 2 + 1, (SCREEN_WIDTH - 10) / 2, "2. Load");
        if (cursor == 1)
            mvprintw(SCREEN_HEIGHT / 2 + 1, (SCREEN_WIDTH - 10) / 2 - 2, ">");
        
        mvprintw(SCREEN_HEIGHT / 2 + 2, (SCREEN_WIDTH - 10) / 2, "3. Exit");
        if (cursor == 2)
            mvprintw(SCREEN_HEIGHT / 2 + 2, (SCREEN_WIDTH - 10) / 2 - 2, ">");

        int ch = getch();
        if(ch == KEY_UP)
        {
            cursor--;
            if(cursor < 0)
                cursor = 2;
        }
        else if(ch == KEY_DOWN)
        {
            cursor++;
            if(cursor > 2)
                cursor = 0;
        }
        else if(ch == ' ')
        {
            if(cursor == 0)
                newGame();
            else if(cursor == 1)
            {
                loadGame();
            }
            else if(cursor == 2)
            {
                destroyCurses();
                exit(0);
            }
                
        }
        refresh();

    }
}
    #pragma endregion

#pragma endregion

void runTests();

void initCurses()
{
    initscr();
    if (has_colors() == FALSE)
    {
        printf("NO COLORS");
        exit(1);
    }
    start_color();
    init_pair(GREEN_COLOR_PAIR, COLOR_GREEN, COLOR_GREEN);
    init_pair(BLUE_COLOR_PAIR, COLOR_BLUE, COLOR_BLUE);
    init_pair(RED_COLOR_PAIR, COLOR_RED, COLOR_RED);
    init_pair(YELLOW_COLOR_PAIR, COLOR_YELLOW, COLOR_YELLOW);

    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
}

void destroyCurses()
{
    endwin();
}

int main() {
    
    initMemory();
    srand(time(NULL)); // Seed the random number generator
    initCurses();


    mainMenu();
    
    
    clear();
    refresh();
    destroyCurses();


    return 0;
}

#pragma region Tests

void assertEquals(int a, int b, char* testName);
void assertEqualsSilent(int a, int b, char* testName);
void runMemoryAllocationTests();
void runMemoryReadingTests();
void runMemoryHandlingTests();
void storeNoise(int objectAmount);
void storeAndDeleteNoise();




void runTests()
{
    runMemoryAllocationTests();
    runMemoryHandlingTests();
}

    #pragma region Malloc

void mallocTest_1();
void mallocTest_2();

void runMemoryAllocationTests()
{
    printf("\nRUNNING MEMORY ALLOCATION TESTS:\n");
 
    mallocTest_1();
    mallocTest_2();


    printf("PASSED\n");
}
void mallocTest_1()
{
    initMemory();
    int a = allocateMemory(10);
    int b = allocateMemory(20);
    int c = allocateMemory(400);
    int d = allocateMemory(123);

    deleteMem(b);
    deleteMem(c);

    assertEquals(_getBlockFree(b), 1, "mallocTest_1");
}
void mallocTest_2()
{
    initMemory();
    storeNoise(50);

    assertEquals(50, blockCounter - 1, "mallocTest_2");
}

    #pragma endregion

    #pragma region Memory Handling

void memHandlingTest_1();
void memHandlingTest_2();
void memHandlingTest_3();
void memHandlingTest_4();
void memHandlingTest_5();
void memHandlingTest_6();
void memHandlingTest_7();
void memHandlingTest_8();
void memHandlingTest_9();
void memHandlingTest_10();
void memHandlingTest_11();



void runMemoryHandlingTests()
{
    printf("\nRUNING MEMORY HANDLING TESTS:\n");

    memHandlingTest_1();
    // memHandlingTest_2(); // FREEING ALREADY FREE MEMORY
    memHandlingTest_3(); 
    memHandlingTest_4();
    memHandlingTest_5();
    memHandlingTest_6();
    memHandlingTest_7();
    memHandlingTest_8();
    memHandlingTest_9();
    memHandlingTest_10();
    memHandlingTest_11();


    printf("PASSED\n");
}

void memHandlingTest_1()
{
    initMemory();

    printf("memHandlingTest_1, ERROR?...");

    int a = createIntegerInit(20);
    int b = createIntegerInit(40);
    int c = createIntegerInit(33);
    int d_arr = createIntegerArray(200);
    int e = createIntegerInit(10);
    int f = createIntegerArray(400);


    deleteMem(a);
    deleteMem(b);
    deleteMem(c);
    deleteMem(d_arr);
    deleteMem(e);
    deleteMem(f);

    printf("NO\n");
}
void memHandlingTest_2()
{
    initMemory();

    printf("memHandlingTest_2, ERROR?...");


    initializeGame(10, 10, 10, 4, 1, 10, "test", "test", "test", 200, 10, 10, 10, 2); 
    freeGameMemory();

    printf("NO\n");

}
void memHandlingTest_3()
{
    initMemory();
    storeNoise(100);
    int a = createCharArrayInit(10, "0123456789");
    storeNoise(100);
    
    assertEquals('2', getCharInArray(a, 2), "memHandlingTest_3");
}
void memHandlingTest_4()
{
    initMemory();
    printf("memHandlingTest_4, ERROR?...");


    maxAliens_int = createIntegerInit(10);
    maxBullets_int = createIntegerInit(10);
    
    alienXSize_int = createIntegerInit(3);
    alienYSize_int = createIntegerInit(3);

    deleteMem(maxAliens_int);
    deleteMem(maxBullets_int);
    deleteMem(alienXSize_int);
    deleteMem(alienYSize_int);

    printf("NO\n");
}
void memHandlingTest_5()
{
    initMemory();
    printf("memHandlingTest_5, ERROR?...");

    storeAndDeleteNoise();

    printf("NO\n");
}
void memHandlingTest_6()
{
    initMemory();
    storeNoise(100);
    int a = createIntegerArray(300);
    writeIntegerInArray(a, 213, 77);
    storeNoise(100);

    assertEquals(getIntegerInArray(a, 213), 77, "memHandlingTest_6");
}
void memHandlingTest_7()
{
    initMemory();
    storeAndDeleteNoise();
    storeNoise(200);
    storeAndDeleteNoise();
    
    int a = createCharArrayInit(4, "12\n4");
    
    storeAndDeleteNoise();
    storeNoise(20);
    storeAndDeleteNoise();

    assertEquals(getCharInArray(a, 2), '\n', "memHandlingTest_7");
}
void memHandlingTest_8()
{
    initMemory();

    initializeGame(
        10 /*maxAliens*/,
        10 /*maxBullets*/ ,
        10 /*bulletSpeed*/, 
        4 /*alienXSize*/, 
        2 /*alienYSize*/, 
        10 /*lane Size*/, 
        "test\ntest" /*alien bitmaps:*/, 
        "test\ntest", 
        "test\ntest", 
        200 /*alienSpeed*/, 
        10 /*scoreOnHit*/, 
        10 /*scoreOnKill*/, 
        10 /*respawnTimerMax*/, 
        2 /*playerSpeed*/
    ); 

    assertEqualsSilent(10, getInteger(maxAliens_int), "memHandlingTest_8, maxAliens");
    
    assertEqualsSilent(0, getInteger(bulletCounter_int), "memHandlingTest_8, bulletCounter_int");
    assertEqualsSilent(10, getInteger(maxBullets_int), "memHandlingTest_8, maxBullets_int");
    assertEqualsSilent(10, getInteger(bulletSpeed_int), "memHandlingTest_8, bulletSpeed_int");
    
    assertEqualsSilent(4, getInteger(alienXSize_int), "memHandlingTest_8, alienYSize_int");
    assertEqualsSilent(2, getInteger(alienYSize_int), "memHandlingTest_8, alienXSize_int");

    assertEqualsSilent(10, getInteger(laneSize_int), "memHandlingTest_8, laneSize_int");

    assertEqualsSilent(200, getInteger(alienSpeed_int), "memHandlingTest_8, alienSpeed_int");
    assertEqualsSilent(10, getInteger(scoreOnHit_int), "memHandlingTest_8, scoreOnHit_int");
    assertEqualsSilent(10, getInteger(scoreOnKill_int), "memHandlingTest_8, scoreOnKill_int");
    assertEqualsSilent(10, getInteger(respawnTimerMax_int), "memHandlingTest_8, respawnTimerMax_int");
    assertEqualsSilent(2, getInteger(playerSpeed_int), "memHandlingTest_8, playerSpeed_int");

    // printf("%d\n", getInteger(alienCounter_int));
    // printf("%d\n", _getBlockFree(alienPtrs_alien_arr));
    // for (int i = 0; i < getInteger(alienCounter_int); i++)
    // {
    //     int alienPtr = getIntegerInArray(alienPtrs_alien_arr, i);
    //     deleteMem(alienPtr);
    //     printf("alienPtr %d Deleted\n", i);
    // }
    // deleteMem(alienPtrs_alien_arr);

    printf("memHandlingTest_8 PASSED?\n");
}
void memHandlingTest_9()
{
    initMemory();
    printf("memHandlingTest_9 ERROR?...");
    #define ARRSIZ 200

    int ptrs = createIntegerArray(ARRSIZ);

    for (int i = 0; i < ARRSIZ; i++)
    {
        int ptr = createAlien(randint(1, 10), randint(1, 10), randint(0, 2), randint(1, 5));
        writeIntegerInArray(ptrs, i, ptr);
    }
    for (int i = 0; i < ARRSIZ; i++)
    {
        int ptr = getIntegerInArray(ptrs, i);
        deleteMem(ptr);
    }
    deleteMem(ptrs);

    printf("NO\n");
}
void memHandlingTest_10()
{
    initMemory();

    storeNoise(1000);
    storeAndDeleteNoise();

    maxAliens_int = createIntegerInit(10);
    maxBullets_int = createIntegerInit(10);
    
    alienXSize_int = createIntegerInit(2);
    alienYSize_int = createIntegerInit(2);

    alienSpeed_int = createIntegerInit(10);

    laneSize_int = createIntegerInit(10);

    int amountOfNewLinesInBitmap = getInteger(alienYSize_int) - 1;
    int bitMapSize = getInteger(alienXSize_int)*getInteger(alienYSize_int) + amountOfNewLinesInBitmap;

    fullLifeAlienBmp_char_arr = createCharArrayInit(
        bitMapSize,
        "ts\nts"
    );

    mediumLifeAlienBmp_char_arr = createCharArrayInit(
        bitMapSize,
        "ts\nts"
    );

    lowLifeAlienBmp_char_arr = createCharArrayInit(
        bitMapSize,
        "ts\nts"
    );

    alienBmpSize_int = createIntegerInit(bitMapSize);

    player = createPlayer(SCREEN_WIDTH / 2, SCREEN_HEIGHT - SCREEN_DANGER_LOWER_BAR_HEIGHT);
    playerSpeed_int = createIntegerInit(10);

    currentScore_int = createIntegerInit(0);
    
    alienCounter_int = createIntegerInit(0);
    alienPtrs_alien_arr = createIntegerArray(getInteger(maxAliens_int));
    fillIntArrayWithNull(alienPtrs_alien_arr, getInteger(maxAliens_int));

    alienRespawnTimes_arr = createIntegerArray(getInteger(maxAliens_int));
    for (int i = 0; i < getInteger(maxAliens_int)/2; i++)
        writeIntegerInArray(alienRespawnTimes_arr, i, 0);
    for (int i = (getInteger(maxAliens_int) + 1)/2; i < getInteger(maxAliens_int); i++)
        writeIntegerInArray(alienRespawnTimes_arr, i, randint(1, 11));

    bulletCounter_int = createIntegerInit(0);
    bulletPtrs_bullet_arr = createIntegerArray(getInteger(maxBullets_int));
    fillIntArrayWithNull(bulletPtrs_bullet_arr, getInteger(maxBullets_int));
    bulletSpeed_int = createIntegerInit(10);

    scoreOnHit_int = createIntegerInit(10);
    scoreOnKill_int = createIntegerInit(10);

    newAlienObject = createIntegerArray(2);
    writeIntegerInArray(newAlienObject, 0, 0);

    respawnTimerMax_int = createIntegerInit(10);


    assertEquals(0, _getBlockFree(alienPtrs_alien_arr), "memHandlingTest_10");

}
void memHandlingTest_11()
{
    initMemory();
    initializeGame(
        10 /*maxAliens*/,
        10 /*maxBullets*/ ,
        10 /*bulletSpeed*/, 
        4 /*alienXSize*/, 
        2 /*alienYSize*/, 
        10 /*lane Size*/, 
        "test\ntest" /*alien bitmaps:*/, 
        "test\ntest", 
        "test\ntest", 
        200 /*alienSpeed*/, 
        10 /*scoreOnHit*/, 
        10 /*scoreOnKill*/, 
        10 /*respawnTimerMax*/, 
        2 /*playerSpeed*/
    ); 

    assertEquals(0, _getBlockFree(alienPtrs_alien_arr), "memHandlingTest_11");
}



    #pragma endregion


    #pragma region Utils

void assertEqualsSilent(int expected, int obtained, char* testName)
{
    if (expected != obtained)
    {
        printf("\n ******************** \n%s FAILED:\nOBTAINED: %d\nEXPECTED %d\n ******************** \n", testName, obtained, expected);
    }

}

void assertEquals(int expected, int obtained, char* testName)
{
    if (expected != obtained)
    {
        printf("\n ******************** \n%s FAILED:\nOBTAINED: %d\nEXPECTED %d\n ******************** \n", testName, obtained, expected);
    }
    else
        printf("%s PASSED\n", testName);

}

void storeNoise(int objectAmount)
{
    for (int i = 0; i < objectAmount; i++)
    {
        int r = randint(0, 10);
        if (r == 0)
        {
            createIntegerArray(randint(1, 101));
        }
        else
        {
            createIntegerInit(400);
        }
    }
}
void storeAndDeleteNoise()
{
    #define ARRSIZ 2000
    int ptrs[ARRSIZ];
    
    for (int i = 0; i < ARRSIZ; i++)
    {

        int r = randint(0, 2);
        if (r == 0)
        {
            ptrs[i] = createIntegerArray(randint(1, 101));
        }
        else
        {            
            ptrs[i] = createIntegerInit(randint(1, 10000));
        }
    }  



    for(int i = 0; i < ARRSIZ; i++)
    {
        deleteMem(ptrs[i]);
    }
}

    #pragma endregion

#pragma endregion