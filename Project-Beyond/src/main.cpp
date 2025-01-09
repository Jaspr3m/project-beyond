#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RedMP3.h>
#include <ChainableLED.h>

// aan en uit knop om alles te laten werken (normale lamp)
// leg je telefoon op de pressure plate en dan rood licht op de (ledstrip)
// in de ochtend krijg je weer oranje licht
// straf (telli eraf) = wit licht & muziek uit

// remove the damn led

// OPTIONAL: ledstrip voor hoe lang geluid afspeelt
// OPTIONAL: ledstrip voor hoe laat je wakker wil worden
// OPTIONAL: ledstrip voor hoe lang het rood licht brandt na telefoon op pressure sensor
// ledstrip voor rood licht

// knob voor wanneer je wakker wil worden
// knob voor hoe lang licht nadat je telefoon erop ligt
// knob voor hoe lang geluid

// D2 = on/off
// D5 & D6 = LED
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
float volume = 0.0; // 0 - 30

// ChainableLED declarations
ChainableLED leds(5, 6, 1);     // 1 LED on D5, D6

// Variables to keep track of the button state and LED state
bool previousButtonState = LOW;
bool ledState = LOW;
bool dimming = false;
bool lighting = false;
bool volumeDown = false;
float volumeOff = 0.0;
int redBrightness = 255; // Initial brightness
int orangeBrightness = 165;

// put (function) declarations here:
// int myFunction(int, int);

void setup() {
  // pin modes
  pinMode(ON_OFF, INPUT);         // Button as ON/OFF input
  pinMode(LED_BUILTIN, OUTPUT);   // Built-in LED as output
  pinMode(KNOB, INPUT);           // Knob 

  Serial.begin(9600);             // Write to Serial Monito
  Serial.println("Serial 9600");

  delay(500); // Requires 500ms to wait for the MP3 module to initialize  
  mp3.playWithVolume(index, volume);
  Serial.println("Playing music...");
  delay(50); // you should wait for >=50ms between two commands
}

void loop() {
  // ON/OFF BUTTON STUFF
  bool buttonState = digitalRead(ON_OFF);
  bool pressed = true;  

  // Check if the button is pressed
  if (buttonState == pressed) {
    // Start the dimming process
    dimming = true;
    volumeDown = true;
  } else {
    dimming = false;
    volumeDown = false;
    redBrightness = 255; // Reset brightness
    volume = 15; // Reset volume
    leds.setColorRGB(0, 255, 255, 255); // Ensure the Chainable LED is off
  }

  // Dimming process
  if (dimming) {
    if (redBrightness > 0) {
      redBrightness -= 5; // Decrease brightness
      leds.setColorRGB(0, redBrightness, 0, 0); // Adjust the Chainable LED brightness
      delay(100); // Delay to make the dimming visible
    } else {
      if (redBrightness == 0) {
        lighting = true;
      }
    }
  }

  if (lighting == true) {
    if (orangeBrightness < 255) {
      redBrightness += 5; // Increase red
      orangeBrightness += 5; // Increase orange
      leds.setColorRGB(0, redBrightness, orangeBrightness, 0); // Adjust the Chainable LED brightness
      delay(100); // Delay to make the dimming visible
    } else {
      lighting = false;
    }
  }

  // Volume down process
  if (volumeDown) {
    if (volume > 0) {
      volume -= 0.30;
      mp3.setVolume(volume); // Update the volume in the MP3 player
      Serial.println(volume);
    } else if (volume == volumeOff) {
      volume = 0;
      mp3.setVolume(volume); // Reset the volume in the MP3 player
      Serial.println(volume);
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
