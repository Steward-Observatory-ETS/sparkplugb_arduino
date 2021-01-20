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

This program generates a 1-row by 12-column data set.
Column 1 is a 16-bit integer

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
const char* MQTT_TOPIC = "spBv1.0/PS/DDATA/PPI12/PP1";

// Sparkplug
sparkplugb_arduino_encoder spark; // splarkplug b encoder object
const char* METRICS_NAME = "feedback";
#define BINARY_BUFFER_SIZE 1024
uint8_t binary_buffer[BINARY_BUFFER_SIZE]; // buffer for writing data to the network
org_eclipse_tahu_protobuf_Payload payload;
org_eclipse_tahu_protobuf_Payload_Metric metrics[1];
org_eclipse_tahu_protobuf_Payload_DataSet_Row feedback_data_rows[1];
org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue feedback_data_values[12];
uint32_t feedback_data_types[12]; // feedback types for dataset
char *names[12]; // pointers to column names
char col_names[12][5]; // buffer for column names

// other globals
const int ledPin = 13; // diagnositic LED pin number
bool ledVal; // diagnositic LED value

// ============== MQTT Subscription callback ===================================
void callback(char* topic, byte* payload, unsigned int length){
  // this is where we would put code to handle an incomming MQTT message
  // but this example does not accept incomming data
}

// ============== Setup all objects ============================================
void setup() {
  int i;
  strcpy(col_names[0], "V0\0");
  strcpy(col_names[1], "V1\0");
  strcpy(col_names[2], "V2\0");
  strcpy(col_names[3], "V3\0");
  strcpy(col_names[4], "IL0\0");
  strcpy(col_names[5], "IL1\0");
  strcpy(col_names[6], "IL2\0");
  strcpy(col_names[7], "IL3\0");
  strcpy(col_names[8], "IR0\0");
  strcpy(col_names[9], "IR1\0");
  strcpy(col_names[10], "IR2\0");
  strcpy(col_names[11], "IR3\0");
  for(i=0; i<12; i++){
    names[i] = col_names[i];
  }

  // --------- TAHU -----------------
  // create a payload and fill in the struct with appropriate values
  spark.set_payload(&payload);
  spark.set_metrics(metrics, 1); // set pointer to metrics data
  metrics[0].name = (char*)METRICS_NAME; // name the metric
  metrics[0].has_alias = false; // not using aliases
  //metrics[0].alias = 0;
  metrics[0].has_timestamp = true; // yes we are using timestamps
  metrics[0].timestamp = 0; // not assigned yet
  metrics[0].has_datatype = true; // yes it has a data type
  metrics[0].datatype = METRIC_DATA_TYPE_DATASET;
  metrics[0].has_is_historical = false;
  metrics[0].has_is_transient = false;
  metrics[0].has_is_null = false;
  metrics[0].has_metadata = false;
  metrics[0].has_properties = false;
  metrics[0].value.string_value = NULL;
  metrics[0].which_value = org_eclipse_tahu_protobuf_Payload_Metric_dataset_value_tag;
  // ^^ This needs to be a "value tag"

  // specifiy the data type for each column, this needs to be a "DATA_SET_DATA_TYPE"
  memset(feedback_data_types, 0, sizeof(feedback_data_types));
  for(i=0; i<12; i++){
    feedback_data_types[i] = DATA_SET_DATA_TYPE_INT16; // column data type
  }

  // dataset
  metrics[0].value.dataset_value = org_eclipse_tahu_protobuf_Payload_DataSet_init_default;

  metrics[0].value.dataset_value.has_num_of_columns = true;
  metrics[0].value.dataset_value.num_of_columns = 12;
  metrics[0].value.dataset_value.columns_count = 12;
  metrics[0].value.dataset_value.columns = names;
  metrics[0].value.dataset_value.types_count = 12;
  metrics[0].value.dataset_value.types = feedback_data_types;
  metrics[0].value.dataset_value.rows_count = 1;
  metrics[0].value.dataset_value.rows = feedback_data_rows;
  metrics[0].value.dataset_value.extensions = NULL;

  // initialize the rows
  memset(feedback_data_rows, 0, sizeof(feedback_data_rows));
  feedback_data_rows[0] = org_eclipse_tahu_protobuf_Payload_DataSet_Row_init_default;
  feedback_data_rows[0].elements_count = 12;
  feedback_data_rows[0].elements = feedback_data_values;

  memset(feedback_data_values, 0, sizeof(feedback_data_values));
  for(i=0; i<12; i++){
    feedback_data_values[i] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;
    feedback_data_values[i].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_int_value_tag;
    feedback_data_values[i].value.int_value = 0;
  }

  // ------- END TAHU --------------

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
  int message_length, i;
  static int j = 0;
  static int last = 0;
  if(!mmqtClient.connected()){
    if(mmqtClient.connect("Teensy1")){
      // subscribe to topics

    }else{
      // oh noes, we failed to connect!
      ledVal = 1;
    }
  }

  if(mmqtClient.connected()){
    // Fill in the payload data
        // If we were keeping track of the time we could set the timestamp
        //spark.payload.timestamp = timeClient.getUTCEpochTime();
        //spark.payload.metrics[0].timestamp = timeClient.getUTCEpochTime();

    // lets report the previous encoded byte length
    feedback_data_values[0].value.int_value = last;
    // sequence data for the rest of the items...
    for(i=1; i<12; i++){
      feedback_data_values[i].value.int_value = i*j;
    }

    payload.seq++;
    uint8_t *buf_ptr = (uint8_t*)binary_buffer;

    // Encode the payload
    message_length = spark.encode(&buf_ptr, BINARY_BUFFER_SIZE);
    last = message_length;

    // Publish Sparkplug B encoded MQTT message to broker
    mmqtClient.publish(MQTT_TOPIC, binary_buffer, message_length, 0);
    mmqtClient.loop();

    ledVal = !ledVal; // toggle the LED so we know we're alive!
    j++;
  }
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledVal);
  delay(1000); // wait a second before looping
}
