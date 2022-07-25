#include "mcu.h"

typedef enum {
	GREEN=12,
	ORANGE,
	RED,
	BLUE
} led;

// LED functions
void led_init(void); // Initialize LED GPIO pins
void led_on(led l); // Turn on LED
void led_off(led l); // Turn off LED
void led_toggle(led l); // Toggle LED state

void mcu_init(void); // Initialize LED and fault handers
