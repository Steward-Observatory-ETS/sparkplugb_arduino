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
const char* MQTT_TOPIC = "spBv1.0/dev/DDATA/Teensy/fib";
sparkplugb_arduino_encoder spark;
org_eclipse_tahu_protobuf_Payload payload;
#define BINARY_BUFFER_SIZE 256
uint8_t binary_buffer[BINARY_BUFFER_SIZE]; // buffer for writing data to the network
org_eclipse_tahu_protobuf_Payload_Metric metrics[1];

const int ledPin = 13;
bool ledVal;
int32_t fib;
int32_t fib_a;
int32_t fib_b;

// ============== MQTT Subscription callback ===================================
void callback(char* topic, byte* payload, unsigned int length){
  // this is where we would put code to handle an incomming MQTT message
  // but this example does not accept incomming data
}

// ============== Setup all objects ============================================
void setup() {
  // --------- TAHU -----------------
  // create a payload and fill in the struct with appropriate values
  spark.set_payload(&payload);
  spark.set_metrics(metrics, 1); // tell obj to set the pointer to metrics data
  metrics[0].name = "fibonacci"; // name the metric
  metrics[0].has_alias = false; // not using aliases
  //metrics[0].alias = 0;
  metrics[0].has_timestamp = true; // yes we are using timestamps
  metrics[0].timestamp = 0; // not assigned yet
  metrics[0].has_datatype = true; // yes it has a data type
  metrics[0].datatype = METRIC_DATA_TYPE_INT32;
  metrics[0].has_is_historical = false;
  metrics[0].has_is_transient = false;
  metrics[0].has_is_null = false;
  metrics[0].has_metadata = false;
  metrics[0].has_properties = false;
  metrics[0].value.string_value = NULL;
  metrics[0].which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
  metrics[0].value.int_value = 0;
  // ------- END TAHU --------------

  // initial values for fibonacci
  fib = 0;
  fib_a = 0;
  fib_b = 1;

  // set LED pin as an output
  pinMode(ledPin, OUTPUT);
  ledVal = 1; // set the LED value

  // set up the ethernet
  Ethernet.begin(mac, ip, dns, gateway, subnet);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {

      digitalWrite(ledPin, HIGH);   // set the LED on
      while(1){
          digitalWrite(ledPin, HIGH);
          delay(100);
          digitalWrite(ledPin, LOW);
          delay(100);
      }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    digitalWrite(ledPin, HIGH);   // set the LED on
  }

  // ----- Setup MqttClient ----------------
  mmqtClient.setServer(mqtt_server, 1883);
  mmqtClient.setCallback(callback);
}

void loop(){
  int message_length;

  if(!mmqtClient.connected()){
    if(mmqtClient.connect("Teensy1")){
      // subscribe to topics

    }else{
      // oh noes, we failed to connect!
      ledVal = 1;
    }
  }

  if(mmqtClient.connected()){
    // compute next value in fibonacci sequence
    fib = fib_a + fib_b;
    if(fib < fib_b){
      fib = 0;
      fib_a = 0;
      fib_b = 1;
    }
    else{
      fib_a = fib_b;
      fib_b = fib;
    }

    // Fill in the payload data
        // If we were keeping track of the time we could set the timestamp
        //spark.payload.timestamp = timeClient.getUTCEpochTime();
        //spark.payload.metrics[0].timestamp = timeClient.getUTCEpochTime();
    payload.metrics[0].value.int_value = fib;
    payload.seq++;
    uint8_t *buf_ptr = (uint8_t*)binary_buffer;

    // Encode the payload
    message_length = spark.encode(&buf_ptr, BINARY_BUFFER_SIZE);

    // Publish Sparkplug B encoded MQTT message to broker
    mmqtClient.publish(MQTT_TOPIC, binary_buffer, message_length, 0);
    mmqtClient.loop();
    ledVal = !ledVal;
  }
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledVal);
  delay(1000); // wait a second before looping
}
