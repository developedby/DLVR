#include "RFreceiver.hpp"

RFreceiver::RFreceiver()
{
    vel_x = 0;
    vel_y = 0;
    vel_ang = 0;
    queue = NULL;
#ifndef XBEE
    radio = new Radio();
    uint8_t address[] = {ADDRESS};
    radio->openReadingPipe(1, address);
    radio->startListening();
#endif
}

void RFreceiver::receiveData()
{
    int i;
    unsigned char data[W_DATA];
#ifndef XBEE
    if(radio->available())
    {
        radio->read(&data,W_DATA);
        if(data[0]==NAME && data[1]!=0)//se for 0 eh pq deu erro no envio
        {
            //Serial.print("Data recebida: ");
            for(i=0; i<W_DATA; i++)
            {
                queue = queue->addByte(data[i], queue);
                //Serial.print(data[i], HEX);
                //Serial.print("  ");
            }
        }
        else
            queue = NULL;
        Serial.println();
    }
    //Serial.println("nao tem pacote");
#else
    int siz = Serial.available();
    if(siz>0)
    {
        for(i=0; i<siz; i++)
        {
            queue = queue->addByte(Serial.read(), queue);
        }
    }
#endif
}

int RFreceiver::updateBuffer()
{
    unsigned char input_byte[DATA_SIZE];
    int i;
    if(queue->getSize()<W_DATA)//nao tem um pacote completo
    {
        return 0;
    }
    while(queue!=NULL&&queue->getByte()!=KEY)
    {
        queue = queue->removeByte(queue);
    }
    if(queue!=NULL&&queue->getByte()==KEY&&queue->getSize()>DATA_SIZE)//mudar isso, passar para constantes
    {
        for(i=0; i<DATA_SIZE; i++)
        {
            queue = queue->removeByte(queue);
            input_byte[i] = queue->getByte();
        }
	//estas operacoes convertem 4 char em 1 float
        vel_x = *(float*)&input_byte[0];
        vel_y = *(float*)&input_byte[4];
        vel_ang = *(float*)&input_byte[8];
    }
    //if(queue!=NULL&&queue->getSize()>(DATA_SIZE+1))//significa que ainda tem um pacote inteiro a ser lido, nesse caso nao se verifica o Buffer do arduino ate que o Buffer local seja totalmente usado
    //{
    //    return 1;
    //}
    return 1;
}

void RFreceiver::debug()
{
#ifndef XBEE
    radio->printDetails();
#endif
}

float RFreceiver::getVelY()
{
    return vel_y;
}

float RFreceiver::getVelX()
{
    return vel_x;
}
float RFreceiver::getVelAng()
{
    return vel_ang;
}
