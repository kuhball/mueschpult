/*INCLUDES*/
#include <Arduino.h>


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

#define SERIAL_BAUD 38400

/* LEDS */
#define LED_INPUT_DJ 9
#define LED_INPUT_BAR 11
#define LED_INPUT_SPARE 10

#define ERROR_LED 13

#define LED_MIC_1 5
#define LED_MIC_2 6

/* Switch Pins */
#define SW_PIN_TO_1 7
#define SW_PIN_TO_2 11

/*Shift Register Pins*/
#define SCLK_PIN 2
#define SR_LATCH_PIN 3
#define SER_DATA_PIN 4


/*MUX STUFF*/
// read pins
#define PIN_MUX0 A3
#define PIN_MUX1 A5
#define PIN_MUX2 A4

// write pins for setting mux channel
#define MUX_S0 A0
#define MUX_S1 A1
#define MUX_S2 A2
/*MUX STUFF*/
/*
Mux_S0
Mux on Mainboard A0 - A4 N.C.
X5->Vol_Mic_2
X6->DELAY_SW
X7->Vol_Mic_1

MUX_S1(COM_1) Input Vol
X4->Vol DJ
X2->Vol_Bar
X1->Vol Spare

MUX_S2(COM_2) EQ and PA Vol

X0->EQ_BAR_LO
X1->EQ_BAR_MID
X2->EQ_BAR_HI
X3->VOL_BAR
X4->EQ_OBEN_HI
X5->VOL_OBEN
X6->EQ_OBEN_MID
X7->EQ_OBEN_LO


* /
/*
enum Mux_0 { VOL_MIC_2 = 5, DELAY_SW = 6, VOL_MIC_1 = 7 };
enum Mux_1 { VOL_INPUT_DJ = 4, VOL_INPUT_BAR = 2, VOL_INPUT_SPARE = 1 };
enum Mux_2 { EQ_BAR_LO, EQ_OBEN_HI, EQ_BAR_HI, EQ_OBEN_MID, EQ_BAR_MID, VOL_OBEN, VOL_BAR, EQ_OBEN_LO };

Mux_2 mux2;
Mux_1 mux1;
Mux_0 mux0;
*/

/* Function declarations */
void Blink();
void enableError();
void disableError();
void setupPins();
void writeLedBar(int, int);


/* variables */

struct inputpin {
	int value;
	uint8_t mux;  // Mux value to read from this input
	uint8_t pin;  // Pin to read from
	uint8_t dme;  // Value needed for DME set
	bool digital; // needed to distinguish analog from digital read
	bool stereo;  // set two dme values if stereo
};

struct outputpin {
	int value;
	uint8_t out; // TLC Number
	uint8_t dme; // DME Read number
	bool bar;    // set if led bar (size 10)
};

const byte numChars = 200;        //  buffer for dme read
char receivedChars[numChars];
char tempChars[numChars];         // temporary array for use when parsing

uint8_t i = 0;
unsigned long time = millis();

boolean newData = false;

// set if outputs were updated
boolean outputUpdate = false;


/* Class definitions */



class LedBar {

public:
	void init() {
		for (int i = 0; i < 20; i++)
		{
			led[i][0] = 0;
			led[i][1] = 0;
			led[i][2] = 0;
		}
		led[19][2] = 1 << 4;
		led[18][2] = 1 << 5;
		led[17][2] = 1 << 6;
		led[16][2] = 1 << 7;
		led[15][1] = 1 << 0;
		led[14][1] = 1 << 1;
		led[13][1] = 1 << 2;
		led[12][1] = 1 << 3;
		led[11][1] = 1 << 4;
		led[10][1] = 1 << 5;
		led[9][1] = 1 << 6;
		led[8][1] = 1 << 7;
		led[7][0] = 1 << 0;
		led[6][0] = 1 << 1;
		led[5][0] = 1 << 2;
		led[4][0] = 1 << 3;
		led[3][0] = 1 << 4;
		led[2][0] = 1 << 5;
		led[1][0] = 1 << 6;
		led[0][0] = 1 << 7;

	}

	void write(int bar0, int bar1) // write erwartet für bar0 und bar1 werte vom 0 bis 10
	{
		bar0 = bar0 - 1;
		for (bar0; bar0 >= 0; bar0--)
		{
			bytes[0] |= led[bar0][0];
			bytes[1] |= led[bar0][1];
			bytes[2] |= led[bar0][2];
			/*
			Serial.write(bytes[0]);
			Serial.write(bytes[1]);
			Serial.write(bytes[2]);
			Serial.println("");
			*/
		}
		bar1 = bar1 + 9;
		for (bar1; bar1 >= 10; bar1--)
		{
			bytes[0] |= led[bar1][0];
			bytes[1] |= led[bar1][1];
			bytes[2] |= led[bar1][2];
			/*
			Serial.write(bytes[0]);
			Serial.write(bytes[1]);
			Serial.write(bytes[2]);
			Serial.println("");
			*/
		}
		sendBytes();
		clearBytes();
	}

private:
	char byte[3] = { 0,0,0 };
	char led[20][3];
	char bytes[3] = { 0,0,0 };
	void clearBytes()
	{
		bytes[0] = 0;
		bytes[1] = 0;
		bytes[2] = 0;
	}
	void sendBytes()
	{
		digitalWrite(SR_LATCH_PIN, LOW);
		shiftOut(SER_DATA_PIN, SCLK_PIN, MSBFIRST, bytes[2]);
		shiftOut(SER_DATA_PIN, SCLK_PIN, MSBFIRST, bytes[1]);
		shiftOut(SER_DATA_PIN, SCLK_PIN, MSBFIRST, bytes[0]);
		digitalWrite(SR_LATCH_PIN, HIGH);
	}

};


class Inputs {
public:
	// send command over serial to DME
	void set(inputpin input) {
		// different command for EQ
		if (input.mux < 7 && input.mux > 3) {
			// map poti value to dme value (only ±6dB)
			int level = map(input.value, 0, 1023, -600, 600);
			send(level, input.dme);
		}
		else {
			send(input.value, input.dme);
			// Setting second value -> stereo
			if (input.stereo) {
				send(input.value, input.dme + 1);
			}
		}
	};

	void update(boolean setall) {
		// setall for initialisation and sending all values to the DME
		// Looping over inputpin inputs[22]
		for (int i = 0; i < 16; i++) {
			// Changing Mux if needed - normally every 3rd time
			if (inputs[i].mux != currentMux) {
				setMux(inputs[i].mux);
				currentMux = inputs[i].mux; // saving current Mux pin
			}

			// analog / digital read the input pin
			int cache = readPin(inputs[i].pin, inputs[i].digital);


			// debouncing value - only values with a bigger difference than 3 are send
			if (cache < inputs[i].value - 3 || cache > inputs[i].value + 3 || setall) {
				inputs[i].value = cache;
				set(inputs[i]);
			}
			else if ((inputs[i].digital && cache != inputs[i].value) || setall) { // digital always
				inputs[i].value = cache;
				set(inputs[i]);
			}
		}
	};
private:
	/*
		Mux_S0
		Mux on Mainboard A0 - A4 N.C.
		X5->Vol_Mic_2
		X6->DELAY_SW
		X7->Vol_Mic_1

		MUX_S1(COM_1) Input Vol
		X4->Vol DJ
		X2->Vol_Bar
		X1->Vol Spare

		MUX_S2(COM_2) EQ and PA Vol

		X0->EQ_BAR_LO
		X1->EQ_BAR_MID
		X2->EQ_BAR_HI
		X3->VOL_BAR
		X4->EQ_OBEN_HI
		X5->VOL_OBEN
		X6->EQ_OBEN_MID
		X7->EQ_OBEN_LO


	*/
	// input array - if needed change size, for loop and add input
	// {level, mux, pin, dme, digital, stereo}
	inputpin inputs[16] = { //  old -> inputs[22]
		// SWITCHES
		{0, 0, SW_PIN_TO_1, 50, true,  false},   // Talkover Mic1
		{0, 0, SW_PIN_TO_2, 51, true,  false},   // Talkover Mic2
		//{0, 0, PIN_MUX3, 52, true,  false},   // Mute Mic1
		//{0, 1, PIN_MUX1, 53, true,  false},   // Mute Mic2
		// POTIS
		{0, 7, PIN_MUX0, 54, false, false},   // Volume Mic1
		{0, 5, PIN_MUX0, 55, false, false},   // Volume Mic2
		{0, 4, PIN_MUX1, 58, false, true},   // Input DJ
		{0, 2, PIN_MUX2, 60, false, true},   // Input Bar
		{0, 1, PIN_MUX1, 62, false, true},   // Input Spare
		{0, 5, PIN_MUX2, 64, false, true},   // Output Oben
		{0, 3, PIN_MUX2, 66, false, true},   // Output Bar
		//{0, 3, PIN_MUX2, 68, false, true},   // Output Unten
		// EQ Oben
		{0, 4, PIN_MUX2, 70, false, false},   // High
		{0, 6, PIN_MUX2, 71, false, false},   // Mid
		{0, 7, PIN_MUX2, 72, false, false},   // Low
		// EQ Bar
		{0, 2, PIN_MUX2, 73, false, false},   // High
		{0, 1, PIN_MUX2, 74, false, false},   // Mid
		{0, 0, PIN_MUX2, 75, false, false},   // Low
		// EQ Keller
		//{0, 6, PIN_MUX1, 76, false, false},   // High
		//{0, 6, PIN_MUX2, 77, false, false},   // Mid
		//{0, 6, PIN_MUX3, 78, false, false},   // Low
		// extra
		{0, 6, PIN_MUX0, 79, true,  true},    // Delay Switch
	};

	// cached mux pin
	uint8_t currentMux;

	// read analog / digital pin
	int readPin(uint8_t pin, bool digital) {
		if (!digital) {
			return analogRead(pin);
		}
		else {
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

	// write dme command on serial, different command for volumes
	void send(int level, uint8_t dme) {
		if (dme < 70 && dme > 53) {
			Serial.print("SVL 0 ");
			Serial.print(dme);
			Serial.print(" ");
			Serial.print(level);
			Serial.print('\n');
		}
		else {
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
		Serial.print("GMT 0 56 0 \n");
	};

	// from https://forum.arduino.cc/index.php?topic=288234.0
	void recvWithStartEndMarkers() {
		static boolean recvInProgress = false;
		static byte ndx = 0;
		char startMarker = 'U';
		char endMarker = 'H';
		char rc;

		while (Serial.available() > 0 && newData == false) {
			rc = Serial.read();

			if (recvInProgress == true) {
				if (rc != endMarker) {
					receivedChars[ndx] = rc;
					ndx++;
					if (ndx >= numChars) {
						ndx = numChars - 1;
					}
				}
				else {
					recvInProgress = false;
					ndx = 0;
					newData = true;
				}
			}
			else if (rc == startMarker) {
				recvInProgress = true;
			}
		}
	}

	void parseData() {      // split the data into its parts

		char *strtokIndx; // this is used by strtok() as an index

		strtokIndx = strtok(tempChars, " ");      // get the first part - the string

		// read the first 8 values of DME response
		// MTR 0 56 0 CUR -9428 -9780 -9376 -13801 -7432 -9004 -9004 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801
		for (uint8_t i = 0; i < 7; i++) {
			strtokIndx = strtok(NULL, " ");      // get the first part - the string
			outputs[i].value = atoi(strtokIndx);     // convert this part to an integer
			set(outputs[i]);
		}
		outputUpdate = true;
	}

	/*
struct outputpin {
	int value;
	uint8_t out; // TLC Number
	uint8_t dme; // DME Read number
	bool bar;    // set if led bar (size 10)
};*/

// set LED Bar output
	void set(outputpin out) {
		if (out.bar)
		{
			// map DME value to 10 LEDs, not dimmed
			int level = map(out.value, -13801, 1, 0, 10);
			writeLedBar(out.out, level);

		}
		else {
			// map DME value to 1 LED, dimmed
			int level = map(out.value, -13801, 1, 0, 255);
			analogWrite(out.out, level);
		}
	}
	void setTest(int b0, int b1)
	{
		writeLedBar(b0, b1);
	}


private:
	// output array {level, Bar number, DME number, bar}
	outputpin outputs[7] = {
			{0, LED_INPUT_DJ, 1,  false}, // input DJ oben
			{0, LED_INPUT_BAR, 2,  false}, // input Bar
			{0, LED_INPUT_SPARE, 3,  false}, // input spare
			{0, 0,  5,  true},  // Output Oben Level
			{0, 1, 6,  true},  // Output Bar Level
			//{0, 33, 7,  true},  // Output Keller Level
			{0, LED_MIC_1, 15, false}, // Mic1 Level
			{0, LED_MIC_2, 16, false}, // Mic2 Level
	};

};


/* Declaring Classes*/

class Inputs;
class Outputs;
class LedBar;

// Creating class objects
Inputs inputs;
Outputs outputs;
LedBar ledBar;

void setup() {
	pinMode(ERROR_LED, OUTPUT);
	enableError();
	// Start Serial - BAUD set in define
	Serial.begin(SERIAL_BAUD);
	setupPins();
	ledBar.init();


	// send all poti values to dme for initialisation
	inputs.update(true);

	// receive all dme values
	outputs.getDME();

	disableError();
	Blink();
};

void loop() {
	/*
	//update all inputs and send to dme
	if (millis() - time > 100) {
		inputs.update(false);
		while (Serial.available() > 0) {
			Serial.read();
		}
		outputs.getDME();
		time = millis();
	}
	*/

	// // send command for new outputs every 3rd loop & enable Error if no DME response
	// if (i > 5 && outputUpdate) {
	//
	//     outputUpdate = false;
	//     disableError();
	//     i=0;
	// } else if (i > 20 && !outputUpdate){
	//     outputs.getDME();
	//     enableError();
	// }
	// i++;
	// read buffer and check for start / end markers
	/*
	outputs.recvWithStartEndMarkers();
	if (newData == true) {
		strcpy(tempChars, receivedChars);
		// this temporary copy is necessary to protect the original data
		//   because strtok() used in parseData() replaces the commas with \0
		//Tlc.clear();
		outputs.parseData();
		newData = false;
	}
	*/
	for (int i = 0; i <= 10; i++)
	{
		//ledBar.write(i, i);
		outputs.setTest(i, i);
		delay(200);
	}
	DPRINTLN("loop done");

	// TODO: enable error LED if no answer from DME
	// TODO: test this shit!
};


/* Function Definitions */

// Startup animation
void Blink()
{
	for (int i = 0; i < 255; i++)
	{
		analogWrite(LED_BUILTIN, i);
		analogWrite(LED_INPUT_BAR, i);
		analogWrite(LED_INPUT_DJ, i);
		analogWrite(LED_INPUT_SPARE, i);
		analogWrite(LED_MIC_1, i);
		analogWrite(LED_MIC_2, i);
		delay(2);
	}
	for (int i = 255; i > 0; i--)
	{
		analogWrite(LED_BUILTIN, i);
		analogWrite(LED_INPUT_BAR, i);
		analogWrite(LED_INPUT_DJ, i);
		analogWrite(LED_INPUT_SPARE, i);
		analogWrite(LED_MIC_1, i);
		analogWrite(LED_MIC_2, i);
		delay(2);
	}
	for (int i = 0; i < 10; i++)
	{
		ledBar.write(i, 0);
		delay(20);
	}
	for (int i = 0; i < 10; i++)
	{
		ledBar.write(0, i);
		delay(20);
	}

	delay(200);

	analogWrite(LED_BUILTIN, 255);
	analogWrite(LED_INPUT_BAR, 255);
	analogWrite(LED_INPUT_DJ, 255);
	analogWrite(LED_INPUT_SPARE, 255);
	analogWrite(LED_MIC_1, 255);
	analogWrite(LED_MIC_2, 255);
	ledBar.write(10, 10);
	delay(200);

	analogWrite(LED_BUILTIN, 0);
	analogWrite(LED_INPUT_BAR, 0);
	analogWrite(LED_INPUT_DJ, 0);
	analogWrite(LED_INPUT_SPARE, 0);
	analogWrite(LED_MIC_1, 0);
	analogWrite(LED_MIC_2, 0);
	ledBar.write(0, 0);
	delay(200);

	analogWrite(LED_BUILTIN, 255);
	analogWrite(LED_INPUT_BAR, 255);
	analogWrite(LED_INPUT_DJ, 255);
	analogWrite(LED_INPUT_SPARE, 255);
	analogWrite(LED_MIC_1, 255);
	analogWrite(LED_MIC_2, 255);
	ledBar.write(10, 10);
	delay(200);

	analogWrite(LED_BUILTIN, 0);
	analogWrite(LED_INPUT_BAR, 0);
	analogWrite(LED_INPUT_DJ, 0);
	analogWrite(LED_INPUT_SPARE, 0);
	analogWrite(LED_MIC_1, 0);
	analogWrite(LED_MIC_2, 0);
	ledBar.write(0, 0);
	delay(200);
}

// enable error led
void enableError() {
	digitalWrite(ERROR_LED, HIGH);
}

// disable Error led
void disableError() {
	digitalWrite(ERROR_LED, LOW);
}

void setupPins() {
	pinMode(PIN_MUX0, INPUT);
	pinMode(PIN_MUX1, INPUT);
	pinMode(PIN_MUX2, INPUT);

	pinMode(MUX_S0, OUTPUT);
	pinMode(MUX_S1, OUTPUT);
	pinMode(MUX_S2, OUTPUT);
	
	pinMode(SCLK_PIN, OUTPUT);
	pinMode(SR_LATCH_PIN, OUTPUT);
	pinMode(SER_DATA_PIN, OUTPUT);
	pinMode(ERROR_LED, OUTPUT);

	pinMode(SW_PIN_TO_1, INPUT_PULLUP);
	pinMode(SW_PIN_TO_2, INPUT_PULLUP);
}

void writeLedBar(int b0, int b1)
{
	ledBar.write(b0, b1);
}