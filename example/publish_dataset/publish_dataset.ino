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
const char* MQTT_TOPIC = "spBv1.0/dev/DDATA/Teensy/dataset";

// Sparkplug
sparkplugb_arduino_encoder spark; // splarkplug b encoder object
const char* METRICS_NAME = "feedback";
#define BINARY_BUFFER_SIZE 256
uint8_t binary_buffer[BINARY_BUFFER_SIZE]; // buffer for writing data to the network
org_eclipse_tahu_protobuf_Payload payload;
org_eclipse_tahu_protobuf_Payload_Metric metrics[1];
org_eclipse_tahu_protobuf_Payload_DataSet_Row feedback_data_rows[2];
org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue feedback_data_values1[2];
org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue feedback_data_values2[2];
uint32_t feedback_data_types[2]; // feedback types for dataset
char *col_names[2]; // pointers to column names
char col0_name[5]; // buffer for column name 1
char col1_name[5]; // buffer for column name 2

// other globals
const int ledPin = 13; // diagnositic LED pin number
bool ledVal; // diagnositic LED value
int32_t fib; // current fibonacci value
int32_t fib_a; // fn-1
int32_t fib_b; // fn-2

// ============== MQTT Subscription callback ===================================
void callback(char* topic, byte* payload, unsigned int length){
  // this is where we would put code to handle an incomming MQTT message
  // but this example does not accept incomming data
}

// ============== Setup all objects ============================================
void setup() {
  strcpy(col0_name, "col0\0");
  strcpy(col1_name, "col1\0");
  col_names[0] = col0_name;
  col_names[1] = col1_name;

  // --------- TAHU -----------------
  // create a payload and fill in the struct with appropriate values
  spark.set_payload(&payload);
  spark.set_metrics(metrics, 1); // pointer to metrics data and size
  metrics[0].name = (char*)METRICS_NAME; // name the metric
  metrics[0].has_alias = false; // not using aliases
  metrics[0].alias = 0;
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
  feedback_data_types[0] = DATA_SET_DATA_TYPE_INT32; // column 0
  feedback_data_types[1] = DATA_SET_DATA_TYPE_BOOLEAN; // column 1

  // dataset
  metrics[0].value.dataset_value = org_eclipse_tahu_protobuf_Payload_DataSet_init_default;

  metrics[0].value.dataset_value.has_num_of_columns = true;
  metrics[0].value.dataset_value.num_of_columns = 2;
  metrics[0].value.dataset_value.columns_count = 2;
  metrics[0].value.dataset_value.columns = col_names;
  metrics[0].value.dataset_value.types_count = 2;
  metrics[0].value.dataset_value.types = feedback_data_types;
  metrics[0].value.dataset_value.rows_count = 2;
  metrics[0].value.dataset_value.rows = feedback_data_rows;
  metrics[0].value.dataset_value.extensions = NULL;

  // initialize the rows
  memset(feedback_data_rows, 0, sizeof(feedback_data_rows));
  feedback_data_rows[0] = org_eclipse_tahu_protobuf_Payload_DataSet_Row_init_default;
  feedback_data_rows[0].elements_count = 2;
  feedback_data_rows[0].elements = feedback_data_values1;
  feedback_data_rows[1] = org_eclipse_tahu_protobuf_Payload_DataSet_Row_init_default;
  feedback_data_rows[1].elements_count = 2;
  feedback_data_rows[1].elements = feedback_data_values2;

  memset(feedback_data_values1, 0, sizeof(feedback_data_values1));
  feedback_data_values1[0] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;

  feedback_data_values1[0].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_int_value_tag;

  feedback_data_values1[0].value.int_value = 0;

  feedback_data_values1[1] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;

  feedback_data_values1[1].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_boolean_value_tag;

  feedback_data_values1[1].value.int_value = false;

  memset(feedback_data_values2, 0, sizeof(feedback_data_values2));
  feedback_data_values2[0] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;

  feedback_data_values2[0].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_int_value_tag;

  feedback_data_values2[0].value.boolean_value = 0;

  feedback_data_values2[1] = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_init_default;

  feedback_data_values2[1].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_boolean_value_tag;

  feedback_data_values2[1].value.boolean_value = false;

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

  // initial values for fibonacci
  fib = 0;
  fib_a = 0;
  fib_b = 1;
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

    feedback_data_values1[0].value.int_value = fib;
    feedback_data_values1[1].value.boolean_value = ledVal;
    feedback_data_values2[0].value.int_value = fib * -1;
    feedback_data_values2[1].value.boolean_value = !ledVal;

    payload.seq++;
    uint8_t *buf_ptr = (uint8_t*)binary_buffer;

    // Encode the payload
    message_length = spark.encode(&buf_ptr, BINARY_BUFFER_SIZE);

    // Publish Sparkplug B encoded MQTT message to broker
    mmqtClient.publish(MQTT_TOPIC, binary_buffer, message_length, 0);
    mmqtClient.loop();

    ledVal = !ledVal; // toggle the LED so we know we're alive!
  }
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledVal);
  delay(1000); // wait a second before looping
}
