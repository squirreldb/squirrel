# Copyright (c) 2015, squirreldb. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include depends.mk

# OPT ?= -O2        # (A) Production use (optimized mode)
OPT ?= -g2      # (B) Debug mode, w/ full line-level debugging symbols
# OPT ?= -O2 -g2  # (C) Profiling mode: opt, but w/debugging symbols

CXX=g++
INCPATH=-I. -I$(SOFA_PBRPC)/include -I$(BOOST_HEADER_DIR) -I$(PROTOBUF_DIR)/include \
		-I$(SNAPPY_DIR)/include -I$(ZLIB_DIR)/include -I$(COMMON_INC) -I$(LEVELDB_DIR)/include

CXXFLAGS += $(OPT) -pipe -W -Wall -fPIC -D_GNU_SOURCE -D__STDC_LIMIT_MACROS $(INCPATH)

LIBRARY=$(SOFA_PBRPC)/lib/libsofa-pbrpc.a $(PROTOBUF_DIR)/lib/libprotobuf.a \
		$(SNAPPY_DIR)/lib/libsnappy.a $(COMMON_LIB)/libcommon.a $(LEVELDB_DIR)/lib/libleveldb.a
LDFLAGS += -L$(ZLIB_DIR)/lib -lrt -lz -lpthread

PROTO_FILE := $(wildcard src/proto/*.proto)
PROTO_SRC := $(PROTO_FILE:.proto=.pb.cc)
PROTO_OBJ := $(PROTO_FILE:.proto=.pb.o)
PROTO_OPTIONS := --proto_path=. --proto_path=$(SOFA_PBRPC)/include --proto_path=$(PROTOBUF_DIR)/include

CONF_SRC := conf.cc
SERVER_SRC := $(wildcard src/server/*.cc) $(CONF_SRC)
SDK_SRC := $(wildcard src/sdk/*.cc) $(CONF_SRC)
DB_SRC := $(wildcard src/db/*.cc) $(CONF_SRC)
UTIL_SRC := $(wildcard src/util/*.cc)
TEST_SRC := $(wildcard src/test/*.cc) $(CONF_SRC)
CLI_SRC := src/cli.cc

SERVER_OBJ := $(SERVER_SRC:.cc=.o)
SDK_OBJ := $(SDK_SRC:.cc=.o)
DB_OBJ := $(DB_SRC:.cc=.o)
UTIL_OBJ := $(UTIL_SRC:.cc=.o)
TEST_OBJ := $(TEST_SRC:.cc=.o)
CLI_OBJ := $(CLI_SRC:.cc=.o)

ALL_OBJ := $(SERVER_OBJ) $(SDK_OBJ) $(DB_OBJ) $(UTIL_OBJ) $(TEST_OBJ) $(CLI_OBJ)

BIN := squirrel_server driver cli

all: $(BIN) $(PROTO_SRC)
	@echo 'Done'

.PHONY: clean

clean:
	@rm -f $(BIN) $(ALL_OBJ) src/proto/*.pb.*

rebuild: clean all

squirrel_server: $(PROTO_OBJ) $(SERVER_OBJ) $(DB_OBJ) $(UTIL_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

squirrel_sdk: $(PROTO_OBJ) $(SDK_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

driver: $(PROTO_OBJ) $(SDK_OBJ) $(TEST_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

cli: $(PROTO_OBJ) $(SDK_OBJ) $(CLI_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

%.pb.cc: %.proto
	$(PROTOBUF_DIR)/bin/protoc $(PROTO_OPTIONS) --cpp_out=. $<

%.pb.h: %.proto
	$(PROTOBUF_DIR)/bin/protoc $(PROTO_OPTIONS) --cpp_out=. $<


$(PROTO_OBJ): %.o: %.cc %.h $(PROTO_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@
