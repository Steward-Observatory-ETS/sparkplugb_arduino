Arduino port of Eclipse Tahu implementation of Sparkplug B encoding/decoding

Currently this library only supports VERY SIMPLE payloads with simple metrics.
The encoder supports more than the decoder, since we can more easily control
what we are sending outbound.

This project is based on Eclipse Tahu ( https://github.com/eclipse/tahu ) and
uses source from /client_libraries/c

All Tahu code is Copyright (c) 2014-2019 Cirrus Link Solutions and others
and released under the Eclipse Public License 2.0 which is available at
http://www.eclipse.org/legal/epl-2.0

### TODO

1. Error checking for writing too many metrics
1. Error checking for writing strings that are too long.
1. Add support for string data
1. Separate Encode and Decode to save space if not using both

### Encoder

This library allocates payload memory based on the following macros:
1. SPB_ARDUINO_METRICS_OUT_SIZE, array size of outbound metrics.

If we're using the encoder and our payload has metrics, we need to allocate
space for each metric. Set SPB_ARDUINO_METRICS_OUT_SIZE to the maximum metrics
count to be used by the encoder.

In order to provide a timestamp and sequence number, the user must update the
payload and/or metric fields directly. These fields are:
1. encoder.payload.timestamp, message time stamp
1. encoder.payload.seq, message sequence number
1. encoder.payload.metrics[i].timestamp, metric time stamp

The user must also set the has_seq and/or has_timestamp fields:
1. encoder.payload.has_timestamp, true if the payload has a timestamp
1. encoder.payload.has_seq, true if the payload has a sequence number
2. encoder.payload.metric[i].has_timestamp, true if the metric has a timestamp

### Decoder

1. SPB_ARDUINO_METRICS_IN_SIZE, array size of inbound metrics.
1. SPB_ARDUINO_METRIC_NAME_SIZE, inbound metric name array size.

If we're using the decoder and our payload has metrics, we need to allocate
space for each metric. Set SPB_ARDUINO_METRICS_IN_SIZE to the maximum metrics
count to be used by the decoder.

If the inbound metrics have names, we need to allocate space for them. Set
SPB_ARDUINO_METRIC_NAME_SIZE to the expected maximum name length.
