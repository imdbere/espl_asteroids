#include "uart.h"
#include "includes.h"
#include "string.h"
#include "sm.h"

// didn't get this to work reliably

int dmaPacketSize = 50;
uint8_t bufferToSend[100] = {{0}};
int txBufferPos = 0;

void sendByteToTxBuffer(uint8_t byte)
{
    bufferToSend[txBufferPos++] = byte;
}

DMA_InitTypeDef dmaInit;

void initDma()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    //RCC_AHB2PeriphResetCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_StructInit(&dmaInit);
    dmaInit.DMA_Channel = DMA_Channel_4;
	dmaInit.DMA_BufferSize = dmaPacketSize;
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
    //DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

    //NVIC_SetPriority(DMA2_Stream7_IRQn, 0);
    //NVIC_EnableIRQ(DMA2_Stream7_IRQn);

    DMA_Cmd(DMA2_Stream7, ENABLE);

    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

void reinitDma(int packetLength)
{
    DMA_Cmd(DMA2_Stream7, DISABLE);
    DMA_DeInit(DMA2_Stream7);

    txBufferPos = 0;
    dmaInit.DMA_BufferSize = packetLength;
    
    //DMA2_Stream7->MAX = packetLength;
    DMA_Init(DMA2_Stream7, &dmaInit);
    //DMA_Cmd(DMA2_Stream7, ENABLE);
}

void startTransfer()
{
    DMA_Cmd(DMA2_Stream7, ENABLE);
    txBufferPos = 0;
}


void DMA2_Stream7_IRQHandler(void) {
    if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET) {
        DMA_Cmd(DMA2_Stream7, DISABLE);
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
        //static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        //xSemaphoreGiveFromISR( semaphore_state_change, NULL);

        //if (xHigherPriorityTaskWoken)
        //    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

void sendBufferDma(uint8_t *buffer, size_t length)
{
    for (int i = 0; i < length; i++)
    {
        sendByteToTxBuffer(buffer[i]);
    }
}


void __attribute__((optimize("O0"))) sendPacketDma(enum packetType type, void *packet)
{
    // Prevents sending of multiple packets at the same time and
    // changing states while sending
    if(xSemaphoreTake(semaphore_state_change, 5) == pdTRUE)
    {
        size_t length = getPacketSize(type);
        uint8_t checksum = calculateChecksum(packet, length);

        int totalLength = length + 4;
        //reinitDma(totalLength);

        sendByteToTxBuffer(startByte);
        sendByteToTxBuffer(type);

        sendBuffer((uint8_t *)packet, length);

        sendByteToTxBuffer(checksum);
        sendByteToTxBuffer(stopByte);

        if (txBufferPos >= dmaPacketSize)
        {
            reinitDma(txBufferPos);
            startTransfer();
        }

        xSemaphoreGive(semaphore_state_change);
    }
}