# Smart Attendance System - Desktop App

This is a desktop application written in Python (using Tkinter and pyserial) for managing RFID attendance, connected via UART to an STM32F103C8T6 microcontroller.

## 1. Project Structure

```text
SmartAttendanceApp/
├── app.py                  # Main application source code (GUI, logic, UART communication)
├── requirements.txt        # Python library dependencies
├── README.md               # Documentation file
└── data/                   # Data storage directory
    ├── cards.csv           # Database for registered cards
    └── attendance_log.csv  # Database for attendance scan history
```

Note: The `data/` directory and CSV files will be generated automatically upon running the application for the first time if they do not exist.

## 2. Environment Setup

### Install Python
You need **Python 3.x** installed on your computer (Python 3.8 or newer is recommended).
Download Python from: [python.org](https://www.python.org/downloads/) (make sure to check the option "Add Python to PATH" during installation).

### Install Dependencies
Open Terminal / Command Prompt in the `SmartAttendanceApp` directory and run the following command to install the required libraries:
```bash
pip install -r requirements.txt
```

(The command installs the `pyserial` library required for serial port communication).

## 3. Running the Application

Run the following command in the `SmartAttendanceApp` directory:
```bash
python app.py
```

## 4. Usage Instructions

### Connecting to STM32
1. Plug the USB-to-TTL module (connected to STM32) into your computer.
2. Launch the application, and click **Refresh** in the **Connection Setup** section.
3. Select the correct **COM Port** associated with the USB-to-TTL converter.
4. Select the **Baudrate** (default is `9600`).
5. Click **Connect**. A green indicator displaying `Connected` confirms a successful connection.

### Communication Flow
* When an RFID card is swiped over the MFRC522 module, the STM32 sends `Card:<UID>\n` to the computer.
* **If the card is registered:**
  * The GUI displays a green scan area showing the user's name.
  * The app sends a response back to the STM32: `Known:<Name>\n`.
* **If the card is unregistered:**
  * The GUI displays a yellow scan area.
  * The app sends a response back to the STM32: `Unknown\n`.
  * You can fill in the Name, Student ID, Class, and click **Register This Card**.
  * Upon successful registration, the app sends a response back to the STM32: `Added:<Name>\n`.

### Testing Without STM32 Hardware
If you want to test the application without the STM32 hardware:
1. Use a second USB-to-TTL converter or virtual serial port software (such as com0com or Virtual Serial Port Driver) to create a virtual COM port pair.
2. Use a serial terminal tool (such as Hercules, XCTU, or Arduino Serial Monitor).
3. Transmit the string `CARD:50C7E85F\n` or `Card:50C7E85F\n` to the COM port.
4. Observe the response from the GUI and terminal output.

## 5. Packaging into an Executable (.exe) for Windows

You can package the application into a standalone `.exe` using the `PyInstaller` library.

**Step 1:** Install PyInstaller
```bash
pip install pyinstaller
```

**Step 2:** Compile to executable
Run the following command in the `SmartAttendanceApp` directory:
```bash
pyinstaller --onefile --windowed app.py
```
* `--onefile`: Packages the entire application into a single executable file.
* `--windowed` (or `--noconsole`): Hides the command line console window during execution.

**Step 3:** Access the executable
After completion, navigate to the `dist/` directory, locate `app.exe`, and copy it to the main `SmartAttendanceApp` directory. The `data/` directory will be created automatically next to the executable when run.
