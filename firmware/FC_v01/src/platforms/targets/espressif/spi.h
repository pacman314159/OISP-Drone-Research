#pragma once
#include "platforms/hal/hal_spi.h"
#include <SPI.h>
#include <FS.h>
#include <SD.h>

class ESP32_SPI : public HAL_SPI {
public:
  explicit ESP32_SPI(SPIClass& spi_instance);

  bool init(int mosi_pin, int miso_pin, int sck_pin, uint32_t default_freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) override;
  uint8_t transfer_byte(int cs_pin, uint8_t data) override;
  virtual bool transfer(int cs_pin, const uint8_t* tx_data, uint8_t* rx_data, size_t len, uint32_t freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) override;
  bool mount_fatfs(int cs_pin, int cd_pin = -1, const char* mount_point = SD_MOUNT_POINT, uint32_t freq_hz = SD_SPI_FREQ_HZ) override;
  void unmount_fatfs(const char* mount_point = SD_MOUNT_POINT) override;
  bool is_fatfs_mounted() override;
  uint64_t get_total_bytes() override;
  uint64_t get_free_bytes() override;
  bool is_card_inserted(int cd_pin) override;

private:
  SPIClass& _spi;
  int _mosi_pin{-1};
  int _miso_pin{-1};
  int _sck_pin{-1};
  bool _initialized{false};
  bool _mounted{false};
};
