# ESP32-S3 Boost Controller

This project is a sophisticated, feature-rich electronic boost controller for turbocharged vehicles, built on the powerful ESP32-S3 platform. It utilizes a dual-core architecture with FreeRTOS to provide responsive, real-time PID-based control over boost pressure, while simultaneously offering a rich user interface through an OLED display and capacitive touch inputs.

![alt text](https://github.com/doingthedoings/ESP32_BoostController/blob/main/Screenshot%202025-09-21%20162025.png?raw=true "Boost Controller Render")

## Features

- **High-Performance Control:** Real-time PID control loop running on a dedicated core for maximum stability and responsiveness.
- **Dual-Core Architecture:** Leverages the ESP32-S3's dual cores, dedicating one to the control loop and the other to the user interface, ensuring no-compromise performance.
- **Intuitive UI:** On-the-fly adjustments and monitoring via a 128x64 SSD1306 OLED display and a six-button capacitive touch interface.
- **Switchable Tuning Profiles:** Two distinct and saveable presets (A and B) allow for quick changes between different boost levels or tuning strategies (e.g., "street" and "race").
- **In-Depth On-Device Tuning:** A comprehensive menu system allows for live tuning of all critical parameters without needing to re-flash the firmware:
    - PID gains (Kp, Ki, Kd)
    - MAP sensor calibration
    - Solenoid frequency
    - Target boost and pressure limits
    - Signal filtering and timing
- **Performance Analytics:** Innovative "Spool Score" and "Torque Score" metrics provide quantitative feedback on your engine's boost response, allowing for data-driven tuning.
- **Persistent Memory:** All settings and tuning profiles are automatically saved to the ESP32's internal EEPROM, ensuring they are retained across power cycles.
- **Hardware-Level Safety:** Includes functionality for factory reset via a touch-hold on boot.

## Hardware Requirements

| Component | Details |
| --- | --- |
| **Microcontroller** | Waveshare ESP32-S3-Zero (Compiled for Adafruit Feather ESP32-S3) |
| **Display** | 128x64 I2C SSD1306 OLED Display |
| **Regulator** | Pololu 5V, 600mA+ Step-Down Regulator |
| **Pressure Sensor** | Configurable for any pressure sensor by entering minkPa, maxkPa, minVoltage, maxVoltage, and mV offset |
| **Boost Solenoid** | Currently only tested with 3-port boost control solenoid |
| **MOSFET** | 30N06 (or similar logic-level) to drive the solenoid |
| **Optocoupler** | PC817C to isolate MAP sensor signal and protect GPIO |
| **Diode** | 1N4744A Zener Diode for back-EMF protection |
| **Capacitors** | 680uF, 10V (x2) for 5V rail stabilization |
| **Resistors** | 330Ω (MOSFET gate), 1kΩ (Optocoupler), and a voltage divider for 5V MAP sensors (e.g., 10kΩ & 15kΩ, software configurable) |
| **Connectors** | JST-XH 6-pin and 4-pin for modular connections |

### Pinout

The firmware is configured for the following pin connections on the Adafruit Feather ESP32-S3:

| ESP32-S3 Pin | Component / Function |
| --- | --- |
| `1` | Boost Control Solenoid (PWM Output) |
| `2` | OLED Display (I2C SDA) |
| `3` | OLED Display (I2C SCK) |
| `6` | MAP Pressure Sensor (Analog Input) |
| `7` | Touch Input 1 (Edit / Back) |
| `8` | Touch Input 2 (Profile A / Save) |
| `9` | Touch Input 3 (Decrement) |
| `10`| Touch Input 4 (Increment) |
| `11`| Touch Input 5 (Config / Info) |
| `12`| Touch Input 6 (Select / Clear Peak-hold, Spool-score, and Torque-score on main display) |

## Software & Installation

This project is built using **PlatformIO**, a professional embedded development platform. The recommended IDE is **Visual Studio Code** with the PlatformIO extension.

### Dependencies

The project relies on the following libraries, which PlatformIO will manage automatically:
- `adafruit/Adafruit GFX Library`
- `adafruit/Adafruit SSD1306`

### Setup Instructions

1.  **Clone the Repository:**
    ```sh
    git clone https://github.com/doingthedoings/ESP32_BoostController
    ```
2.  **Install Development Environment:**
    - Install [Visual Studio Code](https://code.visualstudio.com/).
    - Open VS Code and install the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode) from the Extensions marketplace.
3.  **Open the Project:**
    - In VS Code, go to `File > Open Folder...` and select the cloned project directory.
4.  **Build & Upload:**
    - PlatformIO will automatically detect the `platformio.ini` file and should prompt you to install the required libraries if they are missing.
    - Connect your Adafruit Feather ESP32-S3 board to your computer via USB.
    - Click the **PlatformIO: Upload** task (the right-arrow icon) in the VS Code status bar at the bottom of the window. This will compile the firmware and flash it to your device.

## Code Structure

The source code is organized into several files within the `src/` directory to maintain clarity and modularity.

| File | Description |
| --- | --- |
| `ESP32_BoostController_V2.ino`| Main application entry point. Handles initial setup of hardware, EEPROM, and creates the two primary FreeRTOS tasks. |
| `tasks.cpp` | Contains the core logic for the `pidControlTask` and `displayAndInputTask`, which run concurrently on separate cores. |
| `definitions.h` | A central header defining all hardware pins, EEPROM memory addresses, data structures (`ControllerPreset`, `ScreenState`), and external variable declarations. **This is the primary file to consult for hardware configuration.** |
| `config.h` | Defines constants, menu structures, and the descriptive text used in the UI's info screens. |
| `globals.cpp` | Defines and initializes the global variables used across the application for state management. |
| `display.cpp` | Manages all rendering logic for the OLED display, including drawing menus, values, and status indicators. |
| `input.cpp` | Handles the reading of the capacitive touch inputs and translates them into UI actions and navigation. |
| `persistence.cpp` | Contains all functions related to saving and loading parameters and presets to/from the ESP32's non-volatile EEPROM memory. |
| `helpers.cpp` | Includes utility functions for tasks like sensor voltage scaling and data mapping. |

## Operation

The interface is controlled via the six touch pads, which correspond to labels shown at the bottom of the screen.

- **Main Screen:** Shows primary boost data.
    - **Hold `EDIT`** to enter the boost setpoint screen.
    - **Hold `CFG`** to enter the main configuration menu.
    - **Tap `A` or `B`** to switch to that preset.
    - **Tap `CLR`** to reset the peak-hold pressure value.
- **Menus:**
    - **`+` / `-`:** Navigate up/down or increase/decrease values.
    - **`SEL`:** Select a menu item to edit.
    - **`BACK`:** Return to the previous screen.
    - **`SAVE`:** Hold to save the current settings to EEPROM.

### Factory Reset

To restore all settings to their default values, press and hold **Touch Input 6** (`CLR`) while the device is powering on. A "FACTORY RESET..." message will appear on the screen.

## License

This project is currently unlicensed. You are free to use, modify, and distribute the code as you see fit.

