# Zero-G-Dynamic-Body-Weight-Support-System-Using-a-Motorized-Unloader
Zero G Weight Support Device assists patients struggling with walking by providing weight support to practice and improve mobility. It combines advanced mechanical engineering and electronic control systems, offering a safe and effective rehabilitation tool. This device helps patients build strength and confidence in their walking abilities.

## Project Overview

The zero gravity lifting device is designed to counteract gravity, allowing objects to be lifted and manipulated with minimal effort. It uses a load cell to measure tension, processes this information through a PID controller, and adjusts a stepper motor's velocity to achieve the desired effect.

## Components

1. ATmega328P microcontroller
2. HX711 load cell amplifier
3. Stepper motor
4. Custom PCB

## Key Features

- Real-time weight measurement using HX711 load cell amplifier
- PID control algorithm for smooth and accurate motor control
- Stepper motor control for precise movements
- Millisecond time tracking for accurate derivative calculations

## Setup and Compilation

The project is developed using WinAVR for the ATmega328P microcontroller.

## Main Program Flow

1. Initialize USART, millisecond timer, and interrupts
2. Initialize and calibrate the HX711 load cell amplifier
3. Set up the PID controller
4. Enter the main loop:
   - Read weight from the load cell
   - Calculate weight change rate (derivative)
   - Process through PID controller
   - Control stepper motor based on PID output

## Notes for Developers

- The PID constants (P_FACTOR, I_FACTOR, D_FACTOR) may need tuning for optimal performance
- The DELAY_MICROSECONDS constant in the stepper motor control can be adjusted to change motor speed
- The HX711 calibration process is crucial for accurate weight measurements

## Code Details

### HX711 Library (`HX711.h`)

This custom library provides functions for interfacing with the HX711 load cell amplifier. Key functions include:

- `HX711_init()`: Initialize the HX711 module
- `HX711_read()`: Read raw data from the HX711
- `HX711_set_scale()`: Set the scale factor for weight conversion
- `HX711_tare()`: Set the tare weight
- `HX711_get_units()`: Get weight readings in desired units

### Millisecond Tracking Library (`millis.h`)

This lightweight library provides millisecond-level timing capabilities, which are crucial for calculating the rate of change in weight. Key functions include:

- `millis_init()`: Initialize the millisecond timer
- `millis_get()`: Get the current millisecond count
- `millis_reset()`: Reset the millisecond count to 0

### Main Program (`main.c`)

The main program integrates all components and implements the control logic. Key features include:

- PID controller implementation
- Stepper motor control
- Weight derivative calculation
- Debug output via USART

## Hardware Connections

- HX711 Data (DT) pin connected to PC4
- HX711 Clock (SCK) pin connected to PC3
- Stepper motor STEP pin connected to PD6
- Stepper motor DIR pin connected to PD7

## Calibration

The load cell requires calibration for accurate weight measurements. The current code uses a calibration factor of 222138.f, but this may need adjustment based on your specific load cell and setup.

## Tuning

For optimal performance, you may need to tune the following parameters:

1. PID constants (P_FACTOR, I_FACTOR, D_FACTOR)
2. DELAY_MICROSECONDS for stepper motor speed
3. SCALING_FACTOR for PID calculations

## Debugging

The code includes debug output via USART. You can monitor this output to see real-time information about:

- Weight derivative
- PID controller output

Use this information to help with tuning and troubleshooting.

## Future Improvements

Potential areas for future development include:

1. Implementing a user interface for easy control and monitoring
2. Adding limit switches or other safety features
3. Incorporating different lifting modes or profiles
4. Implementing wireless control or monitoring capabilities

## Contributing

Contributions to improve the code or extend the functionality are welcome. Please ensure you follow good coding practices and thoroughly test any changes.

## License

This project uses libraries with specific licenses:

- The millisecond tracking library is licensed under GNU GPL v3.
- Check the licensing terms for other components before use or distribution.

## Support

For questions or issues related to this project, please open an issue in the project repository or contact the project maintainers.

Remember to always prioritize safety when working with lifting devices and follow all relevant safety guidelines and regulations.
