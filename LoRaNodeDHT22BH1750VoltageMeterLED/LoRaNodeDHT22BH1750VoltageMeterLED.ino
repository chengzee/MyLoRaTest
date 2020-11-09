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

#define DHTVPIN 4
#define DHTPIN 5
#define DHTTYPE DHT22
#define analogInput 14  // read divided voltage
#define digitalOutput 3 // set voltagemeteray on/off 

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
float vout;
float voltage = 0;

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
  pinMode(irqPin, OUTPUT);
  pinMode(DHTVPIN, OUTPUT);
  pinMode(analogInput, INPUT);              // 14
  pinMode(digitalOutput, OUTPUT);           // 3 voltagemeter
  // set initial state
  digitalWrite(DHTVPIN, LOW);
  digitalWrite(digitalOutput, LOW);
}

void loop() {
  //-----------------------------------------------------------------
  // if it work all day, we measure the voltage  
  // read the value at analog input
  if (counter%5 == 1){
    // turn on the voltagemeter's power by setting relay
    digitalWrite(digitalOutput, HIGH);
    int value = analogRead(analogInput);
    Serial.print("value(in pin 14):");
    Serial.println(value);
    vout = (value * 2.5) / 4096.0;
    voltage = vout / (7500.0/(30000.0+7500.0)); 
    Serial.print("INPUT V= ");
    Serial.println(voltage,2);
    //-----------------------------------------------------------------
    // turn off the voltagemeter's power when we complete measure
    digitalWrite(digitalOutput, LOW);
    // Check Input Voltage is still enough
    if (voltage < 3.2){
      digitalWrite(digitalOutput, HIGH); 
    }
    else{
      digitalWrite(digitalOutput, HIGH);
      delay(500);
      digitalWrite(digitalOutput, LOW);
      delay(500);
      digitalWrite(digitalOutput, HIGH);
      delay(500);
      digitalWrite(digitalOutput, LOW);
      delay(500);
    }
  }
  LoRa.setPins(csPin, resetPin, irqPin);
  digitalWrite(DHTVPIN, HIGH);
  // start the DHT22
  dht.begin();
  // Wait a few seconds between measurements.
  delay(1000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
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
  digitalWrite(DHTVPIN, LOW);
  
  // ------------------------------------------------------------------------------
  // Send
  //  start lora
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
  Serial.print("Node94, ");
  Serial.print(counter);
  Serial.print(", ");
  Serial.print(h);
  Serial.print(", ");
  Serial.print(t);
  Serial.print(", ");
  Serial.println(lux);
  digitalWrite(irqPin, HIGH);
 
  if (counter>1){
    // send packet
    LoRa.beginPacket();
    LoRa.print("Node94, ");
    LoRa.print(counter);
    LoRa.print(", ");
    LoRa.print(h);
    LoRa.print(", ");
    LoRa.print(t);
    LoRa.print(", ");
    LoRa.print(lux);
    LoRa.endPacket();
  }
  
  digitalWrite(irqPin, LOW);
  counter++;
  enterSleep(293645,3);
//  enterSleep(3645,3);
}
