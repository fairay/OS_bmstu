obj-m += md1.o
obj-m += md2.o
obj-m += md3.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
