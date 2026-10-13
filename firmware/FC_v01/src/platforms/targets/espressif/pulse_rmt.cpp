#include "platforms/targets/espressif/pulse_rmt.h"
#include <Arduino.h>

HAL_PulseTX& get_pulse_tx0(){
  static ESP32_PulseTX pulse_tx0_inst;
  return pulse_tx0_inst;
}

ESP32_PulseTX::~ESP32_PulseTX(){
  if(initialized){
    rmt_driver_uninstall(channel);
    initialized = false;
  }
}

bool ESP32_PulseTX::init(int gpio_pin, uint32_t clock_resolution_hz){
  if(initialized) return true;

  resolution_hz = clock_resolution_hz;
  uint8_t clk_div = static_cast<uint8_t>(80000000UL / resolution_hz);
  if(clk_div == 0) clk_div = 8;

  rmt_config_t config = {};
  config.rmt_mode = RMT_MODE_TX;
  config.channel = channel;
  config.gpio_num = static_cast<gpio_num_t>(gpio_pin);
  config.clk_div = clk_div;
  config.mem_block_num = 1;
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;

  if(rmt_config(&config) != ESP_OK) return false;
  if(rmt_driver_install(channel, 0, 0) != ESP_OK) return false;

  initialized = true;
  return true;
}

bool ESP32_PulseTX::write_symbols(const PulseSymbol* symbols, size_t count){
  if(!initialized || symbols == nullptr || count == 0) return false;

  rmt_item32_t rmt_symbols[64];
  size_t batch_count = (count > 64) ? 64 : count;

  uint32_t ns_per_tick = 1000000000UL / resolution_hz;
  if(ns_per_tick == 0) ns_per_tick = 100;

  for(size_t i = 0; i < batch_count; ++i){
    uint32_t ticks0 = (symbols[i].duration0_ns + (ns_per_tick / 2)) / ns_per_tick;
    uint32_t ticks1 = (symbols[i].duration1_ns + (ns_per_tick / 2)) / ns_per_tick;

    if(ticks0 > 0x7FFF) ticks0 = 0x7FFF;
    if(ticks1 > 0x7FFF) ticks1 = 0x7FFF;

    rmt_symbols[i].duration0 = ticks0;
    rmt_symbols[i].level0    = symbols[i].level0 & 0x01;
    rmt_symbols[i].duration1 = ticks1;
    rmt_symbols[i].level1    = symbols[i].level1 & 0x01;
  }

  esp_err_t err = rmt_write_items(channel, rmt_symbols, static_cast<int>(batch_count), false);
  return (err == ESP_OK);
}

bool ESP32_PulseTX::wait_tx_done(uint32_t timeout_ms){
  if(!initialized) return false;
  esp_err_t err = rmt_wait_tx_done(channel, pdMS_TO_TICKS(timeout_ms));
  return (err == ESP_OK);
}
