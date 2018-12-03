/*INCLUDES*/
#include <Arduino.h>
#include "Tlc5940.h"
#include <SoftwareSerial.h>
#include <ArduinoSTL.h>
#include <string.h>
#include <map>


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

#define MUX_S0 6
#define MUX_S1 7
#define MUX_S2 8

/*MUX STUFF*/


enum Tlc5940Pins {
	SCLK = 13, SIN=11, BLANK=10, XLAT= 9, GSCLK= 3,
};


struct inputpin {
  int value;
  int mux;
  int pin;
  int dme;
};

std::map<String, inputpin> inputs = {
  {"test",{1,2,3,4}},
  {"TALKOVER_MIC1",{-13801,0,PIN_MUX1,50}}
};


class FrontMux {
public:
  FrontMux(){
  };
private:
  void setMux(int mux)
	{
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

class DME {
public:
  DME() {
    Serial.println("Com started.");
  };
};


void setup() {
  Serial.begin(9600);

  Tlc.init();
  Tlc.clear();

  if(inputs.find("test") != inputs.end()){
    DPRINTLN(inputs["test"].dme);
  };
};

void loop() {

};
