#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
// #include <curses.h>
#include <time.h>
#include <string.h>

#define lockMemory pthread_mutex_lock(&memoryMutex)
#define unlockMemory pthread_mutex_unlock(&memoryMutex)

#pragma region Memory Allocation Declarations

#define sizeofBlock 4
#define BLOCK_MEMORY_LENGTH (10000*sizeofBlock + 3)
#define MEMORY_LENGTH 1000000

int *_MEMORY[MEMORY_LENGTH + BLOCK_MEMORY_LENGTH];
__int8_t *PROGRAM_MEMORY = (__int8_t *)_MEMORY;
int *MEMORY_BLOCKS = (int *)(((__int8_t *)_MEMORY) + MEMORY_LENGTH);
// stores the amount of blocks
#define blockCounter MEMORY_BLOCKS[0]
// stores a pointer to the current block for the nextFit algorithm
#define curBlockPtr MEMORY_BLOCKS[1]
// stores a pinter to the last free space 
#define lastBlockPtr MEMORY_BLOCKS[2]

#define FIRST_BLOCK_PTR 3

pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;



void initMemory();
// returns free block index in MEMORY_BLOCKS
int allocateMemory(int size);  
void freeMemory(int blockPtr);

int _getBlockSize(int blockPtr);
void _setBlockSize(int blockPtr, int newLength);

int _getBlockFree(int blockPtr);
void _setBlockFree(int blockPtr, __int8_t newFree);

int _getBlockNext(int blockPtr);
void _setBlockNext(int blockPtr, int newNext);

int getBlockMemoryPtr(int blockPtr);
int _getBlockMemoryPtr(int blockPtr);
void _setBlockMemoryPtr(int blockPtr, int newMemoryPtr);

void printBlocks();
void printBlockMemory(int length);


#pragma endregion

#pragma region Memory Allocation Definitions

void initMemory()
{
    blockCounter = 1;
    lastBlockPtr = FIRST_BLOCK_PTR + sizeofBlock;
    curBlockPtr = FIRST_BLOCK_PTR;
    _setBlockSize(curBlockPtr, MEMORY_LENGTH);
    _setBlockFree(curBlockPtr, 1);
    _setBlockNext(curBlockPtr, -1);
    _setBlockMemoryPtr(curBlockPtr, 0);
}

int allocateMemory(int size)
{
    pthread_mutex_lock(&memoryMutex);
    
    int firstBlock = curBlockPtr;
    int answ = -1;
    do
    {
        if (_getBlockFree(curBlockPtr) && _getBlockSize(curBlockPtr) >= size)
        {
            if (_getBlockSize(curBlockPtr) > size)
            {            
                blockCounter += 1;
                if (lastBlockPtr >= BLOCK_MEMORY_LENGTH - sizeofBlock)
                {
                    printf("BLOCK MEMORY OVERFLOW");
                    exit(1);
                }
                _setBlockFree(lastBlockPtr, 1);
                _setBlockSize(lastBlockPtr, _getBlockSize(curBlockPtr) - size);
                _setBlockNext(lastBlockPtr, -1);
                _setBlockMemoryPtr(lastBlockPtr, _getBlockMemoryPtr(curBlockPtr) + size);

                _setBlockNext(curBlockPtr, lastBlockPtr);
                lastBlockPtr += sizeofBlock;

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
        printf("NOT ENOUGH MEMORY");
        exit(1);
    } 

    pthread_mutex_unlock(&memoryMutex);

    return answ;
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
                blockCounter += 1;
                if (lastBlockPtr >= BLOCK_MEMORY_LENGTH - sizeofBlock)
                {
                    printf("BLOCK MEMORY OVERFLOW");
                    exit(1);
                }
                _setBlockFree(lastBlockPtr, 1);
                _setBlockSize(lastBlockPtr, _getBlockSize(curBlockPtr) - size);
                _setBlockNext(lastBlockPtr, -1);
                _setBlockMemoryPtr(lastBlockPtr, _getBlockMemoryPtr(curBlockPtr) + size);

                _setBlockNext(curBlockPtr, lastBlockPtr);
                lastBlockPtr += sizeofBlock;

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
        printf("NOT ENOUGH MEMORY");
        exit(1);
    } 
    return answ;
}

void freeMemory(int blockPtr)
{
    pthread_mutex_lock(&memoryMutex);

    if (_getBlockFree(blockPtr))
    {
        printf("FREEING ALREADY FREE MEMORY\n");
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
            _setBlockSize(blockPtr, _getBlockSize(blockPtr) + _getBlockSize(nextBlock));
            _setBlockNext(blockPtr, _getBlockNext(nextBlock));
        }
        else blockPtr = _getBlockNext(blockPtr);
    }while(blockPtr != -1);


    pthread_mutex_unlock(&memoryMutex);
}

#define BLOCK_SIZE 0
#define BLOCK_FREE 1
#define BLOCK_NEXT 2
#define BLOCK_MEMORY_PTR 3

int _getBlockSize(int blockPtr){
    return MEMORY_BLOCKS[blockPtr + BLOCK_SIZE];
}
void _setBlockSize(int blockPtr, int newLength){
    MEMORY_BLOCKS[blockPtr + BLOCK_SIZE] = newLength;
}

int _getBlockFree(int blockPtr){
    return MEMORY_BLOCKS[blockPtr + BLOCK_FREE];
}
void _setBlockFree(int blockPtr, __int8_t newFree){
    MEMORY_BLOCKS[blockPtr + BLOCK_FREE] = newFree;
}

int _getBlockNext(int blockPtr){
    return MEMORY_BLOCKS[blockPtr + BLOCK_NEXT];
}
void _setBlockNext(int blockPtr, int newNext){
    MEMORY_BLOCKS[blockPtr + BLOCK_NEXT] = newNext;
}

int getBlockMemoryPtr(int blockPtr){
    pthread_mutex_lock(&memoryMutex);
    int answ = _getBlockMemoryPtr(blockPtr);
    pthread_mutex_unlock(&memoryMutex);
    return answ;
}
int _getBlockMemoryPtr(int blockPtr){
    return MEMORY_BLOCKS[blockPtr + BLOCK_MEMORY_PTR];
}
void _setBlockMemoryPtr(int blockPtr, int newMemoryPtr){
    MEMORY_BLOCKS[blockPtr + BLOCK_MEMORY_PTR] = newMemoryPtr;
}


void printBlocks()
{
    pthread_mutex_lock(&memoryMutex);
    int curBlock = FIRST_BLOCK_PTR;
    do
    {
        printf(
            "Block Ptr: %d\nBlock Size: %d\nBlock Free: %d\nBlock Next: %d\n\n",
            _getBlockMemoryPtr(curBlock),
            _getBlockSize(curBlock),
            _getBlockFree(curBlock),
            _getBlockNext(curBlock)
        );

        curBlock = _getBlockNext(curBlock);
    }while(curBlock != -1);
    pthread_mutex_unlock(&memoryMutex);

}

void printBlockMemory(int length)
{
    for (int i = 0; i < length; i++)
        printf("%d, ", MEMORY_BLOCKS[i]);
}



#pragma endregion


#pragma region Memory Handling Declarations

    #define int_t_size sizeof(int)
    #define char_t_size sizeof(char)
    #define byte_size sizeof(__int8_t)

    #pragma region Simple Types

        void writeInteger(int blockPtr, int value);
        int getInteger(int blockPtr);

        void writeChar(int blockPtr, char value);
        char getChar(int blockPtr);

    #pragma endregion

    #pragma region Arrays

        void writeIntegerInArray(int blockPtr, int index, int value);
        void _writeIntegerInArray(int blockPtr, int index, int value);
        int getIntegerInArray(int blockPtr, int index);

        void writeCharInArray(int blockPtr, int index, char value);
        void _writeCharInArray(int blockPtr, int index, char value);
        char getCharInArray(int blockPtr, int index);

    #pragma endregion

    #pragma region Game Objects

        // returns blockPointer to new Bullet
        int createBullet(int pos_x, int pos_y);
        void writeBullet(int blockPtr, int pos_x, int pos_y);

        int getBullet_x(int blockPtr);
        int getBullet_y(int blockPtr);

        // returns blockPointer to new alien
        int createAlien(int pos_x, int pos_y, __int8_t direction, int life);
        void writeAlien(int blockPtr, int pos_x, int pos_y, int direction, int life);

        int getAlien_x(int blockPtr);
        int getAlien_y(int blockPtr);
        __int8_t getAlien_dir(int blockPtr);
        int getAlien_life(int blockPtr);

        // returns blockPointer to new score track
        int createScoreTrack(char* nameStr, int score);
        void writeScoreTrack(int blockPtr, char* nameStr, int score);

        char* getScoreName(int blockPtr);
        int getScore(int blockPtr);


    #pragma endregion

    #pragma endregion

#pragma region Memory Handling Definitions

    void writeInteger(int blockPtr, int value)
    {
        lockMemory;
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        *loc = value;
        unlockMemory;
    }
    int getInteger(int blockPtr)
    {
        int *loc = (int *)(PROGRAM_MEMORY + getBlockMemoryPtr(blockPtr));
        return *loc;
    }

    void writeChar(int blockPtr, char value)
    {
        lockMemory;
        char *loc = PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr);
        *loc = value;
        unlockMemory;
    }
    char getChar(int blockPtr)
    {
        char *loc = PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr);
        return *loc;
    }


    void writeIntegerInArray(int blockPtr, int index, int value)
    {
        lockMemory;
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
        unlockMemory;
    }
    void _writeIntegerInArray(int blockPtr, int index, int value)
    {
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
    }

    int getIntegerInArray(int blockPtr, int index)
    {
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        return *loc;
    }

    void writeCharInArray(int blockPtr, int index, char value)
    {
        lockMemory;
        char *loc = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
        unlockMemory;
    }
    void _writeCharInArray(int blockPtr, int index, char value)
    {
        char *loc = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
    }

    char getCharInArray(int blockPtr, int index)
    {
        char *loc = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        return *loc;
    }


    int createBullet(int pos_x, int pos_y)
    {
        lockMemory;
        int blockPtr = _allocateMemory(2*int_t_size);
        _writeIntegerInArray(blockPtr, 0, pos_x);
        _writeIntegerInArray(blockPtr, 1, pos_y);
        unlockMemory;
        return blockPtr;
    }
    void writeBullet(int blockPtr, int pos_x, int pos_y)
    {
        lockMemory;
        _writeIntegerInArray(blockPtr, 0, pos_x);
        _writeIntegerInArray(blockPtr, 1, pos_y);
        unlockMemory;
    }
    int getBullet_x(int blockPtr)
    {
        return getIntegerInArray(blockPtr, 0);
    }
    int getBullet_y(int blockPtr)
    {
        return getIntegerInArray(blockPtr, 1);
    }


    int createAlien(int pos_x, int pos_y, __int8_t direction, int life)
    {
        lockMemory;
        int blockPtr = _allocateMemory(int_t_size*3 + byte_size); 
        _writeIntegerInArray(blockPtr, 0, pos_x);
        _writeIntegerInArray(blockPtr, 1, pos_y);
        _writeIntegerInArray(blockPtr, 2, life);
        __int8_t *dirPtr = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        dirPtr = (__int8_t *)((int *)(dirPtr) + 3);
        *dirPtr = direction;
        unlockMemory;

        return blockPtr;
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
    __int8_t getAlien_dir(int blockPtr)
    {
        __int8_t *dirPtr = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        dirPtr = (__int8_t *)((int *)(dirPtr) + 3);
        return *dirPtr;
    }

#pragma endregion

int main() {
    
    initMemory();
    int alien = createAlien(1, 2, 1, 10);

    printf("x : %d\ny : %d\nHP: %d\nd : %d",
        getAlien_x(alien),
        getAlien_y(alien),
        getAlien_life(alien),
        getAlien_dir(alien)
    );

    freeMemory(alien);

    // int arr = allocateMemory(int_t_size*10);
    // writeIntegerInArray(arr, 0, 20);
    // writeIntegerInArray(arr, 1, 30);
    // printf("%d\n", getIntegerInArray(arr, 1));

    
    return 0;
}