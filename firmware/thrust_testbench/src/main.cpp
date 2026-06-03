#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "motor_interface.h"

// Pins & Parameters
#define PWM_PIN            D10
#define RIGHT_BUTTON_PIN   D9
#define LEFT_BUTTON_PIN    D8
#define TFT_CS_PIN         D1
#define TFT_DC_PIN         D2
#define TFT_RST_PIN        D3
#define TFT_SDA_PIN        D4
#define TFT_SCL_PIN        D5

#define LEDC_BIT_RESOLUTION  12
#define LEDC_CHANNEL         0

#define PWM_DUTY_INCREMENT 20

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_SDA_PIN, TFT_SCL_PIN, TFT_RST_PIN);
QueueHandle_t button_queue;
Motor_Interface bldc;
int duty = PWM_MIN_PULSE_LEN_US;

// --- UI Helper ---
void display_msg(String line1, String line2, uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(color);

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
      bldc.update_pulse_len(0); bldc.drive();
      display_msg("EMERGENCY", "STOPPED", ST77XX_RED);
      vTaskDelete(NULL); // self-destroy task
    }

    // Update UI without clearing full screen (avoids flicker)
    tft.setCursor(20, 30);
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.printf("%d ms  ", duty);

    if (xQueueReceive(button_queue, &btn, 50 / portTICK_PERIOD_MS)) {
      if (btn == RIGHT_BUTTON_PIN && duty < PWM_MAX_PULSE_LEN_US) 
        duty += PWM_DUTY_INCREMENT;
      else if (btn == LEFT_BUTTON_PIN && duty > PWM_MIN_PULSE_LEN_US) 
        duty -= PWM_DUTY_INCREMENT;
      bldc.update_pulse_len(duty); bldc.drive();
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);

  tft.initR(INITR_MINI160x80);
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.fillScreen(ST77XX_BLACK);

  display_msg("WELCOME", "CALIBRATING", ST77XX_RED);

  bldc.config(LEDC_CHANNEL, LEDC_BIT_RESOLUTION);
  bldc.attach_gpio(PWM_PIN);
  bldc.update_pulse_len(PWM_MAX_PULSE_LEN_US); bldc.drive();
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  bldc.update_pulse_len(PWM_MIN_PULSE_LEN_US); bldc.drive();

  tft.fillScreen(ST77XX_BLACK);

  button_queue = xQueueCreate(10, sizeof(int));

  xTaskCreate(buttonTask, "Button Scan", 2048, NULL, 2, NULL);
  xTaskCreate(controlTask, "Control", 4096, NULL, 1, NULL);
}

void loop(){}
