
#include "includes.h"
#include "uart.h"
#include "string.h"

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t xSendMutex;

QueueHandle_t uartHandshakeQueue;
QueueHandle_t uartInviteQueue;

static const uint8_t startByte = 0xAA, stopByte = 0x55;


void initUartQueues()
{
    uartHandshakeQueue = xQueueCreate(2, sizeof(struct uartHandshakePacket));
    uartInviteQueue = xQueueCreate(2, sizeof(struct uartGameInvitePacket));

    xTaskCreate(receivePacketTask, "receivePacketTask", 100, NULL, 2, NULL);
    xSendMutex = xSemaphoreCreateMutex();
}

void sendHandshake(uint8_t isMaster)
{
    struct uartHandshakePacket packet;
    packet.fromMaster = isMaster;

    sendPacket(HandshakePacket, &packet);
}

void sendGameInvitation(uint8_t isMaster, char* name)
{
    struct uartGameInvitePacket packet;
    packet.fromMaster = isMaster;
    strcpy(packet.name, name);

    sendPacket(GameInvitePacket, &packet);
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

void sendPacket(enum packetType type, void *packet)
{
    if(xSemaphoreTake(xSendMutex, 5) == pdTRUE)
    {
        size_t length = getPacketSize(type);
        uint8_t checksum = calculateChecksum(packet, length);

        UART_SendData(startByte);
        UART_SendData(type);

        sendBuffer((uint8_t *)packet, length);

        UART_SendData(checksum);
        UART_SendData(stopByte);
        xSemaphoreGive(xSendMutex);
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
        case FramePacket:
            return sizeof(struct uartFramePacket);
    }
}

void handleGotPacket(enum packetType type, uint8_t* buffer)
{
    switch (type)
    {
        case HandshakePacket:
            xQueueSend(uartHandshakeQueue, buffer, 0);
            break;
        case GameInvitePacket:
            xQueueSend(uartInviteQueue, buffer, 0);
            break;
    }
}

void receivePacketTask(void *params)
{
    uint8_t state = 0;
    uint8_t input;
    enum packetType type;
    size_t packetLength;

    uint8_t pos = 0;
    uint8_t buffer[40];

    while (1)
    {
        // wait for data in queue
        xQueueReceive(ESPL_RxQueue, &input, portMAX_DELAY);

        // Waiting for START
        if (state == 0)
        {
            if (input == startByte)
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
            if (input == stopByte)
            {
                handleGotPacket(type, buffer);
            }

            state = 0;
        }
    }
}
