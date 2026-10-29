---
id: "0048"
title: HAL SPI Interface Segregation & SD Card Driver Filesystem Decoupling
author: Drone Research Team / Developer
date: 2026-10-28
time: 01:35:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#hal", "#spi", "#sdcard", "#fatfs", "#refactoring", "#isp"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0045-rcReceiverCommandBlackboxSdCardLoggingArchitecture]]"
---

# Zettel 0048: HAL SPI Interface Segregation & SD Card Driver Filesystem Decoupling

## 1. Metadata & Scope
- **ID**: `0048`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-28
- **Time**: 01:35:00 +07:00
- **Status**: Approved & Implemented Architectural Refactoring
- **Module Context**: Layer 1 HAL SPI (`src/platforms/hal/hal_spi.h`, `src/platforms/targets/espressif/spi.h/.cpp`), Layer 2 Storage Driver (`src/drivers/storage/sd_card_driver.h/.cpp`)

---

## 2. Problem Statement: Interface Segregation Violation in `HAL_SPI`
Under prior revisions, `HAL_SPI` (`src/platforms/hal/hal_spi.h`) combined low-level SPI bus transactions with high-level FATFS volume mounting and SD card presence detection:

1. **Interface Segregation Principle (ISP) Violation**:
   `HAL_SPI` is the abstract hardware abstraction for the SPI physical bus. Coupling it to `mount_fatfs()`, `unmount_fatfs()`, `get_total_bytes()`, and `is_card_inserted()` meant that any potential secondary SPI peripheral (e.g., an SPI IMU or display) shared an interface littered with filesystem methods.
2. **Layering Bleed**:
   Layer 1 platform code (`src/platforms/targets/espressif/spi.h/.cpp`) had to import Arduino `<FS.h>` and `<SD.h>` and track filesystem mount states (`_mounted`), bleeding high-level storage concepts into the physical bus layer.

---

## 3. Core Architectural Decision: Clean Responsibility Split (Solution A)

### 3.1 Pure Layer 1 Bus Interface (`HAL_SPI`)
`HAL_SPI` is strictly scoped to hardware bus configuration and full-duplex transfers:

```cpp
// src/platforms/hal/hal_spi.h (Layer 1)
class HAL_SPI {
public:
  virtual ~HAL_SPI() = default;
  virtual bool init(int mosi_pin, int miso_pin, int sck_pin, uint32_t default_freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) = 0;
  virtual uint8_t transfer_byte(int cs_pin, uint8_t data) = 0;
  virtual bool transfer(int cs_pin, const uint8_t* tx_data, uint8_t* rx_data, size_t len, uint32_t freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) = 0;
  virtual void* get_native_handle() = 0;
};
```

All references to `<FS.h>` and `<SD.h>` are removed from `src/platforms/targets/espressif/spi.h/.cpp`.

### 3.2 High-Level Storage Encapsulation in `SDCardDriver` (Layer 2)
The responsibility for card presence polling (`_cd_pin`), FATFS mounting (`SD.begin`), unmounting (`SD.end`), sector writing, and disk capacity queries is completely encapsulated inside `SDCardDriver` (`src/drivers/storage/sd_card_driver.h/.cpp`):

```cpp
// src/drivers/storage/sd_card_driver.cpp (Layer 2)
bool SDCardDriver::is_card_inserted(){
  if(_cd_pin < 0) return true;
  pinMode(_cd_pin, INPUT_PULLUP);
  return (digitalRead(_cd_pin) == LOW);
}

bool SDCardDriver::init(){
  if(_mounted) return true;
  if(!is_card_inserted()) return false;

  pinMode(_cs_pin, OUTPUT);
  digitalWrite(_cs_pin, HIGH);

  SPIClass* native_spi = static_cast<SPIClass*>(_spi.get_native_handle());
  if(native_spi == nullptr) return false;

  _mounted = SD.begin(static_cast<uint8_t>(_cs_pin), *native_spi, SD_SPI_FREQ_HZ, SD_MOUNT_POINT);
  return _mounted;
}
```

---

## 4. Architectural Summary & Benefits
1. **Purity**: Layer 1 contains zero knowledge of filesystems or FAT volumes.
2. **Cohesion**: All SD card lifecycle operations (insertion check, mount, file create, header write, sector append, flush, unmount) reside together in Layer 2.
3. **Extensibility**: Future SPI devices can share `HAL_SPI` without inheriting FATFS bloat.
