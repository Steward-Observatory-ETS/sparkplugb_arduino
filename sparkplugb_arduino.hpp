/********************************************************************************
 * Copyright 2020 Steward Observatory
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Steward Observatory - Port to C++ & modifications for Teensy 4.1 usage
 ********************************************************************************/
/*
This project is a port of Eclipse Tahu sutable for a microcontroller, such as
the Teensy 4.1.
2020 M. Sibayan, Steward Observatory
*/
#ifndef __SPARKPLUGB_ARDUINO_H__
#define __SPARKPLUGB_ARDUINO_H__
#include "tahu.pb.h"

//----------------------------------------------------------------------------//
// Constants
// copied from tahu.h, Copyright (c) 2014-2019 Cirrus Link Solutions and others
#define DATA_SET_DATA_TYPE_UNKNOWN 0
#define DATA_SET_DATA_TYPE_INT8 1
#define DATA_SET_DATA_TYPE_INT16 2
#define DATA_SET_DATA_TYPE_INT32 3
#define DATA_SET_DATA_TYPE_INT64 4
#define DATA_SET_DATA_TYPE_UINT8 5
#define DATA_SET_DATA_TYPE_UINT16 6
#define DATA_SET_DATA_TYPE_UINT32 7
#define DATA_SET_DATA_TYPE_UINT64 8
#define DATA_SET_DATA_TYPE_FLOAT 9
#define DATA_SET_DATA_TYPE_DOUBLE 10
#define DATA_SET_DATA_TYPE_BOOLEAN 11
#define DATA_SET_DATA_TYPE_STRING 12
#define DATA_SET_DATA_TYPE_DATETIME 13
#define DATA_SET_DATA_TYPE_TEXT 14

#define METRIC_DATA_TYPE_UNKNOWN 0
#define METRIC_DATA_TYPE_INT8 1
#define METRIC_DATA_TYPE_INT16 2
#define METRIC_DATA_TYPE_INT32 3
#define METRIC_DATA_TYPE_INT64 4
#define METRIC_DATA_TYPE_UINT8 5
#define METRIC_DATA_TYPE_UINT16 6
#define METRIC_DATA_TYPE_UINT32 7
#define METRIC_DATA_TYPE_UINT64 8
#define METRIC_DATA_TYPE_FLOAT 9
#define METRIC_DATA_TYPE_DOUBLE 10
#define METRIC_DATA_TYPE_BOOLEAN 11
#define METRIC_DATA_TYPE_STRING 12
#define METRIC_DATA_TYPE_DATETIME 13
#define METRIC_DATA_TYPE_TEXT 14
#define METRIC_DATA_TYPE_UUID 15
#define METRIC_DATA_TYPE_DATASET 16
#define METRIC_DATA_TYPE_BYTES 17
#define METRIC_DATA_TYPE_FILE 18
#define METRIC_DATA_TYPE_TEMPLATE 19

#define PARAMETER_DATA_TYPE_UNKNOWN 0
#define PARAMETER_DATA_TYPE_INT8 1
#define PARAMETER_DATA_TYPE_INT16 2
#define PARAMETER_DATA_TYPE_INT32 3
#define PARAMETER_DATA_TYPE_INT64 4
#define PARAMETER_DATA_TYPE_UINT8 5
#define PARAMETER_DATA_TYPE_UINT16 6
#define PARAMETER_DATA_TYPE_UINT32 7
#define PARAMETER_DATA_TYPE_UINT64 8
#define PARAMETER_DATA_TYPE_FLOAT 9
#define PARAMETER_DATA_TYPE_DOUBLE 10
#define PARAMETER_DATA_TYPE_BOOLEAN 11
#define PARAMETER_DATA_TYPE_STRING 12
#define PARAMETER_DATA_TYPE_DATETIME 13
#define PARAMETER_DATA_TYPE_TEXT 14

#define PROPERTY_DATA_TYPE_UNKNOWN 0
#define PROPERTY_DATA_TYPE_INT8 1
#define PROPERTY_DATA_TYPE_INT16 2
#define PROPERTY_DATA_TYPE_INT32 3
#define PROPERTY_DATA_TYPE_INT64 4
#define PROPERTY_DATA_TYPE_UINT8 5
#define PROPERTY_DATA_TYPE_UINT16 6
#define PROPERTY_DATA_TYPE_UINT32 7
#define PROPERTY_DATA_TYPE_UINT64 8
#define PROPERTY_DATA_TYPE_FLOAT 9
#define PROPERTY_DATA_TYPE_DOUBLE 10
#define PROPERTY_DATA_TYPE_BOOLEAN 11
#define PROPERTY_DATA_TYPE_STRING 12
#define PROPERTY_DATA_TYPE_DATETIME 13
#define PROPERTY_DATA_TYPE_TEXT 14
//----------------------------------------------------------------------------//

/*
@brief Encoder for Sparkplug B MQTT protocol
*/
class sparkplugb_arduino_encoder{
public:
  // space to store payload data
  org_eclipse_tahu_protobuf_Payload* payload;

  // constructor
  sparkplugb_arduino_encoder();

  /*
  @brief Assigns object pointer to the payload
  @param payload pointer to the payload struct

  This function assigns the payload pointer
  */
  void set_payload(org_eclipse_tahu_protobuf_Payload* payload);

  /*
  @brief add metrics to the payload (assigns the pointer & sets the number)
  @param metrics pointer to the array of metrics
  @param count length of the array of metrics

  This helper function assigns payload.metrics and payload.metrics_count
  */
  bool set_metrics(org_eclipse_tahu_protobuf_Payload_Metric* metrics, int count);

  /*
  @brief perform an encode
  @param buffer buffer to store encoded binary data
  @param buffer_length size of the buffer

  This function encodes the payload in to the binary buffer provided as an arg.
  It basically creates a stream and calls pb_encode.
  */
  size_t encode(uint8_t *buffer,
              size_t buffer_length);

  /*
  @brief perform an encode
  @param buffer buffer to store encoded binary data
  @param buffer_length size of the buffer

  This function encodes the payload in to the binary buffer provided as an arg.
  It basically creates a stream and calls pb_encode.
  */
  size_t encode(org_eclipse_tahu_protobuf_Payload* payload, uint8_t* buffer,
              size_t buffer_length);
  /*
  @brief clear (zeros) the payload and metric data
  */
  void clear_payload();
private:
};

/*
@brief Decoder for Sparkplug B MQTT protocol
*/
class sparkplugb_arduino_decoder{
public:
  // space to store payload data
  org_eclipse_tahu_protobuf_Payload payload;

  sparkplugb_arduino_decoder(); // constructor

  /*
  @brief perform a decode
  @param binary_payload inbound encoded binary data
  @param binary_payloadlen size of the binary payload data

  This function decodes the payload in to payload.
  It basically creates a stream and calls pb_decode.
  */
  bool decode(const pb_byte_t *binary_payload, size_t binary_payloadlen);

  /*
  @brief free the payload's dynamiclly allocated memory and zero the payload.

  This function basically calls pb_release and sets the payload data to zero.
  */
  void free_payload();
private:
};
#endif
