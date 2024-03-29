subdir = ./
 
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
 
SOURCES = $(wildcard $(subdir)*.cc)
SRCOBJS = $(patsubst %.cc,%.o,$(SOURCES))
CC = g++
 
%.o:%.cc
	$(CC) -std=c++11 -I/usr/local/include -pthread -lrocksdb -c $< -o $@
 
all: client server
 
client:	protoc/rpc.grpc.pb.o protoc/rpc.pb.o client.o submodule/encryption.o submodule/tools.o
	$(CC) $^ -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -lrocksdb -ldl -lssl -o $@
 
server:	protoc/rpc.grpc.pb.o protoc/rpc.pb.o server.o submodule/encryption.o submodule/tools.o
	$(CC) $^ -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -lrocksdb -ldl -lssl -o $@
#chmod 777 $@

clean:
	rm *.o