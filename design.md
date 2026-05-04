# Obstacle Awareness System — Design Document

## 1. Overview
A wearable or device-mounted Arduino UNO application that detects obstacles on the **left** and **right** sides using paired sensors and alerts the user through vibration motors on the corresponding side.

---

## 2. Hardware

### 2.1 Components
| Component           | Qty | Notes                                      |
|---------------------|-----|--------------------------------------------|
| Arduino UNO         | 1   | ATmega328P, 5V logic                       |
| PIR Sensor          | 2   | Digital output, detects motion/presence    |
| Ultrasonic Sensor   | 2   | HC-SR04, measures distance via echo pulse  |
| Vibration Motor     | 2   | Coin/ERM motor, driven via transistor/PWM  |

### 2.2 Pin Assignment
| Signal                   | Arduino Pin | Mode   |
|--------------------------|-------------|--------|
| Left PIR signal          | D2          | INPUT  |
| Right PIR signal         | D3          | INPUT  |
| Left Ultrasonic TRIG     | D4          | OUTPUT |
| Left Ultrasonic ECHO     | D5          | INPUT  |
| Right Ultrasonic TRIG    | D6          | OUTPUT |
| Right Ultrasonic ECHO    | D7          | INPUT  |
| Left Vibrator            | D9          | OUTPUT (PWM) |
| Right Vibrator           | D10         | OUTPUT (PWM) |

> **Note:** Use a NPN transistor (e.g. 2N2222) or MOSFET between each vibrator and the Arduino pin to avoid exceeding the 40mA pin current limit.

---

## 3. Architecture

The application is split into four layers to keep concerns separate and make unit testing straightforward.

```
┌────────────────────────────────────────────────────────┐
│                    Main Sketch (.ino)                  │
│            setup() → loop() → controller.update()     │
└────────────────────────┬───────────────────────────────┘
                         │
          ┌──────────────▼──────────────────────────────────┐
          │              ObstacleController                  │
          │                                                  │
          │  left.detect()  ──►  decision logic  ◄──  right.detect()
          │                           │                      │
          │              ┌────────────┴───────────┐          │
          │              ▼                        ▼          │
          │      ┌──────────────┐        ┌──────────────┐   │
          │      │  Vibrator    │        │  Vibrator    │   │
          │      │  (Left)      │        │  (Right)     │   │
          │      └──────────────┘        └──────────────┘   │
          └──────┬──────────────────────────────┬───────────┘
                 │                              │
    ┌────────────▼───────────┐  ┌──────────────▼─────────┐
    │     SideDetector       │  │     SideDetector       │
    │       (Left)           │  │       (Right)          │
    │                        │  │                        │
    │ ┌────────┐ ┌─────────┐ │  │ ┌────────┐ ┌─────────┐ │
    │ │  PIR   │ │Ultrasonic│ │  │ │  PIR   │ │Ultrasonic│ │
    │ │Sensor  │ │ Sensor  │ │  │ │Sensor  │ │ Sensor  │ │
    │ └────────┘ └─────────┘ │  │ └────────┘ └─────────┘ │
    │         ↓ fuse         │  │         ↓ fuse          │
    │   returns bool         │  │   returns bool          │
    └────────────────────────┘  └────────────────────────┘
```

### Layer Responsibilities

| Layer                | Responsibility                                                                 |
|----------------------|--------------------------------------------------------------------------------|
| `PirSensor`          | Read digital signal from PIR, return true/false                                |
| `UltrasonicSensor`   | Trigger pulse, measure echo time, compute distance (cm)                        |
| `Vibrator`           | Turn motor on/off or set intensity via PWM                                     |
| `SideDetector`       | Owns one PIR and one ultrasonic. Fuses their readings into a single `bool isDetected()`. Has no knowledge of the other side and no access to any vibrator. |
| `ObstacleController` | Owns both `SideDetector` instances and both `Vibrator` instances. Queries both detectors each tick, then applies decision logic to drive the vibrators. The only component with the full picture. |

---

## 4. Detection Logic

Each side independently decides whether an obstacle is present using **sensor fusion**:

```
obstacle_detected = pir_triggered OR (ultrasonic_distance < DISTANCE_THRESHOLD_CM)
```

- **PIR** catches moving objects (people, animals).
- **Ultrasonic** catches static objects within range.
- Either sensor firing on a side is sufficient to report detection for that side.
- The threshold can be tuned (default: **50 cm**).

### ObstacleController Decision Table
`ObstacleController` receives the detection state from both sides each tick and applies the following logic:

| Left Detected | Right Detected | Action                              |
|---------------|----------------|-------------------------------------|
| false         | false          | Both vibrators off                  |
| true          | false          | Left vibrator on, right off         |
| false         | true           | Right vibrator on, left off         |
| true          | true           | Both vibrators on (user is warned on both sides) |

> **Design rationale:** `SideDetector` only sees its own side and cannot reason about both simultaneously. `ObstacleController` holds the full picture and is the correct place to make actuation decisions — including handling simultaneous alerts and any future prioritisation logic.

### Vibrator Intensity
- Simple on/off for basic usage.
- PWM intensity can be used to indicate urgency based on distance: closer → stronger vibration.

### State Machine (per side)
```
IDLE ──[obstacle detected]──► ALERT
ALERT ──[obstacle cleared]──► COOLDOWN (500 ms)
COOLDOWN ──[timeout]────────► IDLE
```
The cooldown avoids rapid on/off chatter when a sensor signal fluctuates at the threshold boundary.

---

## 5. File / Folder Structure

```
Arduino-poc/
├── ObstacleAwareness/
│   ├── ObstacleAwareness.ino   ← main sketch
│   ├── Config.h                ← pin numbers, thresholds, timing constants
│   ├── PirSensor.h / .cpp
│   ├── UltrasonicSensor.h / .cpp
│   ├── Vibrator.h / .cpp
│   ├── SideDetector.h / .cpp
│   └── ObstacleController.h / .cpp
└── tests/
    ├── TestSideDetector/
    │   └── TestSideDetector.ino
    ├── TestUltrasonicSensor/
    │   └── TestUltrasonicSensor.ino
    └── TestVibrator/
        └── TestVibrator.ino
```

---

## 6. Unit Testing Strategy

### Framework: AUnit
[AUnit](https://github.com/bxparks/AUnit) is an Arduino-native unit test framework that runs on the board and reports results over Serial. It mirrors the API of common C++ test frameworks (assertTrue, assertEqual, etc.).

Install with:
```bash
arduino-cli lib install AUnit
```

### What to Test

| Test Target          | Test Cases                                                            |
|----------------------|-----------------------------------------------------------------------|
| `UltrasonicSensor`   | Distance calculation from a known echo time                          |
| `SideDetector`       | PIR true + ultrasonic above threshold → detected                      |
|                      | PIR false + ultrasonic below threshold → detected                     |
|                      | PIR false + ultrasonic above threshold → not detected                 |
|                      | PIR true + ultrasonic above threshold → detected                      |
| `Vibrator`           | State matches expected pin output after activate/deactivate           |
| `ObstacleController` | Left only → left vibrator on, right off                               |
|                      | Right only → right vibrator on, left off                              |
|                      | Both detected → both vibrators on                                     |
|                      | Neither detected → both vibrators off                                 |

### Testing Approach
Because hardware (real pins, real sensors) makes deterministic testing hard, the test sketches will use **dependency injection** — the classes accept a mock/fake object that simulates sensor readings without requiring physical hardware connected.

### Test Execution
1. Flash a test sketch to the UNO.
2. Open the Serial Monitor at 115200 baud.
3. AUnit prints `PASSED` / `FAILED` for each test case.
4. All tests pass → flash the production sketch.

---

## 7. Implementation Plan

| Phase | Task                                            | Output                                |
|-------|-------------------------------------------------|---------------------------------------|
| 1     | Scaffold folder structure and `Config.h`        | Empty but organized project           |
| 2     | Implement `PirSensor`                           | Reads PIR digital pin                 |
| 3     | Implement `UltrasonicSensor`                    | Measures distance via HC-SR04         |
| 4     | Implement `Vibrator`                            | Controls motor pin                    |
| 5     | Implement `SideDetector` with sensor fusion     | Per-side obstacle detection           |
| 6     | Implement `ObstacleController`                  | Full system coordination              |
| 7     | Write and run unit tests                        | All tests pass on Serial Monitor      |
| 8     | Integrate into main sketch                      | Working `setup()` / `loop()`          |
| 9     | Hardware test with real sensors                 | Field-verified detection and vibration|

---

## 8. Key Configuration Constants (`Config.h`)

| Constant                   | Default  | Description                            |
|----------------------------|----------|----------------------------------------|
| `LEFT_PIR_PIN`             | 2        | Digital pin for left PIR               |
| `RIGHT_PIR_PIN`            | 3        | Digital pin for right PIR              |
| `LEFT_TRIG_PIN`            | 4        | Ultrasonic trigger pin (left)          |
| `LEFT_ECHO_PIN`            | 5        | Ultrasonic echo pin (left)             |
| `RIGHT_TRIG_PIN`           | 6        | Ultrasonic trigger pin (right)         |
| `RIGHT_ECHO_PIN`           | 7        | Ultrasonic echo pin (right)            |
| `LEFT_VIBRATOR_PIN`        | 9        | PWM pin for left vibrator              |
| `RIGHT_VIBRATOR_PIN`       | 10       | PWM pin for right vibrator             |
| `DISTANCE_THRESHOLD_CM`    | 50       | Obstacle detection range in cm         |
| `COOLDOWN_MS`              | 500      | Minimum time between alert transitions |
| `LOOP_INTERVAL_MS`         | 100      | Main loop update interval              |
