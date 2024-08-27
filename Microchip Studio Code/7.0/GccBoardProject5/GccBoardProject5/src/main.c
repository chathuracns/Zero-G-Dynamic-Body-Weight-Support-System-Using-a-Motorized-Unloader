// Define CPU frequency and baud rate
#define F_CPU 16000000UL  // 16 MHz clock speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>  // For abs()
#include <avr/interrupt.h>

// Define pins for stepper motor control
#define STEP_PIN PD3
#define DIR_PIN PD4
#define EN_PIN PD5

#define safety_btn PB0  // Define your button pins if not already defined
#define start_btn PB1
#define GND_btn PB2

#define DEBOUNCE_DELAY_MS 50

// HX711 load cell amplifier pins
#define HX711_DOUT PD6  // Data output pin
#define HX711_SCK PD7   // Clock pin



// UART Initialization
void uart_init(unsigned int ubrr) {
	// Set baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;

	// Enable transmitter
	UCSR0B = (1 << TXEN0);

	// Set frame format: 8 data bits, 1 stop bit
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
// UART transmit function

void uart_putchar(char c) {
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0)));

	// Put data into buffer, sends the data
	UDR0 = c;
}

// Redirect stdout to UART
int uart_putchar_printf(char var, FILE *stream) {
	if (var == '\n') {
		uart_putchar('\r');
	}
	uart_putchar(var);
	return 0;
}

// Setup stdout for printf
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar_printf, NULL, _FDEV_SETUP_WRITE);


//Initilaize debounce delay
void debounce_delay() {
	_delay_ms(DEBOUNCE_DELAY_MS);
}


// Initialize push buttons
void push_buttons() {
	// Set safety_btn and start_btn as input pins
	DDRB &= ~((1 << safety_btn) | (1 << start_btn));
	
	// Set GND_btn as an output pin
	DDRB |= (1 << GND_btn);
	
	// Set GND_btn to LOW (0V)
	PORTB &= ~(1 << GND_btn);

	// Enable pull-up resistors for the buttons (optional)
	PORTB |= (1 << safety_btn) | (1 << start_btn);
}




//Initialize the stepper motor
void stepper_init() {
	// Set pins as output
	DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN) | (1 << EN_PIN);

	// Enable the stepper driver
	PORTD &= ~(1 << EN_PIN); // Active low enable
}

//Set the Direction of the stepper motor
void stepper_set_direction(uint8_t direction) {
	if (direction == 0) {
		PORTD &= ~(1 << DIR_PIN); // Set direction to one way
		} else {
		PORTD |= (1 << DIR_PIN);  // Set direction to the opposite way
	}
}

//Step the motor
void stepper_step() {
	// Send a pulse to the step pin
	PORTD |= (1 << STEP_PIN);   // Set step pin high
	_delay_us(1);               // Short delay
	PORTD &= ~(1 << STEP_PIN);  // Set step pin low
	_delay_ms(2);               // Step interval delay
}

// Initialize HX711
void hx711_init() {
	// Set HX711 pins
	DDRD &= ~(1 << HX711_DOUT);  // Set DOUT as input (PD6)
	DDRD |= (1 << HX711_SCK);    // Set SCK as output (PD7)
	PORTD |= (1 << HX711_SCK);   // Set SCK high (power down)
	_delay_ms(100);              // Wait for power-up
	PORTD &= ~(1 << HX711_SCK);  // Set SCK low (ready to read)
}

// Read raw data from HX711
long hx711_read() {
	long result = 0;
	// Wait for DOUT to be low
	while (PIND & (1 << HX711_DOUT));
	
	// Read 24 bits of data
	for (int i = 0; i < 24; i++) {
		PORTD |= (1 << HX711_SCK);  // Set SCK high
		result = result << 1;
		PORTD &= ~(1 << HX711_SCK); // Set SCK low
		if (PIND & (1 << HX711_DOUT)) {
			result++;
		}
	}
	
	// Set the gain to 128 (1 additional clock pulse)
	PORTD |= (1 << HX711_SCK);  // Set SCK high
	result = result ^ 0x800000; // Convert to signed 24-bit integer
	PORTD &= ~(1 << HX711_SCK); // Set SCK low

	return result;
}

// Initialize Pin Change Interrupt for PB0
void setup_interrupt() {
	// Enable Pin Change Interrupt for PB0 (part of Port B)
	PCICR |= (1 << PCIE0);    // Enable pin change interrupt for PCINT[7:0] (Port B)
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1);  // Enable interrupt for PCINT0 (PB0) and PCINT1 (PB1)

	// Enable global interrupts
	sei();
}

// ISR for Pin Change Interrupt on Port B
ISR(PCINT0_vect) {
	debounce_delay();  // Wait for bouncing to settle
	// Check which pin caused the interrupt
	if ((PINB & (1 << PCINT0))) {
		// PB0 triggered the interrupt
		stepper_set_direction(1);
		while(!(PINB & (1 << PCINT1))){ // Rotate clockwise
			 stepper_step();
			 }//Uncomment this if you want to use this as a safety button
		}
	else  {
		// PB0 triggered the interrupt
		stepper_set_direction(0);
		while(!(PINB & (1 << PCINT1))){ // Rotate clockwise
			stepper_step();
		}//Uncomment this if you want to use this as a safety button
	}
		
		_delay_ms(50);
		
	}
	
	
void setup(){
	// Set the CPU frequency and initialize UART
	uart_init(MYUBRR);

	// Redirect stdout to UART
	stdout = &uart_output;

	// Set PB0 as output for LED
	DDRB |= (1 << safety_btn);
	
	//Initialize the the motor
	stepper_init();
	
	//Initialize Push Buttons
	push_buttons();
	
	// Initialize HX711
	hx711_init();
	
	setup_interrupt();
}




int main(void) {
	setup();
	uart_init(MYUBRR);    // Initialize UART

	uint32_t previous_weight = 0;  // Variable to store the previous weight reading

	while (1) {
		uint32_t current_weight = hx711_read();  // Read weight from HX711
		int32_t weight_difference = current_weight - previous_weight;  // Calculate the difference
		
		// Convert weight difference to a string and transmit via UART
		char buffer[30];
		// Check if the difference is positive or negative and print accordingly
		if ((previous_weight != 0) && (weight_difference > 50000)) {
			//previous_weight = current_weight;  // Update the previous weight

			snprintf(buffer, sizeof(buffer), "positive: %ld\n", weight_difference);
			uart_transmit_string(buffer);
			stepper_set_direction(0);
			for (int i = 0; i < 10; i++){ // Rotate clockwise
				stepper_step();
				
			}//Uncomment this if you want to use this as a safety button
			previous_weight = 0;  // Variable to store the previous weight reading

			} 
			else if ((previous_weight != 0) && weight_difference < -50000) {
			//previous_weight = current_weight;  // Update the previous weight

			snprintf(buffer, sizeof(buffer), "negative: %ld\n", weight_difference);
			uart_transmit_string(buffer);
			stepper_set_direction(1);
			for (int i = 0; i < 10; i++){ // Rotate clockwise
				stepper_step();
				
			}//Uncomment this if you want to use this as a safety button
			previous_weight = 0;  // Variable to store the previous weight reading

		}
		else {
			snprintf(buffer, sizeof(buffer), "neutral: %ld\n", weight_difference);
			uart_transmit_string(buffer);
			previous_weight = current_weight;  // Update the previous weight
		}

		_delay_ms(500);  // Wait for 100ms before the next reading
		
		
	}
	return 0;
}