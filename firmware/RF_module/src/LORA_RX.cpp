#include "LORA_RX.h"

void Lora_RX::setupRX(){
    Serial.begin(115200);
    
    //Setiing up SPi and Lora pins
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

void Lora_RX::loopRX(){
    //Parse packet
    int packetSize = LoRa.parsePacket();
    
    //Receive packet
    if (packetSize){
        Serial.print("Received packet '");
        //read packet
        while(LoRa.available()){
            Serial.print((char)LoRa.read());
        }
        Serial.print(" with RSSI: ");
        Serial.println(LoRa.packetRssi());
    }
    Serial.println("Sequence RSSI: ");
    Serial.println(LoRa.rssi());
    delay(5000);
}


