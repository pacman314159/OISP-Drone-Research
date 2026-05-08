#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Pins & Parameters
#define PWM_PIN            D10
#define RIGHT_BUTTON_PIN   D9
#define LEFT_BUTTON_PIN    D8
#define TFT_CS_PIN         D1
#define TFT_DC_PIN         D2
#define TFT_RST_PIN        D3
#define TFT_SDA_PIN        D4
#define TFT_SCL_PIN        D5

#define PWM_FREQ_HZ        5000
#define PWM_RES            12
#define PWM_CHANNEL        0
#define MAX_DUTY_VALUE     ((1 << PWM_RES) - 1)

enum SystemState {
  WELCOME,
  CALIB_PROMPT,
  CALIBRATING,
  PWM_GENERATION,
  EMERGENCY_STOP
};
SystemState state = WELCOME;
float duty = 0.0;
QueueHandle_t button_queue;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_SDA_PIN, TFT_SCL_PIN, TFT_RST_PIN);

void pwm_setup() {
    ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_RES);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void update_duty(float duty) {
    ledcWrite(PWM_CHANNEL, duty * MAX_DUTY_VALUE);
}

// --- UI Helper ---
void display_msg(String line1, String line2, uint16_t color) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(color);
    tft.setTextSize(2);
    
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(line1, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, (tft.height() / 2) - 15);
    tft.print(line1);

    tft.getTextBounds(line2, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, (tft.height() / 2) + 5);
    tft.print(line2);
}

void buttonTask(void *pvParameters) {
    bool lastLeftState = HIGH;
    bool lastRightState = HIGH;

    while (1) {
        bool currentLeft = digitalRead(LEFT_BUTTON_PIN);
        bool currentRight = digitalRead(RIGHT_BUTTON_PIN);

        // Falling Edge Detection (High to Low)
        if (lastLeftState == HIGH && currentLeft == LOW) {
            int btn = LEFT_BUTTON_PIN;
            xQueueSend(button_queue, &btn, 0);
        }
        if (lastRightState == HIGH && currentRight == LOW) {
            int btn = RIGHT_BUTTON_PIN;
            xQueueSend(button_queue, &btn, 0);
        }

        lastLeftState = currentLeft;
        lastRightState = currentRight;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void controlTask(void *pvParameters) {
  int btn;
  while (1) {
    if (digitalRead(LEFT_BUTTON_PIN) == LOW && digitalRead(RIGHT_BUTTON_PIN) == LOW) {
      state = EMERGENCY_STOP;
      update_duty(0);
      display_msg("EMERGENCY", "STOPPED", ST77XX_RED);
      while(1) vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    switch (state) {
      case WELCOME:{
        display_msg("Welcome to", "Thrust testbench", ST77XX_WHITE);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        state = CALIB_PROMPT;
        break;
      }

      case CALIB_PROMPT:{
        display_msg("ESC Calib?", "L:No | R:Yes", ST77XX_YELLOW);
        if (xQueueReceive(button_queue, &btn, portMAX_DELAY)) {
          if (btn == RIGHT_BUTTON_PIN) state = CALIBRATING;
          else state = PWM_GENERATION;
        }
        tft.fillScreen(ST77XX_BLACK);
        break;
      }

      case CALIBRATING:{
        display_msg("CALIBRATING", ".........", ST77XX_RED);
        update_duty(1.0);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        update_duty(0.0);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        tft.fillScreen(ST77XX_BLACK);
        state = PWM_GENERATION;
        break;
      }

      case PWM_GENERATION:{
        // Update UI without clearing full screen (avoids flicker)
        tft.setCursor(20, 30);
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        tft.printf("THRUST: %d%%  ", (int)(duty * 100));

        // Don't block here, just peek for 50ms
        if (xQueueReceive(button_queue, &btn, 50 / portTICK_PERIOD_MS)) {
          if (btn == RIGHT_BUTTON_PIN && duty < 1.0) 
            duty += 0.05;
          else if (btn == LEFT_BUTTON_PIN && duty > 0.0) 
            duty -= 0.05;

          duty = constrain(duty, 0, 1);
          update_duty(duty);
        }
        break;
      }

      default: break;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
    pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
    
    tft.initR(INITR_MINI160x80);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);

    pwm_setup();
    update_duty(0);

    button_queue = xQueueCreate(10, sizeof(int));

    xTaskCreate(buttonTask, "Button Scan", 2048, NULL, 2, NULL);
    xTaskCreate(controlTask, "Control", 4096, NULL, 1, NULL);
}

void loop(){}
