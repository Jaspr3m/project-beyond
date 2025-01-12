#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RedMP3.h>
#include <ChainableLED.h>

#define MODE_BUTTON 3
#define PRESSURE_BUTTON 2
#define FEEDBACK_LED 6
#define KNOB A1
#define MP3_RX 8    
#define MP3_TX 9

enum Mode {
    SET_WAKEUP_TIME,
    SET_RED_LIGHT_TIME
};

class LightAndMusicController {
private:
    MP3 mp3;
    ChainableLED leds;
    Mode currentMode;
    int wakeupTime;
    int redLightTime;
    int brightness;
    float volume;
    bool dimming;
    int previousBrightness;
    float previousVolume;
    int musicIndex;

public:
    LightAndMusicController(int mp3Rx, int mp3Tx, int ledPin1, int ledPin2, int ledCount)
        : mp3(mp3Rx, mp3Tx), leds(ledPin1, ledPin2, ledCount), currentMode(SET_WAKEUP_TIME), wakeupTime(1), redLightTime(1), brightness(255), volume(0), dimming(false), previousBrightness(255), previousVolume(0), musicIndex(3) {}

    void initialize() {
        pinMode(MODE_BUTTON, INPUT);
        pinMode(PRESSURE_BUTTON, INPUT);
        pinMode(FEEDBACK_LED, OUTPUT);
        pinMode(KNOB, INPUT);

        Serial.begin(9600);
        Serial.println("Serial communication started at 9600 baud");
    }

    void update() {
        handleModeSwitch();

        switch (currentMode) {
            case SET_WAKEUP_TIME:
                setWakeupTime();
                break;
            case SET_RED_LIGHT_TIME:
                setRedLightTime();
                break;
        }

        handlePressureButton();
    }

    void beginMP3() {
        mp3.playWithVolume(musicIndex, 1);
        Serial.println("MP3 player initialized and playing at volume 1");
    }

private:
    void handleModeSwitch() {
        if (digitalRead(MODE_BUTTON) == HIGH) {
            currentMode = static_cast<Mode>((currentMode + 1) % 2); // Toggle between SET_WAKEUP_TIME and SET_RED_LIGHT_TIME
            Serial.print("Mode switched to: ");
            Serial.println(currentMode == SET_WAKEUP_TIME ? "SET_WAKEUP_TIME" : "SET_RED_LIGHT_TIME");
            delay(500); // Debounce delay
        }
    }

    void setWakeupTime() {
        wakeupTime = map(analogRead(KNOB), 0, 1023, 1, 8);
        Serial.print("Wakeup Time set to: ");
        Serial.println(wakeupTime);
    }

    void setRedLightTime() {
        redLightTime = map(analogRead(KNOB), 0, 1023, 1, 30);
        Serial.print("Red Light Time set to: ");
        Serial.println(redLightTime);
    }

    void handlePressureButton() {
        if (digitalRead(PRESSURE_BUTTON) == HIGH) {
            volume = 15; // Set volume to 15 when starting the dimming process
            mp3.playWithVolume(musicIndex, volume);
            Serial.println("Playing noise from MP3 player at volume 15");
            leds.setColorRGB(0, 255, 0, 0); // Red light
            dimming = true;
            brightness = 255;
            Serial.println("Pressure button pressed: Playing noise and emitting red light");
            delay(500); // Debounce delay
        }

        if (dimming) {
            if (brightness > 0) {
                brightness -= 5;
                volume = max(volume - 0.3, 0.0);
                leds.setColorRGB(0, brightness, 0, 0);
                mp3.setVolume(static_cast<int>(volume));

                if (brightness != previousBrightness || volume != previousVolume) {
                    Serial.print("Dimming... Brightness: ");
                    Serial.print(brightness);
                    Serial.print(", Volume: ");
                    Serial.println(volume);
                    previousBrightness = brightness;
                    previousVolume = volume;
                }
                delay(100);
            } else {
                dimming = false;
                leds.setColorRGB(0, 0, 0, 0);
                mp3.setVolume(0);
                Serial.println("Dimming complete: Light and volume turned off");
            }
        }
    }
};

LightAndMusicController controller(MP3_RX, MP3_TX, 5, 6, 1);

void setup() {
    controller.initialize();

    delay(500); // Requires 500ms to wait for the MP3 module to initialize  
    controller.beginMP3();
    delay(50); // you should wait for >=50ms between two commands
}

void loop() {
    controller.update();
    delay(750); // Add a delay to slow down the loop
}