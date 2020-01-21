#pragma once

#include "includes.h"
#include "asteroids.h"
#include "player.h"
#include "bullets.h"

extern QueueHandle_t uartHandshakeQueue;
extern QueueHandle_t uartInviteQueue;
extern QueueHandle_t uartFramePacketQueue;
extern QueueHandle_t uartGameSetupQueue;

#define MAX_PACKET_LENGTH 800
struct uartFramePacket {
    pointf playerPosition;
    pointf playerSpeed;

    struct bullet newBullet;
    //struct asteroid asteroids[MAX_ASTEROID_COUNT_GAME];
};

struct uartHandshakePacket {
    uint8_t fromMaster;
};

struct uartGameInvitePacket {
    uint8_t isAck;
    char name[10];
};

struct uartGameSetupPacket {
    struct asteroid asteroids[MAX_ASTEROID_COUNT_MP];
};

struct uartInitPacket {
    long seed;
};

enum packetType {
    FramePacket,
    GameInvitePacket,
    GameSetupPacket,
    HandshakePacket
};

/*
    - Master continuously sends HandshakePacket when in pairing mode
    - When receiving a HandshakePacket in the MainMenu, Slave sends a HandshakePacket
      and switches to GameState in slave mode
    - Master receives HandshakePacket and switches to GameState in Master-Mode

    - Master:
        - Determins random seed for asteroid generation
        - Calculates score for both players

*/

/* Beginning of level: 
    - Send random seed for asteroid generation
    - 

*/

/* During level:
    - Send own player XY position (and speed for interpolation ?)
    - Send projectile XY position and speed if new projectile is casted

*/

void initUartQueues();
size_t getPacketSize(enum packetType type);
void sendPacket(enum packetType type, void *packet);
void receivePacketTask(void * params);

void sendHandshake(uint8_t isMaster);
void sendGameInvitation(uint8_t isAck, char* name);
void sendGameSetup(struct asteroid* asteroids, size_t asteroidsLength);
//void sendFramePacket(struct player* player, struct asteroid* asteroids, size_t asteroidsLength);
void sendFramePacket(struct uartFramePacket* packet);