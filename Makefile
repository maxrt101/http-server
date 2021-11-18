# HTTP Server

export TOPDIR     := $(shell pwd)
export PREFIX     ?= $(TOPDIR)/build

export CXX        := g++
export CFLAGS     := -I$(TOPDIR)/src -I$(PREFIX)/include
export CXXFLAGS   := -std=c++17
export LDFLAGS    := -L$(PREFIX)/lib -lpthread -lhttpserver -lmrt

.PHONY: build

ifeq ("$(DEBUG)","1")
export CFLAGS += -D_DEBUG -g
endif

build: dependencies 
	$(info Building HttpServer)
	make -C src

dependencies: prepare
	make -C src/mrt PREFIX=$(PREFIX)

prepare:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/lib

app:
	mkdir -p $(PREFIX)/bin
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) src/main.cc -o $(PREFIX)/bin/server

clean:
	make -C src clean
	make -C src/mrt clean

$(V).SILENT:

