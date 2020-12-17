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
 - remove time calls in get_current_timestamp
 + added function set_tahu_time to allow setting timestamp
 + changed to pre-allocated memory for metrics, limits size to pre-defined
*/

//#include <cstring>
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
  this->clear_payload();
}

bool sparkplugb_arduino_decoder::decode(const void *binary_payload,
                  int binary_payloadlen)
{
  // Local vars for payload decoding
	bool status;
	pb_wire_type_t payload_wire_type;
	uint32_t payload_tag;
	bool payload_eof;
	const pb_field_t *payload_field;
  pb_istream_t stream;
//  int metric_index = 0;
  org_eclipse_tahu_protobuf_Payload_Metric *metric;

  this->payload.metrics_count = 0;
  stream = pb_istream_from_buffer((const pb_byte_t *)binary_payload, binary_payloadlen);

  // Loop over blocks while decoding portions of the payload
	while (pb_decode_tag(&stream, &payload_wire_type, &payload_tag, &payload_eof)) {
    if (payload_wire_type == PB_WT_VARINT) {
      for (payload_field = org_eclipse_tahu_protobuf_Payload_fields; payload_field->tag != 0; payload_field++) {
        if (payload_field->tag == payload_tag && (((payload_field->type & PB_LTYPE_VARINT) == PB_LTYPE_VARINT) ||
                    ((payload_field->type & PB_LTYPE_UVARINT) == PB_LTYPE_UVARINT))) {
          uint64_t dest;
          status = pb_decode_varint(&stream, &dest);
          if (status) {
          } else {
            return false;
          }

          if (payload_field->tag == org_eclipse_tahu_protobuf_Payload_timestamp_tag) {
            this->payload.has_timestamp = true;
            this->payload.timestamp = dest;
          } else if (payload_field->tag == org_eclipse_tahu_protobuf_Payload_seq_tag) {
            this->payload.has_seq = true;
            this->payload.seq = dest;
          }
        } else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_SVARINT) == PB_LTYPE_SVARINT)) {
          int64_t dest;
          status = pb_decode_svarint(&stream, &dest);
          if (status) {
          } else {
            return false;
          }
        }
      }
    }else if (payload_wire_type == PB_WT_STRING) {
			for (payload_field = org_eclipse_tahu_protobuf_Payload_fields; payload_field->tag != 0; payload_field++) {
				if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_SUBMESSAGE) == PB_LTYPE_SUBMESSAGE)) {
					// This is a metric!
					if (payload_field->ptr == NULL) {
						return false;
					}

          if(this->payload.metrics_count >= SPB_ARDUINO_METRICS_IN_SIZE)
            return false; // we've exceeded number of allowed metrics

					metric = &this->metrics[this->payload.metrics_count];
          *metric = org_eclipse_tahu_protobuf_Payload_Metric_init_zero;

					if(this->decode_metric(metric, &stream, this->payload.metrics_count)) {
            this->payload.metrics_count++;
					} else {
						return false;
					}
				} /*else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_STRING) == PB_LTYPE_STRING)) {
					// Get the UUID
					pb_byte_t string_size[1];
					status = pb_read(&stream, string_size, 1);
					if (!status) {
						return false;
					}

					pb_byte_t dest[string_size[0]+1];
					status = pb_read(&stream, dest, string_size[0]);
					if (status) {
						dest[string_size[0]] = '\0';
						payload->uuid = (char *)malloc((strlen(dest)+1)*sizeof(char));
						strcpy(payload->uuid, dest);
					} else {
						return false;
					}
				}*/
			}
		}
  }
  if(this->payload.metrics_count > 0)
    this->payload.metrics = this->metrics;
  return true;
}

bool sparkplugb_arduino_decoder::decode_metric(org_eclipse_tahu_protobuf_Payload_Metric *metric, pb_istream_t *stream, int index) {
	bool status;
	pb_istream_t substream;
  pb_wire_type_t metric_wire_type;
	uint32_t metric_tag;
	bool metric_eof;
	const pb_field_t *metric_field;

  if (!pb_make_string_substream(stream, &substream)) {
		return false;
	}

  while (pb_decode_tag(&substream, &metric_wire_type, &metric_tag, &metric_eof)) {
    if (metric_wire_type == PB_WT_VARINT) {
			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_VARINT) == PB_LTYPE_VARINT) ||
													((metric_field->type & PB_LTYPE_UVARINT) == PB_LTYPE_UVARINT))) {
					uint64_t dest;
					status = pb_decode_varint(&substream, &dest);
					if (status) {
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_alias_tag) {
							metric->has_alias = true;
							metric->alias = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_timestamp_tag) {
							metric->has_timestamp = true;
							metric->timestamp = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_datatype_tag) {
							metric->has_datatype = true;
							metric->datatype = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_is_historical_tag) {
							metric->has_is_historical = true;
							metric->is_historical = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_is_transient_tag) {
							metric->has_is_transient = true;
							metric->is_transient = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_is_null_tag) {
							metric->has_is_null = true;
							metric->is_null = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
							metric->value.int_value = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
							metric->value.long_value = dest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag;
							metric->value.boolean_value = dest;
						}
					} else {
						return false;
					}
				} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_SVARINT) == PB_LTYPE_SVARINT)) {
					int64_t dest;
					status = pb_decode_svarint(&substream, &dest);
					if (!status) {
						return false;
					}
				}
			}
		} else if (metric_wire_type == PB_WT_32BIT) {
			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_FIXED32) == PB_LTYPE_FIXED32))) {
					uint32_t dest;
					status = pb_decode_fixed32(&substream, &dest);
					if (status) {
						float destination_float = *((float*)&dest);
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag;
							metric->value.float_value = destination_float;
						}
					}
				}
			}
		} else if (metric_wire_type == PB_WT_64BIT) {
			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_FIXED64) == PB_LTYPE_FIXED64))) {
					uint64_t dest;
					status = pb_decode_fixed64(&substream, &dest);
					if (status) {
						double destination_double = *((double*)&dest);
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag;
							metric->value.double_value = destination_double;
						}
					}
				}
			}

		} else if (metric_wire_type == PB_WT_STRING) {

			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_STRING) == PB_LTYPE_STRING)) {

					// Get the string size
					pb_byte_t string_size[1];
					status = pb_read(&substream, string_size, 1);
					if (!status) {
						return false;
					}

					pb_byte_t dest[string_size[0]+1];
					status = pb_read(&substream, dest, string_size[0]);
					if (status) {
						dest[string_size[0]] = '\0';

						// This is either the metric name or string value
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_name_tag) {
              // this assumes metric_count is equal to the index of metric being decoded
              metric->name = metric_names[index];
							strcpy((char*)metric->name, (char*)dest);
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag) {

              // string data not supported...
              // we need to figure out how to handle memory to support strings...
              return false;
              /*metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag;
							// JPL 04/05/17... I hope this gets FREE(string_value)'d somewhere
							metric->value.string_value =(char *)malloc((strlen(dest)+1)*sizeof(char));
							strcpy(metric->value.string_value, dest );
							// JPL 04/05/17... local memory?
							//	metric->value.string_value = dest;
              */
						}
					} else {
						return false;
					}
				}
			}

		}
	}

	// Close the substream
	pb_close_string_substream(stream, &substream);
  return true;
}

void sparkplugb_arduino_decoder::clear_payload(){
  int i;
  this->payload = org_eclipse_tahu_protobuf_Payload_init_zero;
  for(i=0; i<SPB_ARDUINO_METRICS_IN_SIZE; i++){
    this->metrics[i] = org_eclipse_tahu_protobuf_Payload_Metric_init_zero;
    this->metric_names[i][0] = '\0';
  }
}
