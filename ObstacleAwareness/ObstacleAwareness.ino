// Diagnostic mode: LEFT ultrasonic sensor only.
// All other subsystems (PIR, vibrators, right sensor) are excluded.
#include "Config.h"

// ── Timing constants ──────────────────────────────────────────────────────────
static const unsigned long ECHO_TIMEOUT_US = 30000UL; // ~5 m round-trip ceiling
static const unsigned long CYCLE_DELAY_MS  =    60UL; // ≥60 ms between cycles

// ── Counters ──────────────────────────────────────────────────────────────────
static unsigned long cycleCount           = 0;
static unsigned long validCount           = 0;
static unsigned long timeoutWaitHighCount = 0;
static unsigned long timeoutWaitLowCount  = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LEFT_TRIG_PIN, OUTPUT);
    pinMode(LEFT_ECHO_PIN, INPUT);
    digitalWrite(LEFT_TRIG_PIN, LOW);

    Serial.println(F("=== LEFT Ultrasonic Sensor Diagnostic ==="));
    Serial.println(F("Pins : TRIG=D4  ECHO=D5"));
    Serial.println(F("Formula: distance_cm = pulse_us / 58"));
    Serial.println(F("  (sound ~0.034 cm/us; round-trip halved => /58)"));
    Serial.print(F("Timeout ceiling: "));
    Serial.print(ECHO_TIMEOUT_US);
    Serial.println(F(" us (~5 m)"));
    Serial.println(F("Tags: [TRIG] [ECHO] [PULSE] [DIST] [STAT]"));
    Serial.println(F("-----------------------------------------"));
}

void loop() {
    cycleCount++;

    // ── [TRIG] Generate trigger pulse ─────────────────────────────────────────
    digitalWrite(LEFT_TRIG_PIN, LOW);
    delayMicroseconds(4);                    // settle LOW before rising edge
    unsigned long trigStart = micros();
    digitalWrite(LEFT_TRIG_PIN, HIGH);
    delayMicroseconds(10);                   // ≥10 µs HIGH per HC-SR04 datasheet
    unsigned long trigEnd = micros();
    digitalWrite(LEFT_TRIG_PIN, LOW);

    Serial.print(F("[TRIG] cycle="));   Serial.print(cycleCount);
    Serial.print(F("  LOW->HIGH@"));    Serial.print(trigStart);
    Serial.print(F("us  HIGH->LOW@"));  Serial.print(trigEnd);
    Serial.print(F("us  pulse="));      Serial.print(trigEnd - trigStart);
    Serial.println(F("us"));

    // ── [ECHO] Wait for echo to go HIGH ───────────────────────────────────────
    unsigned long waitHighStart = micros();
    bool gotHigh = false;
    while (micros() - waitHighStart < ECHO_TIMEOUT_US) {
        if (digitalRead(LEFT_ECHO_PIN) == HIGH) { gotHigh = true; break; }
    }
    unsigned long echoRise = micros();

    if (!gotHigh) {
        timeoutWaitHighCount++;
        Serial.print(F("[ECHO] cycle="));  Serial.print(cycleCount);
        Serial.println(F("  TIMEOUT waiting for echo HIGH"));
        printStat();
        delay(CYCLE_DELAY_MS);
        return;
    }

    Serial.print(F("[ECHO] cycle="));       Serial.print(cycleCount);
    Serial.print(F("  rise@"));             Serial.print(echoRise);
    Serial.print(F("us  waitedForHigh="));  Serial.print(echoRise - waitHighStart);
    Serial.println(F("us"));

    // ── [ECHO] Wait for echo to go LOW ────────────────────────────────────────
    unsigned long waitLowStart = micros();
    bool gotLow = false;
    while (micros() - waitLowStart < ECHO_TIMEOUT_US) {
        if (digitalRead(LEFT_ECHO_PIN) == LOW) { gotLow = true; break; }
    }
    unsigned long echoFall = micros();

    if (!gotLow) {
        timeoutWaitLowCount++;
        Serial.print(F("[ECHO] cycle="));  Serial.print(cycleCount);
        Serial.println(F("  TIMEOUT waiting for echo LOW"));
        printStat();
        delay(CYCLE_DELAY_MS);
        return;
    }

    // ── [PULSE] Echo pulse width ───────────────────────────────────────────────
    unsigned long pulseUs = echoFall - echoRise;
    Serial.print(F("[PULSE] cycle="));  Serial.print(cycleCount);
    Serial.print(F("  rise@"));         Serial.print(echoRise);
    Serial.print(F("us  fall@"));       Serial.print(echoFall);
    Serial.print(F("us  width="));      Serial.print(pulseUs);
    Serial.println(F("us"));

    // ── [DIST] Distance conversion ────────────────────────────────────────────
    float distCm = (float)pulseUs / 58.0f;
    validCount++;
    Serial.print(F("[DIST] cycle="));  Serial.print(cycleCount);
    Serial.print(F("  pulse="));       Serial.print(pulseUs);
    Serial.print(F("us  dist="));      Serial.print(distCm, 1);
    Serial.println(F("cm"));

    printStat();
    delay(CYCLE_DELAY_MS);
}

// ── [STAT] Running counters ────────────────────────────────────────────────────
void printStat() {
    Serial.print(F("[STAT] cycles="));  Serial.print(cycleCount);
    Serial.print(F("  valid="));        Serial.print(validCount);
    Serial.print(F("  toHigh="));       Serial.print(timeoutWaitHighCount);
    Serial.print(F("  toLow="));        Serial.println(timeoutWaitLowCount);
}
