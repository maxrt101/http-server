# HTTP Server

CXX      := g++
CFLAGS   := -c -I./ -I/usr/local/opt/openssl/include/
CXXFLAGS := $(CFLAGS) -std=c++17

NAME      := httpserver
LIBNAME   := lib$(NAME).a
BINDIR    := ../bin
OBJS      := server/server.o server/debug/log.o server/utils/die.o server/http/request.o server/http/response.o server/http/parser.o server/sockets/socket.o server/sockets/bsd_socket.o server/server/handlers.o server/server/http_server.o

.PHONY: all

all: server

server: mrt $(OBJS)
	mkdir -p $(BINDIR)
	rm -f $(BINDIR)/$(LIBNAME)
	$(AR) cr $(BINDIR)/$(LIBNAME) $(OBJS)

debug: mrt $(OBJS)
	mkdir -p $(BINDIR)
	rm -f $(BINDIR)/$(LIBNAME)
	$(AR) cr $(BINDIR)/$(LIBNAME) $(OBJS)


$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) $< -o $@

mrt:
	mkdir -p bin
	make -C src/mrt
	cp src/mrt/bin/libmrt.a bin/libmrt.a

clean:
	rm $(OBJS)
