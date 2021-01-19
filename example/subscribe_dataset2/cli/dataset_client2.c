/*******************************************************************************
Copyright 2019
Steward Observatory Engineering & Technical Services, University of Arizona

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/

/*
	Teensy data-set upload using Sparkplug B MQTT messaging.

	This program is intended to send a Sparkplug B encoded MQTT message to a local
	MQTT broker. The message is topic is "spBv1.0/dev/DCMD/Teensy1/data" with a
	dataset containing 4 integer values.

	Command line arguments are four space-delimted numbers to send.

	Tested on Debian "Stretch" 9.9 running Mosquitto MQTT broker.

	Michael Sibayan 2020
	Steward Observatory
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include "mosquitto.h"
#include "tahu.h"
#include "tahu.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

/* Mosquitto Callbacks */
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void my_connect_callback(struct mosquitto *mosq, void *userdata, int result);
void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str);

/* Local Functions */
void publisher(struct mosquitto *mosq, char *topic, void *buf, unsigned len);
//void publish_ddata_message(struct mosquitto *mosq);

org_eclipse_tahu_protobuf_Payload payload;
org_eclipse_tahu_protobuf_Payload_Metric metrics;

enum alias_map {
	Next_Server = 0,
	Rebirth = 1,
	Reboot = 2,
	Dataset = 3,
	Node_Metric0 = 4,
	Node_Metric1 = 5,
	Node_Metric2 = 6,
	Device_Metric0 = 7,
	Device_Metric1 = 8,
	Device_Metric2 = 9,
	Device_Metric3 = 10,
	My_Custom_Motor = 11
};

uint8_t binary_buffer[4096];

int main(int argc, char *argv[]) {

	// MQTT Parameters
  char *host = "localhost";
  int port = 1883;
  int keepalive = 60;
  bool clean_session = true;
  struct mosquitto *mosq = NULL;
	//int data[4];
	org_eclipse_tahu_protobuf_Payload_DataSet_Row row_data[1];
	org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue elements[240];
	uint32_t datatypes[240];
	const char *column_keys[240];
	char keys[240][1];
	int i;
	int qty = 240;
	int type = 0;


	printf("Usage: publish_dataset [s|r] [N]\n");
	printf("s = sequential\nr = random\nN = number to send (1-240)\n\n");

	// parse data
  if(argc == 3){
		if(strcmp(argv[1], "r") == 0){
			type = 1;
		}else if(strcmp(argv[1], "s") == 0){
			type = 0;
		}

		qty = atoi(argv[2]);
		if(qty <= 0 || qty > 240) qty = 240;

	}

	printf("Sending %i %s elements\n", qty, type?"random":"sequential");

	// MQTT Setup
  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, clean_session, NULL);
  if(!mosq){
    fprintf(stderr, "Error: Out of memory.\n");
    return 1;
  }

  mosquitto_log_callback_set(mosq, my_log_callback);
  mosquitto_connect_callback_set(mosq, my_connect_callback);
  mosquitto_message_callback_set(mosq, my_message_callback);
  mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
  //mosquitto_username_pw_set(mosq,"admin","changeme");
  mosquitto_will_set(mosq, "spBv1.0/dev/NDEATH/client", 0, NULL, 0, false);

	// MQTT Connect
  if(mosquitto_connect(mosq, host, port, keepalive)){
    fprintf(stderr, "Unable to connect.\n");
    return 1;
  }

  // Create the payload
  char *metric_name = "data";
  //get_next_payload(&payload);
	payload.has_timestamp = false;
	payload.timestamp = 0;
	payload.has_seq = true;
	payload.seq = 0;
	payload.uuid = NULL;
	payload.body = NULL;
	payload.extensions = NULL;
  payload.metrics = &metrics;
  payload.metrics_count = 1;
  metrics.name = metric_name;
  metrics.has_alias = false;
  metrics.alias = 0;
  metrics.has_timestamp = false;
  metrics.timestamp = 0;
  metrics.has_datatype = true;
  metrics.datatype = METRIC_DATA_TYPE_DATASET;
  metrics.has_is_historical = false;
  metrics.has_is_transient = false;
  metrics.has_is_null = false;
  metrics.has_metadata = false;
  metrics.has_properties = false;
  metrics.value.string_value = NULL;
  metrics.which_value = org_eclipse_tahu_protobuf_Payload_Metric_dataset_value_tag;
  //metrics.value.dataset_value = org_eclipse_tahu_protobuf_Payload_DataSet_init_zero;

	row_data[0].elements_count = qty;
	row_data[0].elements = elements;
	row_data[0].extensions = NULL;

	memset(keys, 0, sizeof(keys));

	for(i=0; i<qty; i++){
		datatypes[i] = DATA_SET_DATA_TYPE_INT32;
		column_keys[i] = keys[i];
	//	keys[i][0] = '\0';
	//	keys[i][1] = '\0';
		elements[i].which_value = org_eclipse_tahu_protobuf_Payload_DataSet_DataSetValue_int_value_tag;

		if(type == 0){
			elements[i].value.int_value = i;
		}else{
			elements[i].value.int_value = 100*rand();
		}
	}

	//init_dataset(&metrics.value.dataset_value, 1, 4, datatypes, column_keys, row_data);
	metrics.value.dataset_value.has_num_of_columns = true;
	metrics.value.dataset_value.num_of_columns = qty;
	metrics.value.dataset_value.columns_count = qty;
	metrics.value.dataset_value.columns = (char**)column_keys;
	metrics.value.dataset_value.types_count = qty;
	metrics.value.dataset_value.types = datatypes;
	metrics.value.dataset_value.rows_count = 1;
	metrics.value.dataset_value.rows = row_data;
	metrics.value.dataset_value.extensions = NULL;



  // Encode the payload into a binary format so it can be published in the MQTT message.
  // The binary_buffer must be large enough to hold the contents of the binary payload
  //size_t message_length = encode_payload(binary_buffer, 1024, &payload);
	printf("Encoding payload...\n");
	pb_ostream_t buffer_stream = pb_ostream_from_buffer(binary_buffer, 4096);
	const bool encode_result = pb_encode(&buffer_stream, org_eclipse_tahu_protobuf_Payload_fields, &payload);
	if(encode_result == false){
		printf("Failed to encode!\n");
		return -1;
	}
	const size_t message_length = buffer_stream.bytes_written;
	printf("Message length: %zd\n", message_length);

  // Publish the DDATA on the appropriate topic
  mosquitto_publish(mosq, NULL, "spBv1.0/dev/DCMD/Teensy1/data2", message_length, binary_buffer, 0, false);

	int total = sizeof(payload) + sizeof(metrics) + sizeof(row_data) + sizeof(elements);
	total += sizeof(column_keys) + sizeof(datatypes) + sizeof(keys);
	printf("total size: %i\n\n", total);
	// Close and cleanup
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}

/*
 * Helper function to publish MQTT messages to the MQTT server
 */
void publisher(struct mosquitto *mosq, char *topic, void *buf, unsigned len) {
	// publish the data
	mosquitto_publish(mosq, NULL, topic, len, buf, 0, false);
}

/*
 * Callback for incoming MQTT messages. Since this is a Sparkplug implementation these will be NCMD and DCMD messages
 */
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {

}

/*
 * Callback for successful or unsuccessful MQTT connect.  Upon successful connect, subscribe to our Sparkplug NCMD and DCMD messages.
 * A production application should handle MQTT connect failures and reattempt as necessary.
 */
void my_connect_callback(struct mosquitto *mosq, void *userdata, int result) {

}

/*
 * Callback for successful MQTT subscriptions.
 */
void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {

}

/*
 * MQTT logger callback
 */
void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str) {

}
