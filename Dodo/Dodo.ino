const int TRIG_PIN = 9;
const int ECHO_PIN = 11;


void setup() {
 Serial.begin(9600);
 pinMode(TRIG_PIN, OUTPUT);
 pinMode(ECHO_PIN, INPUT);


 Serial.println("Ultrasonic Test (Pins 9 & 11)");
}


void loop() {


 // clean trigger
 digitalWrite(TRIG_PIN, LOW);
 delayMicroseconds(10);


 digitalWrite(TRIG_PIN, HIGH);
 delayMicroseconds(25);   // longer pulse for SR04M-2
 digitalWrite(TRIG_PIN, LOW);


 // read echo
 long duration = pulseIn(ECHO_PIN, HIGH, 80000);


 if (duration == 0) {
   Serial.println("No echo");
 } else {
   long distance = duration * 0.034 / 2;


   Serial.print("Distance: ");
   Serial.print(distance);
   Serial.println(" cm");
 }


 delay(300); // important for stability
}



