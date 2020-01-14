#include <includes.h>

extern QueueHandle_t uartHandshakeQueue;

struct uartFramePacket {
    //struct player ownPlayer;
    //struct bullet newBullet;
};

struct uartHandshakePacket {
    uint8_t isMaster;
};

struct uartInitPacket {
    long seed;
};

enum packetType {
    FramePacket,
    LevelInitPacket,
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
void sendHandshake(uint8_t isMaster);
size_t getPacketSize(enum packetType type);
void sendPacket(enum packetType type, void *packet);
void receivePacketTask(void * params);