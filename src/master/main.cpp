#include "Arduino.h"
#include <SPI.h>             
#include <LoRa.h>
#include "lora.hpp"

#include <ArduinoClock.hpp>

#define TX_LAPSE_MS          10000

const uint8_t localAddress = 0xB0;
uint8_t remoteAddress = 0xFF;       

volatile bool txDoneFlag = true;  
volatile bool transmitting = false;

LoRaConfig localNodeConf = defaultConfig;
LoRaConfig remoteNodeConf;

int remoteRSSI = 0;
float remoteSNR = 0;

void onTxDone()
{
  txDoneFlag = true;
}

void onReceive(int packetSize) 
{
  if (transmitting && !txDoneFlag) txDoneFlag = true;
  
  if (packetSize == 0) return;

  Message message;

  message.destinationAddress = LoRa.read();
  message.sourceAddress = LoRa.read();
  message.messageId = ((uint16_t)LoRa.read() << 7) | 
                            (uint16_t)LoRa.read();
  message.type = MessageType(LoRa.read());
  message.payloadLength = LoRa.read(); 

  uint8_t payload[10];
  message.payload = payload;

  uint8_t receivedBytes = 0;
  while (LoRa.available() && (receivedBytes < uint8_t(sizeof(payload)-1))) {            
    payload[receivedBytes++] = (char)LoRa.read();
  }
  
  if (message.payloadLength != receivedBytes) {
    Serial.print( "Receiving error: declared message length " 
            + String(message.payloadLength));
    Serial.println(" does not match length " + String(receivedBytes));
    return;                             
  }

  if ((message.destinationAddress & localAddress) != localAddress ) {
    Serial.println("Receiving error: This message is not for me.");
    return;
  }

  Serial.println("Received from: 0x" + String(message.sourceAddress, HEX));
  Serial.println("Sent to: 0x" + String(message.destinationAddress, HEX));
  Serial.println("Message ID: " + String(message.messageId));
  Serial.println("Payload length: " + String(message.payloadLength));
  Serial.print("Payload: ");
  printBinaryPayload(message.payload, receivedBytes);
  Serial.print("\nRSSI: " + String(LoRa.packetRssi()));
  Serial.print(" dBm\nSNR: " + String(LoRa.packetSnr()));
  Serial.println(" dB");

  if (receivedBytes == 4) {
    remoteNodeConf.bandwidthIndex = payload[0] >> 4;
    remoteNodeConf.spreadingFactor = 6 + ((payload[0] & 0x0F) >> 1);
    remoteNodeConf.codingRate = 5 + (payload[1] >> 6);
    remoteNodeConf.txPower = 2 + ((payload[1] & 0x3F) >> 1);
    remoteRSSI = -int(payload[2]) / 2.0f;
    remoteSNR  =  int(payload[3]) - 148;
  
    Serial.print("Remote config: BW: ");
    Serial.print(bandwidth_kHz[remoteNodeConf.bandwidthIndex]);
    Serial.print(" kHz, SPF: ");
    Serial.print(remoteNodeConf.spreadingFactor);
    Serial.print(", CR: ");
    Serial.print(remoteNodeConf.codingRate);
    Serial.print(", TxPwr: ");
    Serial.print(remoteNodeConf.txPower);
    Serial.print(" dBm, RSSI: ");
    Serial.print(remoteRSSI);
    Serial.print(" dBm, SNR: ");
    Serial.print(remoteSNR,1);
    Serial.println(" dB\n");
  }
  else {
    Serial.print("Unexpected payload size: ");
    Serial.print(receivedBytes);
    Serial.println(" bytes\n");
  }
}

void setup() 
{
  Serial.begin(115200);  
  while (!Serial); 

  setupLora(localNodeConf, onReceive, onTxDone);
}

void buildPayload(uint8_t* payload, uint8_t& payloadLength) {
    payload[payloadLength]    = (localNodeConf.bandwidthIndex << 4);
    payload[payloadLength++] |= ((localNodeConf.spreadingFactor - 6) << 1);
    payload[payloadLength]    = ((localNodeConf.codingRate - 5) << 6);
    payload[payloadLength++] |= ((localNodeConf.txPower - 2) << 1);
    payload[payloadLength++] = uint8_t(-LoRa.packetRssi() * 2);
    payload[payloadLength++] = uint8_t(148 + LoRa.packetSnr());
}

bool canTransmit(bool transmitting, uint32_t lastSendTime_ms, 
        uint32_t txInterval_ms) { 
    return !transmitting && ((millis() - lastSendTime_ms) > txInterval_ms);
}

bool doneTransmitting(bool transmitting, bool txDoneFlag) {
    return transmitting && txDoneFlag;
}

void updateTxInterval(uint32_t txDuration_ms, uint32_t txBegin_ms, 
        uint32_t& lastSendTime_ms, uint32_t& txInterval_ms){
    uint32_t sinceLastMessage_ms = txBegin_ms - lastSendTime_ms;
    lastSendTime_ms = txBegin_ms; 
    float duty_cycle = (100.0f * txDuration_ms) / sinceLastMessage_ms;
    
    Serial.print("Duty cycle: ");
    Serial.print(duty_cycle, 1);
    Serial.println(" %\n");

    // Solo si el ciclo de trabajo es superior al 1% lo ajustamos
    if (duty_cycle > 1.0f) {
      txInterval_ms = txDuration_ms * 100;
    }
}

void loop() 
{
  static uint32_t lastSendTime_ms = 0;
  static uint16_t msgCount = 0;
  static uint32_t txInterval_ms = TX_LAPSE_MS;
  static uint32_t txBegin_ms = 0;
  
      
  if (canTransmit(transmitting, lastSendTime_ms, txInterval_ms)) {

    uint8_t payload[50];
    uint8_t payloadLength = 0;

    buildPayload(payload, payloadLength);
    
    transmitting = true;
    txDoneFlag = false;
    txBegin_ms = millis();

    Message message;
    message.messageId = msgCount;
    message.type = MessageType::STATUS;
    message.sourceAddress = localAddress; 
    message.destinationAddress = remoteAddress;
    message.payload = payload;
    message.payloadLength = payloadLength;
  
    sendMessage(message);
    Serial.print("Sending packet ");
    Serial.print(msgCount++);
    Serial.print(": ");
    printBinaryPayload(payload, payloadLength);
  }                  
  
  if (doneTransmitting(transmitting, txDoneFlag)) {
    uint32_t txDuration_ms = millis() - txBegin_ms;
    Serial.print("----> TX completed in ");
    Serial.print(txDuration_ms);
    Serial.println(" msecs");
    
    updateTxInterval(
            txDuration_ms, 
            txBegin_ms, 
            lastSendTime_ms, 
            txInterval_ms);
    
    transmitting = false;
    
    // Reactivamos la recepción de mensajes, que se desactiva
    // en segundo plano mientras se transmite
    LoRa.receive();   
  }
}
