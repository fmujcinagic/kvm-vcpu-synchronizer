obj-m += sim.o
# replace with the kernel location on your system
KDIR := /home/faris/Downloads/linux-7.1.3 

all:
        make -C $(KDIR) M=$(PWD) modules
clean:
        make -C $(KDIR) M=$(PWD) clean
