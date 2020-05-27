// Sketch to detect players playing a game of pingpong
// By Troy Morvant

#include <MQTT.h>

int inputPin = D0;                                          // choose the input pin (for PIR sensor)
int ledPin = D7;                                            // LED Pin
int pirState = LOW;                                         // we start, assuming no motion detected
int val = 0;                                                // variable for reading the pin status
bool bypass = false;                                        // if true then bypass all functions except for MQTT
bool sensorIsReady = false;                                 // pir sensor has been calibrated
int calibrateTime = 10000;                                  // wait for the PIR to calibrate

void setBypass(char* topic, byte* payload, unsigned int length);

MQTT client("10.0.10.23", 1883, setBypass);

void setBypass(char* topic, byte* payload, unsigned int length) {
    bypass = !bypass;                                       // recieved bypass message, so flip boolean
}

void setup() {
    pinMode(ledPin, OUTPUT);                                // declare ledPin as output
    pinMode(inputPin, INPUT);                               // declare sensor as input
    client.connect("pongPinger");                           // connect to the MQTT broker
    
    if (client.isConnected()) {
        client.subscribe("pongPinger/bypass");              // subscribe to bypass topic
    }
    calibrate();                                            // calibrate sensor
    
    Particle.function("report", report);                    // publish cloud function for manual trigger
}

void loop() {
   if(!bypass && sensorIsReady){
       Particle.variable("CurrentStatus", &pirState, INT);  // previous state : HIGH/LOW
        Particle.variable("SensorValue", &val, INT);        // current state : HIGH/LOW
        val = readSensor();                                 // read current value from sensor
        report("");                                         // report results
   }
}

int readSensor() {
    return digitalRead(inputPin);
}

bool calibrate() {
    sensorIsReady = millis() - calibrateTime > 0;
}

void setLED(int state) {
    digitalWrite(ledPin, state);
}

// send empty string as particle cloud functions MUST return int and take one String 
int report(String foo) {
    if (pirState != val) {
        if (val == HIGH) {
            pirState = HIGH;                                //update the previous state
            setLED(pirState);                               // turn on status LED to indicate motion detected
            // publish event for game in progress
            client.publish("pongPinger/status","{\"attachments\":[{\"color\": \"danger\",\"text\": \"A Game is in progress!\"}]}");
        } else {
            pirState = LOW;
            setLED(pirState);
            client.publish("pongPinger/status","{\"attachments\":[{\"color\": \"good\",\"text\": \"The table is free\"}]}");
        }
    }
     return pirState;
}