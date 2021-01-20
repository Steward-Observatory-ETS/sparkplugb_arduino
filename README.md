Eclipse Tahu implementation of Sparkplug B with some simplified helpers sutible
for Arduino.

This project is based on Eclipse Tahu ( https://github.com/eclipse/tahu ) and
uses source from /client_libraries/c

All Tahu code is Copyright (c) 2014-2019 Cirrus Link Solutions and others
and released under the Eclipse Public License 2.0 which is available at
http://www.eclipse.org/legal/epl-2.0

### sparkplugb_arduino_encoder

This class aims to help encode payloads that will be published by the client
application. Only the payload struct is allocated by this class, and the
developer is responsible for allocating metric, string, data, etc. memory.

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

The decoder uses pb_decode() which dynamically allocates memory as necessary.
Base payload data is stored in decoder.payload, which can be read directly from
the client program. The pb_decode function will build out the necessary structs
for metrics, datasets, strings, etc. Special care must be taken to properly free
memory after use using pb_release() or decoder.free_payload() as appropriate.

### TODO

1. Add helper functions
