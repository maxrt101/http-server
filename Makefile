# HTTP Server

CXX      := g++
CFLAGS   := -Isrc/ -I/usr/local/opt/openssl/include/ -Lbin/
CXXFLAGS := $(CFLAGS) -std=c++17
LDFLAGS  := -lpthread -lhttpserver -lmrt
# /usr/local/opt/openssl/lib/libssl.dylib /usr/local/opt/openssl/lib/libcrypto.dylib /usr/lib/libxml2.dylib

.PHONY: server

server:
	make -C src

app:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) src/main.cc -o bin/server
