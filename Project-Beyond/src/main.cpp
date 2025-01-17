#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RedMP3.h>
#include <Adafruit_NeoPixel.h>

#define MODE_BUTTON 3
#define PRESSURE_BUTTON 2
#define FEEDBACK_LED 6
#define KNOB A1
#define MP3_RX 8
#define MP3_TX 9
#define NEOPIXEL_PIN 4
#define NUMPIXELS 15
#define SECOND_STRIP_PIN 5
#define SECOND_NUMPIXELS 15

enum Mode
{
    SET_WAKEUP_TIME,
    SET_RED_LIGHT_TIME
};

class LightAndMusicController
{
private:
    MP3 mp3;
    Adafruit_NeoPixel strip;
    Adafruit_NeoPixel secondStrip;
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
    LightAndMusicController(int mp3Rx, int mp3Tx, int neoPixelPin, int numPixels, int secondNeoPixelPin, int secondNumPixels)
        : mp3(mp3Rx, mp3Tx), strip(numPixels, neoPixelPin, NEO_GRB + NEO_KHZ800), secondStrip(secondNumPixels, secondNeoPixelPin, NEO_GRB + NEO_KHZ800), currentMode(SET_WAKEUP_TIME), wakeupTime(1), redLightTime(1), brightness(255), volume(0), dimming(false), previousBrightness(255), previousVolume(0), musicIndex(1), settingMode(false) {}

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

        secondStrip.begin();
        secondStrip.show(); // Initialize all pixels to 'off'

        // Set initial white light on the second LED strip
        setSecondStripColor(secondStrip.Color(255, 255, 255));
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
                setStripColor(strip.Color(0, 0, 100)); // Blue for SET_WAKEUP_TIME
            }
            else
            {
                setStripColor(strip.Color(100, 0, 0)); // Red for SET_RED_LIGHT_TIME
            }
        }
    }

    void setWakeupTime()
    {
        int newWakeupTime = map(analogRead(KNOB), 0, 1023, 1, 8);
        delay(50); // Add a small delay to debounce the knob input
        if (newWakeupTime != wakeupTime)
        {
            wakeupTime = newWakeupTime;
            Serial.print("Wakeup Time set to: ");
            Serial.println(wakeupTime);
            updateStripColor(strip.Color(0, 0, 255), wakeupTime); // Blue intensity based on wakeup time
        }
    }

    void setRedLightTime()
    {
        int newRedLightTime = map(analogRead(KNOB), 0, 1023, 1, 30);
        delay(50); // Add a small delay to debounce the knob input
        if (newRedLightTime != redLightTime)
        {
            redLightTime = newRedLightTime;
            Serial.print("Red Light Time set to: ");
            Serial.println(redLightTime);
            updateStripColor(strip.Color(255, 0, 0), redLightTime); // Red intensity based on red light time
        }
    }

    void updateStripColor(uint32_t color, int value)
    {
        int numPixelsToLight = value; // Directly use the value for the number of pixels
        for (int i = 0; i < NUMPIXELS; i++)
        {
            if (i < numPixelsToLight)
            {
                strip.setPixelColor(i, color);
            }
            else
            {
                strip.setPixelColor(i, 0); // Turn off the remaining pixels
            }
        }
        strip.show();
    }

    void handlePressureButton()
    {
        if (digitalRead(PRESSURE_BUTTON) == HIGH)
        {
            if (!dimming)
            {
                mp3.playWithVolume(4, 10);
                delay(3000); // Play the init sound for 3 seconds

                volume = 15; // Set volume to 15 when starting the dimming process
                mp3.playWithVolume(musicIndex, volume);
                Serial.println("Playing noise from MP3 player at volume 15");
                setSecondStripColor(secondStrip.Color(255, 0, 0)); // Red light on the second LED strip
                dimming = true;
                brightness = 255;
                Serial.println("Pressure button pressed: Playing noise and emitting red light");
                delay(500); // Debounce delay

                // Turn off the main LED strip
                setStripColor(strip.Color(0, 0, 0));
            }
        }
        else
        {
            if (dimming)
            {
                dimming = false;
                setSecondStripColor(secondStrip.Color(255, 255, 255)); // Set white light on the second LED strip
                mp3.setVolume(0);
                Serial.println("Pressure button released: Light and volume turned off. Returning to white light.");
                setStripColor(strip.Color(0, 0, 0)); // Turn off the main LED strip
            }
        }

        if (dimming)
        {
            if (brightness > 0)
            {
                brightness -= 5;
                volume = max(volume - 0.3, 0.0);
                analogWrite(LED_BUILTIN, brightness);
                setSecondStripColor(secondStrip.Color(brightness, 0, 0)); // Adjust brightness on the second LED strip
                mp3.setVolume(static_cast<int>(volume));
                Serial.print("Dimming... Brightness: ");
                Serial.print(brightness);
                Serial.print(", Volume: ");
                Serial.println(volume);
                delay(100);
            }
            else
            {
                dimming = false;
                setSecondStripColor(secondStrip.Color(0, 0, 0)); // Turn off the second LED strip
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
                    setSecondStripColor(secondStrip.Color(red, green, 0)); // Update second LED strip
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
                setSecondStripColor(secondStrip.Color(red, green, 0)); // Orange light on the second LED strip
                Serial.print("Volume: ");
                Serial.print(volume);
                Serial.print(", Red: ");
                Serial.print(red);
                Serial.print(", Green: ");
                Serial.println(green);

                while (digitalRead(PRESSURE_BUTTON) == HIGH)
                {
                    Serial.println("Program completed.");
                }
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

    void setSecondStripColor(uint32_t color)
    {
        for (int i = 0; i < SECOND_NUMPIXELS; i++)
        {
            secondStrip.setPixelColor(i, color);
        }
        secondStrip.show();
    }
};

LightAndMusicController controller(MP3_RX, MP3_TX, NEOPIXEL_PIN, NUMPIXELS, SECOND_STRIP_PIN, SECOND_NUMPIXELS);

void setup()
{
    controller.initialize();
}

void loop()
{
    controller.update();
}