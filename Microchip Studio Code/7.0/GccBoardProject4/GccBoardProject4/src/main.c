#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>  // For snprintf

#define DOUT PD6          // Define data output pin (PD6)
#define SCK PD7           // Define clock pin (PD7)
#define MYUBRR 103        // Define baud rate setting (example for 9600 baud with F_CPU at 16MHz)

// Function prototypes
void uart_init(unsigned int ubrr);
void uart_transmit(unsigned char data);
void uart_transmit_string(const char* str);
void init_hx711_pins(void);
uint32_t read_hx711(void);

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

int main(void) {
	uart_init(MYUBRR);    // Initialize UART
	init_hx711_pins();    // Initialize HX711 pins

	uint32_t previous_weight = 0;  // Variable to store the previous weight reading

	while (1) {
		uint32_t current_weight = read_hx711();  // Read weight from HX711
		int32_t weight_difference = current_weight - previous_weight;  // Calculate the difference

		// Convert weight difference to a string and transmit via UART
		char buffer[30];
		// Check if the difference is positive or negative and print accordingly
		if (weight_difference > 100000) {
			snprintf(buffer, sizeof(buffer), "positive: %ld\n", weight_difference);
			uart_transmit_string(buffer);
			} else if (weight_difference < -100000) {
			snprintf(buffer, sizeof(buffer), "negative: %ld\n", weight_difference);
			uart_transmit_string(buffer);
		}


		previous_weight = current_weight;  // Update the previous weight

		_delay_ms(100);  // Wait for 100ms before the next reading
	}
}
