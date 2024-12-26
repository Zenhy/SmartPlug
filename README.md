# Smart Plug IoT Project

This project is a prototype for a **smart plug** that allows users to monitor and control electrical usage remotely. The smart plug is designed using the **ESP32 microcontroller** for connectivity, with support for **Node-RED** for workflow automation and **MQTT** for real-time communication. The aim is to offer a flexible and cost-effective solution for energy management, ideal for smart homes or energy-conscious environments.

## Features
- **Energy Monitoring**: Track energy consumption in real-time.
- **Remote Control**: Turn the plug on/off remotely via a web-based dashboard.
- **Scheduling**: Set time-based schedules for turning the plug on/off.
- **Open-Source**: Built with open-source software (Node-RED, MQTT) and firmware (Tasmota, MicroPython).

## Components Used
- **ESP32**: Main microcontroller for Wi-Fi connectivity.
- **Relay Module**: Used to control the power connection to the device.
- **ACS712**: Current sensor for measuring power consumption.
- **Node-RED**: Flow-based development tool for creating custom automation workflows.
- **MQTT (Eclipse Mosquitto)**: Protocol for communication between the device and the monitoring platform.

## Setup & Installation
1. Clone the repository.
2. Upload the firmware to the **ESP32** using **MicroPython** or **Tasmota**.
3. Set up the **MQTT broker** (e.g., Eclipse Mosquitto) and configure Node-RED to receive the data.
4. Use the web dashboard to control the smart plug and view energy usage.

## Contributions
Feel free to fork this project, contribute, and open issues if you find any bugs or have suggestions for improvement.
