#include "includes.h"

void levelChangeScreenInit();
void levelChangeScreenEnter();
void levelChangeScreenExit();
void levelChangeScreenDraw(void* data);

extern QueueHandle_t levelChange_queue;

struct changeScreenData
{
    char Title[20];
    char Subtext[20];
    int msWaitingTime;
    uint8_t showCountdown;
    int nextState;
};
