# HTTP Server

NAME      := httpserver
LIBNAME   := lib$(NAME).a
CXXFLAGS  += -c
OBJS      := server/server.o server/debug/log.o server/utils/die.o server/http/request.o server/http/response.o server/http/parser.o server/sockets/socket.o server/sockets/bsd_socket.o server/server/handlers.o server/server/http_server.o

.PHONY: all

all: server

server: $(OBJS)
	rm -f $(BINDIR)/$(LIBNAME)
	$(AR) cr $(BINDIR)/$(LIBNAME) $(OBJS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm $(OBJS)
