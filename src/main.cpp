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

class Inputs {
public:
    // send command over serial to DME
    void set(inputpin input) {
        int level;

        // different command for EQ
        if (input.mux < 7 && input.mux > 3) {
            level = map(input.value, 0, 1023, -1800, 1800);
            Serial.print("SPR 0 ");
            Serial.print(input.dme);
            Serial.print(" ");
            Serial.print(level);
            Serial.print('\n');
        } else {
            if (!input.digital){
              level = map(input.value, 0, 1023, -13801, 1000);
            } else {
              level = input.value;
            }
            Serial.print("SPR 0 ");
            Serial.print(input.dme);
            Serial.print(" ");
            Serial.print(level);
            Serial.print('\n');

            // Setting two values -> Stereo
            if (input.stereo) {
                Serial.print("SPR 0 ");
                Serial.print(input.dme + 1);
                Serial.print(" ");
                Serial.print(level);
                Serial.print('\n');
            }
        }
    };

    void update() {
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
            if (cache < inputs[i].value - 3 || cache > inputs[i].value + 3) {
                inputs[i].value = cache;
                set(inputs[i]);
            } else if (inputs[i].digital && cache != inputs[i].value) { // digital always
                inputs[i].value = cache;
                set(inputs[i]);
            }
        }
    };
private:
    // input array - if needed change size, for loop and add input
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
            {0, 7, PIN_MUX1, 79, true,  false},    // Delay Switch
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
};

class Outputs {
public:
    void getDME(){
        Serial.print("GMT 0 56 0 \n");
        if (Serial.available())
        {
           s = Serial.readStringUntil('\n');   // Until CR (Carriage Return)
        }
        DPRINTLN(s);
        DPRINTLN(s.substring(126));

    };
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
            int level = map(out.value, 0, 1023, 0, 4095);
            Tlc.set(out.out, level);
        }

        Tlc.update();
    };
private:
    String s;

    // output array
    outputpin outputs[9] = {
            {0, 1,  1,  true}, // input DJ oben
            {0, 17, 2,  true}, // input Bar
            {0, 32, 3,  true}, // input spare
            {0, 11, 15, false}, // Mic1 Level
            {0, 12, 16, false}, // Mic2 Level
            {0, 13, 5,  false}, // Output Oben Level
            {0, 14, 6,  false}, // Output Bar Level
            {0, 15, 7,  false}, // Output Keller Level
            {0, 28, 99, false} // Error LED
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

// Creating class objects
Inputs inputs;
Outputs outputs;

void setup() {
    // Start Serial - BAUD set in define
    Serial.begin(SERIAL_BAUD);
    setupPins();

    // init TLC for LEDs
    Tlc.init();
    Tlc.clear();
};


void loop() {
    // Tlc.clear();
    // //update all inputs and send to dme
    // inputs.update();
    // delay(1000);
    outputs.getDME();
    delay(1000);
};
