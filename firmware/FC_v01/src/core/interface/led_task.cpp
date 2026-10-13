#include "core/interface/led_task.h"
#include "drivers/led/ws2812.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "config.h"
#include <cmath>

void status_led_task(void* arg){
  if(arg == nullptr) vTaskDelete(NULL);

  WS2812* led = static_cast<WS2812*>(arg);
  LedCommand current_cmd = {LED_PATTERN_OFF, PALETTE_OFF};

  uint32_t anim_step = 0;
  uint8_t base_brightness = RGB_LED_DEFAULT_BRIGHTNESS;

  while(true){
    SYSVIEW_START(TASK_STATUS_LED_ID);

    LedCommand new_cmd;
    TickType_t wait_ticks = portMAX_DELAY;

    if(current_cmd.pattern == LED_PATTERN_BLINK_SLOW ||
       current_cmd.pattern == LED_PATTERN_BLINK_FAST ||
       current_cmd.pattern == LED_PATTERN_BREATHE ||
       current_cmd.pattern == LED_PATTERN_FLASH_BURST){
      wait_ticks = pdMS_TO_TICKS(20);
    }

    if(xQueueReceive(led_cmd_queue, &new_cmd, wait_ticks) == pdTRUE){
      current_cmd = new_cmd;
      anim_step = 0;
    }

    Color c = get_palette_color(current_cmd.color);

    switch(current_cmd.pattern){
      case LED_PATTERN_OFF:
        led->set_low();
        break;

      case LED_PATTERN_SOLID:
        led->set_brightness(base_brightness);
        led->set_rgb(c.r, c.g, c.b);
        led->set_high();
        break;

      case LED_PATTERN_BLINK_SLOW: {
        bool is_on = ((anim_step % 25) < 13);
        if(is_on){
          led->set_brightness(base_brightness);
          led->set_rgb(c.r, c.g, c.b);
          led->set_high();
        }else{
          led->set_low();
        }
        anim_step++;
        break;
      }

      case LED_PATTERN_BLINK_FAST: {
        bool is_on = ((anim_step % 10) < 5);
        if(is_on){
          led->set_brightness(base_brightness);
          led->set_rgb(c.r, c.g, c.b);
          led->set_high();
        }else{
          led->set_low();
        }
        anim_step++;
        break;
      }

      case LED_PATTERN_BREATHE: {
        float phase = (static_cast<float>(anim_step % 100) / 100.0f) * 6.2831853f;
        float normalized = (sinf(phase) + 1.0f) * 0.5f;
        uint8_t scaled_brightness = static_cast<uint8_t>(normalized * base_brightness);
        if(scaled_brightness < 4) scaled_brightness = 4;
        led->set_brightness(scaled_brightness);
        led->set_rgb(c.r, c.g, c.b);
        led->set_high();
        anim_step++;
        break;
      }

      case LED_PATTERN_FLASH_BURST: {
        if(anim_step < 12){
          bool is_on = ((anim_step % 4) < 2);
          if(is_on){
            led->set_brightness(255);
            led->set_rgb(c.r, c.g, c.b);
            led->set_high();
          }else{
            led->set_low();
          }
          anim_step++;
        }else{
          led->set_low();
          current_cmd.pattern = LED_PATTERN_OFF;
        }
        break;
      }

      default:
        led->set_low();
        break;
    }

    SYSVIEW_END(TASK_STATUS_LED_ID);
  }
}
