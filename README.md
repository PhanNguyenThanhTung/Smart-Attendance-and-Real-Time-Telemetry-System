# Smart Attendance System

A smart attendance system utilizing RFID (13.56 MHz MFRC522) technology, an LCD 2004 (I2C) display, and real-time telemetry communication with a Desktop Application (Python GUI) over UART.

This project is programmed bare-metal on the STM32F103C8T6 microcontroller (without using HAL/SPL libraries).

---

## Quick Start

### 1. Prerequisites
* **Firmware:**
  * **Compiler:** GNU Arm Embedded Toolchain (make sure `arm-none-eabi-gcc` is in your system PATH).
  * **Build Tool:** GNU Make.
  * **Debugger/Programmer:** ST-Link V2.
  * **Flashing Tool:** OpenOCD or STM32CubeProgrammer.
* **Desktop App:**
  * **Python 3.8+**

---

### 2. Compiling the STM32 Firmware

Open a terminal in the root directory of the project (where the Makefile is located) and run:

```bash
make clean
make
```

Upon successful compilation, the following files will be generated in the root directory:
* `main.bin` - The binary file to flash onto the microcontroller.
* `main.elf` - The ELF executable file containing debug symbols.
* `main.map` - The linker map file showing memory allocation.

---

### 3. Flashing the Firmware

Connect the ST-Link V2 debugger to the SWD interface of the STM32F103C8T6 (SWDIO, SWCLK, GND, 3.3V).

#### Method 1: Using OpenOCD (Command Line)
```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program main.elf verify reset exit"
```

#### Method 2: Using STM32CubeProgrammer (GUI)
1. Open STM32CubeProgrammer.
2. Select **ST-LINK** as the connection interface and click **Connect**.
3. Click **Open file** and select `main.bin`.
4. Set the start address to `0x08000000` and click **Start Programming**.

---

### 4. Running the Desktop Application

1. Navigate to the application directory:
   ```bash
   cd SmartAttendanceApp
   ```
2. Create a Python virtual environment:
   ```bash
   python -m venv .venv
   ```
3. Activate the virtual environment:
   * **Windows (PowerShell):**
     ```powershell
     .\.venv\Scripts\Activate.ps1
     ```
   * **Linux / macOS:**
     ```bash
     source .venv/bin/activate
     ```
4. Install the required dependencies:
   ```bash
   pip install -r requirements.txt
   ```
5. Run the application:
   ```bash
   python app.py
   ```

---

## Project Objectives

* Read RFID tag UIDs using the MFRC522 module.
* Display the UID and card status on the LCD 2004 I2C.
* Transmit the UID from the STM32 to the PC via UART.
* PC application checks the UID against a registered user list.
* PC application transmits the authentication result back to the STM32.
* STM32 displays the authentication result on the LCD.
* PC application logs attendance history to a CSV file.
* Transition from breadboard prototyping to a custom PCB design.

---

## Hardware Connection

Caution: The MFRC522 module operates on 3.3V logic. Do not connect it to a 5V power supply.

### 1. MFRC522 RFID to STM32 (SPI1)

| MFRC522 Pin | STM32F103 Pin | Function |
|:---:|:---:|---|
| SDA / SS | PA4 | Chip Select (Software Controlled GPIO) |
| SCK | PA5 | SPI1 Serial Clock |
| MISO | PA6 | SPI1 Master In Slave Out |
| MOSI | PA7 | SPI1 Master Out Slave In |
| RST | PA0 | Reset (GPIO) |
| 3.3V | 3.3V | Power Supply (3.3V) |
| GND | GND | Ground |

### 2. LCD 2004 I2C to STM32

| LCD I2C Pin | STM32F103 Pin | Function |
|:---:|:---:|---|
| SCL | PB6 | I2C1 Clock |
| SDA | PB7 | I2C1 Data |
| VCC | 5V / 3.3V | Power Supply (depending on module version) |
| GND | GND | Ground |

* Default I2C Address: `0x27` (or `0x3F` depending on the PCF8574 chip configuration).

### 3. UART to USB-TTL (PC Connection)

| USB-TTL Pin | STM32F103 Pin | Function |
|:---:|:---:|---|
| RX | PA9 | STM32 USART1 TX |
| TX | PA10 | STM32 USART1 RX |
| GND | GND | Ground |

* **UART Configuration:**
  * Baudrate: `9600`
  * Data bits: `8`
  * Parity: `None`
  * Stop bits: `1`
  * Flow control: `None`

---

## System Architecture

```text
+-------------+        SPI        +----------+
| STM32F103   | <---------------> | MFRC522  |
|             |                   +----------+
|             |
|             |        I2C        +---------------+
|             | <---------------> | LCD 2004 I2C  |
|             |                   +---------------+
|             |
|             |       UART        +----------------+
|             | <---------------> | Desktop App PC |
+-------------+                   +----------------+
```

### Data Flow

```text
RFID Card Scan
      │
      ▼
MFRC522 reads UID
      │
      ▼
STM32 receives UID
      │
      ▼
STM32 displays UID on LCD
      │
      ▼
STM32 transmits UID to PC via UART (CARD:<UID>\r\n)
      │
      ▼
PC App checks UID in cards.csv
      │
      ├───────────────────────────────┐
      ▼ (Registered)                  ▼ (Unregistered)
PC responds: Known:<Name>\n     PC responds: Unknown\n
      │                               │
      │                               ▼
      │                         PC registers card
      │                               │
      │                               ▼
      │                         PC responds: Added:<Name>\n
      │                               │
      └──────────────┬────────────────┘
                     ▼
      STM32 displays result on LCD
```

---

## UART Protocol

### 1. STM32 to PC (Card Detected)
When an RFID card is read, the STM32 transmits:
```text
CARD:<UID>\r\n
```
*Example:* `CARD:50C7E85F\r\n`

### 2. PC to STM32 (Response)
* **Registered card:**
  ```text
  Known:<Username>\n
  ```
  *Example:* `Known:Thanh Tung\n`
* **Unregistered card:**
  ```text
  Unknown\n
  ```
* **Newly registered card (after Admin registers via UI):**
  ```text
  Added:<Username>\n
  ```
  *Example:* `Added:Thanh Tung\n`
* **Error occurred:**
  ```text
  Error\n
  ```

---

## Project Directory Structure

```text
Smart_Attendance/
├── Inc/                    # Header files (.h)
│   ├── main.h              # Register definitions & configurations
│   ├── MFRC522.h           # RFID module library header
│   ├── I2C_LCD.h           # I2C LCD library header
│   └── UART.h              # UART library header
├── Src/                    # Source files (.c)
│   ├── main.c              # Application entry point & main loop
│   ├── MFRC522.c           # SPI communication & MFRC522 driver
│   ├── I2C_LCD.c           # LCD control over I2C driver
│   └── UART.c              # UART driver (polling method)
├── startup.c               # MCU vector table & reset handler
├── stm32f103c8t6.ld        # Linker script defining RAM/Flash layout
├── Makefile                # Build configuration script
├── SmartAttendanceApp/     # Python desktop application directory
│   ├── app.py              # Application main entry point (GUI)
│   ├── requirements.txt    # Python library dependencies
│   └── README.md           # Application documentation
└── README.md               # Main project documentation
```

---

## Desktop Application Details

The desktop application is built with **Python 3** using **Tkinter** for the GUI and **pyserial** for serial communication.

### 1. Features
* Scans and displays available COM ports.
* Establishes/closes connection to the STM32 via GUI controls.
* Reads and writes local database CSV files:
  * `cards.csv`: Stores registered users (UID, Name, Student ID, Class, Registration Timestamp).
  * `attendance_log.csv`: Logs attendance events (UID, Name, Student ID, Class, Timestamp, Status).
* Allows direct registration of unrecognized cards on scan detection.

### 2. CSV File Formats
* **cards.csv:**
  ```csv
  uid,name,student_id,class_name,registered_at,note
  50C7E85F,Thanh Tung,SV001,Embedded,2026-05-27 14:30:00,
  ```
* **attendance_log.csv:**
  ```csv
  uid,name,student_id,class_name,time,status
  50C7E85F,Thanh Tung,SV001,Embedded,2026-05-27 14:35:10,Registered
  ```

---

## Current Status and Roadmap

### Completed Features
* Configured system clock (HSE 8 MHz scaled to 36 MHz System Clock).
* Implemented SPI1 driver for MFRC522 communication (successful UID read).
* Implemented I2C1 driver for LCD 2004 via PCF8574 decoder.
* Implemented UART1 driver for real-time serial transmission.
* Built real-time Python desktop GUI.

### Upcoming Features
* Integrate a buzzer for audio feedback on successful/failed authentication.
* Integrate RTC (Real-Time Clock) for standalone logging (offline mode).
* Utilize STM32 flash memory or external EEPROM to cache offline logs.
* Design custom PCB and 3D-printable enclosure.

---

## PCB Design Guidelines

A custom PCB carrier board is planned to replace breadboard connections.

### 1. Block Diagram
```text
  [ 5V/USB Input ] ───► [ 3.3V LDO Regulator ] 
                               │
  [ SWD ST-Link ]  ◄───────────┼───────────► [ STM32F103C8T6 MCU ]
                               │                    │
  [ USB-TTL Port ] ◄───────────┼───────────► (SPI) ─► [ MFRC522 Module ]
                               │                    │
  [ Buzzer & LED ] ◄───────────┘                    └► (I2C) ─► [ LCD 2004 I2C ]
```

### 2. Implementation Checklist
* **Power Supply:** Use an AMS1117-3.3 LDO regulator to provide 3.3V.
* **Decoupling:** Place a 100nF capacitor near every VDD pin of the STM32.
* **Routing:** Keep SPI trace lengths short and route them away from electromagnetic noise sources.
* **Debug Header:** Expose the SWD interface (`SWDIO`, `SWCLK`, `GND`, `3V3`, `NRST`) for convenient flashing and debugging.

---

## Git Guidelines

To prevent committing binary build artifacts, include the following in your `.gitignore`:

```gitignore
*.o
*.elf
*.bin
*.map
.vscode/
.venv/
__pycache__/
dist/
build/
```

---

## Author

* **Name:** Phan Nguyen Thanh Tung
* **Project:** *Smart Attendance and Real-Time Telemetry System*

---

## Contact

* **GitHub:** [Thanh Tung](https://github.com/PhanNguyenThanhTung)
* **LinkedIn:** [Phan Nguyen Thanh Tung](https://www.linkedin.com/in/phan-nguyen-thanh-tung)
* **Email:** [tungp5656@gmail.com](mailto:tungp5656@gmail.com)
