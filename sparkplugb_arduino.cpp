/********************************************************************************
 * Copyright (c) 2014-2019 Cirrus Link Solutions and others
 * Copyright 2020 Steward Observatory
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Cirrus Link Solutions - initial implementation
 *   Steward Observatory - Port to C++ & modifications for Teensy 4.1 usage
 ********************************************************************************/

/*
 Modified/Ported by M. Sibayan 11/2020
 to make it compatable with a Teensy 4.1 and Arduino
 - remove printf and related
 - remove time calls and get_current_timestamp
 + changed to pre-allocated memory for metrics, limits size to pre-defined
*/

#include "string.h"
#include "sparkplugb_arduino.hpp"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

//----------------------------------------------------------------------------//
//                               Encoder
//----------------------------------------------------------------------------//
sparkplugb_arduino_encoder::sparkplugb_arduino_encoder(){
  this->clear_payload();
}

// perform the encoding using the object's payload data
size_t sparkplugb_arduino_encoder::encode(uint8_t **buffer,
                  size_t buffer_length)
{
  size_t message_length;
  bool node_status;
  // Create the stream
  pb_ostream_t node_stream = pb_ostream_from_buffer(*buffer, buffer_length);
  node_status = pb_encode(&node_stream, org_eclipse_tahu_protobuf_Payload_fields, &this->payload);
  message_length = node_stream.bytes_written;

  if (!node_status)
    message_length = -1;

  return message_length;
}

// if we have metrics, use the memory allocated.. if not, set to NULL
void sparkplugb_arduino_encoder::set_has_metrics(bool flag){
  if(flag){
    this->payload.metrics = this->metrics;
  }
  else{
    this->payload.metrics = NULL;
  }
}

// zero out payload and all metrics
void sparkplugb_arduino_encoder::clear_payload(){
  int i;
  this->payload = org_eclipse_tahu_protobuf_Payload_init_zero;
  for(i=0; i<SPB_ARDUINO_METRICS_OUT_SIZE; i++){
    this->metrics[i] = org_eclipse_tahu_protobuf_Payload_Metric_init_zero;
  }
}


//----------------------------------------------------------------------------//
//                               Decoder
//----------------------------------------------------------------------------//
sparkplugb_arduino_decoder::sparkplugb_arduino_decoder(){
  this->payload = org_eclipse_tahu_protobuf_Payload_init_zero;
}

// perform the decode and save to payload
bool sparkplugb_arduino_decoder::decode(const pb_byte_t *binary_payload,
                  size_t binary_payloadlen)
{
  pb_istream_t node_stream = pb_istream_from_buffer(binary_payload, binary_payloadlen);
	const bool decode_result = pb_decode(&node_stream, org_eclipse_tahu_protobuf_Payload_fields, &this->payload);

  if(!decode_result){
    return false;
  }

  return (node_stream.bytes_left == 0);
}

// free dynamiclly alloated memory and zero payload data
void sparkplugb_arduino_decoder::free_payload(){
  pb_release(org_eclipse_tahu_protobuf_Payload_fields, &this->payload);
  this->payload = org_eclipse_tahu_protobuf_Payload_init_zero;
}
