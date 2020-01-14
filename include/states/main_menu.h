#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "includes.h"

void mainMenuInit();
void mainMenuEnter();
void mainMenuExit();
void mainMenuDrawTask(void* data);

struct userScore
{
    char name[10];
    int score;
};

struct userNameInput
{
    char name[10];
    uint8_t lastJoystickPosition;
    uint8_t charIndex;
    uint16_t cursorOffset;

};
#endif