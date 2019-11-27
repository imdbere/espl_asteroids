#include "states/main_menu.h"
#include "includes.h"

void mainMenuInit()
{

}

void mainMenuEnter()
{

}

void mainMenuExit()
{

}

void mainMenuRun()
{
    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(White);
		gdispDrawBox(100, 100, 100, 100, Black);
    }
}