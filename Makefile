obj-m += lkmasg2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc unittest.c -o unittest

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm unittest
