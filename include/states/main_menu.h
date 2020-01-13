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

#endif