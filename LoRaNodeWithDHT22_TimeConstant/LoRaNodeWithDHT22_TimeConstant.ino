/*
  LoRa Simple Node
  Temperature and Humidity sensor: DHT 22
  Light sensor:BH1750
  Transition module: LoRa sx1278  
*/

#include <SPI.h>
#include <LoRa.h>
#include <BH1750.h>
#include <Wire.h>
#include "DHT.h"

#define DHTPIN GPIO36
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

const long frequency = 433E6;         // LoRa Frequency
const int txPower = 20;               // LoRa TxPower
const int spreadingfactor = 11;       // LoRa SpreadingFactor
const long signalbandwidth = 125E3;  // LoRa SignalBandwidth      
int counter = 1;
float h, t, lux;

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
  // start the dht22
  dht.begin();
  // initialize the i2c bus (bh1750's communicate interface)
  Wire.begin();
  // start the bh1750
  lightMeter.begin();
  // Wait a few seconds between measurements.
  delay(1000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read light strengh in lux (lm/m^2)
  float lux = lightMeter.readLightLevel();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || h >= 100 || h <= 0 || t >= 40 || t <= 0 ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (isnan(lux) || lux > 65535 || lux < 0) {
    Serial.println("Failed to read from bh1750 sensor!");
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
  Serial.print("Node1, ");
  Serial.print(counter);
  Serial.print(", ");
  Serial.print(h);
  Serial.print(", ");
  Serial.print(t);
  Serial.print(", ");
  Serial.println(lux);
  if (counter>1){
    // send packet
    LoRa.beginPacket();
    LoRa.print("Node1, ");
    LoRa.print(counter);
    LoRa.print(", ");
    LoRa.print(h);
    LoRa.print(", ");
    LoRa.print(t);
    LoRa.print(", ");
    LoRa.print(lux);
    LoRa.endPacket();
  }
  counter++;
  delay(300000);
}
