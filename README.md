# Smart Home & Emergency Response System (IoT)

**Authors:** Asmaeil Mohammed Jamil Elgadi(24009871)/Walid Salem balied elbagi (24008232)
**Target Device:** ESP32-C3  
**Framework:** ESP-IDF (v5.5.1) | FreeRTOS | ESP RainMaker

## 📖 Project Overview
This project is an advanced IoT-based Smart Home and Safety System designed for the **ESP32-C3** microcontroller. It utilizes **FreeRTOS** for efficient multitasking and **ESP RainMaker** for cloud integration.

The system is designed to monitor environmental conditions (Temperature & Humidity) and automatically trigger safety protocols in case of emergencies (e.g., high temperature or manual emergency activation), activating alarms, sprinklers, and sending real-time push notifications to the user's mobile phone.

## ✨ Key Features
* **Real-Time Monitoring:** Continuous reading of Temperature and Humidity using DHT11 sensor.
* **Cloud Integration:** Full remote control and monitoring via **ESP RainMaker App**.
* **Emergency Automation:** Automatic activation of "Fire Mode" (Water Sprinkler, Alarm, Extractor Fan) when thresholds are breached.
* **Voice Control:** Compatible with **Alexa** and **Google Assistant** via RainMaker standard device types.
* **Push Notifications:** Instant alerts sent to the mobile app during emergency events.
* **OLED Display:** Local status visualization (Sensor data, System State, Wi-Fi Status).
* **OTA Updates:** Over-the-Air firmware update capability.

## 🛠️ Hardware Requirements
* **Microcontroller:** ESP32-C3 DevKitM-1
* **Sensors:** DHT11 (Temperature & Humidity)
* **Display:** 0.96" OLED Display (SSD1306 I2C)
* **Actuators (Simulated via LEDs/Relays):**
    * Air Conditioner (Fan)
    * Fire Water Sprinkler
    * Sound Alarm (Buzzer)
    * Fire Indicator LED
    * Extractor Fan
* **Input:** Emergency Push Button

## 🏗️ Software Architecture (RTOS Implementation)
The firmware is structured using **FreeRTOS** to ensure non-blocking operation and efficient resource management.

| Task / Mechanism | Functionality |
| :--- | :--- |
| **Sensor Task** | Periodically reads DHT11 data and sends it to the Controller via **Queue**. |
| **Controller Task** | Processes sensor data, handles logic (Auto-Cooling / Emergency), and updates the OLED Display protected by **Mutex**. |
| **Emergency Monitor** | Interrupt-driven or polled task to detect physical emergency button presses instantly. |
| **RainMaker Agent** | Background task handling MQTT communication, Cloud updates, and OTA. |
| **Inter-Task Comm.** | Uses `xQueue` for sensor data and `xSemaphore` (Mutex) for shared I2C display access. |

## ⚙️ Configuration & Optimization
To run this project on the single-core **ESP32-C3** with RainMaker and Bluetooth provisioning, specific optimizations were applied in `sdkconfig.defaults`:
* **Stack Size Optimization:** Increased `CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH` to 4096 to prevent stack overflow during heavy logging.
* **Bluetooth:** Explicitly enabled `Bluedroid` (Dual-mode) to ensure successful provisioning.
* **Memory:** Partition table set to `custom` to accommodate the application and OTA partitions.

## 🚀 Getting Started

### 1. Prerequisites
* VS Code with **ESP-IDF Extension**.
* ESP-IDF v5.x installed.

### 2. Clone the Repository
```bash
git clone [https://github.com/YOUR_USERNAME/EEM5043-Smart-Home-System.git](https://github.com/YOUR_USERNAME/EEM5043-Smart-Home-System.git)
cd EEM5043-Smart-Home-System
