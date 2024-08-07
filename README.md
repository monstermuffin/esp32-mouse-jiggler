![](mousedownload.png)

## ESP32 BLE Mouse Jiggler

This allows an ESP32 to emulate a mouse and does a jiggle and/or right click on random (configurable) intervals.

Undetectable. This *is* a mouse.

## Setup Instructions

#### Prepare the Development Environment:
* Install [Visual Studio Code](https://code.visualstudio.com/).
* Install the [PlatformIO IDE](https://platformio.org/platformio-ide) extension in VS Code.

#### Get the Project Files:
Clone this repository:
```bash
git clone https://github.com/monstermuffin/mouse-jiggler.git
```

#### Configure the Project (if necessary):
- Open `platformio.ini` in the project root.
- Ensure the correct board is selected (e.g., `board = esp32dev`).
- Modify any other settings as needed for your specific ESP32 board.
#### Customize the Code (optional):
- Open `src/main.cpp`.
- Modify the configurable settings as described in the "Optional Configuration Settings" section.
#### Upload the Code:
- Use PlatformIO to flash your ESP32 board.
#### Monitor Serial Output (optional):
- In PlatformIO, under "Project Tasks", click "Monitor" to view serial output.
#### Connect to a Bluetooth Host:
- Pair with this device as you would with a regular Bluetooth mouse.
#### Operate the Device:
- Use the BOOT button to toggle enabled/disabled as needed.
- Observe the LED (if enabled) and/or a mouse jiggle to check if enabled.

## Optional Configuration Settings
To customize the behaviour of your ESP32 BLE Jiggler, you can modify the following constants in the `main.cpp` file:

Device Name: Change the name that appears in the OS. Note: MacOS doesn't seem to care about this.
   ```cpp
   #define DEVICE_NAME "Microsoft Ergonomic Mouse"
   ```

Initial Feature State: Set these to `true` or `false` to enable or disable features on startup.
   ```cpp
   bool enableMouseMovement = true;
   bool enableRightClick = false;
   ```

Movement Range: How many pixels (maximum) are moved in each direction.
   ```cpp
   #define X_RANDOM_RANGE 5
   #define Y_RANDOM_RANGE 5
   ```

Movement Interval: Range for the random intervals in ms between movements.
   ```cpp
   #define MIN_MOVE_INTERVAL 5000    // 5 seconds
   #define MAX_MOVE_INTERVAL 30000   // 30 seconds
   ```

Right-Click Interval: Range for the random intervals in ms between right clicks.
   ```cpp
   #define MIN_CLICK_INTERVAL 10000  // 10 seconds
   #define MAX_CLICK_INTERVAL 60000  // 60 seconds
   ```

Disconnect Timeout: How long the device waits before restarting if disconnected.
   ```cpp
   #define DISCONNECT_TIMEOUT 10000  // 10 seconds
   ```

LED Configuration:
   - Set `USE_LED` to `false` if your board doesn't have an LED or you don't want to use it.
   - Change `LED_PIN` to match your board's LED pin, if different.
   ```cpp
   #define USE_LED true
   #define LED_PIN 2
   ```

## Original vs This Fork

### Additions

* Device Emulation:
    * Emulates a "Microsoft Ergonomic Mouse" mouse.
* Randomized Movement:
    * Uses random intervals for mouse movements (between 5 and 30 seconds).
    * Random movement in X and Y directions (range: -5 to 5 pixels).
* Right-Click Functionality:
    * Adds option for automated right-clicks
    * Random intervals for right-clicks (between 10 to 60 seconds)
* Configuration Toggle:
    * Uses BOOT button (GPIO0) to toggle enabled/disabled.
* LED Indicator:
    * Optional LED support to show enabled/disabled status.
* Connection Handling:
    * Implements a 10-second timeout for reconnection attempts.
    * Auto-restarts ESP32 if connection is lost for too long.
* Wiggle on Connect:
    * Performs a "wiggle" movement upon successful connection.
* Enhanced Logging:
    * Extensive serial logging for debugging and status updates.
* Simplified Board Support:
    * Uses generic ESP32 board configuration in platformio.ini.

### Removals

* Display Functionality:
    * Removed OLED display support and related code.
* Multi-Button Interface:
    * Removed support for multiple buttons, now uses single BOOT button.
* Preferences Storage:
    * Removed use of preferences library for storing settings.
* Fixed Movement Patterns:
    * Removed predetermined "jiggle" patterns
* MAC Address Customization:
    * Removed functionality to customize MAC address.

## Credits

* Original project forked from [tornado67/DroChill](https://github.com/perryflynn/mouse-jiggler)
* This version forked from [perryflynn/mouse-jiggler](https://github.com/perryflynn/mouse-jiggler)


## License

[MIT](https://choosealicense.com/licenses/mit/)
