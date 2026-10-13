#pragma once
#include <cstdint>
#include <cstddef>
#include "config.h"

class HAL_SPI {
public:
  virtual ~HAL_SPI() = default;
  virtual bool init(int mosi_pin, int miso_pin, int sck_pin, uint32_t default_freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) = 0;
  virtual uint8_t transfer_byte(int cs_pin, uint8_t data) = 0;
  virtual bool transfer(int cs_pin, const uint8_t* tx_data, uint8_t* rx_data, size_t len, uint32_t freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) = 0;
  virtual bool mount_fatfs(int cs_pin, int cd_pin = -1, const char* mount_point = SD_MOUNT_POINT, uint32_t freq_hz = SD_SPI_FREQ_HZ) = 0;
  virtual void unmount_fatfs(const char* mount_point = SD_MOUNT_POINT) = 0;
  virtual bool is_fatfs_mounted() = 0;
  virtual uint64_t get_total_bytes() = 0;
  virtual uint64_t get_free_bytes() = 0;
  virtual bool is_card_inserted(int cd_pin) = 0;
};

HAL_SPI& get_spi_bus();
