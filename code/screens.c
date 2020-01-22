#include "screens.h"

void nextLevel(struct player *player, uint8_t gameMode)
{
    struct changeScreenData changeScreen = {{0}};
    struct gameStartInfo gameStart = {{0}};

    player->level++;
    changeScreen.msWaitingTime = COUNTDOWN_NEXT_LEVEL * 1000;
    changeScreen.nextState = gameStateId;
    changeScreen.showCountdown = 1;
    gameStart.mode = gameMode;
    gameStart.level = player->level;
    sprintf(gameStart.name, player->name);
    sprintf(changeScreen.Title, "Level %i", player->level);
    xQueueSend(levelChange_queue, &changeScreen, 0);
    xQueueSend(game_start_queue, &gameStart, 0);
    xQueueSend(state_queue, &levelChangeScreenId, 0);
}

void winGame(struct player *player, uint8_t gameMode)
{
    struct changeScreenData changeScreen = {{0}};
    struct gameStartInfo gameStart = {{0}};

    changeScreen.msWaitingTime = 2000;
    changeScreen.nextState = mainMenuStateId;
    changeScreen.showCountdown = 0;
    sprintf(changeScreen.Title, "You Win!");
    sprintf(changeScreen.Subtext, "Score: %i", player->score);

    struct userScore userScore = {{0}};
    userScore.gameMode = gameMode;
    userScore.score = player->score;
    sprintf(userScore.name, player->name);

    xQueueSend(levelChange_queue, &changeScreen, 0);
    xQueueSend(score_queue, &userScore, 0);
    xQueueSend(state_queue, &levelChangeScreenId, 0);
}

void looseGame(struct player *player, uint8_t gameMode)
{
    struct changeScreenData changeScreen = {{0}};
    changeScreen.msWaitingTime = 2000;
    changeScreen.nextState = mainMenuStateId;
    changeScreen.showCountdown = 0;
    sprintf(changeScreen.Title, "Game over");
    sprintf(changeScreen.Subtext, "Score: %i", player->score);

    struct userScore userScore = {{0}};
    userScore.gameMode = gameMode;
    userScore.score = player->score;
    sprintf(userScore.name, player->name);

    xQueueSend(levelChange_queue, &changeScreen, 0);
    xQueueSend(score_queue, &userScore, 0);
    xQueueSend(state_queue, &levelChangeScreenId, 0);
}

void pauseGame(struct player *player, uint8_t gameMode)
{
    struct changeScreenData changeScreen = {{0}};

    changeScreen.isPauseScreen = 1;
    sprintf(changeScreen.Title, "Game Over");
    sprintf(changeScreen.Subtext, "Score: %i", player->score);
    changeScreen.nextState = mainMenuStateId;
    changeScreen.msWaitingTime = 2000;

    struct userScore userScore = {{0}};
    userScore.gameMode = gameMode;
    userScore.score = player->score;
    sprintf(userScore.name, player->name);

    xQueueSend(levelChange_queue, &changeScreen, 0);
    xQueueSend(score_queue, &userScore, 0);
    xQueueSend(state_queue, &levelChangeScreenId, 0);
}

void showDisconnected(struct player *player)
{
    struct changeScreenData changeScreen = {{0}};

    changeScreen.msWaitingTime = 2000;
    changeScreen.nextState = mainMenuStateId;
    changeScreen.showCountdown = 0;
    changeScreen.isPauseScreen = 0;
    sprintf(changeScreen.Title, "Disconnected!");
    sprintf(changeScreen.Subtext, "");

    xQueueSend(levelChange_queue, &changeScreen, 0);
    xQueueSend(score_queue, &player, 0);
    xQueueSend(state_queue, &levelChangeScreenId, 0);
}