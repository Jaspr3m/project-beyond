#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RedMP3.h>
#include <ChainableLED.h>
#include <Adafruit_NeoPixel.h>

#define MODE_BUTTON 3
#define PRESSURE_BUTTON 2
#define FEEDBACK_LED 6
#define KNOB A1
#define MP3_RX 8
#define MP3_TX 9
#define NEOPIXEL_PIN 7
#define NUMPIXELS 15

enum Mode
{
    SET_WAKEUP_TIME,
    SET_RED_LIGHT_TIME
};

class LightAndMusicController
{
private:
    MP3 mp3;
    ChainableLED leds;
    Adafruit_NeoPixel strip;
    Mode currentMode;
    int wakeupTime;
    int redLightTime;
    int brightness;
    float volume;
    bool dimming;
    int previousBrightness;
    float previousVolume;
    int musicIndex;
    bool settingMode;

public:
    LightAndMusicController(int mp3Rx, int mp3Tx, int ledPin1, int ledPin2, int ledCount, int neoPixelPin, int numPixels)
        : mp3(mp3Rx, mp3Tx), leds(ledPin1, ledPin2, ledCount), strip(numPixels, neoPixelPin, NEO_GRB + NEO_KHZ800), currentMode(SET_WAKEUP_TIME), wakeupTime(1), redLightTime(1), brightness(255), volume(0), dimming(false), previousBrightness(255), previousVolume(0), musicIndex(1), settingMode(false) {}

    void initialize()
    {
        pinMode(MODE_BUTTON, INPUT);
        pinMode(PRESSURE_BUTTON, INPUT);
        pinMode(FEEDBACK_LED, OUTPUT);
        pinMode(KNOB, INPUT);

        Serial.begin(9600);
        Serial.println("Serial communication started at 9600 baud");

        strip.begin();
        strip.show(); // Initialize all pixels to 'off'
    }

    void update()
    {
        handleModeSwitch();

        switch (currentMode)
        {
        case SET_WAKEUP_TIME:
            setWakeupTime();
            break;
        case SET_RED_LIGHT_TIME:
            setRedLightTime();
            break;
        }

        handlePressureButton();
    }

private:
    void handleModeSwitch()
    {
        if (digitalRead(MODE_BUTTON) == HIGH)
        {
            currentMode = static_cast<Mode>((currentMode + 1) % 2); // Toggle between SET_WAKEUP_TIME and SET_RED_LIGHT_TIME
            settingMode = true;
            Serial.print("Mode switched to: ");
            Serial.println(currentMode == SET_WAKEUP_TIME ? "SET_WAKEUP_TIME" : "SET_RED_LIGHT_TIME");
            mp3.playWithVolume((currentMode + 2), 10);
            delay(500); // Debounce delay

            // Update NeoPixel strip based on mode
            if (currentMode == SET_WAKEUP_TIME)
            {
                setStripColor(strip.Color(0, 0, 255)); // Blue for SET_WAKEUP_TIME
            }
            else
            {
                setStripColor(strip.Color(255, 0, 0)); // Red for SET_RED_LIGHT_TIME
            }
        }
    }

    void setWakeupTime()
    {
        int newWakeupTime = map(analogRead(KNOB), 0, 1023, 1, 8);
        if (newWakeupTime != wakeupTime)
        {
            wakeupTime = newWakeupTime;
            Serial.print("Wakeup Time set to: ");
            Serial.println(wakeupTime);
            setStripColor(strip.Color(0, 0, 255)); // Blue for feedback
        }
    }

    void setRedLightTime()
    {
        int newRedLightTime = map(analogRead(KNOB), 0, 1023, 1, 30);
        if (newRedLightTime != redLightTime)
        {
            redLightTime = newRedLightTime;
            Serial.print("Red Light Time set to: ");
            Serial.println(redLightTime);
            setStripColor(strip.Color(255, 0, 0)); // Red for feedback
        }
    }

    void handlePressureButton()
    {
        if (digitalRead(PRESSURE_BUTTON) == HIGH)
        {
            volume = 15; // Set volume to 15 when starting the dimming process
            mp3.playWithVolume(musicIndex, volume);
            Serial.println("Playing noise from MP3 player at volume 15");
            leds.setColorRGB(0, 255, 0, 0); // Red light
            dimming = true;
            brightness = 255;
            Serial.println("Pressure button pressed: Playing noise and emitting red light");
            delay(500); // Debounce delay
        }

        if (dimming)
        {
            if (brightness > 0)
            {
                brightness -= 5;
                volume = max(volume - 0.3, 0.0);
                analogWrite(LED_BUILTIN, brightness);
                leds.setColorRGB(0, brightness, 0, 0);
                mp3.setVolume(static_cast<int>(volume));

                if (brightness != previousBrightness || volume != previousVolume)
                {
                    Serial.print("Dimming... Brightness: ");
                    Serial.print(brightness);
                    Serial.print(", Volume: ");
                    Serial.println(volume);
                    previousBrightness = brightness;
                    previousVolume = volume;
                }

                delay(100);
            }
            else
            {
                dimming = false;
                leds.setColorRGB(0, 0, 0, 0);
                mp3.setVolume(0);
                Serial.println("Dimming complete: Light and volume turned off. Good night!");

                // Wait for 5 seconds
                delay(5000);
                Serial.println("Starting brighting process...");

                // Increment LED brightness from 0 to orange and nature sounds volume from 0 to 10
                int red = 0;
                int green = 0;
                volume = 0;
                while (volume < 10)
                {
                    volume += 0.2;
                    red += 5;
                    green += 1;
                    mp3.setVolume(volume);
                    leds.setColorRGB(0, red, green, 0);
                    setStripColor(strip.Color(red, green, 0)); // Update NeoPixel strip
                    Serial.print("Volume: ");
                    Serial.print(volume);
                    Serial.print(", Red: ");
                    Serial.print(red);
                    Serial.print(", Green: ");
                    Serial.println(green);
                    delay(100);
                }
                volume = 10;
                mp3.setVolume(volume);
                leds.setColorRGB(0, 255, 165, 0);        // Orange light
                setStripColor(strip.Color(255, 165, 0)); // Update NeoPixel strip
                Serial.print("Volume: ");
                Serial.print(volume);
                Serial.print(", Red: ");
                Serial.print(red);
                Serial.print(", Green: ");
                Serial.println(green);
            }
        }
    }

    void setStripColor(uint32_t color)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            strip.setPixelColor(i, color);
        }
        strip.show();
    }
};

LightAndMusicController controller(MP3_RX, MP3_TX, 5, 6, 1, NEOPIXEL_PIN, NUMPIXELS);

void setup()
{
    controller.initialize();
    delay(500);
}

void loop()
{
    controller.update();
    delay(750); // Add a delay to slow down the loop
}
