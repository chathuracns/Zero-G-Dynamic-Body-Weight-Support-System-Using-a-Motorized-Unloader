#define F_CPU 16000000UL  // Define CPU frequency as 16 MHz
#include <avr/io.h>
#include <util/delay.h>

void step(uint16_t delay);  // Prototype for the step function

int main(void)
{
	uint16_t delay = 1000;  // Initial delay in microseconds

	// Configure pins
	DDRD |= (1 << DDD4);  // Set PD4 as output for Direction
	DDRD |= (1 << DDD5);  // Set PD5 as output for Enable
	DDRD |= (1 << DDD3);  // Set PD3 as output for Pulse

	// Enable the stepper motor
	PORTD |= ~(1 << PORTD5);  // Set PD5 high to enable the stepper motor controller

	// Set direction
	//PORTD |= (1 << PORTD4);  // Set PD4 high for one direction
	PORTD &= ~(1 << PORTD4);

	// Main loop
	while (1) {
		// Acceleration phase
		for (uint16_t i = 0; i < 500; i++) {
			step(delay);
			if (delay > 100) delay -= 2;  // Decrease delay for acceleration
		}

		// Constant speed phase
		for (uint16_t i = 0; i < 1000; i++) {
			step(100);  // Run at higher speed
		}

		// Deceleration phase
		for (uint16_t i = 0; i < 500; i++) {
			step(delay);
			delay += 2;  // Increase delay for deceleration
		}

		// Pause between cycles
		//_delay_ms(50);
	}
}

// Function to generate a pulse with a specified delay
void step(uint16_t delay) {
	// Pulse high
	PORTD |= (1 << PORTD3);
	for (uint16_t i = 0; i < delay; i++) {
		_delay_us(1);  // Delay for 1 microsecond
	}
	// Pulse low
	PORTD &= ~(1 << PORTD3);
	for (uint16_t i = 0; i < delay; i++) {
		_delay_us(1);  // Delay for 1 microsecond
	}
}
