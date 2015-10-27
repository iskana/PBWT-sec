TOPDIR:=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))/
CFLAGS=-O3
CFLAGS+= -I$(TOPDIR)include -I$(TOPDIR)../../cybozulib/include/ -I$(TOPDIR)../../mie/include -I$(TOPDIR)../../xbyak/ -I$(TOPDIR)../../Lifted-ElGamal/include/ -I$(TOPDIR)../include/
LDFLAGS+= -lgmp -lgmpxx -lcrypto
SERVER_OBJS=rot.o cpbwt.o comm.o server.o
CLIENT_OBJS=rot.o cpbwt.o comm.o client.o
CPBWT_OBJS=rot.o cpbwt.o cpbwt_main.o

all: server client cpbwt

clean:
	rm server client $(SERVER_OBJS) $(CLIENT_OBJS) *~

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -c $< -o $@ $(CFLAGS) -fopenmp

cpbwt: $(CPBWT_OBJS)
	$(CXX) $(CPBWT_OBJS) -o $@ $(LDFLAGS) -fopenmp
	mv cpbwt $(TOPDIR)../bin/

server: $(SERVER_OBJS)
	$(CXX) $(SERVER_OBJS) -o $@ $(LDFLAGS) -fopenmp
	mv server $(TOPDIR)../bin/

client: $(CLIENT_OBJS)
	$(CXX) $(CLIENT_OBJS) -o $@ $(LDFLAGS) -fopenmp
	mv client $(TOPDIR)../bin/
