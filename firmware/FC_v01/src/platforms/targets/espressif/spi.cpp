#include "platforms/targets/espressif/spi.h"
#include <Arduino.h>

ESP32_SPI::ESP32_SPI(SPIClass& spi_instance) : _spi(spi_instance){}

bool ESP32_SPI::init(int mosi_pin, int miso_pin, int sck_pin, uint32_t default_freq_hz){
  _mosi_pin = mosi_pin;
  _miso_pin = miso_pin;
  _sck_pin  = sck_pin;

  _spi.begin(_sck_pin, _miso_pin, _mosi_pin, -1);
  _initialized = true;
  return true;
}

uint8_t ESP32_SPI::transfer_byte(int cs_pin, uint8_t data){
  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, LOW);
  _spi.beginTransaction(SPISettings(SPI_BUS_DEFAULT_FREQ_HZ, MSBFIRST, SPI_MODE0));
  uint8_t ret = _spi.transfer(data);
  _spi.endTransaction();
  digitalWrite(cs_pin, HIGH);
  return ret;
}

bool ESP32_SPI::transfer(int cs_pin, const uint8_t* tx_data, uint8_t* rx_data, size_t len, uint32_t freq_hz){
  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, LOW);
  _spi.beginTransaction(SPISettings(freq_hz, MSBFIRST, SPI_MODE0));
  if(tx_data != nullptr && rx_data != nullptr){
    memcpy(rx_data, tx_data, len);
    _spi.transferBytes(rx_data, rx_data, len);
  }else if(tx_data != nullptr){
    _spi.writeBytes(tx_data, len);
  }else if(rx_data != nullptr){
    _spi.transferBytes(nullptr, rx_data, len);
  }
  _spi.endTransaction();
  digitalWrite(cs_pin, HIGH);
  return true;
}

HAL_SPI& get_spi_bus(){
  static ESP32_SPI spi_inst(SPI);
  return spi_inst;
}
