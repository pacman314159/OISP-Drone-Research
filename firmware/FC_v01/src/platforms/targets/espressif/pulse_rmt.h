#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/rmt_tx.h>
#include <driver/rmt_rx.h>
#include <driver/rmt_encoder.h>
#include <driver/rmt_common.h>
#include "platforms/hal/hal_pulse.h"

class ESP32_PulseTX : public HAL_PulseTX {
private:
  rmt_channel_handle_t tx_channel{nullptr};
  rmt_encoder_handle_t copy_encoder{nullptr};
  bool                 initialized{false};
  uint32_t             resolution_hz{10000000}; // 10 MHz (1 tick = 100 ns)

public:
  ESP32_PulseTX() = default;
  ~ESP32_PulseTX() override;

  bool init(int gpio_pin, uint32_t clock_resolution_hz = 10000000) override;
  bool write_symbols(const PulseSymbol* symbols, size_t count) override;
  bool wait_tx_done(uint32_t timeout_ms = 10) override;
};

class ESP32_PulseRX : public HAL_PulseRX {
private:
  rmt_channel_handle_t rx_channel{nullptr};
  bool                 initialized{false};
  bool                 is_receiving{false};
  uint32_t             resolution_hz{1000000}; // 1 MHz (1 tick = 1 us)
  rmt_receive_config_t receive_config{};
  rmt_symbol_word_t    raw_symbols[64]{};
  QueueHandle_t        rx_queue{nullptr};

  struct RxFrame {
    uint32_t pulses[12]; // Microseconds per channel pulse
    uint8_t  count;      // Number of valid decoded channels in frame
  };

  static bool IRAM_ATTR rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* edata, void* user_data);

public:
  ESP32_PulseRX() = default;
  ~ESP32_PulseRX() override;

  bool init(int gpio_pin, uint32_t clock_resolution_hz = 1000000, uint32_t idle_threshold_us = 3000) override;
  size_t read_pulses(uint32_t* pulse_durations_us, size_t max_pulses, uint32_t timeout_ms = 25) override;
  void start() override;
  void stop() override;
};
