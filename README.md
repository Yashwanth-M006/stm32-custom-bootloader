\# STM32 Custom Bootloader



\## Overview



This project implements a \*\*custom bootloader for the STM32F407 microcontroller\*\* designed to support \*\*secure and reliable firmware updates\*\*. The bootloader allows new firmware to be uploaded through multiple communication interfaces and safely written to flash memory while ensuring firmware integrity.



The system is built with \*\*modular components\*\* responsible for update management, verification, flash operations, and recovery mechanisms.



This bootloader demonstrates core embedded firmware concepts including:



\* Firmware update mechanisms

\* Flash memory management

\* Boot process control

\* Data integrity verification

\* Secure firmware validation

\* System recovery and watchdog handling



The project is implemented using \*\*STM32CubeIDE and STM32 HAL drivers\*\*.



---



\# Target Hardware



Microcontroller:



STM32F407VGT6

ARM Cortex-M4 core

1 MB Flash

192 KB SRAM



The bootloader resides at the \*\*start of flash memory\*\* and manages firmware execution.



---



\# Bootloader Responsibilities



At system reset the bootloader performs the following operations:



1\. Initializes minimal hardware required for boot process

2\. Validates the currently active firmware image

3\. Checks if a firmware update request exists

4\. Receives firmware through communication interface

5\. Writes firmware to the inactive flash slot

6\. Verifies firmware integrity

7\. Updates metadata

8\. Switches active firmware slot

9\. Jumps to application firmware



If the firmware is invalid or corrupted, the system enters \*\*recovery mode\*\*.



---



\# Flash Memory Layout



The flash memory is divided into regions for bootloader and application slots.



Example layout:



| Memory Region      | Address Range    | Purpose              |

| ------------------ | ---------------- | -------------------- |

| Bootloader         | 0x08000000       | Bootloader firmware  |

| Metadata           | After bootloader | Firmware metadata    |

| Application Slot A |                  | Active firmware      |

| Application Slot B |                  | Update slot          |

| Configuration      |                  | System configuration |





```text
Flash Memory (1 MB)
-------------------------------------------------

0x08000000
+---------------------------------------------+
|                Bootloader                   |
|            (Boot code region)               |
|               ~64 KB                        |
+---------------------------------------------+

0x08010000
+---------------------------------------------+
|                Metadata                     |
|       Firmware version / CRC / status       |
+---------------------------------------------+

0x08011000
+---------------------------------------------+
|           Application Slot A                |
|           Active Firmware                   |
|               ~448 KB                       |
+---------------------------------------------+

0x08080000
+---------------------------------------------+
|           Application Slot B                |
|           Update Firmware                   |
|               ~448 KB                       |
+---------------------------------------------+

0x080F0000
+---------------------------------------------+
|         Reserved / Configuration            |
+---------------------------------------------+

End of Flash
```



The \*\*bootloader never overwrites itself\*\*, ensuring safe firmware upgrades.



---



\# Firmware Update Flow



The firmware update process is controlled by the \*\*Update Manager module\*\*.



Update steps:



1\. Host sends firmware image

2\. Bootloader receives data in chunks

3\. Data is written to inactive slot

4\. CRC verification is performed

5\. Cryptographic verification is performed

6\. Metadata is updated

7\. Slot activation occurs

8\. Bootloader jumps to new firmware



This design allows \*\*atomic firmware upgrades\*\* and protects against corrupted updates.



---



\# Boot Process



System reset triggers the bootloader.



Boot sequence:



```

Reset

&nbsp;  │

&nbsp;  ▼

Bootloader starts

&nbsp;  │

&nbsp;  ├─ Check update request

&nbsp;  │

&nbsp;  ├─ Validate firmware image

&nbsp;  │

&nbsp;  ├─ Enter recovery mode (if invalid)

&nbsp;  │

&nbsp;  ▼

Jump to application firmware

```



The bootloader transfers execution by:



1\. Setting the \*\*Main Stack Pointer (MSP)\*\*

2\. Jumping to the \*\*application reset handler\*\*



---



\# Project Architecture

The core bootloader files are located in the `STM32CubeIDE/bootloader/` directory, specifically within `Src/` and `Inc/`. The main bootloader execution starts in `bootloader.c`.

The project is organized into modular components:



```

STM32CubeIDE\\bootloader

│

├── bootloader

│   Core bootloader logic

│

├── update\_manager

│   Coordinates firmware updates

│

├── uart\_update

│   Firmware transfer via UART

│

├── can\_update

│   Firmware transfer via CAN

│

├── ota\_update

│   OTA update framework

│

├── flash\_if

│   Flash memory interface

│

├── firmware\_image

│   Image parsing and handling

│

├── crc\_verify

│   CRC integrity verification

│

├── crypto\_verify

│   Cryptographic signature validation

│

├── metadata

│   Firmware metadata management

│

├── recovery\_mode

│   System recovery mechanisms

│

└── watchdog\_manager

&nbsp;   Watchdog control during update

```



This modular architecture makes the bootloader \*\*scalable and maintainable\*\*.



---



\# Key Modules



\## Bootloader Core



Handles system startup and application jump.



Responsibilities:



\* Boot decision logic

\* Hardware initialization

\* Application validation

\* Execution transfer



---



\## Update Manager



Central controller for firmware update process.



Responsibilities:



\* Receive firmware

\* Write firmware to flash

\* Trigger verification

\* Update firmware metadata

\* Activate firmware slot



---



\## UART Update



Handles firmware transfer over UART.



Responsibilities:



\* Packet reception

\* Data buffering

\* Error handling



---



\## CAN Update



Allows firmware updates through CAN bus for distributed embedded systems.



---



\## OTA Update



Framework for wireless firmware updates through external communication modules.



---



\## Flash Interface



Provides abstraction for flash operations.



Functions include:



\* Flash erase

\* Flash programming

\* Flash read operations



---



\## Firmware Image Manager



Handles firmware structure including:



\* Image header parsing

\* Image size validation

\* Image metadata extraction



---



\## CRC Verification



Ensures firmware integrity.



Process:



```

Firmware received

&nbsp;     │

&nbsp;     ▼

Compute CRC

&nbsp;     │

&nbsp;     ▼

Compare with expected CRC

```



If mismatch occurs the update is rejected.



---



\## Cryptographic Verification



Uses \*\*mbedTLS\*\* to verify firmware authenticity.



Supports:



\* Digital signature verification

\* Secure firmware validation



This prevents unauthorized firmware execution.



---



\## Metadata Manager



Maintains firmware information such as:



\* Active slot

\* Firmware version

\* Firmware size

\* CRC checksum

\* Update state



Metadata enables safe boot decisions.



---



\## Recovery Mode



If firmware validation fails:



\* Bootloader prevents application execution

\* System enters recovery mode

\* New firmware can be uploaded



---



\## Watchdog Manager



Prevents system reset during long firmware updates by refreshing watchdog timers.



---



\# Security Features



The bootloader implements several security mechanisms:



\* CRC validation

\* Cryptographic signature verification

\* Firmware authenticity checking

\* Protected bootloader region



These features protect the system from corrupted or malicious firmware.



---



\# Development Environment



Development tools:



STM32CubeIDE

STM32 HAL Drivers

mbedTLS

Git



---



\# Building the Project



Steps to build:



1\. Open STM32CubeIDE

2\. Import project

3\. Build project

4\. Flash bootloader to MCU



---



\# Firmware Upload



Firmware images can be uploaded using:



\* UART interface

\* CAN interface ( Under Development )

\* OTA interface ( Under Development )



The host sends the firmware image which is then written to the inactive flash slot.



---



\# Future Improvements



Planned enhancements:



* AES encrypted firmware updates

* Secure boot with hardware root of trust

* OTA update server integration

* Version management system



---



\# Author



Yashwanth M



Embedded Systems and Firmware Development



---



\# License



This project is provided for educational and research purposes.



