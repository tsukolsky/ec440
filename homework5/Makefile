obj-m += sukolsky_hwk5.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules \
	set
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  \ 

set:
	sudo insmod ./sukolsky_hwk5.ko   
	sudo mknod /dev/morse-communicator c 69 0 


