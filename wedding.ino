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
LPD8806 strip = LPD8806(160);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);


#define PIPES_COUNT 7
#define PIPE_LEN 20
#define WAIT_TIME 20


byte inputPin[PIPES_COUNT] = {
	1, 2, 2, 2, 2, 2, 2
};

byte handIsOver[PIPES_COUNT] = {
	0, 0, 0, 0, 0, 0, 0
};

byte pipeIsOn[PIPES_COUNT] = {
	0, 0, 0, 0, 0, 0, 0
};

int pipeEnds[2][PIPES_COUNT] = {
	{	0, 8, 16 },
	{	0, 8, 16 },
};


void setup()
{
	/* Init the LED strip */

  strip.begin();
  strip.show();


	/* Input PINs */

	for (byte i = 0; i < PIPES_COUNT; i++) {
		pinMode(inputPin[i], INPUT);
		digitalWrite(inputPin[i], HIGH);
	}


	/* Init PWM (36kHz) for IR transmitters */

	pinMode(3, OUTPUT);
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS21);
  OCR2A = 54;
  OCR2B = 26;
}


void loop()
{
	detect_hand_position();

	for (byte i = 0; i < PIPES_COUNT; i++) {
		if (handIsOver[i] && ! pipeIsOn[i]) {
			switchPipeOn(i);
		} else if (! handIsOver[i] && pipeIsOn[i]) {
			switchPipeOff(i);
		}
	}
}


/* HELPERS */

void detect_hand_position()
{
	for (byte i = 0; i < PIPES_COUNT; i++) {
		if (digitalRead(inputPin[i])) {
			handIsOver[i] = 1;
		} else {
			handIsOver[i] = 0;
		}
	}
}


void switchPipeOn(byte i)
{
  for (int led_index = 0; led_index < PIPE_LEN; led_index++) {
      strip.setPixelColor(pipeEnds[0][i] + led_index, 10, 2, 0);
      strip.setPixelColor(pipeEnds[1][i] - led_index, 2, 10, 0);

      strip.show();
      delay(WAIT_TIME);
  }

}


void switchPipeOff(byte i)
{
  for (int led_index = PIPE_LEN; led_index >= 0 ; led_index--) {
      strip.setPixelColor(pipeEnds[0][i] + led_index, 0, 0, 0);
      strip.setPixelColor(pipeEnds[1][i] - led_index, 0, 0, 0);

      strip.show();
      delay(WAIT_TIME);
  }
}

