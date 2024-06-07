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

#define sizeofBlockData (3*sizeof(int))
#define MEMORY_LENGTH 1000000

int _MEMORY[MEMORY_LENGTH];
__int8_t *PROGRAM_MEMORY = (__int8_t *)_MEMORY;
// stores the amount of blocks
#define blockCounter _MEMORY[0]
// stores a pointer to the current block for the nextFit algorithm
#define curBlockPtr _MEMORY[1]
#define GLOBAL_DATA_SIZE (sizeof(int)*2)
#define FIRST_BLOCK_PTR GLOBAL_DATA_SIZE

pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;



void initMemory();
// returns free block index in MEMORY_BLOCKS
int allocateMemory(int size);  
void freeMemory(int blockPtr);

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
    lockMemory;
    int answ = _allocateMemory(size);
    unlockMemory;

    return answ;
}

void freeMemory(int blockPtr)
{
    lockMemory;

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
            _setBlockSize(blockPtr, _getBlockSize(blockPtr) + _getBlockSize(nextBlock) + sizeofBlockData);
            _setBlockNext(blockPtr, _getBlockNext(nextBlock));
        }
        else blockPtr = _getBlockNext(blockPtr);
    }while(blockPtr != -1);


    unlockMemory;
}

#define BLOCK_SIZE 0
#define BLOCK_FREE 1
#define BLOCK_NEXT 2

int _getBlock(int blockPtr, int offset){
    __int8_t *loc = PROGRAM_MEMORY + blockPtr;
    int *infoPtr = (int *)loc;
    
    return infoPtr[blockPtr + offset];
}
int _setBlock(int blockPtr, int offset, int new){
    __int8_t *loc = PROGRAM_MEMORY + blockPtr;
    int *infoPtr = (int *)loc;
    
    infoPtr[blockPtr + offset] = new;
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
        int getIntegerInArray(int blockPtr, int index);

        void writeCharInArray(int blockPtr, int index, char value);
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
        void writeAlien(int blockPtr, int pos_x, int pos_y, __int8_t direction, int life);

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


    void _writeIntegerInArray(int blockPtr, int index, int value)
    {
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
    }
    void writeIntegerInArray(int blockPtr, int index, int value)
    {
        lockMemory;
        _writeIntegerInArray(blockPtr, index, value);
        unlockMemory;
    }  
    int getIntegerInArray(int blockPtr, int index)
    {
        int *loc = (int *)(PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        return *loc;
    }

    void _writeCharInArray(int blockPtr, int index, char value)
    {
        char *loc = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        *loc = value;
    }
    void writeCharInArray(int blockPtr, int index, char value)
    {
        lockMemory;
        _writeCharInArray(blockPtr, index, value);
        unlockMemory;
    }
    

    char getCharInArray(int blockPtr, int index)
    {
        char *loc = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        loc += index;
        return *loc;
    }


    void _writeBullet(int blockPtr, int pos_x, int pos_y)
    {
        _writeIntegerInArray(blockPtr, 0, pos_x);
        _writeIntegerInArray(blockPtr, 1, pos_y);
    }
    int createBullet(int pos_x, int pos_y)
    {
        lockMemory;
        int blockPtr = _allocateMemory(2*int_t_size);
        _writeBullet(blockPtr, pos_x, pos_y);
        unlockMemory;
        return blockPtr;
    }
    void writeBullet(int blockPtr, int pos_x, int pos_y)
    {
        lockMemory;
        _writeBullet(blockPtr, pos_x, pos_y);
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


    void _writeAlien(int blockPtr, int pos_x, int pos_y, __int8_t direction, int life)
    {
        _writeIntegerInArray(blockPtr, 0, pos_x);
        _writeIntegerInArray(blockPtr, 1, pos_y);
        _writeIntegerInArray(blockPtr, 2, life);
        __int8_t *dirPtr = (PROGRAM_MEMORY + _getBlockMemoryPtr(blockPtr));
        dirPtr = (__int8_t *)((int *)(dirPtr) + 3);
        *dirPtr = direction;
    }
    int createAlien(int pos_x, int pos_y, __int8_t direction, int life)
    {
        lockMemory;
        int blockPtr = _allocateMemory(int_t_size*3 + byte_size); 
        _writeAlien(blockPtr, pos_x, pos_y, direction, life);
        unlockMemory;

        return blockPtr;
    }
    void writeAlien(int blockPtr, int pos_x, int pos_y, __int8_t direction, int life)
    {
        lockMemory;
        _writeAlien(blockPtr, pos_x, pos_y, direction, life);
        unlockMemory;
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

    

    return 0;
}