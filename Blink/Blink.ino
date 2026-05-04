// Hello World for Arduino UNO
// Blinks the built-in LED and prints debug messages over Serial.
// No extra hardware required.

#define LED_PIN LED_BUILTIN   // Pin 13 on UNO
#define BLINK_INTERVAL_MS 1000

int blinkCount = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("=== Arduino UNO Hello World ===");
  Serial.println("Setup complete. Starting blink loop...");
}

void loop() {
  blinkCount++;

  digitalWrite(LED_PIN, HIGH);
  Serial.print("[");
  Serial.print(blinkCount);
  Serial.println("] LED ON");

  delay(BLINK_INTERVAL_MS);

  digitalWrite(LED_PIN, LOW);
  Serial.print("[");
  Serial.print(blinkCount);
  Serial.println("] LED OFF");

  delay(BLINK_INTERVAL_MS);
}
