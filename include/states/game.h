#include "includes.h"

void gameInit();
void gameEnter();
void gameExit();
void gameDrawTask(void* data);

struct gameStartInfo
{
    uint8_t isMaster;
    uint8_t mode;
    uint8_t level;
    char name[10];
};

extern QueueHandle_t game_start_queue;