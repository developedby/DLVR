#include "RadioWriter.h"
RadioWriter* rWriter = new RadioWriter();

void setup() {
  rWriter->setChannel(0);
  rWriter->setAddress((uint8_t*)"DLVRT");
}

void loop() {
  rWriter->sendData("ola from arduino");
  delay(1000);
}
