/*
  LoRa Simple Node
  Temperature and Humidity sensor: DHT 11
  Light sensor:BH1750 
*/

#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"

#define DHTPIN GPIO36
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const long frequency = 433E6;         // LoRa Frequency
const int txPower = 20;               // LoRa TxPower
const int spreadingfactor = 11;       // LoRa SpreadingFactor
const long signalbandwidth = 125E3;  // LoRa SignalBandwidth      
int counter = 1;
float h, t;

void setup() {
  Serial.begin(9600);                 // initialize serial
  while (!Serial);
  Serial.println("LoRa Node");
  
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                      // if failed, do nothing
  }else{
    Serial.println("Great, you have init LoRa");
  }
  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingfactor);
  LoRa.setSignalBandwidth(signalbandwidth);
}

void loop() {
  // start the dht11
  dht.begin();
  // Wait a few seconds between measurements.
  delay(1000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read light strengh in lux (lm/m^2)
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // ------------------------------------------------------------------------------
  // Send
  //  start lora
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingfactor);
  LoRa.setSignalBandwidth(signalbandwidth);
  
  Serial.print("Sending packet: ");
  Serial.print("Node2, ");
  Serial.print(counter);
  Serial.print(", ");
  Serial.print(h);
  Serial.print(", ");
  Serial.print(t);
  
  if (counter>1){
    // send packet
    LoRa.beginPacket();
    LoRa.print("Node2, ");
    LoRa.print(counter);
    LoRa.print(", ");
    LoRa.print(h);
    LoRa.print(", ");
    LoRa.print(t);
    LoRa.endPacket();
  }
  counter++;
  delay(300000);
  
}
