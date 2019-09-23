#include "RadioWriter.h"

RadioWriter::RadioWriter()
{
    radio = new Radio();
}

void RadioWriter::setChannel(char name)
{
    radio->setChannel(uint8_t(CHANNEL+2*name));
}

void RadioWriter::setAddress(const uint8_t* address)
{
    radio->openWritingPipe(address);
}

void RadioWriter::sendData(const void* data)
{
    radio->write(data, W_DATA);
}

void RadioWriter::debug()
{
     radio->printDetails();
}
