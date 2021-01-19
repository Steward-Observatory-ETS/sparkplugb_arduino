/*
Copyright (c) 2020
Steward Observatory Engineering & Technical Services, University of Arizona

This program and the accompanying materials are made available under the
terms of the Eclipse Public License 2.0 which is available at
http://www.eclipse.org/legal/epl-2.0.
*/

/*
Test program for sparkplugb_arduino library, written for a Teensy 4.1
This program periodically sends data to a an MMQT broker, encoded as Sparkplug B

This program generates a 2-row by 2-column data set.
Column 1 is a 32-bit integer
Column 2 is a boolean value

*** Important Notes ***
---> Failure to follow the below rules may cause garbage messages!
- The number of columns, num_of_columns, dictates number of row elements, elements_count
- Number of names, col_names, must be equal to num_of_columns
- Number of data types, types_count, must be equal to num_of_columns
- rows_count determines number of rows, not number of elements in a row

2020 M. Sibayan
*/

#include <NativeEthernet.h> // Teensy requires NativeEthernet to use onboard NIC
#include <PubSubClient.h> // pub&sub MQTT messages
#include <sparkplugb_arduino.hpp> // Sparkplug B encoding

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 249, 99); // Teensy's IP address
IPAddress dns(192, 168, 249, 1);
IPAddress gateway(192, 168, 249, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress mqtt_server(192, 168, 249, 98); // IP address of MQTT broker

// MQTT
EthernetClient enetClient;
PubSubClient mmqtClient(enetClient);
const char* MQTT_TOPIC = "spBv1.0/dev/DCMD/Teensy/led";

// Sparkplug
sparkplugb_arduino_decoder spark; // splarkplug b encoder object
const char* METRICS_NAME = "feedback";
#define BINARY_BUFFER_SIZE 256
uint8_t binary_buffer[BINARY_BUFFER_SIZE]; // buffer for writing data to the network

// other globals
const int ledPin = 13; // diagnositic LED pin number
int ledState; // LED state: 0 = off, 1 = on, 2 = slow blink, 3 = fast blink
bool ledVal;

// ============== MQTT Subscription callback ===================================
void callback(char* topic, byte* payload, unsigned int length){
  // this is where we would put code to handle an incomming MQTT message
  // but this example does not accept incomming data
  if(spark.decode(payload, length)){
    if(String(topic).compareTo(MQTT_TOPIC) == 0){
      if(spark.payload.metrics_count == 1){
        if(spark.payload.metrics[0].value.boolean_value)
          ledState = 1;
        else
          ledState = 0;
      }
      else{
        ledState = 2;
      }
    }
    else{
      ledState = 2;
    }
  }else{
    ledState = 3;
  }
}

// ============== Setup all objects ============================================
void setup() {
  // set LED pin as an output
  pinMode(ledPin, OUTPUT);
  ledVal = 0; // set the LED value
  ledState = 0;

  // set up the ethernet
  Ethernet.begin(mac, ip, dns, gateway, subnet);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {

      digitalWrite(ledPin, HIGH);   // set the LED on
      while(1){
          digitalWrite(ledPin, HIGH);
          delay(50);
          digitalWrite(ledPin, LOW);
          delay(50);
      }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    //ledState = 3;
  }

  // ----- Setup MqttClient ----------------
  mmqtClient.setServer(mqtt_server, 1883);
  mmqtClient.setCallback(callback);
}

void loop(){
  int delay_time = 0;

  if(!mmqtClient.connected()){
    if(mmqtClient.connect("Teensy1")){
      // subscribe to topics
      mmqtClient.subscribe(MQTT_TOPIC);
    }else{
      // oh noes, we failed to connect!
      ledState = 3;
    }
  }

  if(mmqtClient.connected()){
    mmqtClient.loop();
  }

  pinMode(ledPin, OUTPUT);
  switch(ledState){
    case 0: ledVal = 0; delay_time = 500; break;
    case 1: ledVal = 1; delay_time = 500; break;
    case 2: ledVal = !ledVal; delay_time = 500; break;
    case 3: ledVal = !ledVal; delay_time = 100; break;
  }
  digitalWrite(ledPin, ledVal);
  delay(delay_time); // wait a second before looping
}
