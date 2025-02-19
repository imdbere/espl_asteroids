#pragma once

#include "includes.h"
#include "asteroids.h"
#include "player.h"
#include "bullets.h"

extern QueueHandle_t uartHandshakeQueue;
extern QueueHandle_t uartInviteQueue;
extern QueueHandle_t uartGameSetupQueue;
extern QueueHandle_t uartFramePacketQueue;
extern QueueHandle_t uartCollosionPacketQueue;
extern QueueHandle_t uartPauseQueue;

extern TimerHandle_t disconnectTimer;
extern SemaphoreHandle_t disconnectSemaphore;

#define MAX_PACKET_LENGTH 800
#define UART_START_BYTE 0xAA
#define UART_STOP_BYTE 0x55

struct uartFramePacket {
    pointf playerPosition;
    pointf playerSpeed;

    struct bullet newBullet;
    //struct asteroid asteroids[MAX_ASTEROID_COUNT_GAME];
};

enum CollisionElement {
    COLL_NOTHING,
    COLL_PLAYER, 
    COLL_UFO,
    COLL_ASTEROID,
    COLL_BULLET
};

struct uartCollisionPacket {
    enum CollisionElement collider1;
    enum CollisionElement collider2;
    int collider1Id;
    int collider2Id;

    int nextAsteroidSeed;
};

/*struct uartFullSyncPacket {
    struct asteroid asteroids[MAX_ASTEROID_COUNT_MP];
    struct bullet bullets[MAX_BULLET_COUNT];
    struct player player;
    struct ufo ufo;
};*/

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

enum PauseMode
{
    PAUSE_MODE_PAUSE,
    PAUSE_MODE_RESUME,
    PAUSE_MODE_EXIT
};

struct uartPausePacket
{
    enum PauseMode mode;
};

struct uartInitPacket {
    long seed;
};

enum packetType {
    FramePacket,
    GameInvitePacket,
    GameSetupPacket,
    HandshakePacket,
    CollisionPacket,
    PausePacket
};

/*
    - Master continuously sends HandshakePacket when in pairing mode
    - When receiving a HandshakePacket in the MainMenu, Slave sends a HandshakePacket
      and switches to GameState in slave mode
    - Master receives HandshakePacket and switches to GameState in Master-Mode

    - Master:
        - Calculates Asteroids at the beginning of game and sends it to slave
        - Calculates collisions and sends it to slave

*/


/* During level:
    - Send own player XY position (and speed for interpolation ?)
    - Send projectile XY position and speed if new projectile is casted
*/

void initUartQueues();
void resetDisconnectTimer();
size_t getPacketSize(enum packetType type);
void sendPacket(enum packetType type, void *packet);
void receivePacketTask(void * params);

void sendHandshake(uint8_t isMaster);
void sendGameInvitation(uint8_t isAck, char* name);
void sendGameSetup(struct asteroid* asteroids, size_t asteroidsLength);
//void sendFramePacket(struct player* player, struct asteroid* asteroids, size_t asteroidsLength);
void sendFramePacket(struct uartFramePacket* packet);
void sendCollisionPacket(struct uartCollisionPacket* packet);
void sendPause();
void sendResume();
void sendExit();