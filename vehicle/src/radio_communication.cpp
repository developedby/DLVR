#include "radio_communication.hpp"
#include <iostream>
#include "constants.hpp"

RadioCommunication::RadioCommunication():
    received_data{0}, size_(0), attempts(0), has_new_msg(false)
{
    this->radio.openReadingPipe(1, consts::radio_address);
    this->radio.enableDynamicPayloads();
    this->setAddress(consts::coord_address);
    this->radio.startListening();
    this->message_pooler = gpioStartThread(newMessagePoolingThread, this);
}

// Pools the radio object to check if there are new messages regularly
void RadioCommunication::newMessagePooling()
{
    while (true)
    {
        const bool new_msg = this->receiveFromRadio();
        if (new_msg)
        {
            this->has_new_msg = true;
            std::cout << "Nova mensagem recebida" << std::endl;
        }
        this->has_new_msg = this->has_new_msg or new_msg;
        gpioSleep(PI_TIME_RELATIVE, 0, consts::radio_pooling_period);
    }
}

void RadioCommunication::setAddress(uint8_t const *address)
{
    for(int i=0; i<consts::radio_width_address; i++)
    {
        if(address[i] != this->last_address[i])
        {
            this->radio.openWritingPipe(address);
            for(int j=0; j<consts::radio_width_address; j++)
            {
                this->last_address[j] = address[j];
            }
            break;
        }
    }
}

void RadioCommunication::sendToRadio(SentMessage message)
{
    this->last_sended_message = message;
    std::vector<uint8_t> data;
    data.push_back(consts::radio_start_byte);
    data.push_back(1);//id do robo
    if(message.ultrassound_reading > 0)
    {
        //float* p = new float(message.ultrassound_reading);
        const int len = sizeof((message.ultrassound_reading));
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
        const int len = sizeof((message.movement.amplitude));
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
    else if(message.status_ != status::NO_STATUS)
    {
        data.push_back(message.status_);
    }
    for(auto i : message.qr_codes_read)
    {
        data.push_back((int(i) & 0xff00) >> 8);
        data.push_back(int(i) & 0xff);
    }
    data.push_back(consts::radio_start_byte);
    const int len = static_cast<uint8_t>(data.size());
    uint8_t send_data[len];
    std::cout << "pacote enviado: ";
    for(int i = 0; i<int(data.size()); i++)
    {
        std::cout << int(data[i]) << " ";
        send_data[i] = data[i];
    }
    std::cout << ". Mandando para o radio" << std::endl;
    this->radio.stopListening();
    this->radio.write(send_data, len);
    //radio.startListening();
    //radio_ack_thread = gpioStartThread(waitAck, this);
}

bool RadioCommunication::receiveFromRadio()
{
    this->radio.startListening();
    if(this->radio.available() and this->radio.getDynamicPayloadSize() >= 1)
    {
        //std::cout << "opa" << std::endl;
        this->radio.read(this->received_data, sizeof(this->received_data));
        //this->ack++;
        //radio.writeAckPayload(1, &(ack), 1);
        this->size_ = 2;
        std::cout << "pacote recebido: ";
        for (int i=1; (this->received_data[i] != consts::radio_start_byte) and (i < consts::radio_width_data); i++)
        {
            std::cout << int(received_data[i]) << " ";
            this->size_++;
        }
        if((this->received_data[0] == consts::radio_start_byte)
           && (this->received_data[this->size_ - 1] == consts::radio_start_byte))
        {
            std::cout << " vou decodificar" << std::endl;
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
    int8_t finnal_qr_code_direction = 0;
    int i = 3;
    if((received_data[2] & 0x8) == 0x8)
    {
        for(i=0; i<received_data[3]; i++)
        {
            path.push_back(received_data[4+i]);
        }
        i+=4;
        switch(Directions(received_data[i]))
        {
            case TO_THE_LEFT:
                finnal_qr_code_direction = 1;
                break;
            case TO_THE_RIGHT:
                finnal_qr_code_direction = -1;
                break;
            case FORWARD:
                finnal_qr_code_direction = 0;
                break;
            default:
                finnal_qr_code_direction = 0;
                break;
        }
        i++;
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
    return ReceivedMessage(path, finnal_qr_code_direction, qr_code, command, sensor_to_read, required_status);
}

int RadioCommunication::getDataSize()
{
    return this->size_;
}

void RadioCommunication::debug()
{
     radio.printDetails();
}

bool RadioCommunication::isChipConnected()
{
    return radio.isChipConnected();
}

void* newMessagePoolingThread(void *arg)
{
    static_cast<RadioCommunication*>(arg)->newMessagePooling();
    return nullptr;
}
