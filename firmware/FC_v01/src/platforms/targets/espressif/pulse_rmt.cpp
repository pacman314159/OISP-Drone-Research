#include "platforms/targets/espressif/pulse_rmt.h"
#include <Arduino.h>
#include <soc/soc_caps.h>

HAL_PulseTX& get_pulse_tx0(){
  static ESP32_PulseTX pulse_tx0_inst;
  return pulse_tx0_inst;
}

ESP32_PulseTX::~ESP32_PulseTX(){
  if(initialized){
    if(tx_channel != nullptr){
      rmt_disable(tx_channel);
      if(copy_encoder != nullptr){
        rmt_del_encoder(copy_encoder);
        copy_encoder = nullptr;
      }
      rmt_del_channel(tx_channel);
      tx_channel = nullptr;
    }
    initialized = false;
  }
}

bool ESP32_PulseTX::init(int gpio_pin, uint32_t clock_resolution_hz){
  if(initialized) return true;

  resolution_hz = clock_resolution_hz;

  rmt_tx_channel_config_t tx_chan_config = {};
  tx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT;
  tx_chan_config.gpio_num = static_cast<gpio_num_t>(gpio_pin);
#if defined(SOC_RMT_MEM_WORDS_PER_CHANNEL)
  tx_chan_config.mem_block_symbols = SOC_RMT_MEM_WORDS_PER_CHANNEL;
#else
  tx_chan_config.mem_block_symbols = 64;
#endif
  tx_chan_config.resolution_hz = resolution_hz;
  tx_chan_config.trans_queue_depth = 4;
  tx_chan_config.intr_priority = 0;
  tx_chan_config.flags.invert_out = 0;
  tx_chan_config.flags.with_dma = 0;

  if(rmt_new_tx_channel(&tx_chan_config, &tx_channel) != ESP_OK) return false;

  rmt_copy_encoder_config_t copy_encoder_config = {};
  if(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder) != ESP_OK){
    rmt_del_channel(tx_channel);
    tx_channel = nullptr;
    return false;
  }

  if(rmt_enable(tx_channel) != ESP_OK){
    rmt_del_encoder(copy_encoder);
    copy_encoder = nullptr;
    rmt_del_channel(tx_channel);
    tx_channel = nullptr;
    return false;
  }

  initialized = true;
  return true;
}

bool ESP32_PulseTX::write_symbols(const PulseSymbol* symbols, size_t count){
  if(!initialized || tx_channel == nullptr || copy_encoder == nullptr || symbols == nullptr || count == 0) return false;

  rmt_symbol_word_t rmt_symbols[64];
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

  rmt_transmit_config_t tx_config = {};
  tx_config.loop_count = 0;

  esp_err_t err = rmt_transmit(tx_channel, copy_encoder, rmt_symbols, batch_count * sizeof(rmt_symbol_word_t), &tx_config);
  return (err == ESP_OK);
}

bool ESP32_PulseTX::wait_tx_done(uint32_t timeout_ms){
  if(!initialized || tx_channel == nullptr) return false;
  esp_err_t err = rmt_tx_wait_all_done(tx_channel, static_cast<int>(timeout_ms));
  return (err == ESP_OK);
}

HAL_PulseRX& get_pulse_rx0(){
  static ESP32_PulseRX pulse_rx0_inst;
  return pulse_rx0_inst;
}

ESP32_PulseRX::~ESP32_PulseRX(){
  if(initialized){
    stop();
    if(rx_channel != nullptr){
      rmt_disable(rx_channel);
      rmt_del_channel(rx_channel);
      rx_channel = nullptr;
    }
    if(rx_queue != nullptr){
      vQueueDelete(rx_queue);
      rx_queue = nullptr;
    }
    initialized = false;
  }
}

bool IRAM_ATTR ESP32_PulseRX::rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* edata, void* user_data){
  ESP32_PulseRX* self = static_cast<ESP32_PulseRX*>(user_data);
  if(self == nullptr || edata == nullptr) return false;

  BaseType_t high_task_wakeup = pdFALSE;

  if(edata->num_symbols > 0 && edata->received_symbols != nullptr){
    RxFrame frame{};
    frame.count = 0;
    for(size_t i = 0; i < edata->num_symbols; ++i){
      uint32_t d0 = edata->received_symbols[i].duration0;
      uint32_t d1 = edata->received_symbols[i].duration1;

      // Detect frame sync gap (> 3000 us pause in either half)
      if(d0 >= 3000 || d1 >= 3000){
        if(frame.count >= 4) break;
        frame.count = 0;
        continue;
      }

      uint32_t pulse = d0 + d1;
      if(pulse >= 800 && pulse <= 2200 && frame.count < 12){
        frame.pulses[frame.count++] = pulse;
      }
    }

    if(frame.count >= 4 && self->rx_queue != nullptr)
      xQueueOverwriteFromISR(self->rx_queue, &frame, &high_task_wakeup);
  }

  // Immediately re-arm reception for next PPM frame
  if(rmt_receive(self->rx_channel, self->raw_symbols, sizeof(self->raw_symbols), &self->receive_config) == ESP_OK)
    self->is_receiving = true;
  else
    self->is_receiving = false;

  return (high_task_wakeup == pdTRUE);
}

bool ESP32_PulseRX::init(int gpio_pin, uint32_t clock_resolution_hz, uint32_t idle_threshold_us){
  if(initialized) return true;

  resolution_hz = clock_resolution_hz;

  rmt_rx_channel_config_t rx_chan_config = {};
  rx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT;
  rx_chan_config.resolution_hz = resolution_hz;
#if defined(SOC_RMT_MEM_WORDS_PER_CHANNEL)
  rx_chan_config.mem_block_symbols = SOC_RMT_MEM_WORDS_PER_CHANNEL;
#else
  rx_chan_config.mem_block_symbols = 64;
#endif
  rx_chan_config.gpio_num = static_cast<gpio_num_t>(gpio_pin);
  rx_chan_config.intr_priority = 0;
  rx_chan_config.flags.invert_in = 0;
  rx_chan_config.flags.with_dma = 0;

  if(rmt_new_rx_channel(&rx_chan_config, &rx_channel) != ESP_OK) return false;

  rmt_rx_event_callbacks_t cbs = {
    .on_recv_done = rx_done_callback
  };
  if(rmt_rx_register_event_callbacks(rx_channel, &cbs, this) != ESP_OK){
    rmt_del_channel(rx_channel);
    rx_channel = nullptr;
    return false;
  }

  receive_config.signal_range_min_ns = 1000; // 1 us filter
  receive_config.signal_range_max_ns = idle_threshold_us * 1000UL; // 3 ms idle timeout for sync gap

  if(rx_queue == nullptr)
    rx_queue = xQueueCreate(1, sizeof(RxFrame));
  if(rx_queue == nullptr){
    rmt_del_channel(rx_channel);
    rx_channel = nullptr;
    return false;
  }

  // Ensure internal pull-up is active for clean PPM signal transitions
  gpio_set_pull_mode(static_cast<gpio_num_t>(gpio_pin), GPIO_PULLUP_ONLY);

  if(rmt_enable(rx_channel) != ESP_OK){
    vQueueDelete(rx_queue);
    rx_queue = nullptr;
    rmt_del_channel(rx_channel);
    rx_channel = nullptr;
    return false;
  }

  initialized = true;
  start();
  return true;
}

void ESP32_PulseRX::start(){
  if(initialized && !is_receiving){
    if(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config) == ESP_OK)
      is_receiving = true;
  }
}

void ESP32_PulseRX::stop(){
  if(initialized && is_receiving){
    rmt_disable(rx_channel);
    rmt_enable(rx_channel);
    is_receiving = false;
  }
}

size_t ESP32_PulseRX::read_pulses(uint32_t* pulse_durations_us, size_t max_pulses, uint32_t timeout_ms){
  if(!initialized || rx_queue == nullptr || pulse_durations_us == nullptr || max_pulses == 0) return 0;

  RxFrame frame;
  if(xQueueReceive(rx_queue, &frame, pdMS_TO_TICKS(timeout_ms)) != pdPASS) return 0;

  size_t count = (frame.count > max_pulses) ? max_pulses : frame.count;
  for(size_t i = 0; i < count; ++i)
    pulse_durations_us[i] = frame.pulses[i];

  return count;
}
