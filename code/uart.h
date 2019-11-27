struct uartPacket {
    int screenOffsetX;
    int screenOffsetY;
    
    int buttonACounter;
    int buttonBCounter;
    int buttonCCounter;
    int buttonDCounter;
};

extern QueueHandle_t UartPacketQueue;

void sendPacket(struct uartPacket* packet);
void receivePacketTask(void * params);