obj-m = driver5.o
 
KDIR = /lib/modules/`uname -r`/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
