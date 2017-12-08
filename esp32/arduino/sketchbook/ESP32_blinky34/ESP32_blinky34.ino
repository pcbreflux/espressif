/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */
static int ledpos;
static int ledmode;
static uint8_t ledpins[] = { 33, 34, 35, 36, 37, 38, 39 }; // GPIO pins above 33 can be only inputs, so call them GPI Pins

// the setup function runs once when you press reset or power the board
void setup() {
  ledmode=0;
  ledpos=0;
  Serial.begin(115200);
  // initialize digital pin 13 as an output.
  for (int i=0;i<sizeof(ledpins);i++) {
    pinMode(ledpins[i], OUTPUT);
  }
}

// the loop function runs over and over again forever
void loop() {
  for (int i=0;i<sizeof(ledpins);i++) {
   digitalWrite(ledpins[i], ledmode);   // turn the LED on (HIGH is the voltage level)
  }
  ledmode=1-ledmode;
  delay(500);              // wait for a second
}
