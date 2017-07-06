/*
 * dma_drv.c
 *
 *  Created on: 2017Äê6ÔÂ29ÈÕ
 *      Author: haobo
 */
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/netlink.h>
#include <linux/device.h>
#include <linux/irq.h>

#define DMA_REG_BASE_0	0x4B000000
#define DMA_REG_BASE_1	0x4B000040
#define DMA_REG_BASE_2	0x4B000080
#define DMA_REG_BASE_3	0x4B0000C0
#define TRASMIT_DMA 1
#define TRASMIT_CPU 0
struct dma_regs_2440_t {
	/*Base address (start address) of source data to transfer. This bit value will be loaded into
	 * CURR_SRC only if the CURR_SRC is 0 	and the DMA ACK is 1.)*/
	unsigned int  DISRC;
	/*
	 * DMA 0 initial source control register
	 * */
	unsigned int  DISRCC;
/*
 *	 DMA  initial destination register
 */
	unsigned int  DIDST;
	/*
	 *DMA initial destination control register
	 */
	unsigned int  DIDSTC;
	/*
	 * DMA  control register
	 * */
	unsigned int  DCON;
	/*
	 * DMA 0 count register
	 */
	unsigned int  DSTAT;
	/*
	 * DMA  current source register
	 */
	unsigned int  DCSRC;
	/*
	 *DMA current destination register
	 */
	unsigned int  DCDST;
	/*
	 *DMA 0 mask trigger register
	 */
	unsigned int  DMASKTRIG;
};


struct dma_t{
	unsigned int *	 	 	vm_src;
	unsigned int *	 	 	vm_dest;
	unsigned int *	 	 	phy_src;
	unsigned int *	 	 	phy_dest;
	dev_t 				 	dev_num;
	struct cdev 		 	dev;
	struct class	   	 	cls;
	struct class_device  	cls_dev;
	struct dma_regs_2440_t* dma_reg;


};

struct dma_t * dma_dev;


//int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
int dma_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)
{
	switch(cmd){
		case TRASMIT_DMA:{

		}break;

		case TRASMIT_CPU:{

		}break;

	}
	return 0;
}
//int (*open) (struct inode *, struct file *);
int dma_open(struct inode *inode, struct file *file)
{
	dma_alloc_writecombine();
	return 0;
}

struct file_operations dma_ops = {
		.ioctl =  dma_ioctl,
		.open = dma_open,

};



int __init dma_init(void)
{

	dma_dev = kzalloc(sizeof(dma_t),GFP_KERNEL);
	if(!dma_dev){
		printk(KERN_INFO"Kzalloc: faild alloc memory!\n");
		return -ENOMEM;
	}

	//allocate a char device
	if(alloc_chrdev_region( &(dma_dev->dev) ,0,1,"dma_test") != 0){
		printk(KERN_INFO"alloc_chrdev_region:faild!\n");
		return -1;
	}
	dma_dev->dev.owner = THIS_MODULE;
	cdev_init((&dma_dev->dev),&dma_ops);
	cdev_add( &(dma_dev->dev), (dma_dev->dev_num),1);
	dma_dev->cls =  class_create(THIS_MODULE,"dma_test");
	dma_dev->cls_dev  = class_device_create( &(dma_dev->cls_dev),NULL,dma_dev->dev_num,NULL,"dma",0);
	dma_dev->dma_reg = ioremap(DMA_REG_BASE_3, sizeof(struct dma_regs_2440_t) );
	dma_alloc_writecombine()
	return 0;
}

void __exit dma_exit(void)
{
	unregister_chrdev_region(dma_dev->dev_num,1);
	cdev_del(dma_dev->dev);
	class_device_destroy(&(dma_dev->cls),dma_dev->dev_num);
	class_destroy( &(dma_dev->cls) );
	kfree(dma_dev);
}

#include <linux/mm.h>

module_init(dma_init);
module_exit(dma_exit);
MODULE_LICENSE("GPL v2");
