
# Light Source and Object Proximity Detector System

## Project Overview

A radar-based MCU system designed to detect and monitor objects and light sources in a space. It uses ultrasonic distance sensors and light sensors (LDR), along with a servo motor to perform a 180 degree sweep to gather spatial data.

## Hardware Components

1. **MCU**: TI MSP430
2. **Sensors**: Ultrasonic Distance Sensor (HC-SR04), Light Dependent Resistors (LDR)
3. **Actuator**: Servo Motor
4. **Display**: LCD, PC Screen
5. **Communication**: UART for PC communication

## Software Components

The code is organized into several layers, including the API (Application Programming Interface), BSP (Board Support Package), and HAL (Hardware Abstraction Layer).

**API**:
Provides high-level functions that abstract hardware details, interface with the HAL layer to perform specific tasks.

**BSP**:
Contains definitions and initializations specific to the hardware platform, such as setting up the MCU, configuring peripherals, and hardware-specific macros.

**HAL**:
Provides low-level functions to interface directly with the hardware, includes the interrupt handlers and utility functions to manage the hardware peripherals.

**Main Application**:
Initializes the system and manages the FSM to handle different operational states.
Main Loop:
- Initializes the system state and low power mode.
- Implements a switch-case loop to manage different states (e.g., idle, monitoring objects, reading light intensity).

## Project Tasks

**Dynamic Object Monitoring System**:

- Perform dynamic monitoring of objects within a specified range and a 180 degree sweep.
- Display results on the PC screen, with a circular map presenting the objects locations.
  
**Distance Measurement Display**:

- Display the measured distance from the sensor dynamically and in real-time with a resolution of 1 cm.
- Log the measurement history on the PC screen based on user-selected servo angles.

**Light Source Detection**:

- Use LDR sensors to detect light sources during a 180 degree sweep.
- Display results on the PC screen, with a circular map presenting the light sources locations.

**Script Mode**:

- Store a text file in the MCU's flash memory.
- Execute functions on the MCU by reading from the stored script file.

## User interface
A PC-based GUI implemented using Python to interact with the MCU.
The GUI allows setting parameters, sending commands, and visualizing the radar and light detection data in real-time.
Communication with the MCU is handled via asynchronous serial communication (RS-232).
