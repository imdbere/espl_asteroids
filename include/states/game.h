#include "includes.h"

void gameInit();
void gameEnter();
void gameExit();
void gameDrawTask(void* data);

struct gameStartInfo
{
    uint8_t isMultiplayer;
    uint8_t isMaster;
    char name[10]
};

extern QueueHandle_t game_start_queue;