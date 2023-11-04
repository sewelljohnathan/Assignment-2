obj-m += lkmasg2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc test.c -o test
	sudo insmod lkmasg2.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test
	sudo rmmod lkmasg2
