
/**
 *  DLVR Project
 *  City Traffic Lights Controller
 *  @author Matheus Dias (@BulbThinker)
 */

const int light0 = 6;
const int light1 = 5;
const int base_time = 15000;

void setup() {
    // Lights 0
    pinMode(light0, OUTPUT);    //Set mode to output
    digitalWrite(light0, HIGH); //Output to high, leds off
    // Lights 1
    pinMode(light1, OUTPUT);    //Set mode to output
    digitalWrite(light1, HIGH); //Output to high, leds off
}

void loop() {
//    digitalWrite(light0, LOW);  //Lights 0 On 
//    digitalWrite(light1, HIGH); //Lights 1 Off
//    delay(base_time);           //Delay base time
//    digitalWrite(light0, HIGH); //Lights 0 Off 
//    digitalWrite(light1, LOW);  //Lights 1 On
//    delay(base_time);           //Delay base time
}
