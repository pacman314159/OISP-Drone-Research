#pragma once
#include "platforms/hal/hal_spi.h"
#include <SPI.h>

class ESP32_SPI : public HAL_SPI {
public:
  explicit ESP32_SPI(SPIClass& spi_instance);

  bool init(int mosi_pin, int miso_pin, int sck_pin, uint32_t default_freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) override;
  uint8_t transfer_byte(int cs_pin, uint8_t data) override;
  bool transfer(int cs_pin, const uint8_t* tx_data, uint8_t* rx_data, size_t len, uint32_t freq_hz = SPI_BUS_DEFAULT_FREQ_HZ) override;
  void* get_native_handle() override { return &_spi; }

private:
  SPIClass& _spi;
  int _mosi_pin{-1};
  int _miso_pin{-1};
  int _sck_pin{-1};
  bool _initialized{false};
};
