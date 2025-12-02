Indoor Fire Suppression System (IoT-Based)
Asmaeil Elgadi 24009871 /Walid  Elbagi 24008232

Project Overview

This project implements a smart indoor fire detection and suppression system using the ESP32-C3 microcontroller. It integrates IoT technologies to provide real-time environmental monitoring, automated emergency response, and remote control via cloud platforms. The system includes audible and visual alarms, water sprinklers, air extractors, and manual emergency overrides to ensure occupant safety.

Features

Real-Time Fire Detection: Activates when temperature reaches 30 °C.

Automated Fire Response: Operates water sprinklers and air extractors automatically.

Audible & Visual Alerts: Sound alarms and LED indicators to warn occupants.

Manual Override: Emergency push button allows activation if automatic controls fail.

Cloud Integration: Remote monitoring and control via ESP RainMaker.

Voice Control: Compatible with Google Assistant and Amazon Alexa.

OTA Firmware Updates: Supports Over-the-Air updates for maintainability.

Event Notifications: Push notifications for critical alerts on mobile devices.

Hardware
Component	Model / Type	Interface	GPIO Assignment
Microcontroller	ESP32-C3 DevKitM-1	—	RISC-V 32-bit
Temperature Sensor	DHT11	1-Wire	GPIO 2
Display	0.96" OLED (SSD1306)	I²C (0x3C)	SDA: GPIO 20, SCL: GPIO 8
Emergency Button	Push Button	Digital Input	GPIO 10 (Pull-up)
Extractor Fan	LED Simulation	Digital Output	GPIO 3
Air Conditioner	LED Simulation	Digital Output	GPIO 4
Water Sprinkler	LED Simulation	Digital Output	GPIO 5
Sound Alarm	LED Simulation	Digital Output	GPIO 6
Fire Indicator	LED Simulation	Digital Output	GPIO 7
Software & Firmware

Framework: ESP-IDF v5.5.1

Programming Language: C 

IDE: Visual Studio Code with ESP-IDF extension

RTOS: FreeRTOS v10.5.1

Key Features:

Task-based concurrent architecture

Inter-task communication with Queues, Semaphores, and Event Groups

OTA updates via RainMaker

Push notifications for critical events

Voice assistant integration

System Architecture

Hardware Layer: Sensors, actuators, and user interface connected to ESP32-C3

Firmware Layer: FreeRTOS tasks handling sensing, control, notifications, and emergency monitoring

Cloud Layer: ESP RainMaker dashboard for remote monitoring, OTA updates, and voice control integration

Installation & Usage

Flash the firmware to ESP32-C3 using ESP-IDF.

Connect sensors and actuators to the assigned GPIO pins.

Configure Wi-Fi and link the device to your RainMaker account.

Test temperature sensing and emergency activation.

Use mobile app or voice assistants for remote monitoring and control.

Safety Notes

Ensure proper electrical connections to prevent damage to the microcontroller.

Always test the emergency manual override before relying on automated controls.

Only use the system in controlled environments during testing.