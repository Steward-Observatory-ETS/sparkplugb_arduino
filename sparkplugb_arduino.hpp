#ifndef __SPARKPLUGB_ARDUINO_H__
#define __SPARKPLUGB_ARDUINO_H__
#include "tahu.pb.h"

#ifndef SPB_ARDUINO_METRICS_IN_SIZE
#define SPB_ARDUINO_METRICS_IN_SIZE 1
#endif
#ifndef SPB_ARDUINO_METRICS_OUT_SIZE
#define SPB_ARDUINO_METRICS_OUT_SIZE 1
#endif
#ifndef SPB_ARDUINO_METRIC_NAME_SIZE
#define SPB_ARDUINO_METRIC_NAME_SIZE 48
#endif


// Constants
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

class sparkplugb_arduino_encoder{
public:
  org_eclipse_tahu_protobuf_Payload payload_in;
  org_eclipse_tahu_protobuf_Payload payload_out;
  org_eclipse_tahu_protobuf_Payload_Metric metrics_in[SPB_ARDUINO_METRICS_IN_SIZE];
  org_eclipse_tahu_protobuf_Payload_Metric metrics_out[SPB_ARDUINO_METRICS_OUT_SIZE];
  char metric_in_names[SPB_ARDUINO_METRICS_IN_SIZE][SPB_ARDUINO_METRIC_NAME_SIZE];

  sparkplugb_arduino();
  void set_time(uint64_t time);
  size_t encode(uint8_t **buffer,
              size_t buffer_length);

  /* Decode an inbound payload*/
  bool decode(const void *binary_payload,
              int binary_payloadlen);

  void clear_payload_in();
  void clear_payload_out();
private:
  uint64_t time;
  uint64_t seq;
  bool decode_metric(org_eclipse_tahu_protobuf_Payload_Metric *metric,
                    pb_istream_t *stream, int index);

};

#endif
