
/**
 *  DLVR Project
 *  City Traffic Lights Controller
 *  @author Matheus Dias (@BulbThinker)
 */

const int light0 = 6;
const int light1 = 5;
const int base_time = 15000;
const int low = 0;
const int high = 255;

void setup() {
    // Lights 0
    pinMode(light0, OUTPUT);    //Set mode to output
    analogWrite(light0, low); //Output to high, leds off
    // Lights 1
    pinMode(light1, OUTPUT);    //Set mode to output
    analogWrite(light1, high); //Output to high, leds off
}

void loop() {
    analogWrite(light0, low);  //Lights 0 On 
    analogWrite(light1, high); //Lights 1 Off
    delay(base_time);           //Delay base time
    analogWrite(light0, high); //Lights 0 Off 
    analogWrite(light1, low);  //Lights 1 On
    delay(base_time);           //Delay base time
}
