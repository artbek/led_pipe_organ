#include "LPD8806.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma


// Example to control LPD8806-based RGB LED Modules in a strip

/*****************************************************************************/

// Chose 2 pins for output; can be any valid output pins:
// int dataPin  = 2;
// int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(100);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);

#define PIN_PWM   3
#define PIN_DATA  11
#define PIN_CLOCK 13
#define PIN_BRIGHTNESS 2
#define PIN_RED 4
#define PIN_GREEN 5
#define PIN_BLUE 6
#define PIN_CHANGE_COLOUR 7

#define PIPES_COUNT  7
#define PIPE_LEN    10
#define WAIT_TIME   10

#define COLOURS_COUNT 6
#define LED_COUNT 10

int LED_COLOUR[LED_COUNT];

byte inputPin[PIPES_COUNT] = {
	4, 5, 6, 7, 8, 9, 12
};

int pipeEnds[2][PIPES_COUNT] = {
	{ 0,  20, 40, 60, 80, 100, 120 },
	{ 19, 39, 59, 79, 99, 119, 139 },
};

byte handIsOver[PIPES_COUNT] = {
	0, 0, 0, 0, 0, 0, 0
};

byte pipeIsOn[PIPES_COUNT] = {
	0, 0, 0, 0, 0, 0, 0
};

uint8_t RED = 0;
uint8_t GREEN = 0;
uint8_t BLUE = 0;

float RED_RATIO = 0.1;
float GREEN_RATIO = 0;
float BLUE_RATIO = 0;

uint8_t BRIGHTNESS = 10;
uint8_t BRIGHTNESS_MAX = 50;

bool processingBrighness = false;


bool processingChange = false;

void setup()
{
	/* Init the LED strip */

	strip.begin();
	strip.show();


	/* Input PINs */

/*
	for (byte i = 0; i < PIPES_COUNT; i++) {
		pinMode(inputPin[i], INPUT);
		digitalWrite(inputPin[i], HIGH);
	}
*/

		pinMode(PIN_BRIGHTNESS, INPUT);
		digitalWrite(PIN_BRIGHTNESS, HIGH);

		pinMode(PIN_CHANGE_COLOUR, INPUT);
		digitalWrite(PIN_CHANGE_COLOUR, HIGH);

		pinMode(PIN_RED, INPUT);
		digitalWrite(PIN_RED, HIGH);

		pinMode(PIN_GREEN, INPUT);
		digitalWrite(PIN_GREEN, HIGH);

		pinMode(PIN_BLUE, INPUT);
		digitalWrite(PIN_BLUE, HIGH);


	/* Init PWM (36kHz) for IR transmitters */

/*
	pinMode(PIN_PWM, OUTPUT);
	TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(WGM22) | _BV(CS21);
	OCR2A = 54;
	OCR2B = 26;
*/
//Serial.begin(9600);


	for (int i = 0; i < LED_COUNT; i++) {
		LED_COLOUR[i] = 0;
	}
}


int col = 0;


void loop()
{
	/*
	detect_hand_position();

	for (byte i = 0; i < PIPES_COUNT; i++) {
		if (handIsOver[i] && ! pipeIsOn[i]) {
			switchPipeOn(i);
		} else if (! handIsOver[i] && pipeIsOn[i]) {
			switchPipeOff(i);
		}
	}
	*/

	//brightness_toggle();

	Serial.begin(9600);


	for (int led_index = 0; led_index < LED_COUNT; led_index++) {
		strip.setPixelColor(led_index, get_colour(LED_COLOUR[led_index]));
		strip.show();
		process_colour_buttons(led_index);
		delay(500);
	}

	for (int i = 0; i < 10; i++) {
		process_colour_buttons(0);
		delay(100);
	}

	for (int led_index = 0; led_index < LED_COUNT; led_index++) {
		strip.setPixelColor(led_index, 0, 0, 0);
		strip.show();
		delay(500);
	}

}


uint8_t colours[COLOURS_COUNT][3] = {
	{ 100, 0, 0 },
	{ 100, 100, 0 },
	{ 0, 100, 0 },
	{ 0, 100, 100 },
	{ 0, 0, 100 },
	{ 100, 0, 100 },
};

uint32_t get_colour(uint8_t c)
{
  uint8_t r, g, b;

	r = colours[c][0];
	b = colours[c][1];
	g = colours[c][2];

	return(strip.Color(r,g,b));
}


int get_next_colour_index(int current_colour)
{
	int new_colour = current_colour + 1;

	if (new_colour >= COLOURS_COUNT) new_colour = 0;

	return new_colour;

}


int get_next_led_index(int current_index)
{
	int new_index = current_index + 1;

	if (new_index >= LED_COUNT) new_index = 0;

	return new_index;
}

void dip_in_the_next_paint_bucket()
{
	col = get_next_colour_index(col);
}


bool process_colour_buttons(int led_index)
{
	if (! processingChange && digitalRead(PIN_CHANGE_COLOUR) == LOW) {
		Serial.println(led_index);
		Serial.println(get_next_led_index(led_index));
		processingChange = true;
		dip_in_the_next_paint_bucket();
		LED_COLOUR[get_next_led_index(led_index)] = col;
		Serial.print(get_next_led_index(led_index));
		Serial.print(" = ");
		Serial.println(get_next_colour_index(LED_COLOUR[led_index]));
		delay(2);
		return true;
	}

	if (digitalRead(PIN_CHANGE_COLOUR) == HIGH) {
		processingChange = false;
	}

	//Serial.println(LED_COLOUR);

	return false;
}


/* HELPERS */


//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}

void brightness_toggle()
{
	if (processingBrighness) return;

	if (digitalRead(PIN_BRIGHTNESS) == LOW) {
		processingBrighness = true;

		BRIGHTNESS_MAX += 5;
		
		if (BRIGHTNESS_MAX > 50) {
			BRIGHTNESS_MAX = 0;
		}

		delay(100);

		processingBrighness = false;
	}
}


void detect_hand_position()
{
	for (byte i = 0; i < PIPES_COUNT; i++) {
		if (digitalRead(inputPin[i]) == LOW) {
			handIsOver[i] = 1;
		} else {
			handIsOver[i] = 0;
		}
	}
}


void switchPipeOn(byte i)
{
	while(true) {

		for (int led_index = 0; led_index < PIPE_LEN; led_index++) {
			strip.setPixelColor(pipeEnds[0][i] + led_index, RED, BLUE, GREEN);
			strip.show();
			delay(50);
		}

		delay(60);
	}
}


void switchPipeOff(byte i)
{
	for (int led_index = PIPE_LEN; led_index >= 0 ; led_index--) {
		strip.setPixelColor(pipeEnds[0][i] + led_index, 0, 0, 0);
		strip.setPixelColor(pipeEnds[1][i] - led_index, 0, 0, 0);

		strip.show();
		pipeIsOn[i] = 0;
		delay(WAIT_TIME);
	}
}

