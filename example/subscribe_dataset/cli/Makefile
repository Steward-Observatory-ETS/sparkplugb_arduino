# Copyright 2019
# Steward Observatory Engineering & Technical Services, University of Arizona
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE. See the GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.

LIBS = -Llib -L/usr/local/lib -lmosquitto
CC = gcc
CFLAGS = -g -Wall -lm

.PHONY: clean client

client:
	$(CC) ../../../pb_common.c ../../../pb_decode.c ../../../pb_encode.c \
				../../../tahu.pb.c -I../../../ -I../../../exclude \
		dataset_client.c -D__TEST_CLIENT__=1 -o publish_dataset $(CFLAGS) $(LIBS)

clean:
	-rm -f publish_dataset
