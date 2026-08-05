# STM32F407 Custom Bootloader

A production-grade, dual-slot bootloader for the **STM32F407VGTx** microcontroller with ECDSA firmware authentication, CRC32 integrity checking, anti-rollback protection, IWDG watchdog management, and UART-based firmware update support.

---

## Table of Contents

1. [Overview](#overview)
2. [Target Hardware](#target-hardware)
3. [Flash Memory Layout](#flash-memory-layout)
4. [Architecture](#architecture)
5. [Boot State Machine](#boot-state-machine)
6. [Firmware Image Format](#firmware-image-format)
7. [Metadata Block](#metadata-block)
8. [Module Reference](#module-reference)
9. [UART Update Protocol](#uart-update-protocol)
10. [Security Model](#security-model)
11. [Watchdog Strategy](#watchdog-strategy)
12. [Recovery Mechanisms](#recovery-mechanisms)
13. [Building & Flashing](#building--flashing)
14. [Sending a Firmware Update](#sending-a-firmware-update)
15. [Future Work](#future-work)

---

## Overview

This bootloader sits permanently in the first 64 KB of STM32F407 flash (Sectors 0–3). On every power-up or reset it decides which firmware image to run, verifies it is authentic and uncorrupted, and then transfers execution to it. If the active image is invalid or missing, the bootloader automatically rolls back to the last known-good slot or enters firmware recovery mode over UART.

**Key design goals:**

| Goal | Implementation |
|------|---------------|
| A/B dual-slot update | Two independent flash regions; only one is ever active at a time |
| Atomic activation | New firmware is written and fully verified *before* the active slot pointer is updated |
| Anti-rollback | Firmware version in image header is checked against installed version — downgrades rejected |
| Firmware authenticity | ECDSA P-256 signature over SHA-256 hash using embedded public key via mbedTLS |
| Integrity | CRC32 (IEEE 802.3 polynomial) calculated directly from flash before every boot |
| Crash-loop detection | Boot-attempt counter decremented before every jump; rollback triggered after 3 failed boots |
| Recovery | Physical GPIO pin or UART trigger byte enters forced-update mode |

---

## Target Hardware

| Property | Value |
|----------|-------|
| MCU | STM32F407VGTx |
| Core | ARM Cortex-M4 with FPU |
| Flash | 1 MB (12 sectors) |
| SRAM | 128 KB (+ 64 KB CCM) |
| Toolchain | STM32CubeIDE (GCC ARM) |
| HAL | STM32 HAL v1.x |
| Crypto | mbedTLS 2.x |

**Pin assignments used by the bootloader:**

| Signal | Pin | Direction | Description |
|--------|-----|-----------|-------------|
| UART1 TX | PA9 | Output | Firmware upload & ACK/NACK responses |
| UART1 RX | PA10 | Input | Incoming packet data |
| Recovery | PA0 | Input (PULL-UP) | Pull LOW to force update mode |

UART1 is configured at **115200 baud, 8N1, no hardware flow control**.

---

## Flash Memory Layout

The STM32F407VGTx has 12 flash sectors. The bootloader partitions them as follows:

```
Flash (1 MB total)
────────────────────────────────────────────────────────────
 Address       Size     Sectors   Region
────────────────────────────────────────────────────────────
 0x08000000    64 KB    S0–S3     Bootloader (this code)
 0x08010000   448 KB    S4–S7     Slot A  (active firmware)
 0x08080000   384 KB    S8–S10    Slot B  (update firmware)
 0x080E0000   128 KB    S11       Metadata block
────────────────────────────────────────────────────────────
```

**Sector boundary details (critical for erase operations):**

```
Sector 0  : 0x08000000 – 0x08003FFF   16 KB ─┐
Sector 1  : 0x08004000 – 0x08007FFF   16 KB  │ Bootloader
Sector 2  : 0x08008000 – 0x0800BFFF   16 KB  │
Sector 3  : 0x0800C000 – 0x0800FFFF   16 KB ─┘
Sector 4  : 0x08010000 – 0x0801FFFF   64 KB ─┐
Sector 5  : 0x08020000 – 0x0803FFFF  128 KB  │ Slot A (448 KB)
Sector 6  : 0x08040000 – 0x0805FFFF  128 KB  │
Sector 7  : 0x08060000 – 0x0807FFFF  128 KB ─┘
Sector 8  : 0x08080000 – 0x0809FFFF  128 KB ─┐
Sector 9  : 0x080A0000 – 0x080BFFFF  128 KB  │ Slot B (384 KB)
Sector 10 : 0x080C0000 – 0x080DFFFF  128 KB ─┘
Sector 11 : 0x080E0000 – 0x080FFFFF  128 KB ── Metadata
```

> **Why these sizes?** Slots must align to sector boundaries — flash sectors cannot be partially erased. Slot A is 448 KB (one 64 KB + three 128 KB sectors). Slot B is 384 KB (three 128 KB sectors). Metadata occupies Sector 11 entirely so it can be independently erased without touching either slot.

---

## Architecture

```
Core/Src/main.c
    │
    ├── HAL_Init()
    ├── SystemClock_Config()
    ├── MX_GPIO_Init()          ← recovery pin (PA0, PULL-UP)
    ├── MX_MBEDTLS_Init()       ← crypto library init
    ├── MX_IWDG_Init()          ← 5-second hardware watchdog
    ├── MX_USART1_UART_Init()   ← 115200 baud
    │
    ├── Bootloader_Init()       ─── watchdog_manager, recovery_mode, metadata
    └── Bootloader_Run()        ─── full boot decision logic
            │
            ├── [UART window]   wait BOOT_TIMEOUT_MS for CMD_START_UPDATE
            ├── [GPIO pin]      PA0 low → forced update
            │
            ├── [Normal boot]
            │       ├── Metadata_GetActiveSlot()
            │       ├── boot_attempts == 0 → Rollback + Reset
            │       ├── Firmware_Validate(slot_addr)
            │       │       ├── Check magic (0xDEADBEEF)
            │       │       ├── CRC_Verify()
            │       │       └── Crypto_VerifyFirmware() [ECDSA P-256]
            │       └── Bootloader_JumpToApplication()
            │
            ├── [Rollback]
            │       └── try other slot → Jump
            │
            └── [Force update]
                    └── UpdateManager full cycle → Reset
```

### Module dependency map

```
bootloader.c
├── update_manager.c ──► flash_if.c
│                   ──► firmware_image.c ──► crc_verify.c
│                                       ──► crypto_verify.c (mbedTLS)
│                   ──► metadata.c ──► flash_if.c
│                   ──► uart_update.c
│                   ──► watchdog_manager.c
├── recovery_mode.c
├── metadata.c
└── watchdog_manager.c
```

---

## Boot State Machine

```
                         Reset
                           │
                           ▼
                    ┌─────────────┐
                    │ Init (HW,   │
                    │  WDG, GPIO, │
                    │  Metadata)  │
                    └──────┬──────┘
                           │
               ┌───────────▼────────────┐
               │  UART trigger window?  │  ◄── waits BOOT_TIMEOUT_MS (3 s)
               └───────────┬────────────┘
                    YES ───┘    NO
                    │            │
                    │     ┌──────▼──────┐
                    │     │  PA0 LOW?   │
                    │     └──────┬──────┘
                    │      YES ──┘   NO
                    │      │          │
                    ▼      ▼          ▼
                ┌──────────────┐   ┌────────────────────────┐
                │ RunRecovery  │   │ boot_attempts == 0?    │
                │ Update()     │   └────────┬───────────────┘
                │              │      YES ──┘     NO
                │  1. Init     │      │             │
                │  2. Erase    │      ▼             ▼
                │  3. Receive  │  Rollback    ┌───────────────┐
                │  4. Finalize │  + Reset     │ Validate slot │
                │  5. Activate │              └──────┬────────┘
                │  6. Reset    │               VALID │  INVALID
                └──────────────┘                    │         │
                                                    │    ┌────▼──────────┐
                                                    │    │ Try other slot│
                                                    │    └────┬──────────┘
                                                    │   VALID │  INVALID
                                                    │         │         │
                                                    ▼         ▼         ▼
                                               Decrement  Switch   RunRecovery
                                               attempts   + Jump    Update()
                                               + Jump
```

**Boot attempt counter logic:**
- Counter starts at `MAX_BOOT_ATTEMPTS` (3) after a successful update
- Decremented by 1 *before* every jump to application
- If counter hits 0 on entry → the previous boot must have crashed → trigger rollback
- Application firmware is expected to confirm a successful boot (e.g., communicate back to host tool) and the host tool can reset the counter via a fresh update

---

## Firmware Image Format

Every firmware image starts with an 80-byte header at the base of the slot:

```c
typedef struct
{
    uint32_t magic_number;        // Must be 0xDEADBEEF
    uint32_t version;             // Monotonically increasing; anti-rollback enforced
    uint32_t image_size;          // Size of firmware payload (bytes after header)
    uint32_t crc;                 // CRC32 of the firmware payload
    uint8_t  signature[64];       // ECDSA P-256 signature (r || s, 32 bytes each)
} FirmwareHeader_t;               // Total: 80 bytes
```

The **application code starts immediately after the header** at `slot_addr + sizeof(FirmwareHeader_t)` (i.e., `slot_addr + 80`). The application's vector table must be at this address, and the application's linker script must set `FLASH` origin to this address too.

**Validation sequence applied by `Firmware_Validate()`:**

```
1.  Read FirmwareHeader_t* from slot_addr (zero-copy pointer, no RAM copy)
2.  Check magic_number == 0xDEADBEEF           ← reject blank/wrong flash
3.  CRC32 over [slot_addr+80 .. slot_addr+80+image_size]
    compare with header->crc                   ← reject corruption
4.  SHA-256 over same range
    ECDSA P-256 verify(hash, header->signature, PUBLIC_KEY_X, PUBLIC_KEY_Y)
                                               ← reject unsigned/tampered images
```

All three checks must pass. Failure at any step returns `FW_STATUS_INVALID` immediately.

---

## Metadata Block

The metadata is stored at the start of Sector 11 (`0x080E0000`) and holds the bootloader's persistent state across resets.

### In-flash layout

```
Offset   Size   Field           Description
──────   ────   ─────           ───────────
+0x00    4 B    magic           0xB007AB1E — validity sentinel
+0x04    4 B    active_slot     0 = Slot A, 1 = Slot B
+0x08    4 B    boot_attempts   Remaining boots before rollback (max 3)
+0x0C    4 B    slotA.crc       CRC32 stored at last Slot A activation
+0x10    4 B    slotA.size      Image size stored at last Slot A activation
+0x14    4 B    slotA.version   Version stored at last Slot A activation
+0x18    4 B    slotB.crc       Same for Slot B
+0x1C    4 B    slotB.size
+0x20    4 B    slotB.version
──────   ────
Total   36 B
```

### Initialization logic (`Metadata_Init`)

On every boot, `Metadata_Init()` reads the block and validates:
- `magic == 0xB007AB1E` — detects blank flash OR partial/corrupted writes
- `active_slot` in range {0, 1}

If either check fails, defaults are written: `active_slot=SLOT_A`, `boot_attempts=3`, all slot fields zeroed, and `magic` is set. This handles first-boot on a blank chip and torn writes caused by power loss mid-update.

### Write safety

`Metadata_Write()` always erases Sector 11 entirely before writing — there is no partial-sector update. The write sequence is:

```
1. FLASH_Erase(METADATA_ADDR, sizeof(FirmwareMetadata_t))
   └── checks HAL_FLASHEx_Erase return + sectorError == 0xFFFFFFFF
2. FLASH_Write(METADATA_ADDR, &meta, sizeof(FirmwareMetadata_t))
   └── writes 32-bit words, read-back verifies each word
```

---

## Module Reference

### `bootloader.c / bootloader.h`

| Function | Description |
|----------|-------------|
| `Bootloader_Init()` | Calls `HAL_Init()`, `Watchdog_Init()`, `Recovery_Init()`, `Metadata_Init()` |
| `Bootloader_Run()` | Full boot decision state machine (see flowchart above) |
| `Bootloader_JumpToApplication(addr)` | Disables IRQs, de-inits HAL, relocates VTOR, sets MSP, jumps |
| `Bootloader_ValidateSlot(addr)` | Thin wrapper around `Firmware_Validate()` returning `uint8_t` |
| `Bootloader_Rollback()` | Validates fallback slot before switching; does nothing if fallback also invalid |

**Jump sequence in `Bootloader_JumpToApplication()`:**
```c
__disable_irq();
HAL_DeInit();
SysTick->CTRL = 0; SysTick->LOAD = 0; SysTick->VAL = 0;
SCB->VTOR = app_addr;               // relocate vector table
__set_MSP(*(uint32_t*)app_addr);    // set application stack pointer
((void(*)(void))(*(uint32_t*)(app_addr + 4)))();  // jump to reset handler
```

---

### `flash_if.c / flash_if.h`

Thin HAL wrapper. All functions return `FLASH_OK (0)` or `FLASH_ERROR (1)`.

| Function | Description |
|----------|-------------|
| `FLASH_Erase(addr, size)` | Erases all sectors covering `[addr, addr+size)`. Returns error if any sector fails. |
| `FLASH_Write(addr, data, len)` | Writes in 32-bit words with read-back verify per word. |
| `FLASH_ReadWord(addr)` | Returns `*(volatile uint32_t*)addr`. |

`GetSector()` maps the full 12-sector layout of the STM32F407VGTx:

| Sector | Range | Size |
|--------|-------|------|
| S0 | 0x08000000–0x08003FFF | 16 KB |
| S1 | 0x08004000–0x08007FFF | 16 KB |
| S2 | 0x08008000–0x0800BFFF | 16 KB |
| S3 | 0x0800C000–0x0800FFFF | 16 KB |
| S4 | 0x08010000–0x0801FFFF | 64 KB |
| S5–S11 | 0x08020000–0x080FFFFF | 128 KB each |

---

### `firmware_image.c / firmware_image.h`

| Function | Returns | Description |
|----------|---------|-------------|
| `Firmware_ReadHeader(slot_addr)` | `FirmwareHeader_t*` | Zero-copy pointer directly into flash |
| `Firmware_CheckMagic(header)` | `uint8_t` | 1 if `magic == 0xDEADBEEF` |
| `Firmware_CheckCRC(slot_addr, header)` | `uint8_t` | CRC32 of payload vs `header->crc` |
| `Firmware_CheckSignature(slot_addr, header)` | `uint8_t` | ECDSA verify of payload hash vs `header->signature` |
| `Firmware_Validate(slot_addr)` | `FirmwareStatus_t` | Runs all three checks in order; first failure exits early |

---

### `crc_verify.c / crc_verify.h`

Software CRC32 using the IEEE 802.3 (Ethernet, ZIP) reflected polynomial `0xEDB88320`:

```c
// For each byte:
crc ^= byte;
for 8 bits:
    if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
    else         crc >>= 1;
return ~crc;    // final XOR
```

| Function | Description |
|----------|-------------|
| `CRC_Calculate(data, length)` | CRC32 over RAM buffer |
| `CRC_CalculateFlash(start_addr, size)` | CRC32 directly from flash (pointer cast, no copy) |
| `CRC_Verify(start_addr, size, expected_crc)` | Calculates and compares; returns 1 if match |

---

### `crypto_verify.c / crypto_verify.h`

Implements ECDSA P-256 signature verification using **mbedTLS**.

**Pipeline:**

```
firmware payload (in flash)
        │
        ▼
SHA-256 hash (32 bytes)         ← Crypto_SHA256()
        │
        ▼
ECDSA P-256 verify              ← mbedtls_ecdsa_verify()
    using embedded public key
    (PUBLIC_KEY_X[32], PUBLIC_KEY_Y[32])
        │
        ▼
  0 = valid signature
```

The public key is hard-coded in `crypto_verify.c`. The corresponding private key is kept on the build server and never shipped in firmware. All released firmware images must be signed with this private key before distribution.

> ⚠️ **Before production deployment:** Replace the placeholder `PUBLIC_KEY_X` / `PUBLIC_KEY_Y` arrays in `crypto_verify.c` with your real generated key pair.

---

### `metadata.c / metadata.h`

| Function | Description |
|----------|-------------|
| `Metadata_Init()` | Validates magic + slot; re-initializes if corrupted |
| `Metadata_Read()` | Returns a copy of `FirmwareMetadata_t` from flash |
| `Metadata_Write(meta*)` | Erase Sector 11 then write full struct |
| `Metadata_GetActiveSlot()` | Returns `SLOT_A` or `SLOT_B` |
| `Metadata_SetActiveSlot(slot)` | Read-modify-write full metadata block |
| `Metadata_GetBootAttempts()` | Returns remaining boot attempts |
| `Metadata_DecrementBootAttempts()` | Subtracts 1 (floor at 0) |
| `Metadata_ResetBootAttempts()` | Sets to `MAX_BOOT_ATTEMPTS` (3) |
| `Metadata_GetSlotA() / GetSlotB()` | Returns `FirmwareInfo_t` for that slot |
| `Metadata_SetSlotA(crc, size, ver)` | Updates slot A info |
| `Metadata_SetSlotB(crc, size, ver)` | Updates slot B info |

---

### `update_manager.c / update_manager.h`

Orchestrates the full firmware update cycle.

| Function | Returns | Description |
|----------|---------|-------------|
| `UpdateManager_Init()` | — | Determines inactive slot and its size; sets `write_address` |
| `UpdateManager_Start()` | `UpdateStatus_t` | Erases inactive slot; returns `UPDATE_ERROR` on erase failure |
| `UpdateManager_Receive()` | `UpdateStatus_t` | Packet receive loop (see UART protocol); bounds-checks every write |
| `UpdateManager_Finalize()` | `UpdateStatus_t` | Validates image; enforces anti-rollback |
| `UpdateManager_Activate()` | — | Updates metadata (active slot, version, CRC) atomically |

**`UpdateStatus_t` values:**

| Value | Meaning |
|-------|---------|
| `UPDATE_OK` | Success |
| `UPDATE_ERROR` | Flash erase/write failure or bounds overflow |
| `UPDATE_TIMEOUT` | UART timeout on receive |
| `UPDATE_INVALID_PACKET` | 3 consecutive malformed frames |
| `UPDATE_INVALID_FIRMWARE` | Magic / CRC / signature validation failed |
| `UPDATE_ROLLBACK_DETECTED` | Incoming version < installed version |

---

### `uart_update.c / uart_update.h`

| Function | Description |
|----------|-------------|
| `UART_Update_Init()` | Re-initializes USART1 (115200 baud) |
| `UART_Update_Send(data, len)` | HAL TX with 1-second timeout |
| `UART_Update_Receive(data, len)` | HAL RX with **5-second** timeout per chunk |
| `UART_Update_ReceiveTimeout(data, len, timeout)` | HAL RX with caller-specified timeout |
| `UART_Send_ACK()` | Sends `0x79` |
| `UART_Send_NACK()` | Sends `0x1F` |

---

### `recovery_mode.c / recovery_mode.h`

| Function | Description |
|----------|-------------|
| `Recovery_Init()` | Configures PA0 as digital input with internal pull-up |
| `Recovery_IsRequested()` | Returns 1 if PA0 reads LOW (active-low button / jumper) |

---

### `watchdog_manager.c / watchdog_manager.h`

The IWDG is initialized by `MX_IWDG_Init()` in `main.c` with:
- Prescaler: `/64` → tick period ≈ 1.6 ms (from LSI ~40 kHz)
- Reload: `3125` → timeout = 3125 × 1.6 ms ≈ **5 seconds**

`watchdog_manager.c` uses the same `hiwdg` handle (via `extern` from `main.h`) so there is a single hardware owner.

| Function | Description |
|----------|-------------|
| `Watchdog_Init()` | No-op; IWDG already started by `MX_IWDG_Init()` |
| `Watchdog_Refresh()` | `HAL_IWDG_Refresh()` — called in the firmware receive loop |
| `Watchdog_Disable()` | No-op; IWDG cannot be stopped once started on STM32 |

> The receive loop calls `Watchdog_Refresh()` after every successfully written data packet. A 5-second stall (host crash, cable disconnect) will trigger a reset.

---

## UART Update Protocol

The update manager uses a simple binary packet protocol over UART1.

### Packet structure

```
┌────────┬─────┬──────────┬──────────────────────┬─────────┐
│ start  │ cmd │  length  │         data         │   crc   │
│ 1 byte │1 B  │  2 bytes │    0–256 bytes       │ 4 bytes │
└────────┴─────┴──────────┴──────────────────────┴─────────┘
```

| Field | Value | Description |
|-------|-------|-------------|
| `start` | `0xAA` | Frame synchronization byte |
| `cmd` | see below | Command identifier |
| `length` | 0–256 | Number of valid bytes in `data[]` |
| `data` | — | Payload (firmware chunk for `CMD_FIRMWARE_DATA`) |
| `crc` | CRC32 | CRC32 of `data[0..length-1]` only |

### Command codes

| Command | Value | Direction | Description |
|---------|-------|-----------|-------------|
| `CMD_START_UPDATE` | `0x01` | Host → MCU | Begin update session |
| `CMD_FIRMWARE_DATA` | `0x02` | Host → MCU | Firmware chunk |
| `CMD_END_UPDATE` | `0x03` | Host → MCU | Last chunk sent |
| `CMD_ACK` | `0x79` | MCU → Host | Packet accepted |
| `CMD_NACK` | `0x1F` | MCU → Host | Packet rejected (retry) |

### Transfer sequence

```
Host                              MCU (bootloader)
────                              ────────────────
UART trigger byte (0x01)     ──►  Enter update mode
or PA0 pulled LOW            ──►  Enter update mode

CMD_START_UPDATE packet      ──►  Erase inactive slot
                             ◄──  ACK (0x79)

CMD_FIRMWARE_DATA [chunk 1]  ──►  Verify packet CRC
                             ◄──  ACK  (CRC ok, write to flash)
                             ◄──  NACK (CRC bad, retry same chunk)

... repeat for all chunks ...

CMD_END_UPDATE               ──►  Run Firmware_Validate()
                                  Check anti-rollback version
                                  UpdateManager_Activate()
                             ◄──  ACK
                                  HAL_NVIC_SystemReset()
```

### Error handling

- Malformed frame (wrong start byte, length > 256, or UART timeout): NACK + retry up to **3 consecutive times** before aborting
- CRC mismatch on data: NACK + `continue` (sender retransmits same packet)
- Write would overflow slot boundary: NACK + `UPDATE_ERROR` (abort entire transfer)
- Firmware validation fails after `CMD_END_UPDATE`: `UPDATE_INVALID_FIRMWARE` (slot remains erased; host must retry)
- Anti-rollback violation: `UPDATE_ROLLBACK_DETECTED`

---

## Security Model

### Threat model

| Threat | Mitigation |
|--------|-----------|
| Corrupted firmware (bit errors, truncated transfer) | CRC32 integrity check |
| Tampered / unsigned firmware | ECDSA P-256 signature verification |
| Downgrade attack (install older, vulnerable firmware) | Anti-rollback: `new.version >= active.version` enforced |
| Boot-loop crash exploit | Boot-attempt counter (max 3); automatic rollback to last-good slot |
| Partial metadata write (power loss during update) | Magic sentinel `0xB007AB1E`; if absent, metadata is re-initialized |
| Unauthorized recovery mode trigger | Physical access required (PA0 pull-down or UART byte) |

### Signature generation (build server side)

```bash
# Generate ECDSA P-256 key pair (one-time setup)
openssl ecparam -name prime256v1 -genkey -noout -out private.pem
openssl ec -in private.pem -pubout -out public.pem

# Hash the firmware payload
openssl dgst -sha256 -binary firmware_payload.bin > firmware.hash

# Sign the hash
openssl dgst -sha256 -sign private.pem firmware_payload.bin > signature.der

# Convert DER to raw r||s (64 bytes) for embedding in the header
```

The 64-byte raw signature is placed in `FirmwareHeader_t.signature` before the image is transferred.

### Public key rotation

To rotate the embedded public key:
1. Generate a new key pair on the build server
2. Extract `PUBLIC_KEY_X[32]` and `PUBLIC_KEY_Y[32]` from the new public key
3. Update `crypto_verify.c` with the new coordinates
4. Re-sign all future firmware releases with the new private key
5. Flash the updated bootloader (requires programmer access — UART cannot update the bootloader itself)

---

## Watchdog Strategy

The IWDG runs continuously from the moment `MX_IWDG_Init()` is called and **cannot be stopped**. The bootloader must keep it fed during long operations.

| Phase | Watchdog fed by |
|-------|----------------|
| Boot decision (fast) | Completes well within 5-second window |
| Flash erase | Each sector erase takes < 2 s on STM32F407; single erase call fits in window |
| Firmware receive loop | `Watchdog_Refresh()` called after each data packet write |
| Crypto verification | SHA-256 + ECDSA over up to 448 KB; mbedTLS software implementation; may approach timeout on largest images — consider chunking if issues arise |

Application firmware must also refresh the IWDG. If the application crashes or stops feeding the watchdog, a reset will occur after 5 seconds. On next boot `Metadata_DecrementBootAttempts()` will reduce the counter, and after 3 such crashes `Bootloader_Rollback()` will switch to the other slot.

---

## Recovery Mechanisms

### 1. UART trigger window (software)

At the start of `Bootloader_Run()`, the bootloader listens on UART1 for **3 seconds** (`BOOT_TIMEOUT_MS`). If `CMD_START_UPDATE` (0x01) is received, it immediately enters update mode. This allows host tools to trigger OTA updates without physically interacting with the board.

### 2. Recovery GPIO pin (hardware)

If PA0 is pulled LOW at boot (physical jumper or button), `Recovery_IsRequested()` returns 1 and the bootloader skips normal boot entirely, going straight into firmware receive mode.

### 3. Boot-attempt counter rollback (automatic)

The counter starts at 3 after every successful update. Before each jump to the application, it is decremented and written to flash. If the bootloader sees counter == 0 on entry (meaning 3 consecutive boots ended in crash/reset without the counter being restored), it calls `Bootloader_Rollback()`, resets the counter, and resets the MCU. On next boot the other slot will be tried.

### 4. Both slots invalid → forced update

If both Slot A and Slot B fail `Firmware_Validate()`, the bootloader enters `RunRecoveryUpdate()` and waits indefinitely for a UART firmware upload. There is no timeout in this state — the device will not boot until valid firmware is received.

---

## Building & Flashing

### Prerequisites

- STM32CubeIDE (1.13.x or later)
- ST-LINK programmer or J-Link

### Steps

1. Clone the repository
2. Open STM32CubeIDE → **File → Import → Existing Projects into Workspace**
3. Select the repository root
4. Choose the **Debug** or **Release** build configuration
5. Build: **Project → Build Project** (`Ctrl+B`)
6. Flash: **Run → Debug** or **Run → Run** with your programmer connected

### Linker script

The bootloader uses `STM32F407VGTX_FLASH.ld`:

```
FLASH    (rx) : ORIGIN = 0x08000000, LENGTH = 64K   ← bootloader
SLOT_A   (rx) : ORIGIN = 0x08010000, LENGTH = 448K  ← for reference only
SLOT_B   (rx) : ORIGIN = 0x08080000, LENGTH = 384K  ← for reference only
METADATA (r)  : ORIGIN = 0x080E0000, LENGTH = 128K  ← for reference only
RAM     (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
CCMRAM  (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
```

Only `FLASH` and `RAM` are used for bootloader code placement.

### Application linker script requirements

The application firmware must start its `FLASH` region at `0x08010000 + 80` (header size) for Slot A, or adjust accordingly for Slot B. It must also set `SCB->VTOR` to its own flash base on startup (the bootloader already does this via `SCB->VTOR = app_addr`).

---

## Sending a Firmware Update

A minimal Python host-side script outline:

```python
import serial, struct, zlib

UART_START_BYTE   = 0xAA
CMD_START_UPDATE  = 0x01
CMD_FIRMWARE_DATA = 0x02
CMD_END_UPDATE    = 0x03
CMD_ACK           = 0x79
CMD_NACK          = 0x1F
CHUNK_SIZE        = 256

def build_packet(cmd, data: bytes) -> bytes:
    length = len(data)
    crc    = zlib.crc32(data) & 0xFFFFFFFF
    return bytes([UART_START_BYTE, cmd]) + struct.pack('<H', length) + data + struct.pack('<I', crc)

def send_packet(ser, cmd, data=b''):
    pkt = build_packet(cmd, data)
    for attempt in range(3):
        ser.write(pkt)
        resp = ser.read(1)
        if resp and resp[0] == CMD_ACK:
            return True
        # NACK → retry
    return False   # 3 failures

with serial.Serial('COM3', 115200, timeout=6) as ser:
    # Trigger: send CMD_START_UPDATE byte raw first (UART window)
    ser.write(bytes([CMD_START_UPDATE]))

    # Start session
    assert send_packet(ser, CMD_START_UPDATE)

    # Send firmware in chunks
    with open('firmware.bin', 'rb') as f:
        # Skip/prepend FirmwareHeader_t (80 bytes, pre-built)
        image = f.read()
    for i in range(0, len(image), CHUNK_SIZE):
        chunk = image[i:i+CHUNK_SIZE]
        assert send_packet(ser, CMD_FIRMWARE_DATA, chunk), f"chunk {i} failed"

    # End session
    assert send_packet(ser, CMD_END_UPDATE)
    print("Update complete. Board will reset.")
```

---

## Future Work

| Feature | Status | Notes |
|---------|--------|-------|
| CAN bus update | 🔧 Stub only | `can_update.c` / `can_update.h` exist; implementation pending |
| OTA (Wi-Fi/BLE) update | 🔧 Stub only | `ota_update.c` / `ota_update.h` exist; requires external module |
| AES-256 encrypted firmware | 📋 Planned | Decrypt in RAM before writing; key management TBD |
| Secure boot (RDP level 2) | 📋 Planned | Lock bootloader flash region against read-out |
| Application boot confirmation | 📋 Planned | App sends "boot OK" packet; bootloader resets counter on receipt |
| UART Xmodem/YMODEM protocol | 📋 Planned | Compatibility with standard terminal tools |
| Version management server | 📋 Planned | Central store for firmware versions and rollback history |

---

## Project Structure

```
Custom_bootloader/
├── Core/
│   ├── Inc/
│   │   ├── main.h                    HAL handles, pin definitions
│   │   ├── stm32f4xx_hal_conf.h      HAL module enable/disable
│   │   └── stm32f4xx_it.h            Interrupt handler declarations
│   └── Src/
│       ├── main.c                    Entry point, peripheral init, calls Bootloader_Init/Run
│       ├── stm32f4xx_hal_msp.c       MSP init/deinit callbacks
│       ├── stm32f4xx_it.c            Interrupt handlers (SysTick, etc.)
│       └── system_stm32f4xx.c        SystemInit, clock setup
├── Drivers/
│   ├── CMSIS/                        ARM CMSIS core headers
│   └── STM32F4xx_HAL_Driver/         ST HAL library source
├── MBEDTLS/
│   └── App/
│       ├── mbedtls.c / mbedtls.h     mbedTLS HAL integration shim
│       └── mbedtls_config.h          mbedTLS compile-time configuration
├── Middlewares/
│   └── Third_Party/                  mbedTLS source tree
└── STM32CubeIDE/
    ├── bootloader/
    │   ├── Inc/
    │   │   ├── bootloader.h          Bootloader API
    │   │   ├── bootloader_config.h   BOOT_TIMEOUT_MS, MAX_BOOT_ATTEMPTS, state enum
    │   │   ├── can_update.h          CAN update API (stub)
    │   │   ├── crc_verify.h          CRC32 API
    │   │   ├── crypto_verify.h       ECDSA / SHA-256 API
    │   │   ├── firmware_image.h      FirmwareHeader_t, validation API
    │   │   ├── flash_if.h            Flash erase/write/read API
    │   │   ├── memory_map.h          All flash addresses and sizes
    │   │   ├── metadata.h            FirmwareMetadata_t, metadata API
    │   │   ├── ota_update.h          OTA update API (stub)
    │   │   ├── recovery_mode.h       Recovery GPIO API
    │   │   ├── uart_update.h         UART packet API
    │   │   ├── update_manager.h      Update orchestration API
    │   │   ├── utils.h               Common includes (HAL, stdint, string)
    │   │   └── watchdog_manager.h    IWDG API
    │   └── Src/
    │       ├── bootloader.c          Boot state machine
    │       ├── can_update.c          CAN update (stub)
    │       ├── crc_verify.c          Software CRC32
    │       ├── crypto_verify.c       mbedTLS SHA-256 + ECDSA
    │       ├── firmware_image.c      Header parsing + validation
    │       ├── flash_if.c            HAL flash wrapper
    │       ├── metadata.c            Metadata read/write/init
    │       ├── ota_update.c          OTA update (stub)
    │       ├── recovery_mode.c       Recovery GPIO
    │       ├── uart_update.c         UART send/receive
    │       ├── update_manager.c      Full update cycle
    │       └── watchdog_manager.c    IWDG refresh
    ├── STM32F407VGTX_FLASH.ld        Linker script (Flash + RAM regions)
    ├── STM32F407VGTX_RAM.ld          RAM-execution linker script
    └── .cproject / .project          STM32CubeIDE project files
```

---

## Author

**Yashwanth M** — Embedded Systems & Firmware Development

---

## License

This project is provided for educational and portfolio purposes.
