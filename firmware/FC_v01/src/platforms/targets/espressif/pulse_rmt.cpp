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

HAL_PulseRX& get_pulse_rx0(){
  static ESP32_PulseRX pulse_rx0_inst;
  return pulse_rx0_inst;
}

ESP32_PulseRX::~ESP32_PulseRX(){
  if(initialized){
    rmt_rx_stop(channel);
    rmt_driver_uninstall(channel);
    initialized = false;
    rb_handle = nullptr;
  }
}

bool ESP32_PulseRX::init(int gpio_pin, uint32_t clock_resolution_hz, uint32_t idle_threshold_us){
  if(initialized) return true;

  resolution_hz = clock_resolution_hz;
  uint8_t clk_div = static_cast<uint8_t>(80000000UL / resolution_hz);
  if(clk_div == 0) clk_div = 80;

  rmt_config_t config = {};
  config.rmt_mode = RMT_MODE_RX;
  config.channel = channel;
  config.gpio_num = static_cast<gpio_num_t>(gpio_pin);
  config.clk_div = clk_div;
  config.mem_block_num = 1;
  config.rx_config.filter_en = false; // Disable glitch filter to ensure narrow delimiters (~300-400us) are never dropped
  config.rx_config.filter_ticks_thresh = 0;
  config.rx_config.idle_threshold = static_cast<uint16_t>(idle_threshold_us);

  if(rmt_config(&config) != ESP_OK) return false;
  if(rmt_driver_install(channel, 2048, 0) != ESP_OK) return false;
  if(rmt_get_ringbuf_handle(channel, &rb_handle) != ESP_OK || rb_handle == nullptr){
    rmt_driver_uninstall(channel);
    return false;
  }

  // Ensure internal pull-up is active for clean PPM signal transitions
  gpio_set_pull_mode(static_cast<gpio_num_t>(gpio_pin), GPIO_PULLUP_ONLY);

  initialized = true;
  start();
  return true;
}

void ESP32_PulseRX::start(){
  if(initialized)
    rmt_rx_start(channel, true);
}

void ESP32_PulseRX::stop(){
  if(initialized)
    rmt_rx_stop(channel);
}

size_t ESP32_PulseRX::read_pulses(uint32_t* pulse_durations_us, size_t max_pulses, uint32_t timeout_ms){
  if(!initialized || rb_handle == nullptr || pulse_durations_us == nullptr || max_pulses == 0) return 0;

  size_t rx_size = 0;
  rmt_item32_t* items = static_cast<rmt_item32_t*>(xRingbufferReceive(rb_handle, &rx_size, pdMS_TO_TICKS(timeout_ms)));
  if(items == nullptr) return 0;

  size_t num_items = rx_size / sizeof(rmt_item32_t);
  size_t count = 0;

  for(size_t i = 0; i < num_items; ++i){
    uint32_t d0 = items[i].duration0;
    uint32_t d1 = items[i].duration1;

    // Detect frame sync gap (> 3000 us pause in either half)
    if(d0 >= 3000 || d1 >= 3000){
      if(count >= 4)
        break; // A complete valid frame was collected prior to this sync gap
      count = 0; // Sync gap marks start of new channel sequence
      continue;
    }

    uint32_t pulse = d0 + d1;
    if(pulse >= 800 && pulse <= 2200 && count < max_pulses){
      pulse_durations_us[count++] = pulse;
    }
  }

  vRingbufferReturnItem(rb_handle, items);
  return count;
}
