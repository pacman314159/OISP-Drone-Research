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

bool ESP32_SPI::mount_fatfs(int cs_pin, int cd_pin, const char* mount_point, uint32_t freq_hz){
  if(_mounted) return true;
  if(!_initialized) return false;

  if(cd_pin >= 0){
    pinMode(cd_pin, INPUT_PULLUP);
    if(digitalRead(cd_pin) == HIGH)
      return false;
  }

  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, HIGH);

  _mounted = SD.begin(static_cast<uint8_t>(cs_pin), _spi, freq_hz, mount_point);
  return _mounted;
}

void ESP32_SPI::unmount_fatfs(const char* mount_point){
  if(_mounted){
    SD.end();
    _mounted = false;
  }
}

bool ESP32_SPI::is_fatfs_mounted(){
  return _mounted;
}

uint64_t ESP32_SPI::get_total_bytes(){
  if(!_mounted) return 0;
  return SD.totalBytes();
}

uint64_t ESP32_SPI::get_free_bytes(){
  if(!_mounted) return 0;
  uint64_t total = SD.totalBytes();
  uint64_t used  = SD.usedBytes();
  return (total > used) ? (total - used) : 0;
}

bool ESP32_SPI::is_card_inserted(int cd_pin){
  if(cd_pin < 0) return true;
  pinMode(cd_pin, INPUT_PULLUP);
  return (digitalRead(cd_pin) == LOW);
}

HAL_SPI& get_spi_bus(){
  static ESP32_SPI spi_inst(SPI);
  return spi_inst;
}
