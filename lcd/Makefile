#Makefile
# Created on: May 9, 2017
#     Author: haobo
KVERS =/home/haobo/linux_2.26_eclipse_project/

obj-m +=lcd_drv.o

all:kernel_modules
kernel_modules:
	make -C $(KVERS) M=$(CURDIR) modules
	cp ./*.ko  ~/NFS/2440board/
clean:
	make -C $(KVERS) M=$(CURDIR) clean
