#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RedMP3.h>
#include <ChainableLED.h>

// D2 = on/off
// D3 = pressure
// D8 = music player
// A1 = knob

#define ON_OFF 2
#define PRESSURE 3
#define LIGHT 4
#define KNOB 1
#define MP3_RX 8 
#define MP3_TX 9 

// MP3 player declarations
MP3 mp3(MP3_RX, MP3_TX);        // MP3 Player
int8_t index  = 3; // 1 - 10
int8_t volume = 15; // 0 - 30

// ChainableLED declarations
ChainableLED leds(5, 6, 1);     // 1 LED on D5, D6

// Variables to keep track of the button state and LED state
bool previousButtonState = LOW;
bool ledState = LOW;
bool dimming = false;
int brightness = 255; // Initial brightness

// put (function) declarations here:
// int myFunction(int, int);

void setup() {
  // pin modes
  pinMode(ON_OFF, INPUT);         // Button as ON/OFF input
  pinMode(LED_BUILTIN, OUTPUT);   // Built-in LED as output
  pinMode(KNOB, INPUT);           // Knob 

  // start serial monitor
  Serial.begin(9600);             // Write to Serial Monitor

  // music starts playing 
  delay(500);
  mp3.playWithVolume(index, volume);
  delay(50);  
}

void loop() {
  // ON/OFF BUTTON STUFF
  bool buttonState = digitalRead(ON_OFF);
  bool pressed = true;  

  // Check if the button is pressed
  if (buttonState == pressed) {
    // Start the dimming process
    dimming = true;
  } else {
    // Stop the dimming process
    dimming = false;
    brightness = 255; // Reset brightness
    digitalWrite(LED_BUILTIN, LOW); // Ensure the built-in LED is off
    leds.setColorRGB(0, 0, 0, 0); // Ensure the Chainable LED is off
  }

  // Dimming process
  if (dimming) {
    if (brightness > 0) {
      brightness -= 5; // Decrease brightness
      analogWrite(LED_BUILTIN, brightness); // Adjust the built-in LED brightness
      leds.setColorRGB(0, brightness, 0, 0); // Adjust the Chainable LED brightness
      delay(50); // Delay to make the dimming visible
    } else {
      dimming = false; // Stop dimming when brightness reaches 0
      digitalWrite(LED_BUILTIN, LOW); // Ensure the built-in LED is off
      leds.setColorRGB(0, 0, 0, 0); // Ensure the Chainable LED is off
    }
  }

  // ----------------- KNOB STUFF -----------------

  // int state = analogRead(KNOB);
  
  // if (state > 511)
  // {
  //   Serial.print("Past the threshold: ");
  //   Serial.println(state);
  // } else {
  //   Serial.print("Below the threshold: ");
  //   Serial.println(state);
  // }
  // delay(500);
}
