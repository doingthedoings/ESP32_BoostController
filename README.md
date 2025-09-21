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
    - **Spool Score:** This metric measures the instantaneous rate of change (peak-derivative) of the boost pressure rise. A higher spool score indicates a faster and more aggressive boost build-up, reflecting efficient turbocharger response.
    - **Torque Score:** This score is derived from the integral of boost pressure over time. It factors in how early boost is achieved and how well it's maintained until a gear shift. Crucially, the torque score penalizes overboosting by subtracting any integral value accumulated above the target boost pressure. This ensures that configurations which avoid undesirable overboost conditions receive a higher, more favorable torque score for accurate comparison and optimal tuning.
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

The firmware is configured for the following pin connections on the Waveshare ESP32-S3-Zero:

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
    - Connect your Waveshare ESP32-S3-Zero board to your computer via USB. Identify the COM port for your device and add it to the platformio.ini file accordingly.
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

The interface is controlled via six capactive touch inputs which correspond to labels shown at the bottom of the screen. These labels change depending on the menu and current function.

- **Main Screen:** Shows primary boost data.
    - **Hold `EDIT`** to enter the boost setpoint screen.
    - **Hold `CFG`** to enter the main configuration menu.
    - **Tap `A` or `B`** to switch to that preset.
    - **Tap `TS`** to enter the tune-scoring menu(additional explanations below).
    - **Tap `CLR`** to clear the peak-hold pressure value, or to enter the Tune Scoring (TS) menu from the main screen.
- **Menus:**
    - **`+` / `-`:** Navigate up/down or increase/decrease values.
    - **`SEL`:** Select a menu item to edit.
    - **`BACK`:** Return to the previous screen.
    - **`SAVE`:** Hold to save the current settings to EEPROM.

### Tune Scoring (TS) Menu

The Tune Scoring menu provides insights into your boost control performance.

*   **Score Reset Behavior:** If any configuration settings are changed for the active profile (Preset A or B), the Spool Score and Torque Score for that profile will be automatically reset. This ensures that the scores accurately reflect the performance of the *new* settings, requiring re-testing to generate fresh scores.
*   **Saving Scores:** To save the current profile's configuration along with its associated Spool Score and Torque Score values, navigate to the TS menu and press the corresponding `SaveA` or `SaveB` buttons. This allows you to store and compare the performance of different tuning strategies.

## Configuration Menus

The ESP32 Boost Controller features a comprehensive menu system accessible directly on the device via the OLED display and capacitive touch inputs. This section details each configuration parameter, its function, and the context provided by its associated info text.

### PID Tuning Menu

This menu allows for fine-tuning of the Proportional-Integral-Derivative (PID) control loop, which is responsible for maintaining target boost pressure.

*   **Kp (P-Gain)**
    *   **Description:** The main driving force of the PID controller. It determines the strength of the immediate response to a boost error.
*   **Ki (I-Gain)**
    *   **Description:** Addresses steady-state errors by accumulating error over time. It helps to eliminate boost droop and ensure the target boost is held consistently.
*   **Kd (D-Gain)**
    *   **Description:** Reacts to the rate of change of the boost error, helping to dampen oscillations and prevent overshoot.
*   **Max I term (Max Integral)**
    *   **Description:** Sets a limit on the accumulated integral error. This prevents "integral 'windup,'" a condition where a large integral value can cause significant overshoot after a disturbance.
*   **Trig. Thres. (PID Trigger)**
    *   **Unit:** kPa
    *   **Description:** Defines how close the current boost pressure needs to be to the target pressure before the PID control loop fully activates. This prevents the PID from reacting to minor fluctuations far from the target.
*   **Press. Limiter (Pressure Limiter)**
    *   **Unit:** kPa
    *   **Description:** A safety feature that forces the wastegate open if the boost pressure exceeds the target pressure by this specified value. This prevents overboost conditions that could damage the engine.
*   **Solenoid Freq. (Solenoid Frequency)**
    *   **Unit:** Hz
    *   **Description:** Sets the operating frequency for the boost control solenoid. This value should be matched to the specifications of your particular solenoid for optimal performance.
    
### MAP Sensor Menu

This menu is used for calibrating the Manifold Absolute Pressure (MAP) sensor to ensure accurate boost readings.

*   **Pressure Offset**
    *   **Unit:** kPa
    *   **Description:** Allows for a manual correction to the MAP sensor reading to match a known, accurate external gauge.
*   **Min kPa**
    *   **Unit:** kPa
    *   **Description:** The minimum pressure (in kPa) that your MAP sensor is designed to read. This value, along with Max kPa, Min Volts, and Max Volts, must accurately reflect your sensor's datasheet.
*   **Max kPa**
    *   **Unit:** kPa
    *   **Description:** The maximum pressure (in kPa) that your MAP sensor is designed to read.
*   **V Offset (Voltage Offset)**
    *   **Unit:** V
    *   **Description:** The voltage scaling offset from the MAP sensor data sheet(if applicable). If there is no specified voltage offset, leave this value at 0.
*   **Min Volts**
    *   **Unit:** V
    *   **Description:** The minimum voltage output by your MAP sensor at its lowest pressure reading.
*   **Max Volts**
    *   **Unit:** V
    *   **Description:** The maximum voltage output by your MAP sensor at its highest pressure reading.

### Filtering & Misc. Menu

This menu contains parameters related to signal filtering, timing, and general system behavior.

*   **Slow EMA-A (Slow Exponential Moving Average Alpha)**
    *   **Description:** Controls the smoothing applied to boost pressure readings for stable conditions. A lower value results in heavier smoothing, making the readings more stable but less responsive to rapid changes.
    
*   **Fast EMA-A (Fast Exponential Moving Average Alpha)**
    *   **Description:** Controls the smoothing applied during rapid boost changes. A higher value results in less smoothing, making the readings more responsive but potentially more susceptible to noise.
    
*   **P-Rate Thres. (Pressure Rate Threshold)**
    *   **Unit:** kPa
    *   **Description:** The change in pressure (in kPa) required to switch from using the slow EMA filter to the fast EMA filter. This allows for stable readings during steady boost and responsive readings during spool-up.
    
*   **Rate period (Rate Period)**
    *   **Unit:** ms
    *   **Description:** The time window (in milliseconds) over which the pressure rate of change is calculated. This influences how quickly the system detects rapid boost changes.
    
*   **Oversampling**
    *   **Description:** The number of Analog-to-Digital Converter (ADC) reads averaged together to produce a single pressure measurement. Higher oversampling reduces noise but consumes more CPU time.
    
*   **Save/Reset Delay**
    *   **Unit:** ms
    *   **Description:** The duration (in milliseconds) that the SAVE or RESET button must be held down to activate its function. This prevents accidental activation.
    
*   **Edit/CFG Delay**
    *   **Unit:** ms
    *   **Description:** The duration (in milliseconds) that the EDIT or CFG button must be held down to enter its respective menu.
    
*   **Sleep Delay**
    *   **Unit:** s
    *   **Description:** The amount of idle time (in seconds) at atmospheric pressure before the display and solenoid turn off to conserve power.
    
*   **TS Rate (Torque Score Rate)**
    *   **Unit:** ms
    *   **Description:** The sample rate (in milliseconds) at which data is collected for the Torque Score calculation. A lower value provides more granular data but increases processing load.
    


### Factory Reset

To restore all settings to their default values, press and hold **Touch Input 6** (`CLR`) while the device is powering on. A "FACTORY RESET..." message will appear on the screen.


## License

This project is licensed under the MIT License.

The MIT License is a permissive free software license, meaning it allows you to do almost anything you want with the code, such as using, copying, modifying, merging, publishing, distributing, sublicensing, and/or selling copies of the software. The only requirements are that the original copyright notice and permission notice are included in all copies or substantial portions of the software.

This choice was made to encourage broad adoption and collaboration, allowing developers and enthusiasts to freely use and build upon this project for their own purposes, both open source and commercial, with minimal restrictions.

