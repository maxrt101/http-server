# HTTP Server

NAME        := httpserver
LIBNAME     := lib$(NAME).a
INSTALLDIR  := $(PREFIX)/include/mrt/server
CFLAGS      += -c -I$(PREFIX)/include
OBJS        := server/server.o server/debug/log.o server/utils/die.o server/http/request.o server/http/response.o server/http/parser.o server/sockets/socket.o server/sockets/bsd_socket.o server/server/handlers.o server/server/http_server.o

.PHONY: all

all: build

build: install $(OBJS)
	$(info Building $(LIBNAME))
	rm -f $(PREFIX)/lib/$(LIBNAME)
	$(AR) cr $(PREFIX)/lib/$(LIBNAME) $(OBJS)

$(OBJS): %.o : %.cc
	$(info Building $@)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $< -o $@

install: prepare
	$(info Installing Headers)
	mkdir -p $(INSTALLDIR)
	mkdir -p $(INSTALLDIR)/config
	mkdir -p $(INSTALLDIR)/debug
	mkdir -p $(INSTALLDIR)/http
	mkdir -p $(INSTALLDIR)/server
	mkdir -p $(INSTALLDIR)/sockets
	mkdir -p $(INSTALLDIR)/utils
	cp server/*.h $(INSTALLDIR)/
	cp server/config/*.h $(INSTALLDIR)/config
	cp server/debug/*.h $(INSTALLDIR)/debug
	cp server/http/*.h $(INSTALLDIR)/http
	cp server/server/*.h $(INSTALLDIR)/server
	cp server/sockets/*.h $(INSTALLDIR)/sockets
	cp server/utils/*.h $(INSTALLDIR)/utils

prepare:
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/lib

clean:
	rm -rf $(OBJS)

$(V).SILENT:

