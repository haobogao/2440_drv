#Makefile
# Created on: May 9, 2017
#     Author: haobo
KVERS =/home/haobo/Desktop/current_work_dir/linux_kernel_src/2440_board_kernel/

obj-m +=led_dev_v2.o

all:kernel_modules
kernel_modules:
	make -C $(KVERS) M=$(CURDIR) modules
	cp ./*.ko  ~/NFS/2440board/
clean:
	make -C $(KVERS) M=$(CURDIR) clean
