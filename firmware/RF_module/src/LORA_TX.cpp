#include "LORA_TX.h"

void Lora_TX::setupTX(){
    Serial.begin(115200);

    //Setting up SPI pins and Lora pins
    SPI.begin(SCK, MISO, MOSI, NSS);
    LoRa.setPins(NSS, RST, DIO0);
 
    //Two modules needs to be in 433Hz
    if(!LoRa.begin(FREQUENCY)){
        Serial.println("Connecting failed");
        while(1);
    }
    Serial.println("Connecting successfully");

    Serial.println("=========BEIGN RESIGNERS=========");
    LoRa.dumpRegisters(Serial);
    Serial.println("=========END RESIGNERS=========");
}

void Lora_TX::loopTX(){
    static uint32_t previousTime = 0;

    if (millis() - previousTime >= 1000)
    {
        previousTime = millis();
        Serial.println("Receiver waiting for packet...");
    }

    Serial.print("Sending package: ");
    Serial.println(counter);
    LoRa.beginPacket();
    LoRa.print("Hello number: ");
    LoRa.print(counter);
    LoRa.endPacket();
    Serial.println("Packet sent");
    counter++;
    delay(5000);
}

  



