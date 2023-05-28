#include <Arduino.h>
#include <BleMouse.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// Bluetooth Configs
#define X_RANDOM_RANGE 3
#define Y_RANDOM_RANGE 3
#define JIGGLE_STEP_INTERVAL 50
#define JIGGLE_MIN_DISTANCE 5
#define JIGGLE_MAX_DISTANCE 20
#define INTERVAL_LIST { 30, 90, 180, 300, 600, 900 }
#define DEFAULT_INTERVAL 2
#define NUM_CHANNELS 3

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
#define LONG_PRESS 1000

// Types
#define BUTTON_NONE 0
#define BUTTON_PRESS 1
#define BUTTON_LONGPRESS 2

struct ButtonState {
    bool init;
    u_int64_t pressed;
    u_int64_t longpress;
    u_int64_t released;
};

// Initialize Bluetooth
BleMouse bleMouse("Bluetooth Mouse 4.2", "Important Technologies", 42);

// Initialize Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize preferences from flash
Preferences preferences;

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

short buttonState(int pin, unsigned long now, ButtonState *buttonState)
{
    if (buttonState->init == false && digitalRead(pin) == HIGH)
    {
        // first unpressed state registered
        buttonState->init = true;
    }
    else if(buttonState->init == false)
    {
        // button was pressed before firmware started, ignore
        return BUTTON_NONE;
    }

    if (buttonState->pressed == 0 && digitalRead(pin) == LOW)
    {
        // button pressed
        buttonState->pressed = millis();
    }
    else if (buttonState->pressed > 0 && buttonState->released == 0 && buttonState->longpress == 0 && digitalRead(pin) == LOW)
    {
        // button still pressed
        if (millis() - buttonState->pressed > LONG_PRESS)
        {
            // it was a long press
            buttonState->longpress = millis();
            return BUTTON_LONGPRESS;
        }
    }
    else if (buttonState->pressed > 0 && buttonState->released == 0 && digitalRead(pin) == HIGH)
    {
        // button was released
        buttonState->released = millis();
        if (buttonState->longpress == 0)
        {
            // no longpress recorded, so it's a short press
            return BUTTON_PRESS;
        }
    }
    else if (buttonState->pressed > 0 && buttonState->released > 0 && millis() - buttonState->released > DEBOUNCE_DELAY)
    {
        // button released, debounce time expired
        buttonState->pressed = 0;
        buttonState->released = 0;
        buttonState->longpress = 0;
    }

    return BUTTON_NONE;
}

// --> Logic

unsigned short bluetoothChannelOffset = 0;
unsigned long now = 0;
ButtonState buttonStateTop;
ButtonState buttonStateBottom;
short buttonResult;
unsigned long lastJiggle;
unsigned long lastDisplayUpdate = 0;
bool running = true;
bool connected = false;
bool newConnectState = false;
bool dirty = true;
int nextJiggleDiff;
int intervals[] = INTERVAL_LIST;
size_t numIntervals = sizeof(intervals) / sizeof(intervals[0]);
int current_interval;
int jiggle_interval;
char animation[] = { '-', '\\', '|', '/' };
size_t numAnimations = sizeof(animation) / sizeof(animation[0]);
int8_t i_animation = 0;
char s [22];

void setup()
{
    // Serial
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // Preferences
    preferences.begin("app", false);
    current_interval = preferences.getShort("intv", DEFAULT_INTERVAL);
    jiggle_interval = intervals[current_interval] * 1000;

    // mac address
    // https://generate.plus/en/address/mac
    // Logitech Inc
    bluetoothChannelOffset = preferences.getUShort("macoffset", 0);
    uint8_t macoffset = 0xAE + bluetoothChannelOffset;
    uint8_t new_mac[8] = { 0xEC, 0x81, 0x93, 0x37, macoffset, 0xCB };
    esp_base_mac_addr_set(new_mac);

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

    buttonResult = buttonState(BUTTON_UP, now, &buttonStateTop);
    if (buttonResult == BUTTON_PRESS)
    {
        Serial.println("shortpressed_top");
        running = !running;
        dirty = true;
        lastJiggle = now;
    }
    else if (buttonResult == BUTTON_LONGPRESS)
    {
        Serial.println("longpressed_top");
    }

    buttonResult = buttonState(BUTTON_DOWN, now, &buttonStateBottom);
    if (buttonResult == BUTTON_PRESS)
    {
        Serial.println("shortpressed_bottom");
        dirty = true;
        current_interval = (current_interval + 1) % numIntervals;
        jiggle_interval = intervals[current_interval] * 1000;
        lastJiggle = now;
        preferences.putShort("intv", current_interval);
    }
    else if (buttonResult == BUTTON_LONGPRESS)
    {
        Serial.println("longpressed_bottom");
        preferences.putUShort("macoffset", (preferences.getUShort("macoffset", 0) + 1) % NUM_CHANNELS);

        // TODO: solve issues with restarting BleMouse
        ESP.restart();
    }

    newConnectState = bleMouse.isConnected();
    if (newConnectState != connected)
    {
        connected = newConnectState;
        jiggle_interval = intervals[current_interval] * 1000;
        dirty = true;

        if (!connected)
        {
            // TODO: solve issues with restarting BleMouse
            ESP.restart();
        }
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
            sprintf (s, "Next in %ds", nextJiggleDiff / 1000);
            display.print(F(s));
        }

        display.setCursor(2, 22);
        sprintf (s, "Intv: %ds", intervals[current_interval]);
        display.print(F(s));

        display.setCursor(96, 22);
        sprintf (s, "Ch: %d", bluetoothChannelOffset);
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
