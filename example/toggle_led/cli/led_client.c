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
	Teensy LED toggler using Sparkplug B MQTT messaging.

	This program is intended to send a Sparkplug B encoded MQTT message to a local
	MQTT broker. The message is topic is "spBv1.0/dev/DCMD/Teensy1/led" with a
	value of either TRUE or FALSE. This is a command message that tells a Teensy4.1
	to set the red LED to either ON or OFF.

	Command line argument "1" sends a TRUE value, any other arg sends a FALSE value.

	Tested on Debian "Stretch" 9.9 running Mosquitto MQTT broker.

	Michael Sibayan 2020
	Steward Observatory
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "tahu.h"
#include "tahu.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "mosquitto.h"
#include <inttypes.h>

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

uint8_t binary_buffer[1024];

int main(int argc, char *argv[]) {

	// MQTT Parameters
  char *host = "localhost";
  int port = 1883;
  int keepalive = 60;
  bool clean_session = true;
  struct mosquitto *mosq = NULL;
  bool cmd_val = false;

  if(argc == 2){
    if(strcmp(argv[1], "1") == 0){
			printf("setting CMD to true\n");
		  cmd_val = true;
		}
		else{
			printf("setting CMD to false\n");
		}
  }
	else{
		printf("setting CMD to false\n");
	}

	// MQTT Setup
  srand(time(NULL));
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

  // Create the DDATA payload
  char *metric_name = "led_cmd";
  get_next_payload(&payload);
  payload.metrics = &metrics;
  payload.metrics_count = 1;
  metrics.name = metric_name;
  metrics.has_alias = true;
  metrics.alias = Device_Metric0;
  metrics.has_timestamp = true;
  metrics.timestamp = 0;
  metrics.has_datatype = true;
  metrics.datatype = METRIC_DATA_TYPE_BOOLEAN;
  metrics.has_is_historical = false;
  metrics.has_is_transient = false;
  metrics.has_is_null = false;
  metrics.has_metadata = false;
  metrics.has_properties = false;
  metrics.value.string_value = NULL;
  metrics.which_value = org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag;
  metrics.value.boolean_value = cmd_val;

  // Encode the payload into a binary format so it can be published in the MQTT message.
  // The binary_buffer must be large enough to hold the contents of the binary payload
	//uint8_t *buf_ptr = (uint8_t*)binary_buffer;
  size_t message_length = encode_payload(binary_buffer, 1024, &payload);

  // Publish the DDATA on the appropriate topic
  mosquitto_publish(mosq, NULL, "spBv1.0/dev/DCMD/Teensy/led", message_length, binary_buffer, 0, false);

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
