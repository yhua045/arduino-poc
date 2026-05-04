# Diagnostic Mode — Left Ultrasonic Sensor
## Design Plan

**Document scope:** Standalone Arduino UNO sketch that isolates and stress-tests only
the Left ultrasonic sensor after module replacement. No production code is
modified. This plan is the reference for both the `developer` agent (firmware
implementation) and any future host-side tooling.

> **Mobile-UI note:** This is purely firmware; there is no UI surface. The
> mobile-ui agent consultation required by orchestrator policy is not applicable
> here. No UI abstractions are designed or deferred.

---

## 1. Problem Statement

The Left ultrasonic module was physically replaced. Before re-integrating it into
the `ObstacleAwareness` production sketch we need to verify:

- The trigger sequence reaches the sensor pin at the correct timing.
- The ECHO pulse arrives and its duration is measurable.
- The derived distance is plausible across expected ranges.
- Edge cases (no ECHO, very short pulse, very long pulse) are caught and counted.

---

## 2. Assumptions About the Replacement Module

The existing `UltrasonicSensor` class used a UART (SoftwareSerial / SR04M-2)
protocol. Given the pin names in `Config.h` (`LEFT_TRIG_PIN = 4`,
`LEFT_ECHO_PIN = 5`) match the HC-SR04 naming convention, **this diagnostic
assumes the replacement is a standard HC-SR04 (TRIG + ECHO digital interface)**.

If the replacement turns out to be UART-based again, see §7 (Optional UART Path).

---

## 3. Sketch Location and Scope

```
Arduino-poc/
└── LeftSensorDiag/
    └── LeftSensorDiag.ino    ← standalone diagnostic sketch (new)
```

- Single `.ino` file, no external libraries, no dependency on ObstacleAwareness
  classes.
- Uses the same pin constants as `Config.h` (copied locally to avoid coupling).
- Compiles and uploads independently:

```bash
arduino-cli compile --upload -p /dev/cu.usbmodem2101 \
  --fqbn arduino:avr:uno LeftSensorDiag/LeftSensorDiag.ino
arduino-cli monitor -p /dev/cu.usbmodem2101 -c baudrate=115200
```

---

## 4. Pin Definitions (Left sensor only)

| Constant          | Pin | Direction | Note                          |
|-------------------|-----|-----------|-------------------------------|
| `DIAG_TRIG_PIN`   | D4  | OUTPUT    | Same as `LEFT_TRIG_PIN`       |
| `DIAG_ECHO_PIN`   | D5  | INPUT     | Same as `LEFT_ECHO_PIN`       |

No other pins are driven. Vibrators and right-side sensors are untouched.

---

## 5. Measurement Cycle — Logic and Log Fields

Each cycle runs every **200 ms** (configurable via `DIAG_INTERVAL_MS`).

### 5.1 Step-by-step sequence

```
Step 1 — PRE-TRIGGER
  a. Assert TRIG LOW for ≥ 4 µs (clear any residual)
  b. Log: [TRIG] pin=D4 state=LOW  t_us=<micros()>

Step 2 — TRIGGER PULSE
  a. Assert TRIG HIGH
  b. Log: [TRIG] pin=D4 state=HIGH t_us=<micros()>
  c. Hold 10 µs (delayMicroseconds(10))
  d. Assert TRIG LOW
  e. Log: [TRIG] pin=D4 state=LOW  t_us=<micros()>  pulse_us=10

Step 3 — ECHO WAIT
  a. Record t_echo_start = micros()
  b. Poll ECHO pin until HIGH or timeout (ECHO_WAIT_TIMEOUT_US = 30 000 µs)
  c. Log (on HIGH): [ECHO] pin=D5 state=HIGH t_us=<micros()>
     Log (on timeout): [ECHO] WAIT_TIMEOUT after <elapsed> µs

Step 4 — ECHO MEASURE
  a. If ECHO is HIGH: record t_pulse_start = micros()
  b. Poll ECHO until LOW or timeout (ECHO_MEASURE_TIMEOUT_US = 38 000 µs)
  c. Record t_pulse_end = micros()
  d. Log: [ECHO] pin=D5 state=LOW  t_us=<t_pulse_end>
          [PULSE] duration_us=<t_pulse_end - t_pulse_start>
     or
          [ECHO] MEASURE_TIMEOUT — obstacle beyond max range or no return

Step 5 — DISTANCE
  a. If valid pulse:
       cm = duration_us / 58.0   (standard HC-SR04 constant: sound round-trip)
     Log: [DIST] formula="us/58" raw_us=<N> cm=<F.FF>
     Log: [DIST] status=<IN_RANGE|BELOW_MIN(2cm)|ABOVE_MAX(400cm)>
  b. If timeout:
     Log: [DIST] status=TIMEOUT cm=999.00

Step 6 — COUNTERS
  Log: [STAT] cycle=<N> timeouts=<T> valid=<V> last_cm=<F.FF>
```

### 5.2 Log field summary table

| Tag       | Fields                                      | When emitted              |
|-----------|---------------------------------------------|---------------------------|
| `[TRIG]`  | `pin`, `state`, `t_us`, `pulse_us`          | Every cycle (3 lines)     |
| `[ECHO]`  | `pin`, `state`, `t_us`                      | When ECHO changes         |
| `[ECHO]`  | `WAIT_TIMEOUT` / `MEASURE_TIMEOUT`, elapsed | On timeout                |
| `[PULSE]` | `duration_us`                               | Every valid reading       |
| `[DIST]`  | `formula`, `raw_us`, `cm`, `status`         | Every cycle               |
| `[STAT]`  | `cycle`, `timeouts`, `valid`, `last_cm`     | Every cycle               |

Each cycle is delimited by a blank line for easy parsing.

---

## 6. Timeout and Counter Design

```
DIAG_ECHO_WAIT_TIMEOUT_US    = 30 000   // 30 ms — sensor should respond in < 1 ms
DIAG_ECHO_MEASURE_TIMEOUT_US = 38 000   // 38 ms — HC-SR04 max pulse for ~650 cm
```

Global counters (never reset while powered):

- `uint32_t g_cycleCount` — total measurement attempts
- `uint32_t g_timeoutCount` — ECHO_WAIT or ECHO_MEASURE timeouts
- `uint32_t g_validCount` — cycles producing a usable distance

---

## 7. Optional UART Path (if replacement is UART-based)

If the new module does NOT respond to the TRIG/ECHO sequence (all
`[ECHO] WAIT_TIMEOUT` in the log), add a second compile-time flag:

```cpp
#define DIAG_UART_SNIFF   // uncomment to enable SoftwareSerial capture
```

When enabled:
- Open SoftwareSerial on `DIAG_TRIG_PIN` (TX) / `DIAG_ECHO_PIN` (RX) at 9600 baud.
- Send the `0x55` probe byte instead of a TRIG pulse.
- Log all received bytes verbatim: `[UART] rx_bytes=FF 01 A0 ... count=<N>`.
- This path is disabled by default to avoid pin-mode conflicts with the HC-SR04
  path.

---

## 8. Invariants and Constraints

| Constraint                         | Rationale                                      |
|------------------------------------|------------------------------------------------|
| No `delay()` inside measurement    | `delay()` would inflate pulse timing readings  |
| `micros()` used for all timing     | `millis()` resolution (1 ms) too coarse        |
| TRIG held LOW before every trigger | Prevents phantom double-triggers               |
| `Serial.begin(115200)`             | Matches the existing monitor baud rate         |
| No `pulseIn()` in production path  | `pulseIn()` blocks IRQs; manual polling is used so the log can capture intermediate states |

---

## 9. Acceptance Criteria for "Sensor OK"

The sensor passes diagnostic if across 30 consecutive cycles:

1. `timeouts == 0` — every trigger gets an ECHO response.
2. All `cm` values fall within the physically expected range for the test
   distance (e.g., obstacle placed at ~20 cm → readings 17–23 cm, ±15%).
3. No `BELOW_MIN` or `ABOVE_MAX` status lines.

---

## 10. Next Steps for Developer Agent

1. Implement `LeftSensorDiag/LeftSensorDiag.ino` per §4–§6.
2. Compile and upload using the command in §3.
3. Capture a full 30-cycle log at a known distance (e.g., wall at 20 cm).
4. If all acceptance criteria (§9) pass, update `UltrasonicSensor.cpp` to use the
   HC-SR04 TRIG/ECHO protocol instead of the SoftwareSerial/0x55 protocol.
5. Re-run the `ObstacleAwareness` production sketch and confirm left-side detection
   works end-to-end.

---

*Design owner: architect agent | Revision: 2026-05-05*
