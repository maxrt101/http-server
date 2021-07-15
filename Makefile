# HTTP Server

export TOPDIR   := $(shell pwd)
export BINDIR   := $(TOPDIR)/bin

export CXX      := g++
export CFLAGS   := -I$(TOPDIR)/src -I/usr/local/opt/openssl/include/
export CXXFLAGS := $(CFLAGS) -std=c++17
export LDFLAGS  := -lpthread -lhttpserver -lmrt
# /usr/local/opt/openssl/lib/libssl.dylib /usr/local/opt/openssl/lib/libcrypto.dylib /usr/lib/libxml2.dylib

LIBDIRS         :=  -L$(BINDIR)

.PHONY: server

server:
	mkdir -p $(BINDIR)
	make -C src

mrt:
	mkdir -p $(BINDIR)
	make -C src/mrt
	cp src/mrt/bin/libmrt.a $(BINDIR)/

app: mrt
	$(CXX) $(CXXFLAGS) $(LIBDIRS) $(LDFLAGS) src/main.cc -o bin/server

clean:
	make -C src clean
	make -C src/mrt clean
