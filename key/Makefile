#Makefile
# Created on: May 11, 2017
#     Author: haobo
KVERS =~/Desktop/current_work_dir/linux_kernel_src/2440_board_kernel/

obj-m +=key_sem.o

all:kernel_modules
kernel_modules:
	make -C $(KVERS) M=$(CURDIR) modules
	cp ./*.ko  ~/NFS/2440board/
clean:
	make -C $(KVERS) M=$(CURDIR) clean
