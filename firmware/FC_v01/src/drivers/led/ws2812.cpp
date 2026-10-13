#include "drivers/led/ws2812.h"

Color get_palette_color(PaletteColorId id){
  switch(id){
    case PALETTE_RED:     return {255, 0, 0};
    case PALETTE_GREEN:   return {0, 255, 0};
    case PALETTE_BLUE:    return {0, 40, 255};   // Baro
    case PALETTE_CYAN:    return {0, 255, 255};  // Gyro
    case PALETTE_MAGENTA: return {255, 0, 255};  // Mag
    case PALETTE_YELLOW:  return {255, 220, 0};
    case PALETTE_ORANGE:  return {255, 80, 0};   // Accel
    case PALETTE_PURPLE:  return {130, 0, 255};
    case PALETTE_LIME:    return {100, 255, 0};
    case PALETTE_TEAL:    return {0, 180, 120};
    case PALETTE_PINK:    return {255, 60, 140};
    case PALETTE_AMBER:   return {255, 140, 0};
    case PALETTE_WHITE:   return {255, 255, 255};
    case PALETTE_OFF:
    default:              return {0, 0, 0};
  }
}

bool WS2812::init(int gpio_pin){
  if(!pulse_tx.init(gpio_pin, 10000000)) return false;
  set_low();
  return true;
}

void WS2812::set_rgb(uint8_t r, uint8_t g, uint8_t b){
  current_r = r;
  current_g = g;
  current_b = b;
  if(is_on) encode_and_transmit(current_r, current_g, current_b);
}

void WS2812::set_brightness(uint8_t scale){
  brightness = scale;
  if(is_on) encode_and_transmit(current_r, current_g, current_b);
}

void WS2812::set_high(){
  is_on = true;
  encode_and_transmit(current_r, current_g, current_b);
}

void WS2812::set_low(){
  is_on = false;
  encode_and_transmit(0, 0, 0);
}

void WS2812::encode_and_transmit(uint8_t r, uint8_t g, uint8_t b){
  uint8_t r_scaled = static_cast<uint8_t>((static_cast<uint16_t>(r) * brightness) / 255);
  uint8_t g_scaled = static_cast<uint8_t>((static_cast<uint16_t>(g) * brightness) / 255);
  uint8_t b_scaled = static_cast<uint8_t>((static_cast<uint16_t>(b) * brightness) / 255);

  uint32_t grb_color = (static_cast<uint32_t>(g_scaled) << 16) |
                       (static_cast<uint32_t>(r_scaled) << 8)  |
                        static_cast<uint32_t>(b_scaled);

  size_t symbol_idx = 0;
  for(int bit = 23; bit >= 0; --bit){
    bool bit_val = (grb_color >> bit) & 0x01;
    if(bit_val){
      // Bit 1: T1H = 900ns, T1L = 350ns
      symbol_buffer[symbol_idx].duration0_ns = 900;
      symbol_buffer[symbol_idx].level0       = 1;
      symbol_buffer[symbol_idx].duration1_ns = 350;
      symbol_buffer[symbol_idx].level1       = 0;
    }else{
      // Bit 0: T0H = 350ns, T0L = 900ns
      symbol_buffer[symbol_idx].duration0_ns = 350;
      symbol_buffer[symbol_idx].level0       = 1;
      symbol_buffer[symbol_idx].duration1_ns = 900;
      symbol_buffer[symbol_idx].level1       = 0;
    }
    symbol_idx++;
  }

  pulse_tx.write_symbols(symbol_buffer, 24);
  pulse_tx.wait_tx_done(5);
}
