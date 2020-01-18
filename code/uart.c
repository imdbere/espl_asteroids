
#include "uart.h"
#include "includes.h"
#include "string.h"
#include "sm.h"


QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t xSendMutex;

QueueHandle_t uartHandshakeQueue;
QueueHandle_t uartInviteQueue;
QueueHandle_t uartFramePacketQueue;

static const uint8_t startByte = 0xAA, stopByte = 0x55;

uint8_t bufferToSend[100];
int txBufferPos = 0;

void sendByteToTxBuffer(uint8_t byte)
{
    bufferToSend[txBufferPos++] = byte;
}

DMA_InitTypeDef dmaInit;

void initUartQueues()
{
    uartHandshakeQueue = xQueueCreate(2, sizeof(struct uartHandshakePacket));
    uartInviteQueue = xQueueCreate(2, sizeof(struct uartGameInvitePacket));
    uartFramePacketQueue = xQueueCreate(2, sizeof(struct uartFramePacket));

    xTaskCreate(receivePacketTask, "receivePacketTask", 100, NULL, 2, NULL);
    xSendMutex = xSemaphoreCreateMutex();

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    //RCC_AHB2PeriphResetCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_StructInit(&dmaInit);
    dmaInit.DMA_Channel = DMA_Channel_4;
	dmaInit.DMA_BufferSize = sizeof(bufferToSend);
	dmaInit.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dmaInit.DMA_Memory0BaseAddr = &bufferToSend;
    dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInit.DMA_Mode = DMA_Mode_Normal;
    dmaInit.DMA_PeripheralBaseAddr = &USART1->DR;
    dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInit.DMA_Priority = DMA_Priority_High;
    //dmaInit.DMA_PeripheralBurst

    DMA_Init(DMA2_Stream7, &dmaInit);
    DMA_Cmd(DMA2_Stream7, ENABLE);

    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

}

void reinitDma(int packetLength)
{
    DMA_DeInit(DMA2_Stream7);
    txBufferPos = 0;
    DMA_Cmd(DMA2_Stream7, DISABLE);
    dmaInit.DMA_BufferSize = packetLength;
    DMA_Init(DMA2_Stream7, &dmaInit);
}

void startTransfer()
{
    DMA_Cmd(DMA2_Stream7, ENABLE);
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

void sendFramePacket(struct player* player, struct asteroid* asteroids, size_t asteroidsLength)
{
    struct uartFramePacket packet;
    packet.playerPosition = player->position;
    packet.playerSpeed = player->speed;
    memcpy(packet.asteroids, asteroids, asteroidsLength);

    sendPacket(FramePacket, &packet);
}

void sendBuffer(uint8_t *buffer, size_t length)
{
    for (int i = 0; i < length; i++)
    {
        sendByteToTxBuffer(buffer[i]);
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
        uint8_t checksum = calculateChecksum(packet, length);

        int totalLength = length + 4;
        reinitDma(totalLength);

        sendByteToTxBuffer(startByte);
        sendByteToTxBuffer(type);

        sendBuffer((uint8_t *)packet, length);

        sendByteToTxBuffer(checksum);
        sendByteToTxBuffer(stopByte);

        startTransfer();
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
        case FramePacket:
            return sizeof(struct uartFramePacket);
    }

    return 0;
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
        case FramePacket:
            xQueueSend(uartFramePacketQueue, buffer, 0);
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
