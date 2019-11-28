#include <iostream>
#include "constants.hpp"
#include "radio_communication.hpp"

RadioCommunication::RadioCommunication()
{
    int i = 0;
    //radio = new Radio();
    for(i=0; i<consts::radio_width_data; i++)
    {
        received_data[i] = 0;
    }
    siz = 0;
    ack = 0;
    radio.openReadingPipe(1, consts::radio_address);
    radio.enableAckPayload();               // Allow optional ack payloads
    radio.enableDynamicPayloads();
    radio.startListening();
    eventSetFuncEx(1, sendFailure, this);
    eventSetFuncEx(2, sendOk, this);
    attemps = 0;
}

void RadioCommunication::setAddress(uint8_t const *address)
{
    int i = 0, j=0;
    for(i=0; i<consts::radio_width_address; i++)
    {
        if(address[i] != last_address[i])
        {
            radio.openWritingPipe(address);
            ack = 0;
            for(j=0; j<consts::radio_width_address; j++)
            {
                last_address[j] = address[j];
            }
            break;
        }
    }
}

void RadioCommunication::sendToRadio(SentMessage message)
{
    last_sended_message = message;
    std::vector<uint8_t> data;
    data.push_back(consts::radio_start_byte);
    data.push_back(1);//id do robo
    if(message.ultrassound_reading > 0)
    {
        //float* p = new float(message.ultrassound_reading);
        int len = sizeof((message.ultrassound_reading));
        uint8_t ch[len];
        memcpy(ch, &(message.ultrassound_reading),len);
        for(int i =0; i<len; i++)
        {
            data.push_back(ch[i]);
        }
    }
    else if(message.other_sensors_reading != NO_SENSOR_READ)
    {
        data.push_back(message.other_sensors_reading);
    }
    if(message.movement.read)
    {
        int len = sizeof((message.movement.amplitude));
        uint8_t ch[len];
        memcpy(ch, &(message.movement.amplitude),len);
        for(int i =0; i<len; i++)
        {
            data.push_back(ch[i]);
        }
        memcpy(ch, &(message.movement.curvature),len);
        for(int i =0; i<len; i++)
        {
            data.push_back(ch[i]);
        }
    }
    else if(message.status != status::NO_STATUS)
    {
        data.push_back(message.status);
    }
    for(auto i : message.qr_codes_read)
    {
        data.push_back((int(i) & 0xff00) >> 8);
        data.push_back(int(i) & 0xff);
    }
    data.push_back(consts::radio_start_byte);
    radio.stopListening();
    int len = static_cast<uint8_t>(data.size());
    uint8_t send_data[len];
    for(int i = 0; i<int(data.size()); i++)
    {
        send_data[i] = data[i];
    }
    radio.write(send_data, len);
    //radio.startListening();
    //radio_ack_thread = gpioStartThread(waitAck, this);
}

bool RadioCommunication::receiveFromRadio()
{
    radio.startListening();
    if(radio.available())
    {
        std::cout <<"ok";
        if(radio.getDynamicPayloadSize() < 1)
        {
            return false;
        }
        std::cout <<"ok"<<std::endl;
        radio.read(received_data, sizeof(received_data));
        ack++;
        
        //radio.writeAckPayload(1, &(ack), 1);
        siz = 2;
        std::cout << "mandei ack, pacote recebido: " <<std::endl;
        for (int i=1; received_data[i] != consts::radio_start_byte and i < consts::radio_width_data; i++)
        {
            std::cout << int(received_data[i]) << std::endl;
            siz++;
        }
        if((received_data[0] == consts::radio_start_byte) && (received_data[siz - 1] == consts::radio_start_byte))
        {
            return true;
        }
    }
    return false;
}

ReceivedMessage RadioCommunication::getData()
{
    std::vector<uint8_t> path;
    uint16_t qr_code = 0;
    Commands command = NO_COMMAND; 
    PossibleSensors sensor_to_read = NO_SENSORS;
    PossibleStatus required_status = NO_STATUS;
    int i = 3;
    if((received_data[2] & 0x8) == 0x8)
    {
        for(i=0; i<received_data[3]; i++)
        {
            path.push_back(received_data[4+i]);
        }
        i+=4;
        qr_code = (received_data[i] << 8) + received_data[i+1];
        i+=2;
    }
    if((received_data[2] & 0x4) == 0x4)
    {
        command = (Commands)received_data[i];
        i++;
        if(command == GET_QR_CODE)
        {
             qr_code = (received_data[i] << 8) + received_data[i+1];
             i += 2;
        }
            
    }
    if((received_data[2] & 0x2) == 0x2)
    {
        sensor_to_read = (PossibleSensors)received_data[i];
        i++;
    }
    if((received_data[2] & 0x1) == 0x1)
    {
        required_status = (PossibleStatus)received_data[i];
        i++;
    }
    return ReceivedMessage(path, qr_code, command, sensor_to_read, required_status);
}

int RadioCommunication::getDataSize()
{
    return siz;
}

void RadioCommunication::debug()
{
     radio.printDetails();
}

bool RadioCommunication::isChipConnected()
{
    return radio.isChipConnected();
}

void* waitAck(void *obj)
{
    RadioCommunication *internal_radio = static_cast<RadioCommunication*>(obj);
    uint32_t startTick = gpioTick();
    while(!internal_radio->radio.isAckPayloadAvailable())
    {
        if((gpioTick() - startTick) > consts::time_out_radio_ack)
        {
            eventTrigger(1);
            return nullptr;
        }
    }
    return nullptr;
}

void sendOk(int event, uint32_t tick, void* obj)
{
    RadioCommunication *internal_radio = static_cast<RadioCommunication*>(obj);
    internal_radio->radio.writeAckPayload(1, &(internal_radio->ack), 1);
    internal_radio->siz = 0;
    internal_radio->attemps = 0;
    gpioStopThread(internal_radio->radio_ack_thread);
}

void sendFailure(int event, uint32_t tick, void* obj)
{
    RadioCommunication *internal_radio = static_cast<RadioCommunication*>(obj);
    internal_radio->attemps++;
    gpioStopThread(internal_radio->radio_ack_thread);
    if(internal_radio->attemps > consts::radio_retries)
    {
        eventTrigger(3);
    }
    else
    {
        internal_radio->sendToRadio(internal_radio->last_sended_message);
    }
}
