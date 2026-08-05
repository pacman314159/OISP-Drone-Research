#ifndef LORA_RX_H
  #define LORA_RX_H

  #include <Arduino.h>
  #include <SPI.h>
  #include <LoRa.h>

  #define NSS 13
  #define RST 6
  #define DIO0 1
  #define SCK 9
  #define MISO 10
  #define MOSI 11
  #define FREQUENCY 433E6
  
  class Lora_RX{
    private:

    public:
      void setupRX();
      void loopRX();
    };
#endif