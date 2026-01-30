#!/usr/bin/env bash

protoc --cpp_out=./ lib/libtabnet/src/protobuf/transfere_protocol.proto
