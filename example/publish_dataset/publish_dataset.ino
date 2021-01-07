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
const char* MQTT_TOPIC = "spBv1.0/dev/DDATA/Teensy/dataset";
const char* METRICS_NAME = "feedback";
sparkplugb_arduino_encoder spark;
#define BINARY_BUFFER_SIZE 256
uint8_t binary_buffer[BINARY_BUFFER_SIZE]; // buffer for writing data to the network
#define FEEDBACK_DATA_COUNT 10
org_eclipse_tahu_protobuf_Payload_DataSet_Row feedback_data_rows[1];
_org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue feedback_data_values[FEEDBACK_DATA_COUNT];
uint32_t feedback_data_types[10];
char *col_names[10];
char col0_name[5];
char col1_name[5];
char col2_name[5];
char col3_name[5];
const int ledPin = 13;
bool ledVal;

// ============== MQTT Subscription callback ===================================
void callback(char* topic, byte* payload, unsigned int length){
  // this is where we would put code to handle an incomming MQTT message
  // but this example does not accept incomming data
}

// ============== Setup all objects ============================================
void setup() {
  int i; // iterator
  strcpy(col0_name, "col0\0");
  strcpy(col1_name, "col1\0");
  strcpy(col2_name, "col2\0");
  strcpy(col3_name, "col3\0");
  col_names[0] = col0_name;
  col_names[1] = col1_name;
  col_names[2] = col2_name;
  col_names[3] = col3_name;

  // --------- TAHU -----------------
  // create a payload and fill in the struct with appropriate values
  spark.set_has_metrics(true); // tell obj to set the pointer to metrics data
  spark.payload.metrics_count = 1; // we only have one metric
  spark.metrics[0].name = (char*)METRICS_NAME; // name the metric
  spark.metrics[0].has_alias = false; // not using aliases
  //spark.metrics[0].alias = 0;
  spark.metrics[0].has_timestamp = true; // yes we are using timestamps
  spark.metrics[0].timestamp = 0; // not assigned yet
  spark.metrics[0].has_datatype = true; // yes it has a data type
  spark.metrics[0].datatype = METRIC_DATA_TYPE_DATASET;
  spark.metrics[0].has_is_historical = false;
  spark.metrics[0].has_is_transient = false;
  spark.metrics[0].has_is_null = false;
  spark.metrics[0].has_metadata = false;
  spark.metrics[0].has_properties = false;
  spark.metrics[0].value.string_value = NULL;
  spark.metrics[0].which_value = org_eclipse_tahu_protobuf_Payload_Metric_dataset_value_tag;
  // ^^ This needs to be a "value tag"

  // specifiy the data type for each column, this needs to be a "DATA_SET_DATA_TYPE"
  feedback_data_types[0] = DATA_SET_DATA_TYPE_INT32;
  // **WARNING** if the above is not a DATA_SET_DATA_TYPE, data will fail to parse

  // dataset
  spark.metrics[0].value.dataset_value = org_eclipse_tahu_protobuf_Payload_DataSet_init_default;

  spark.metrics[0].value.dataset_value.has_num_of_columns = true;
  spark.metrics[0].value.dataset_value.num_of_columns = 1;
  spark.metrics[0].value.dataset_value.columns_count = 1;
  spark.metrics[0].value.dataset_value.columns = col_names;
  spark.metrics[0].value.dataset_value.types_count = 1;
  spark.metrics[0].value.dataset_value.types = feedback_data_types;
  spark.metrics[0].value.dataset_value.rows_count = 1;
  spark.metrics[0].value.dataset_value.rows = feedback_data_rows;
  spark.metrics[0].value.dataset_value.extensions = NULL;

  // initialize the rows
  feedback_data_rows[0] = org_eclipse_tahu_protobuf_Payload_DataSet_Row_init_default;
  feedback_data_rows[0].elements_count = 1;
  feedback_data_rows[0].elements = feedback_data_values;

  feedback_data_values[0] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;

  feedback_data_values[0].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_int_value_tag;

  feedback_data_values[0].value.int_value = 1234;

/*
  feedback_data_rows[0].elements_count = FEEDBACK_DATA_COUNT;
  feedback_data_rows[0].elements = feedback_data_values;
  feedback_data_rows[0].extensions = NULL;


  for(i=0; i<FEEDBACK_DATA_COUNT; i++){
    feedback_data_values[i] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;
    feedback_data_values[i].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_int_value_tag;
    feedback_data_values[i].value.int_value = i;
  }
*/
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
    for(i=0; i<FEEDBACK_DATA_COUNT; i++){
      //feedback_data_values[i].value.int_value = i + j;
    }
    j = j + FEEDBACK_DATA_COUNT;
    spark.payload.seq++;
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
