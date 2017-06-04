/*
 * key_drv.c
 *
 *  Created on: May 11, 2017
 *      Author: haobo
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/moduleparam.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/netlink.h>
#include <linux/device.h>


volatile unsigned long * ioaddr_gpfcon = NULL;
volatile unsigned long * ioaddr_gpgcon = NULL;
volatile unsigned long * ioaddr_gpfdat = NULL;
volatile unsigned long * ioaddr_gpgdat = NULL;

#define KEY_MAJOR	100
#define KEY_MINOR	0


static int major = 0;
static int count_button = 4;
//the device num
static dev_t dev_num;
module_param(major,int,S_IRUGO);
struct key_dev_t {
	struct cdev keydev;
};
static struct key_dev_t * key_dev;
/*
 *eint0 -->GPF0
 *eint2 -->GPF2
 *eint11 -->GPG3
 *eint19 -->GPG11
**/
//int (*open) (struct inode *, struct file *);
int key_open(struct inode * Inode,struct file * File)
{
	int mi = MINOR(Inode->i_rdev);
	switch(mi){
		case 0:{
			*ioaddr_gpfcon &= ~0x03;
		}break;
		case 1:{
			*ioaddr_gpfcon &= ~(0x03<<4);
		}break;
		case 2:{
			*ioaddr_gpgcon &= ~(0x03<<6);
		}break;
		case 3:{
			*ioaddr_gpgcon &= ~(0x03<<22);
		}break;
		default :break;

	}
	return 0;
}

//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t key_read(struct file * File, char __user * buff, size_t size, loff_t * loff)
{
	int count,mi = MINOR(File->f_path.dentry->d_inode->i_rdev);
	 char scan_val = 1;
	if(size != 1){
		return -1;
	}
	switch(mi){
		case 0:{
			scan_val = *ioaddr_gpfdat & 1;
		}break;
		case 1:{
			(scan_val) = (*ioaddr_gpfdat & (1<<2))>>2;
		}break;
		case 2:{
			(scan_val) = (*ioaddr_gpgdat & (1<<3))>>3;
		}break;
		case 3:{
			(scan_val) = (*ioaddr_gpgdat & (1<<11))>>11;
		}break;
		default :break;

	}
	//printk(KERN_INFO"scan_val:%d",scan_val);
	count = copy_to_user(buff, (const void *)&scan_val, 1);
	return count;
}


static struct file_operations key_ops={
	.open = key_open,
	.read = key_read,

};
//we use mdev here
static struct class * chartest;
static struct class_device * keydev;

int key_set_up_cdev(struct key_dev_t * key_dev,int index)
{
	int err;
	dev_t dev_num_ma_mi;
	//we made a complete device number here
	dev_num_ma_mi = MKDEV(major,index);
	keydev = class_device_create(chartest, NULL,dev_num_ma_mi, NULL,"key%d",index);
	key_dev->keydev.owner = THIS_MODULE;
	//bind the key_ops to the cdev
	cdev_init(&key_dev->keydev,&key_ops);
	err =  cdev_add(&key_dev->keydev, dev_num_ma_mi, 1);
	if(err){
		printk(KERN_INFO"error to add_cdev!\n");
		return err;
	}
	return 0;
}

//this driver's entry function
int __init key_init(void)
{
	int ret,dev_count;
    dev_num = MKDEV(major,0);
	//if the major come from module_param we just use it region it
	if(major){
		//register four device num here ,cause we have four button on board
		register_chrdev_region(dev_num, count_button, "key");
		major = MAJOR(dev_num); //we made the static int major be a right value anyway
	}
	else{ //if there is no module parameter send  in, we dynamic allocate
		alloc_chrdev_region(&dev_num, 0, count_button, "key");
		major = MAJOR(dev_num);//we made the static int major be a right value anyway
	}
	//create the class and a dev at sysfs
	chartest = class_create(THIS_MODULE, "chardev_test");

	//allocate a key_dev_t's memory
	key_dev =kzalloc(sizeof(struct key_dev_t)*count_button, GFP_KERNEL);
	for(dev_count = 0;dev_count<count_button;dev_count++)
			key_set_up_cdev(&(*(key_dev+dev_count)),dev_count);
	ioaddr_gpfcon = (volatile unsigned long * )ioremap(0x56000050,16);
	ioaddr_gpgcon = (volatile unsigned long * )ioremap(0x56000060,16);
	ioaddr_gpfdat = (volatile unsigned long * )ioremap(0x56000054,16);
	ioaddr_gpgdat = (volatile unsigned long * )ioremap(0x56000064,16);
	if(!key_dev){
		ret = -ENOMEM;
		goto fail_malloc ;
	}
	return 0;
fail_malloc:
		unregister_chrdev_region(dev_num, count_button);
		return -ENOMEM;
}
//this driver's exit function
void __exit key_exit(void)
{
	int i;
	iounmap(ioaddr_gpgcon);
	iounmap(ioaddr_gpfcon);
	iounmap(ioaddr_gpgdat);
	iounmap(ioaddr_gpgdat);
	unregister_chrdev_region(dev_num, count_button);
	for(i = 0;i<count_button;i++)
		cdev_del(&(key_dev+i)->keydev);
	kfree(key_dev);
	class_device_destroy(chartest, dev_num);
	class_destroy(chartest);


}
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("haobo.gao@qq.com");
MODULE_VERSION("key Ver0.1");
MODULE_DESCRIPTION("The 2440 key driver in board!");
module_init(key_init);
module_exit(key_exit);
