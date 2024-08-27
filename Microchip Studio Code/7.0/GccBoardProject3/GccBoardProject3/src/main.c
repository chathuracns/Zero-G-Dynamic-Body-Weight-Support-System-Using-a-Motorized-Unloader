#define F_CPU 16000000UL  // Define CPU frequency as 16 MHz
#define BAUD 9600         // Define baud rate
#define MYUBRR F_CPU/16/BAUD-1  // Calculate UBRR value for baud rate

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>  // For snprintf

#define DOUT PD6          // Define data output pin (PD6)
#define SCK PD7           // Define clock pin (PD7)
#define DIR_PIN PD4       // Define Direction pin (PD4)
#define EN_PIN PD5        // Define Enable pin (PD5)
#define PULSE_PIN PD3     // Define Pulse pin (PD3)
#define WEIGHT_THRESHOLD 10000  // Define the threshold for weight

// Function prototypes
void uart_init(unsigned int ubrr);
void uart_transmit(unsigned char data);
void uart_transmit_string(const char* str);
void init_hx711_pins(void);
uint32_t read_hx711(void);
void stepper_init(void);
void step(uint16_t delay);
void control_stepper(uint32_t weight);

int main(void) {
	uart_init(MYUBRR);      // Initialize UART
	init_hx711_pins();      // Initialize HX711 pins
	stepper_init();         // Initialize stepper motor pins

	while (1) {
		uint32_t weight = read_hx711();  // Read weight from HX711

		// Convert weight to a string and transmit via UART
		char buffer[20];
		snprintf(buffer, sizeof(buffer), "Weight: %lu\n", weight/100000);
		uart_transmit_string(buffer);

		control_stepper(weight);  // Control stepper motor based on weight

		_delay_ms(100);  // Wait for 100ms before the next reading
	}
}

void uart_init(unsigned int ubrr) {
	// Set baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	// Enable transmitter
	UCSR0B = (1 << TXEN0);
	// Set frame format: 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(unsigned char data) {
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0)));
	// Put data into buffer, sends the data
	UDR0 = data;
}

void uart_transmit_string(const char* str) {
	while (*str) {
		uart_transmit(*str++);
	}
}

void init_hx711_pins(void) {
	DDRD &= ~(1 << DOUT);  // Set DOUT (PD6) as input
	DDRD |= (1 << SCK);    // Set SCK (PD7) as output
	PORTD &= ~(1 << SCK);  // Initialize SCK to low
}

uint32_t read_hx711(void) {
	uint32_t count = 0;

	// Wait for the HX711 to be ready (DOUT to go low)
	while (PIND & (1 << DOUT));

	// Read 24-bit data
	for (uint8_t i = 0; i < 24; i++) {
		PORTD |= (1 << SCK);         // Set SCK high
		_delay_us(1);                // Short delay for stable clock
		count = count << 1;          // Shift count to the left
		PORTD &= ~(1 << SCK);        // Set SCK low
		if (PIND & (1 << DOUT)) {    // If DOUT is high
			count++;                 // Increment count
		}
	}

	// Set the gain to 128 by providing one more clock pulse
	PORTD |= (1 << SCK);  // Set SCK high
	_delay_us(1);         // Short delay
	PORTD &= ~(1 << SCK); // Set SCK low

	// Return the 24-bit data as a 32-bit value
	return count;
}

void stepper_init(void) {
	DDRD |= (1 << DIR_PIN) | (1 << EN_PIN) | (1 << PULSE_PIN);  // Set stepper pins as output
	PORTD |= (1 << EN_PIN);  // Enable the stepper motor controller
}

void step(uint16_t delay) {
	// Pulse high
	PORTD |= (1 << PULSE_PIN);
	for (uint16_t i = 0; i < delay; i++) {
		_delay_us(1);  // Delay for 1 microsecond
	}
	// Pulse low
	PORTD &= ~(1 << PULSE_PIN);
	for (uint16_t i = 0; i < delay; i++) {
		_delay_us(1);  // Delay for 1 microsecond
	}
}

void control_stepper(uint32_t weight) {
	if (weight + 100 > WEIGHT_THRESHOLD) {
		PORTD |= (1 << DIR_PIN);  // Set direction to clockwise
		} else if (weight - 100 < WEIGHT_THRESHOLD) {
		PORTD &= ~(1 << DIR_PIN); // Set direction to anti-clockwise
	}
	uint16_t delay = 1000;  // Initial delay in microseconds

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
}
