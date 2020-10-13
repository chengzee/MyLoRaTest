/*
  LoRa Simple Node
  Temperature and Humidity sensor: DHT22
  Light sensor:BH1750
  Transition module: LoRa sx1278  
*/

#include <SPI.h>
#include <LoRa.h>
#include <BH1750.h>
#include <Wire.h>
#include "DHT.h"
#include <LSleep.h>

#define DHTPIN 4
#define DHTTYPE DHT22
//#define powerSwitch 5
#define DHTVPIN 5
#define RED_LED 0

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
LSleepClass Sleep;

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 7;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

const long frequency = 433E6;         // LoRa Frequency
const int txPower = 20;               // LoRa TxPower
const int spreadingfactor = 11;       // LoRa SpreadingFactor
const long signalbandwidth = 125E3;  // LoRa SignalBandwidth      
int counter = 1;
float h, t, lux;
const int analogInput = 14;
float vout = 0.0;
float vin = 0.0;
float R1 = 30000.0; //  
float R2 = 7500.0; // 
int value = 0;

void enterSleep(int _ms, int _mode)
{
  float _sleepTime = _ms * 0.001;
  Sleep.init(true);
  Sleep.setTime(_ms);       // 設定時間
  Sleep.setMode(_mode);     // 睡眠模式：0 不睡；1 待機狀態；2 wifi睡眠狀態；3 傳統睡眠狀態
  Serial.print("晚安(");
  Serial.print(_sleepTime, 3);
  Serial.println(" s)");
  Sleep.sleep();
  Serial.println("起床");
}

void setup() {
  Serial.begin(9600);                 // initialize serial
  while (!Serial);
  Serial.println("LoRa Node");
  pinMode(irqPin, OUTPUT);                  // 7
  pinMode(DHTVPIN, OUTPUT);                 // 5
  pinMode(RED_LED, OUTPUT);                 // 0
  pinMode(analogInput, INPUT);              // 14
}

void loop() {
  //-----------------------------------------------------------------
  // turn on the power
//  digitalWrite(powerSwitch, HIGH);
  digitalWrite(DHTVPIN, HIGH);
  
  //-----------------------------------------------------------------
  // read the value at analog input
  value = analogRead(analogInput);
  Serial.print("value(in pin 2):");
  Serial.println(value);
  vout = (value * 2.5) / 4096.0;
  vin = vout / (R2/(R1+R2)); 
  Serial.print("INPUT V= ");
  Serial.println(vin,2);
  
  // ------------------------------------------------------------------------------
  // Check Input Voltage is still enough
  if (vin < 3.5){
    digitalWrite(RED_LED, HIGH);    
  }
  else{
    digitalWrite(RED_LED, HIGH);
    delay(500);
    digitalWrite(RED_LED, LOW);
    delay(500);
    digitalWrite(RED_LED, HIGH);
    delay(500);
    digitalWrite(RED_LED, LOW);
    delay(500);
    digitalWrite(RED_LED, HIGH);
    delay(500);
    digitalWrite(RED_LED, LOW);
    delay(500);
  }

  //-----------------------------------------------------------------
  // Set LoRa pins
  LoRa.setPins(csPin, resetPin, irqPin);
  
  //-----------------------------------------------------------------
  // start the DHT22
  dht.begin();
  // Wait a few seconds between measurements.
  delay(1000);
  //-----------------------------------------------------------------
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  //-----------------------------------------------------------------
  // Read light strengh in lux (lm/m^2)
  // initialize the i2c bus (bh1750's communicate interface)
  Wire.begin();
  // start the bh1750
  lightMeter.begin();
  float lux = lightMeter.readLightLevel();
  Serial.print(h);
  Serial.print(",");
  Serial.print(t);
  Serial.print(",");
  Serial.println(lux);
  //-----------------------------------------------------------------
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (h >= 100 || h <= 0 || t >= 40 || t <= 0 ) {
    Serial.println("Read error value from DHT sensor!");
    return;
  }
  if (isnan(lux) || lux > 65535 || lux < 0) {
    Serial.println("Failed to read from bh1750 sensor!");
    return;
  }
  
  // ------------------------------------------------------------------------------
  // Send
  // Start lora
  for (int i = 0; i < 5; i++){
    if (!LoRa.begin(frequency)) {
      Serial.println("LoRa init failed. Check your connections.");
      continue;                    // if failed, try again
    }else{
      Serial.println("Great, you have init LoRa");
      break;
    }
  }
  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingfactor);
  LoRa.setSignalBandwidth(signalbandwidth);
  
  Serial.print("Sending packet: ");
  Serial.print("Node91, ");
  Serial.print(counter);
  Serial.print(", ");
  Serial.print(h);
  Serial.print(", ");
  Serial.print(t);
  Serial.print(", ");
  Serial.println(lux);
//  Serial.print(", ");
//  Serial.println(vin);
  
  // ------------------------------------------------------------------------------
  // Sending packet(turn LoRa power on)
  digitalWrite(irqPin, HIGH);
  if (counter>1){
    // send packet
    
    LoRa.beginPacket();
    LoRa.print("Node91, ");
    LoRa.print(counter);
    LoRa.print(", ");
    LoRa.print(h);
    LoRa.print(", ");
    LoRa.print(t);
    LoRa.print(", ");
    LoRa.print(lux);
//    LoRa.print(", ");
//    LoRa.print(vin);
    LoRa.endPacket();
  }
  Serial.println("1, flag");
  digitalWrite(irqPin, LOW);
  digitalWrite(DHTVPIN, LOW);
//  digitalWrite(powerSwitch, LOW);
  counter++;
  enterSleep(293645,3);
//  enterSleep(7730,3);
}
