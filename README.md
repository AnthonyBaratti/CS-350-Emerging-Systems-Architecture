# CS-350-Emerging-Systems-Architecture

A project developed in C programming language, this project (CC3220S_Thermostat_nortos_css) uses a microcontroller to mock an in home thermostat which would be connected to a smart system and programmable database. <br><br>
[Return to eportfolio](https://github.com/AnthonyBaratti/AnthonyBaratti.github.io)

## Features
- Read current temperature of the room
- Adjust temperature control up and down
- Turn on heat/turn off heat according to set temperature

## Tech Stack
- C programming language
- State Machine architecture
- Texas Instrument CC3220S MicroController

## Purpose
This microcontroller is designed to mock an in home thermostat system. Is uses a state machine and "ticks" (millisecond checks of the state). The GCD is 100, the button timer is 200, the temperature check is 500 (sensor that checks current room temperature), and the display update is 1000 ticks. The system checks for a state changes at the appropriate ticks (200, 500, and 1000). If the set temperature up and set temperature down buttons are hit, they will run their adjustment to the stored temperature value (+/-). Then the sensor will run every 500 ticks. If the set temp is lower than the current temp, the heater will shut off. If the set temp is higher than the current temp, then the heater turns on. When the heat in the room rises, the sensor continues to check it, and when the heat surpasses the set temp, the heater will turn off. The display (set temp and current temp) are adjusted every 1000 ticks. This mimics a digital home thermostat. See [gpioinerrupt.c](https://github.com/AnthonyBaratti/CS-350-Emerging-Systems-Architecture/blob/main/CC3220S_Thermostat_nortos_ccs/gpiointerrupt.c) for the code produced for the project.

## State Machine Diagram
[Thermostat State Machine](https://github.com/AnthonyBaratti/CS-350-Emerging-Systems-Architecture/blob/main/Final%20Project.drawio.pdf)
