obj-m += sukolsky_hwk5.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules \

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
