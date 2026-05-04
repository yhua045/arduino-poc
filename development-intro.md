# Arduino Development Intro

## What a Sketch Is
A *sketch* is the Arduino term for a program that runs on a board. In this project, the sketch is the `.ino` file under `Blink/`.

## Core Arduino Concepts

### Board
The physical device, such as an Arduino UNO. It contains a microcontroller, pins, memory, and a USB connection for power, uploading code, and serial communication.

### Sketch
Your application code for the board. Arduino sketches usually have:
- `setup()`: runs once at startup or reset
- `loop()`: runs forever after `setup()` finishes

### Core / SDK / Libraries
- The *core* is the board support package for the microcontroller family.
- It provides APIs like `Serial`, `pinMode`, `digitalWrite`, `delay`, and `LED_BUILTIN`.
- Libraries add reusable functionality for sensors, displays, communications, and more.

### Compile
Compilation turns your sketch into firmware for the target board. For the UNO, that means AVR firmware for the ATmega328P.

### Upload
Uploading sends the compiled firmware to the board over USB/serial. This replaces the program currently on the board.

### Run
After upload, the board resets and starts running the new sketch immediately.

## Local Development Workflow

### 1. Edit locally
Write the sketch in VS Code. IntelliSense helps with symbols and APIs.

### 2. Compile locally
Use:
```bash
arduino-cli compile --fqbn arduino:avr:uno Blink/Blink.ino
```
This verifies the sketch builds for the UNO.

### 3. Deploy locally
Use the detected serial port from `arduino-cli board list`:
```bash
arduino-cli upload --fqbn arduino:avr:uno --port /dev/cu.usbmodemXXXX Blink/Blink.ino
```
This uploads the firmware to the board.

### 4. Observe locally
Use the onboard LED and Serial output to confirm the sketch is working.

## Arduino Mindset
Arduino development is more like firmware work than desktop app development:
- You are building code for a tiny embedded computer.
- There is no desktop OS layer in the usual sense.
- Debugging is commonly done with LEDs, Serial output, and hardware tools like a multimeter.

## Blink Example
For the Blink sketch:
- `setup()` initializes Serial and sets the LED pin as output.
- `loop()` turns the LED on, waits, turns it off, waits, and repeats forever.

## Mental Model
- `setup()` = startup/init
- `loop()` = main runtime
- upload = deployment
- Serial Monitor = lightweight local debug console

## Quick Summary
1. Write a sketch in `.ino`
2. Compile with `arduino-cli`
3. Upload to the UNO over USB
4. Observe behavior through LEDs and Serial output
