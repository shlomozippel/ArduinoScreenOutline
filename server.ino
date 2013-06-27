#include <FastSPI_LED2.h> 

// limit max number of LEDs
#define MAX_NUM_LEDS	150

enum PROTOCOL {
	PROTOCOL_ESCAPE = 0x13,
	PROTOCOL_START = 0x37,
};

enum STATE {
	STATE_WAITING_FOR_START = 0,
	STATE_WAITING_FOR_COMMAND,
	STATE_WAITING_FOR_LED_COUNT,
	STATE_LED_DATA,
};

enum COMMAND {
	COMMAND_HELLO = 1,
	COMMAND_LEDS = 2,
};

byte _ledTable[256] = {
  0,   1,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,
  8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  11,  12,  12,  12,  13,  13,
 14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,
 22,  22,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,
 33,  33,  34,  35,  36,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
 46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
 61,  62,  63,  64,  65,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
 80,  81,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,
101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
153, 155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255 
}; 

struct CRGB { 
  byte g; 
  byte r; 
  byte b;
  void interpolate(CRGB& from, CRGB &to, byte alpha) {
    this->r = _ledTable[map(alpha, 0, 255, from.r, to.r)];
    this->g = _ledTable[map(alpha, 0, 255, from.g, to.g)];
    this->b = _ledTable[map(alpha, 0, 255, from.b, to.b)];
  }
}; 

struct CRGB leds[MAX_NUM_LEDS];

WS2811Controller800Mhz<9> LED;


void setup() {
	LED.init();
	Serial.begin(115200);

	memset(leds, 0,  MAX_NUM_LEDS * sizeof(struct CRGB));
	LED.showRGB((byte*)leds, MAX_NUM_LEDS); 
}

// state
int state = STATE_WAITING_FOR_START;

byte input = 0;
byte prev_input = 0;

int led_count = 0;
int current_pos = 0;
int target_pos = 0;

void loop() {

	while (Serial.available() > 0) {
		prev_input = input;
		input = Serial.read();

		// reset state machine if we need to
		if (PROTOCOL_ESCAPE == prev_input && PROTOCOL_START == input) {
			state = STATE_WAITING_FOR_COMMAND;
			continue;
		}

		// if we got the escape char, lets see what the next one will be
		if (PROTOCOL_ESCAPE != prev_input && PROTOCOL_ESCAPE == input) {
			continue;
		}

		// state machine
		switch (state) {
		case STATE_WAITING_FOR_START: break;
		case STATE_WAITING_FOR_COMMAND:
			switch (input) {
			case COMMAND_HELLO:
				Serial.println("Hello, world!");
				break;

			case COMMAND_LEDS:
				state = STATE_WAITING_FOR_LED_COUNT;
				memset(leds, 0,  MAX_NUM_LEDS * sizeof(struct CRGB)); 
				break;
			}
			break;

		case STATE_WAITING_FOR_LED_COUNT:
			led_count = min(input, MAX_NUM_LEDS);
			current_pos = 0;
			target_pos = led_count * sizeof(struct CRGB);
			state = STATE_LED_DATA;
			break;

		case STATE_LED_DATA:
			((byte *)leds)[current_pos] = _ledTable[input];
			current_pos++;
			
			// show the LEDs if we reached the end of the data
			if (current_pos == target_pos) {
			    LED.showRGB((byte*)leds, MAX_NUM_LEDS); 
				state = STATE_WAITING_FOR_START;
			}
			break;
		}
	}
}
