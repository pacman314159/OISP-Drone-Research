#include "platforms/targets/espressif/rgb_led_rmt.h"
#include "config.h"
#include <driver/gpio.h>

#if __has_include(<driver/rmt_tx.h>)
#include <driver/rmt_tx.h>
#define USE_ESP_IDF_V5_RMT 1
#else
#include <driver/rmt.h>
#define USE_ESP_IDF_V5_RMT 0
#endif

RGB_LED_RMT::RGB_LED_RMT()
  : _gpio_pin(RGB_LED_PIN), _r(0), _g(0), _b(0), _initialized(false), _rmt_channel_handle(nullptr){}

void RGB_LED_RMT::init(uint8_t gpio_pin){
  _gpio_pin = gpio_pin;

#if USE_ESP_IDF_V5_RMT
  rmt_tx_channel_config_t tx_chan_config = {};
  tx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT;
  tx_chan_config.gpio_num = (gpio_num_t)_gpio_pin;
  tx_chan_config.mem_block_symbols = 64;
  tx_chan_config.resolution_hz = 10000000;
  tx_chan_config.trans_queue_depth = 4;

  rmt_channel_handle_t tx_channel = nullptr;
  if(rmt_new_tx_channel(&tx_chan_config, &tx_channel) == ESP_OK){
    rmt_enable(tx_channel);
    _rmt_channel_handle = (void*)tx_channel;
    _initialized = true;
  }
#else
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX((gpio_num_t)_gpio_pin, RMT_CHANNEL_0);
  config.clk_div = 8;
  if(rmt_config(&config) == ESP_OK && rmt_driver_install(RMT_CHANNEL_0, 0, 0) == ESP_OK)
    _initialized = true;
#endif
}

void RGB_LED_RMT::set_pixel(uint8_t r, uint8_t g, uint8_t b){
  _r = r;
  _g = g;
  _b = b;
}

void RGB_LED_RMT::show(){
  if(!_initialized) return;

  uint32_t grb = ((uint32_t)_g << 16) | ((uint32_t)_r << 8) | (uint32_t)_b;

#if USE_ESP_IDF_V5_RMT
  rmt_symbol_word_t symbols[25];
  for(int bit = 0; bit < 24; bit++){
    bool bit_is_one = ((grb >> (23 - bit)) & 0x01) != 0;
    if(bit_is_one){
      symbols[bit].duration0 = 8;
      symbols[bit].level0 = 1;
      symbols[bit].duration1 = 5;
      symbols[bit].level1 = 0;
    }else{
      symbols[bit].duration0 = 4;
      symbols[bit].level0 = 1;
      symbols[bit].duration1 = 9;
      symbols[bit].level1 = 0;
    }
  }
  symbols[24].duration0 = 300;
  symbols[24].level0 = 0;
  symbols[24].duration1 = 300;
  symbols[24].level1 = 0;

  rmt_copy_encoder_config_t copy_encoder_config = {};
  rmt_encoder_handle_t copy_encoder = nullptr;
  if(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder) == ESP_OK){
    rmt_transmit_config_t tx_config = {};
    tx_config.loop_count = 0;
    rmt_channel_handle_t tx_channel = (rmt_channel_handle_t)_rmt_channel_handle;
    rmt_transmit(tx_channel, copy_encoder, symbols, sizeof(symbols), &tx_config);
    rmt_del_encoder(copy_encoder);
  }
#else
  rmt_item32_t items[25];
  for(int bit = 0; bit < 24; bit++){
    bool bit_is_one = ((grb >> (23 - bit)) & 0x01) != 0;
    if(bit_is_one){
      items[bit].duration0 = 8;
      items[bit].level0 = 1;
      items[bit].duration1 = 5;
      items[bit].level1 = 0;
    }else{
      items[bit].duration0 = 4;
      items[bit].level0 = 1;
      items[bit].duration1 = 9;
      items[bit].level1 = 0;
    }
  }
  items[24].duration0 = 300;
  items[24].level0 = 0;
  items[24].duration1 = 300;
  items[24].level1 = 0;

  rmt_write_items(RMT_CHANNEL_0, items, 25, true);
#endif
}

HAL_RGB_LED& get_rgb_led(){
  static RGB_LED_RMT led_inst;
  return led_inst;
}
