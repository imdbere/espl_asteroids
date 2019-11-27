#include "includes.h"
#include "uart.h"

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
QueueHandle_t UartPacketQueue;
static const uint8_t startByte = 0xAA, stopByte = 0x55;

void sendBuffer(uint8_t *buffer, size_t length)
{
    for (int i = 0; i < length; i++)
    {
        UART_SendData(buffer[i]);
    }
}

uint8_t calculateChecksum(struct uartPacket *packet)
{
    struct uartPacket p = *packet;
    return p.screenOffsetX ^ p.screenOffsetY ^ p.buttonACounter ^ p.buttonBCounter ^ p.buttonCCounter ^ p.buttonDCounter;
}

void sendPacket(struct uartPacket *packet)
{
    uint8_t checksum = calculateChecksum(packet);

    UART_SendData(startByte);

    sendBuffer((uint8_t *)packet, sizeof(struct uartPacket));

    UART_SendData(checksum);
    UART_SendData(stopByte);
}

void receivePacketTask(void *params)
{
    uint8_t state = 0;
    uint8_t input;

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

        // Reading Data
        else if (state == 1)
        {
            buffer[pos] = input;
            pos++;

            if (pos == sizeof(struct uartPacket))
            {
                pos = 0;
                state = 2;
            }
        }

        // Waiting for checksum
        else if (state == 2)
        {
            struct uartPacket *packet = (struct uartPacket *)buffer;
            uint8_t desiredChecksum = calculateChecksum(packet);
            if (input == desiredChecksum)
                state = 3;
            else
                state = 0;
        }

        // Waiting for STOP
        else if (state == 3)
        {
            if (input == stopByte)
            {
                xQueueSend(UartPacketQueue, &buffer, 100);
            }

            state = 0;
        }
    }
}
