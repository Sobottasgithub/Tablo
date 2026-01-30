#!/usr/bin/env bash

protoc --cpp_out=./ lib/libtabnet/src/protobuf/transfer_protocol.proto
