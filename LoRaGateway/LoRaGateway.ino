#include <SPI.h>
#include <LoRa.h>

const long frequency = 433E6;         // LoRa Frequency
const int txPower = 20;               // LoRa TxPower
const int spreadingfactor = 12;       // LoRa SpreadingFactor
const long signalbandwidth = 31.25E3;  // LoRa SignalBandwidth
String nodes[] = {"Node1", "Node2", "Node3"};
int nodecall = 0;
int count = 1;
void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Gateway");
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                      // if failed, do nothing
  }
  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingfactor);
  LoRa.setSignalBandwidth(signalbandwidth);
}

void loop() {
  // Initialize nodecall
  if (nodecall > 2){                 //there are only 3 node (0, 1, 2)
    nodecall= 0;
  }
  // each 15 sec sending request
  if (runEvery(15000)) { 
    LoRa.beginPacket();
    LoRa.print(nodes[nodecall]);
    LoRa.endPacket();
    Serial.print("send ");
    Serial.print(count);
    Serial.println(" times");
    count++;
    
    // if request over 3 times, assume we lost the node
    if (count > 3){
      count = 1;
      nodecall++;
    }
  }
  
  String msg = "";
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      msg += (char)LoRa.read();
    }
    Serial.print(msg);
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
  String nodename = "";
  for (int i = 0; i < 5; i++){
    nodename += msg[i];
  }
  
  // check node name
  if (nodename == nodes[nodecall]){          // correct response
    nodecall++;
    Serial.println("Got correct response");
  }
  
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
