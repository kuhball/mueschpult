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
// MUX1 fader 1-8
#define PIN_MUX1 A0
// Fader 9-16 and Button 1-6
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
    bool changed; // only sent value to DME if changed
};

struct outputpin {
    int value;
    uint8_t out; // TLC Number
    uint8_t dme; // DME Read number
    bool bar;    //
};

class Inputs {
public:
    struct inputpin get(unsigned char i) {
        return inputs[i];
    };

    void update() {
        for (inputpin x: inputs) {
            if (x.mux != currentMux) {
                setMux(x.mux);
                currentMux = x.mux;
            }

            int cache = readPin(x.pin, x.digital);


            if (cache < x.value - 3 || cache > x.value + 3) {
                x.value = cache;
                x.changed = true;
            } else if (x.digital && cache != x.value){
                x.value = cache;
                x.changed = true;
            } else {
              x.changed = false;
            }
        }
    };
private:
    inputpin inputs[21] = {
            // SWITCHES
            {0, 0, PIN_MUX1, 50, true,  false},   // Talkover Mic1
            {0, 0, PIN_MUX2, 51, true,  false},   // Talkover Mic2
            {0, 0, PIN_MUX3, 52, true,  false},   // Mute Mic1
            {0, 1, PIN_MUX1, 53, true,  false},   // Mute Mic2
            // POTIS
            {0, 1, PIN_MUX2, 54, false, false},   // Volume Mic1
            {0, 1, PIN_MUX3, 55, false, false},   // Volume Mic2
            {0, 2, PIN_MUX1, 58, false, false},   // Input DJ
            {0, 2, PIN_MUX2, 60, false, false},   // Input Bar
            {0, 2, PIN_MUX3, 62, false, false},   // Input Spare
            {0, 3, PIN_MUX1, 64, false, false},   // Output Oben
            {0, 3, PIN_MUX2, 66, false, false},   // Output Bar
            {0, 3, PIN_MUX3, 68, false, false},   // Output Unten
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
    };

    uint8_t currentMux;

    int readPin(uint8_t pin, bool digital) {
        if (!digital) {
            return analogRead(pin);
        } else {
            return !(digitalRead(pin));
        }
    }

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
    void setBar(outputpin out) {
        Tlc.clear();
        int level = map(out.value, 0, 1023, 0, 10);
        for (int i = out.out; i < out.out + level; i++) {
            Tlc.set(i, 4095);
        }
        Tlc.update();
    };
};

class DME {
public:
    DME() {
        Serial.println("Com started.");
    };
};

void setupPins()
{
	pinMode(PIN_MUX1, INPUT);
	pinMode(PIN_MUX2, INPUT);
	pinMode(PIN_MUX3, INPUT);

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
}

// Creating class objects
Inputs inputs;
Outputs outputs;

void setup() {
    Serial.begin(9600);
    setupPins();
    Tlc.init();
    Tlc.clear();

    digitalWrite(MUX_S0, HIGH);
    digitalWrite(MUX_S1, LOW);
    digitalWrite(MUX_S2, LOW);
};

void loop() {
  // for (int i = 0; i < 48; i++) {
  //   Tlc.set(i, 4095);
  // }
  // Tlc.update();
  // delay(1000);
  outputs.setBar({analogRead(PIN_MUX2),1,7,true});
};
