#include <Arduino.h>
#include <BleMouse.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// Bluetooth Configs
#define X_RANDOM_RANGE 3
#define Y_RANDOM_RANGE 3
#define JIGGLE_STEP_INTERVAL 50
#define JIGGLE_MIN_DISTANCE 5
#define JIGGLE_MAX_DISTANCE 20
#define INTERVAL_LIST { 30, 90, 180, 300, 600, 900 }
#define DEFAULT_INTERVAL 2

// Display Configs
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define DISPLAY_UPDATE_INTERVAL 500

// Button Configs
#define BUTTON_UP 5
#define BUTTON_DOWN 27
#define DEBOUNCE_DELAY 250

// Initialize Bluetooth
BleMouse bleMouse("Bluetooth Mouse 4.2", "Important Technologies", 42);

// Initialize Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --> Functions

void moveMouse()
{
    int distance = random(JIGGLE_MIN_DISTANCE, JIGGLE_MAX_DISTANCE);
    int x = random(X_RANDOM_RANGE) - 1;
    int y = random(Y_RANDOM_RANGE) - 1;

    for (int i = 0; i < distance; i++)
    {
        bleMouse.move(x, y);
        delay(JIGGLE_STEP_INTERVAL);
    }

    for (int i = 0; i < distance; i++)
    {
        bleMouse.move(0 - x, 0 - y);
        delay(JIGGLE_STEP_INTERVAL);
    }
}

bool buttonState(int pin, unsigned long now, unsigned long *lastBounce)
{
    if ((now - *lastBounce) > DEBOUNCE_DELAY && digitalRead(pin) == LOW)
    {
        *lastBounce = now;
        return true;
    }

    return false;
}

// --> Logic

unsigned long now = 0;
unsigned long lastDebounceTimeUp = 0;
unsigned long lastDebounceTimeDown = 0;
unsigned long lastJiggle;
unsigned long lastDisplayUpdate = 0;
bool running = true;
bool connected = false;
bool newConnectState = false;
bool dirty = true;
int nextJiggleDiff;
int intervals[] = INTERVAL_LIST;
size_t numIntervals = sizeof(intervals) / sizeof(intervals[0]);
int current_interval = DEFAULT_INTERVAL;
int jiggle_interval = intervals[current_interval] * 1000;
char animation[] = { '-', '\\', '|', '/' };
size_t numAnimations = sizeof(animation) / sizeof(animation[0]);
int8_t i_animation = 0;

void setup()
{
    // Serial
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // Button pins
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);

    // Bluetooth
    bleMouse.begin();

    // Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.setRotation(2);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    lastJiggle = millis();
}

void loop()
{
    now = millis();

    if (connected && running && now - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL)
    {
        dirty = true;
    }

    if (buttonState(BUTTON_UP, now, &lastDebounceTimeUp))
    {
        Serial.println("pressed_up");
        running = !running;
        dirty = true;
        lastJiggle = now;
    }

    if (buttonState(BUTTON_DOWN, now, &lastDebounceTimeDown))
    {
        Serial.println("pressed_down");
        dirty = true;
        current_interval = (current_interval + 1) % numIntervals;
        jiggle_interval = intervals[current_interval] * 1000;
        lastJiggle = now;
    }

    newConnectState = bleMouse.isConnected();
    if (newConnectState != connected)
    {
        connected = newConnectState;
        jiggle_interval = intervals[current_interval] * 1000;
        dirty = true;
    }

    nextJiggleDiff = jiggle_interval - (now - lastJiggle);

    if (dirty)
    {
        display.clearDisplay();
        display.setCursor(2, 0);
        display.print(F(connected ? (running ? "Jiggling..." : "Standby") : "Not connected"));

        if (connected && running)
        {
            display.setCursor(120, 0);
            i_animation = (i_animation + 1) % numAnimations;
            display.print(animation[i_animation]);

            display.setCursor(2, 11);
            char s [22];
            sprintf (s, "Next in %ds", nextJiggleDiff / 1000);
            display.print(F(s));
        }

        display.setCursor(2, 22);
        char s [22];
        sprintf (s, "Interval: %ds", intervals[current_interval]);
        display.print(F(s));

        display.display();

        dirty = false;
        lastDisplayUpdate = now;
    }

    if (connected && running && nextJiggleDiff <= 0)
    {
        lastJiggle = now;
        moveMouse();
    }
}
