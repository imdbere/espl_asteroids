
#include "uart.h"
#include "includes.h"
#include "string.h"
#include "sm.h"


QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t xSendMutex;

QueueHandle_t uartHandshakeQueue;
QueueHandle_t uartInviteQueue;
QueueHandle_t uartGameSetupQueue;
QueueHandle_t uartFramePacketQueue;
QueueHandle_t uartCollosionPacketQueue;
QueueHandle_t uartPauseQueue;

TimerHandle_t disconnectTimer;
SemaphoreHandle_t disconnectSemaphore;

void disconnectTimerElapsed(TimerHandle_t xTimer);

void initUartQueues()
{
    uartHandshakeQueue = xQueueCreate(1, sizeof(struct uartHandshakePacket));
    uartInviteQueue = xQueueCreate(1, sizeof(struct uartGameInvitePacket));
    uartGameSetupQueue = xQueueCreate(1, sizeof(struct uartGameSetupPacket));
    uartFramePacketQueue = xQueueCreate(1, sizeof(struct uartFramePacket));
    uartCollosionPacketQueue = xQueueCreate(1, sizeof(struct uartCollisionPacket));
    uartPauseQueue = xQueueCreate(1, sizeof(struct uartPausePacket));

    xTaskCreate(receivePacketTask, "receivePacketTask", MAX_PACKET_LENGTH + 100, NULL, 2, NULL);
    xSendMutex = xSemaphoreCreateMutex();

    disconnectSemaphore = xSemaphoreCreateBinary();
    disconnectTimer = xTimerCreate("disconnectTimer", pdMS_TO_TICKS(500), pdFALSE, NULL, disconnectTimerElapsed);

    //initDma();
}

void disconnectTimerElapsed(TimerHandle_t xTimer)
{
    xSemaphoreGive(disconnectSemaphore);
}


void sendHandshake(uint8_t isMaster)
{
    struct uartHandshakePacket packet;
    packet.fromMaster = isMaster;

    sendPacket(HandshakePacket, &packet);
}

void sendGameInvitation(uint8_t isAck, char* name)
{
    struct uartGameInvitePacket packet;
    packet.isAck = isAck;
    strcpy(packet.name, name);

    sendPacket(GameInvitePacket, &packet);
}

void sendGameSetup(struct asteroid* asteroids, size_t asteroidsLength)
{
    struct uartGameSetupPacket packet;
    memcpy(packet.asteroids, asteroids, asteroidsLength);

    sendPacket(GameSetupPacket, &packet);
}

void sendFramePacket(struct uartFramePacket* packet)
{
    sendPacket(FramePacket, packet);
}

void sendCollisionPacket(struct uartCollisionPacket* packet)
{
    sendPacket(CollisionPacket, packet);
}

void sendPause()
{
    struct uartPausePacket packet;
    packet.mode = PAUSE_MODE_PAUSE;
    sendPacket(PausePacket, &packet);
}

void sendResume()
{
    struct uartPausePacket packet;
    packet.mode = PAUSE_MODE_RESUME;
    sendPacket(PausePacket, &packet);
}

void sendExit()
{
    struct uartPausePacket packet;
    packet.mode = PAUSE_MODE_EXIT;
    sendPacket(PausePacket, &packet);
}

void sendBuffer(uint8_t *buffer, size_t length)
{
    for (int i = 0; i < length; i++)
    {
        UART_SendData(buffer[i]);
    }
}

uint8_t calculateChecksum(void *packet, size_t length)
{
    uint8_t checksum = 0;
    uint8_t* data = (uint8_t*) packet;

    for (int i=0; i<length; i++)
    {
        checksum ^= data[i];
    }
    return checksum;
}

void __attribute__((optimize("O0"))) sendPacket(enum packetType type, void *packet)
{
    // Prevents sending of multiple packets at the same time and
    // changing states while sending
    if(xSemaphoreTake(semaphore_state_change, 5) == pdTRUE)
    {
        size_t length = getPacketSize(type);
        int totalLength = length + 4;
        if (totalLength > MAX_PACKET_LENGTH)
        {
            return;
        }

        uint8_t checksum = calculateChecksum(packet, length);

        UART_SendData(UART_START_BYTE);
        UART_SendData(type);

        sendBuffer((uint8_t *)packet, length);

        UART_SendData(checksum);
        UART_SendData(UART_STOP_BYTE);

        xSemaphoreGive(semaphore_state_change);
    }
}

size_t getPacketSize(enum packetType type)
{
    switch(type)
    {
        case HandshakePacket:
            return sizeof(struct uartHandshakePacket);
        case GameInvitePacket:
            return sizeof(struct uartGameInvitePacket);
        case GameSetupPacket:
            return sizeof(struct uartGameSetupPacket);
        case FramePacket:
            return sizeof(struct uartFramePacket);
        case CollisionPacket:
            return sizeof(struct uartCollisionPacket);
        case PausePacket:
            return sizeof(struct uartPausePacket);
    }

    return 0;
}

void resetDisconnectTimer()
{
    xTimerReset(disconnectTimer, 0);
    xSemaphoreTake(disconnectSemaphore, 0);
}

void handleGotPacket(enum packetType type, uint8_t* buffer)
{
    xTimerReset(disconnectTimer, 0);
    //xSemaphoreTake(disconnectSemaphore, 0);
    
    switch (type)
    {
        case HandshakePacket:
            xQueueSend(uartHandshakeQueue, buffer, 0);
            break;
        case GameInvitePacket:
            xQueueSend(uartInviteQueue, buffer, 0);
            break;
        case GameSetupPacket:
            xQueueSend(uartGameSetupQueue, buffer, 0);
            break;
        case FramePacket:
            xQueueSend(uartFramePacketQueue, buffer, 0);
            break;
        case CollisionPacket:
            xQueueSend(uartCollosionPacketQueue, buffer, 0);
            break;
        case PausePacket:
            xQueueSend(uartPauseQueue, buffer, 0);
            break;
    }
}

void receivePacketTask(void *params)
{
    uint8_t state = 0;
    uint8_t input;
    enum packetType type;
    size_t packetLength;

    int pos = 0;
    uint8_t buffer[MAX_PACKET_LENGTH];

    while (1)
    {
        // wait for data in queue
        xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

        // Waiting for START
        if (state == 0)
        {
            if (input == UART_START_BYTE)
                state = 1;
        }

        // Reading packet type
        else if (state == 1)
        {
            type = input;
            packetLength = getPacketSize(type);
            if (packetLength == 0)
                state = 3;
            else
                state = 2;
        }

        // Reading Data
        else if (state == 2)
        {
            buffer[pos] = input;
            pos++;

            if (pos == packetLength)
            {
                pos = 0;
                state = 3;
            }
            // Should never happen
            if (pos == MAX_PACKET_LENGTH)
            {
                pos--;
                return;
                //exit(1);
            }
        }

        // Waiting for checksum
        else if (state == 3)
        {
            //struct uartPacket *packet = (struct uartPacket *)buffer;
            uint8_t desiredChecksum = calculateChecksum(buffer, packetLength);
            if (input == desiredChecksum)
                state = 4;
            else
                state = 0;
        }

        // Waiting for STOP
        else if (state == 4)
        {
            if (input == UART_STOP_BYTE)
            {
                handleGotPacket(type, buffer);
            }

            state = 0;
        }
    }
}
