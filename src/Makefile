TOPDIR:=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))/
CFLAGS=-O3 -g
CFLAGS+= -I$(TOPDIR)include -I$(TOPDIR)../../cybozulib/include/ -I$(TOPDIR)../../mcl/include -I$(TOPDIR)../../xbyak/ -I$(TOPDIR)../include/
LDFLAGS+= -lgmp -lgmpxx -lcrypto -lmcl -L../../mcl/lib/
SERVER_OBJS=rot.o cpbwt.o comm.o server.o
CLIENT_OBJS=rot.o cpbwt.o comm.o client.o
CPBWT_OBJS=rot.o cpbwt.o cpbwt_main.o

MCL_LIB=../../mcl/lib/libmcl.a
all: server client cpbwt $(MCL_LIB)
../../mcl/lib/libmcl.a:
	make -C ../../mcl USE_LLVM=0
clean:
	rm -rf server client $(SERVER_OBJS) $(CLIENT_OBJS) *~

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -c $< -o $@ $(CFLAGS) -fopenmp

cpbwt: $(CPBWT_OBJS) $(MCL_LIB)
	$(CXX) $(CPBWT_OBJS) -o $@ $(LDFLAGS) -fopenmp
	mv cpbwt $(TOPDIR)../bin/

server: $(SERVER_OBJS) $(MCL_LIB)
	$(CXX) $(SERVER_OBJS) -o $@ $(LDFLAGS) -fopenmp
	mkdir -p $(TOPDIR)../bin/
	mv server $(TOPDIR)../bin/

client: $(CLIENT_OBJS) $(MCL_LIB)
	$(CXX) $(CLIENT_OBJS) -o $@ $(LDFLAGS) -fopenmp
	mkdir -p $(TOPDIR)../bin/
	mv client $(TOPDIR)../bin/
