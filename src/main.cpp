/*INCLUDES*/
#include <Arduino.h>
#include "Tlc5940.h"


/*DEBUG macros*/
#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
#define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#endif

/*DEFINES*/
#define SERIAL_RX 2 //Serial Pins for DME Communication
#define SERIAL_TX 3 //possible pins: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)
#define SERIAL_BAUD 38400

#define ERROR_LED 7

/*MUX STUFF*/
#define PIN_MUX1 A0
#define PIN_MUX2 A1
#define PIN_MUX3 A2

#define MUX_S0 A3
#define MUX_S1 A4
#define MUX_S2 A5
/*MUX STUFF*/

struct inputpin {
    int value;
    uint8_t mux; // Mux value to read from this input
    uint8_t pin; // Pin to read from
    uint8_t dme; // Value needed for DME set
    bool digital; // needed to distinguish analog from digital read
    bool stereo;
};

struct outputpin {
    int value;
    uint8_t out; // TLC Number
    uint8_t dme; // DME Read number
    bool bar;
};

const byte numChars = 100;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

boolean newData = false;

// set if outputs were updated
boolean outputUpdate = false;


class Inputs {
public:
    // send command over serial to DME
    void set(inputpin input) {
        // different command for EQ
        if (input.mux < 7 && input.mux > 3) {
            int level = map(input.value, 0, 1023, -1800, 1800);
            send(level, input.dme);
        } else {
            send(input.value, input.dme);
            // Setting second value -> Stereo
            if (input.stereo) {
                send(input.value, input.dme + 1);
            }
        }
    };

    void update(boolean setall) {
        // setall for initialisation and sending all values to the DME
        // Looping over private inputs array
        for (int i = 0; i < 22; i++) {
            // Changing Mux if needed - normally every 3rd time
            if (inputs[i].mux != currentMux) {
                setMux(inputs[i].mux);
                currentMux = inputs[i].mux; // saving current Mux pin
            }

            // analog / digital read the input pin
            int cache = readPin(inputs[i].pin, inputs[i].digital);

            // debouncing value - only values with a bigger difference than 3 are send
            if (cache < inputs[i].value - 5 || cache > inputs[i].value + 5 || setall) {
                inputs[i].value = cache;
                set(inputs[i]);
            } else if ((inputs[i].digital && cache != inputs[i].value) || setall) { // digital always
                inputs[i].value = cache;
                set(inputs[i]);
            }
        }
    };
private:
    // input array - if needed change size, for loop and add input
    // {level, mux, pin, dme, digital, stereo}
    inputpin inputs[22] = {
            // SWITCHES
            {0, 0, PIN_MUX1, 50, true,  false},   // Talkover Mic1
            {0, 0, PIN_MUX2, 51, true,  false},   // Talkover Mic2
            {0, 0, PIN_MUX3, 52, true,  false},   // Mute Mic1
            {0, 1, PIN_MUX1, 53, true,  false},   // Mute Mic2
            // POTIS
            {0, 1, PIN_MUX2, 54, false, false},   // Volume Mic1
            {0, 1, PIN_MUX3, 55, false, false},   // Volume Mic2
            {0, 2, PIN_MUX1, 58, false, true},   // Input DJ
            {0, 2, PIN_MUX2, 60, false, true},   // Input Bar
            {0, 2, PIN_MUX3, 62, false, true},   // Input Spare
            {0, 3, PIN_MUX1, 64, false, true},   // Output Oben
            {0, 3, PIN_MUX2, 66, false, true},   // Output Bar
            {0, 3, PIN_MUX3, 68, false, true},   // Output Unten
            // EQ Oben
            {0, 4, PIN_MUX1, 70, false, false},   // High
            {0, 4, PIN_MUX2, 71, false, false},   // Mid
            {0, 4, PIN_MUX3, 72, false, false},   // Low
            // EQ Bar
            {0, 5, PIN_MUX1, 73, false, false},   // High
            {0, 5, PIN_MUX2, 74, false, false},   // Mid
            {0, 5, PIN_MUX3, 75, false, false},   // Low
            // EQ Keller
            {0, 6, PIN_MUX1, 76, false, false},   // High
            {0, 6, PIN_MUX2, 77, false, false},   // Mid
            {0, 6, PIN_MUX3, 78, false, false},   // Low
            // extra
            //{0, 7, PIN_MUX1, 79, true,  false},    // Delay Switch
    };

    // cached mux pin
    uint8_t currentMux;

    // read analog / digital pin
    int readPin(uint8_t pin, bool digital) {
        if (!digital) {
            return analogRead(pin);
        } else {
            return !(digitalRead(pin));
        }
    }

    // set mux pins
    void setMux(uint8_t mux) {
        switch (mux) {
            case 0:
                digitalWrite(MUX_S0, LOW);
                digitalWrite(MUX_S1, LOW);
                digitalWrite(MUX_S2, LOW);
                break;
            case 1:
                digitalWrite(MUX_S0, HIGH);
                digitalWrite(MUX_S1, LOW);
                digitalWrite(MUX_S2, LOW);
                break;
            case 2:
                digitalWrite(MUX_S0, LOW);
                digitalWrite(MUX_S1, HIGH);
                digitalWrite(MUX_S2, LOW);
                break;
            case 3:
                digitalWrite(MUX_S0, HIGH);
                digitalWrite(MUX_S1, HIGH);
                digitalWrite(MUX_S2, LOW);
                break;
            case 4:
                digitalWrite(MUX_S0, LOW);
                digitalWrite(MUX_S1, LOW);
                digitalWrite(MUX_S2, HIGH);
                break;
            case 5:
                digitalWrite(MUX_S0, HIGH);
                digitalWrite(MUX_S1, LOW);
                digitalWrite(MUX_S2, HIGH);
                break;
            case 6:
                digitalWrite(MUX_S0, LOW);
                digitalWrite(MUX_S1, HIGH);
                digitalWrite(MUX_S2, HIGH);
                break;
            case 7:
                digitalWrite(MUX_S0, HIGH);
                digitalWrite(MUX_S1, HIGH);
                digitalWrite(MUX_S2, HIGH);
                break;
            default:
                break;
        };
    };

    void send(int level, uint8_t dme) {
        if (dme < 70 && dme > 53){
            Serial.print("SVL 0 ");
            Serial.print(dme);
            Serial.print(" ");
            Serial.print(level);
            Serial.print('\n');
        } else {
            Serial.print("SPR 0 ");
            Serial.print(dme);
            Serial.print(" ");
            Serial.print(level);
            Serial.print('\n');
        }

    };
};

class Outputs {

public:

    void getDME() {
        Serial.print("GMT 0 49 0 \n");
    };

    void recvWithStartEndMarkers() {
        static boolean recvInProgress = false;
        static byte ndx = 0;
        char startMarker = 'U';
        char endMarker = 'H';
        char rc;

        while (Serial.available() > 0 && newData == false) {  // & -> boolean &&->Bitwise
            rc = Serial.read();

            if (recvInProgress == true) {
                DPRINTLN("recvInProgress == true");
                if (rc != endMarker) {
                    receivedChars[ndx] = rc;
                    ndx++;
                    if (ndx >= numChars) {
                        ndx = numChars - 1;
                    }
                } else {
                    recvInProgress = false;
                    ndx = 0;
                    newData = true;
                }
            } else if (rc == startMarker) {
                recvInProgress = true;
            }
        }
    }

    void parseData() {      // split the data into its parts

        char *strtokIndx; // this is used by strtok() as an index

        strtokIndx = strtok(tempChars, " ");      // get the first part - the string

        // read the first 8 values of DME response
        // MTR 0 56 0 CUR -9428 -9780 -9376 -13801 -7432 -9004 -9004 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801
        for (uint8_t i = 0; i < 8; i++) {
            strtokIndx = strtok(NULL, " ");      // get the first part - the string
            outputs[i].value = atoi(strtokIndx);     // convert this part to an integer
            set(outputs[i]);
            DPRINTLN(outputs[i].value);
        }
        outputUpdate = true;
    }

    // set LED Bar output
    void set(outputpin out) {
        if (out.bar) {
            // map DME value to 10 LEDs, not dimmed
            int level = map(out.value, -13801, 1, 0, 10);
            for (int i = out.out; i < out.out + level; i++) {
                Tlc.set(i, 4095);
            }
        } else {
            // map DME value to 1 LED, dimmed
            int level = map(out.value, -13801, 1, 0, 4095);
            Tlc.set(out.out, level);
        }

        Tlc.update();
    }

private:
    // output array {level, TLC number, DME number, bar}
    outputpin outputs[8] = {
            {0, 29, 1,  false}, // input DJ oben
            {0, 13, 2,  false}, // input Bar
            {0, 14, 3,  false}, // input spare
            {0, 1,  5,  true},  // Output Oben Level
            {0, 17, 6,  true},  // Output Bar Level
            {0, 33, 7,  true},  // Output Keller Level
            {0, 30, 15, false}, // Mic1 Level
            {0, 44, 16, false}, // Mic2 Level
    };

};

void setupPins() {
    pinMode(PIN_MUX1, INPUT);
    pinMode(PIN_MUX2, INPUT);
    pinMode(PIN_MUX3, INPUT);

    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
};

// enable error led
void enableError() {
    digitalWrite(ERROR_LED, HIGH);
}

// disable Error led
void disableError() {
    digitalWrite(ERROR_LED, LOW);
}

// Creating class objects
Inputs inputs;
Outputs outputs;

void setup() {
    pinMode(ERROR_LED, OUTPUT);
    enableError();
    // Start Serial - BAUD set in define
    Serial.begin(SERIAL_BAUD);
    setupPins();

    // init TLC for LEDs
    Tlc.init();
    Tlc.clear();

    inputs.update(true);

    disableError();

    // outputs.getDME();
};

void loop() {
    //update all inputs and send to dme

    // Tlc.clear();

    // send command for new outputs every 3rd loop & enable Error if no DME response
    // if (i > 2 && outputUpdate) {
    //     outputs.getDME();
    //     outputUpdate = false;
    //     disableError();
    // } else if (i > 10 && !outputUpdate){
    //     outputs.getDME();
    //     enableError();
    // }
    // i++;
    // read buffer and check for start / end markers
    // outputs.recvWithStartEndMarkers();
    // if (newData == true) {
    //     strcpy(tempChars, receivedChars);
    //     DPRINTLN(receivedChars);
    //     // this temporary copy is necessary to protect the original data
    //     //   because strtok() used in parseData() replaces the commas with \0
    //     outputs.parseData();
    //     newData = false;
    // }
    // outputs.getDME();
    inputs.update(false);
    delay(100);
};
