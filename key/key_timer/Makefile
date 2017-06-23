#Makefile
# Created on: May 11, 2017
#     Author: haobo
#KVERS =~/Desktop/current_work_dir/linux_kernel_src/2440_board_kernel/
KVERS =~/Desktop/current_work_dir/linux_kernel_src/2440_board_kernel/
<<<<<<< HEAD:key/key_timer/Makefile
=======

>>>>>>> b85b32930570caa21bdf4a79389ca0de29f47420:key/key_timer/Makefile
obj-m +=key_timer_drv.o

all:kernel_modules
kernel_modules:
	make -C $(KVERS) M=$(CURDIR) modules
	cp ./*.ko  ~/NFS/2440board/
clean:
	make -C $(KVERS) M=$(CURDIR) clean
