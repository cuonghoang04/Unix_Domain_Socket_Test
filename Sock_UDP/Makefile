KDIR = /lib/modules/`uname -r`/build
obj-m        = kernel_module_client.o kernel_module_server.o

all:  dir binary module
	rm -f *.o

dir:
	mkdir -p ../bin/

binary: app_client app_server

app_client: app_client.cpp socket_path.h
	g++ -o ../bin/app_client app_client.cpp

app_server: app_server.cpp socket_path.h
	g++ -o ../bin/app_server app_server.cpp

module: kernel_module_client.o kernel_module_server.o
	mv kernel_module_client.ko ../bin/kernel_module_client.ko
	mv kernel_module_server.ko ../bin/kernel_module_server.ko

kernel_module_client.o: kernel_module_client.c socket_path.h
	make -C $(KDIR) M=`pwd`

kernel_module_server.o: kernel_module_server.c socket_path.h
	make -C $(KDIR) M=`pwd`
