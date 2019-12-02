//Este programa tem por objetivo captar as informacoes recebidas da porta serial e transmití-las aos receptores

#include "Principal.h"
#include <printf.h>
char buf[10];
void setup()
{
  //--Sets
     Serial.begin(BDRATE);
     //printf_begin();
     pinMode(LED_BUILTIN, OUTPUT);
     Principal principal = Principal();
     principal.start();
}

void loop()
{
//    if(Serial.available())
//    {
//        char buf[10];
//        digitalWrite(LED_BUILTIN, HIGH);
//        // = Serial.available();
//        int siz = Serial.readBytes(buf, 9);
//        Serial.flush();
//        if(siz > 1)
//        {
//            Serial.write(buf, 9);
//        }
//        /*for(int i=0; i<siz; i++)
//        {
//            Serial.print(buf[i]);
//        }
//        Serial.println();*/
//        
//        Serial.println();
//        digitalWrite(LED_BUILTIN, LOW);
//    }
}
