/*
Copyright (c) 2020
Steward Observatory Engineering & Technical Services, University of Arizona

This program and the accompanying materials are made available under the
terms of the Eclipse Public License 2.0 which is available at
http://www.eclipse.org/legal/epl-2.0.
*/

/*
Test program for sparkplugb_arduino library, written for a Teensy 4.1
This program subscribes to data from an MMQT broker, encoded as Sparkplug B

This program expects a 240-row by 1-column data set.
Column 1 is a 32-bit signed integer

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

// HTTP server
EthernetServer server(80);

// MQTT
EthernetClient enetClient;
PubSubClient mmqtClient(enetClient);
const char* MQTT_TOPIC = "spBv1.0/dev/DCMD/Teensy1/data";

// Sparkplug
sparkplugb_arduino_decoder spark; // splarkplug b encoder object
#define BINARY_BUFFER_SIZE 2048
uint8_t binary_buffer[BINARY_BUFFER_SIZE]; // buffer for writing data to the network
char metric_str_bufs[2][32];
char metric_name_bufs[2][32];
char rx_as_hex[2560];
org_eclipse_tahu_protobuf_Payload_DataSet dataset;
org_eclipse_tahu_protobuf_Payload_DataSet_Row rows[250];
org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue elements[250];
char *column_keys[4];
char ck[4][16];
uint32_t datatypes[4];

// other globals
const int ledPin = 13; // diagnositic LED pin number
bool ledVal; // diagnositic LED value
int32_t data[250];
int32_t data2[250];
String mqtt_msg;
unsigned long dt;
int nCol = 0;
int nRow = 0;

// ============== MQTT Subscription callback ===================================
void callback(char* topic, byte* payload, unsigned int length){
  // this is where we would put code to handle an incomming MQTT message
  char *ptr;
  unsigned int i;
  static int rec = 0;
  unsigned long t0;
  bool res;

  // convert the message to hex, so we can se it in HTTP diagnostic
  /*ptr=rx_as_hex;
  for(i=0; i<length; i++){
    if(payload[i] > 31 && payload[i] < 127){
      ptr += sprintf(ptr, "%c", payload[i]);
    }
    else{
      ptr += sprintf(ptr, " 0x%x ", payload[i]);
    }
  }*/

  rec++;
  t0 = millis();
  res = spark.decode(payload, length);
  dt = millis() - t0;

  if(!res){
    mqtt_msg = "Failed to decode payload " + String(rec);
  }
  else{
    mqtt_msg = "Decode Time: " + String(dt) + "<br>";
    mqtt_msg += "Topic: " + String(topic) + "<br>";
    mqtt_msg += "Packet# " + String(rec) + "<br>";
    mqtt_msg += "metric size: " + String(spark.payload.metrics_count) + "<br>";

    if(String(topic).compareTo(MQTT_TOPIC) == 0){
      if(spark.payload.metrics_count == 1){
        mqtt_msg += "name: " + String(spark.payload.metrics[0].name) + "<br>";
        mqtt_msg += "Datatype: " + String(spark.payload.metrics[0].datatype) + " ? " + String(METRIC_DATA_TYPE_DATASET) + "<br>";

        nCol = spark.payload.metrics[0].value.dataset_value.num_of_columns;
        nRow = spark.payload.metrics[0].value.dataset_value.rows_count;
        mqtt_msg += "N_columns: " + String(nCol) + "<br>";
        mqtt_msg += "N_rows: " + String(nRow) + "<br>";
        t0 = millis();
        for(i=0;i<nRow;i++){
          data[i] = (int)spark.payload.metrics[0].value.dataset_value.rows[i].elements[0].value.int_value;
        }
        dt = millis() - t0;
        mqtt_msg += "Copy time: " + String(dt) + "<br>";
      }
    }
  }
  spark.free_payload();
}

// ============== Setup all objects ============================================
void setup() {
  int i;
  memset(data, 0, sizeof(data));
  sprintf(rx_as_hex, "no data");
  mqtt_msg = "no message";

  // ~~~~~ PubSubClient ~~~~~~~
  mmqtClient.setBufferSize(2560); // set the buffer size, default is 256
  // ~~~~ End PubSubClient ~~~~

  // set LED pin as an output
  pinMode(ledPin, OUTPUT);
  ledVal = 1; // set the LED value
  digitalWrite(ledPin, ledVal);   // set the LED on

  // set up the ethernet
  Ethernet.begin(mac, ip, dns, gateway, subnet);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      while(1){
          digitalWrite(ledPin, HIGH);
          delay(100);
          digitalWrite(ledPin, LOW);
          delay(100);
      }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    digitalWrite(ledPin, LOW);   // set the LED on
  }

  // ----- Setup MqttClient ----------------
  mmqtClient.setServer(mqtt_server, 1883);
  mmqtClient.setCallback(callback);

  // http server
  server.begin();
}

void loop(){
  static int ctr = 0;
  int i;
  EthernetClient client = server.available(); // listen for incoming clients
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        //// if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          client.println("Counter: " + String(ctr) + "<br>");
          client.println("Connected to MQTT: ");
          if(mmqtClient.connected())
            client.println("TRUE");
          else
            client.println("FALSE");

          client.println("<br><br>" + mqtt_msg + "<br>");
          //client.println("<br><br>" + String(rx_as_hex));

          client.println(String(1) + "> " + String(data[0]) + "<br>");
          client.println(String(nRow) + "> " + String(data[nRow-1]) + "<br>");

        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(200);
    client.stop();
  }

  if(!mmqtClient.connected()){
    if(mmqtClient.connect("Teensy1")){
      // subscribe to topics
      mmqtClient.subscribe(MQTT_TOPIC);
    }else{
      // oh noes, we failed to connect!
      ledVal = 1;
    }
  }

  if(mmqtClient.connected()){
    mmqtClient.loop();

    ledVal = !ledVal; // toggle the LED so we know we're alive!
  }

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledVal);
  delay(1000); // wait a second before looping
  ctr++;
}
