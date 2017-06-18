#include "LPD8806.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma

#define PIN_PWM   3
#define PIN_DATA  11
#define PIN_CLOCK 13
#define PIN_CHANGE_COLOUR 7

#define COLOURS_COUNT 6
#define LED_COUNT 60
#define BRIGHTNESS_MAX 50

// Example to control LPD8806-based RGB LED Modules in a strip

/*****************************************************************************/

// Chose 2 pins for output; can be any valid output pins:
// int dataPin  = 2;
// int clockPin = 3;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(LED_COUNT);

// You can optionally use hardware SPI for faster writes, just leave out
// the data and clock pin parameters.  But this does limit use to very
// specific pins on the Arduino.  For "classic" Arduinos (Uno, Duemilanove,
// etc.), data = pin 11, clock = pin 13.  For Arduino Mega, data = pin 51,
// clock = pin 52.  For 32u4 Breakout Board+ and Teensy, data = pin B2,
// clock = pin B1.  For Leonardo, this can ONLY be done on the ICSP pins.
//LPD8806 strip = LPD8806(nLEDs);


bool isProcessingChange = false;
int current_colour = 0;


void setup()
{
	/* Init the LED strip */

	strip.begin();
	strip.show();


	/* Init PWM (36kHz) for IR transmitters */

	pinMode(PIN_PWM, OUTPUT);
	TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(WGM22) | _BV(CS21);
	OCR2A = 54;
	OCR2B = 26;
	delay(100);


	/* Input PINs */

	pinMode(PIN_CHANGE_COLOUR, INPUT);
	digitalWrite(PIN_CHANGE_COLOUR, HIGH);
}


void loop()
{
	int brightness;

	for (brightness = 0; brightness < BRIGHTNESS_MAX; brightness++) {
		for (int led_index = 0; led_index < LED_COUNT; led_index++) {
			strip.setPixelColor(led_index, get_colour(current_colour, brightness));
		}
			strip.show();
			delay(30);
			process_colour_buttons();
	}

	for (brightness; brightness >= 0; brightness--) {
		for (int led_index = 0; led_index < LED_COUNT; led_index++) {
			strip.setPixelColor(led_index, get_colour(current_colour, brightness));
		}
			strip.show();
			delay(30);
			process_colour_buttons();
	}
}


void rainbow(uint8_t wait) {
  int i, j;

  for (j=0; j < 384; j++) {     // 3 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 384));
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }
}


uint8_t colours[COLOURS_COUNT][3] = {
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 },
	{ 0, 1, 1 },
	{ 0, 0, 1 },
	{ 1, 0, 1 },
};

uint32_t get_colour(uint8_t c, uint8_t brightness)
{
  uint8_t r, g, b;

	r = colours[c][0] * brightness;
	b = colours[c][1] * brightness;
	g = colours[c][2] * brightness;

	return(strip.Color(r,g,b));
}


int cycle_colours()
{
	int new_colour = current_colour + 1;

	if (new_colour >= COLOURS_COUNT) {
		new_colour = 0;
	}

	current_colour = new_colour;
}


bool process_colour_buttons()
{
	if (! isProcessingChange && digitalRead(PIN_CHANGE_COLOUR) == LOW) {
		isProcessingChange = true;

		cycle_colours();

		delay(50);
		return true;
	}

	if (digitalRead(PIN_CHANGE_COLOUR) == HIGH) {
		isProcessingChange = false;
	}


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

