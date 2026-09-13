# Passage System

A small embedded access-control project built around an Arduino Nano and an ESP32. It reads RFID cards and keypad input, sends the access request to a backend API, and controls door access with LEDs and a servo.

This project is intended to work with the backend at https://github.com/JesperTennevik/cpp_access_server.

## Project layout

- Arduino/: AVR firmware for the card reader, keypad, LEDs, and door logic
- Esp32/: ESP32 firmware for Wi-Fi and HTTP communication

## Hardware flow

1. User scans RFID card or triggers the access flow.
2. Arduino sends UID + door identifier to the ESP32.
3. ESP32 sends a request to the backend.
4. User enters PIN on keypad.
5. ESP32 sends PIN validation to backend.
6. Backend returns access result.
7. Arduino updates status lights and door state.

## Setup

### 1. Configure your network and backend
Edit the ESP32 config before building:

- Esp32/main/Kconfig.projbuild
- Esp32/sdkconfig

Set:

- Wi-Fi SSID
- Wi-Fi password
- backend base URL

### 2. Build the ESP32 firmware
### 3. Build and flash the Arduino firmware
### 4. Run the backend server