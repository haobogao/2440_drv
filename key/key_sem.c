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
#include <asm/irq.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/arch/regs-gpio.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <asm/semaphore.h>

volatile unsigned long * ioaddr_gpfcon = NULL;
volatile unsigned long * ioaddr_gpgcon = NULL;
volatile unsigned long * ioaddr_gpfdat = NULL;
volatile unsigned long * ioaddr_gpgdat = NULL;
#define KEY_STAT_UP 0
#define KEY_STAT_DOWN 1

#define KEY_MAJOR	100
#define KEY_MINOR	0



static int count_button = 4;
//the device num
static dev_t dev_num;	//use this storge the device number.
static int major = 0;	//if there are no paramer send in.major would be zero.
module_param(major,int,S_IRUGO);

struct pin_t{
	unsigned int pin_num;
	unsigned char pin_val;
	unsigned char stat;
};

struct key_dev_t {
	struct cdev	 keydev;
	struct pin_t pin;
	struct fasync_struct * fasync_q;
	wait_queue_head_t button_wq;
	struct semaphore sem;
};
static struct key_dev_t * key_dev;


static irqreturn_t handler_button(int irq,void * devid)
{
	struct key_dev_t * p =(struct key_dev_t *)devid;
	if(IRQ_EINT0 == irq){
		printk(KERN_INFO"button1 clicked!\n");
		p->pin.stat = KEY_STAT_DOWN;
		printk(KERN_INFO"p is Ok!\n");
		wake_up_interruptible(&(p->button_wq));
		printk(KERN_INFO"weakup is Ok!\n");
		kill_fasync(&(p->fasync_q), SIGIO, POLLIN);
	}
	if( IRQ_EINT2 == irq){
			printk(KERN_INFO"button2 clicked!\n");
			p->pin.stat = KEY_STAT_DOWN;
			printk(KERN_INFO"p is Ok!\n");
			wake_up_interruptible(&(p->button_wq));
			printk(KERN_INFO"weakup is Ok!\n");
			kill_fasync(&(p->fasync_q), SIGIO, POLLIN);
	}
	if( IRQ_EINT11 == irq){
			printk(KERN_INFO"button3 clicked!\n");
			p->pin.stat = KEY_STAT_DOWN;
			printk(KERN_INFO"p is Ok!\n");
			wake_up_interruptible(&(p->button_wq));
			printk(KERN_INFO"weakup is Ok!\n");
			kill_fasync(&(p->fasync_q), SIGIO, POLLIN);
	}
	if(IRQ_EINT19 == irq){
			printk(KERN_INFO"button4 clicked!\n");
			p->pin.stat = KEY_STAT_DOWN;
			printk(KERN_INFO"p is Ok!\n");
			wake_up_interruptible(&(p->button_wq));
			printk(KERN_INFO"weakup is Ok!\n");
			kill_fasync(&(p->fasync_q), SIGIO, POLLIN);
	}
	return IRQ_HANDLED;
}
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
	int ret;
	switch(mi){
		case 0:{
			File->private_data = &key_dev[0];
			ret =request_irq(IRQ_EINT0, handler_button, IRQT_FALLING, "button1",(key_dev));
		}break;
		case 1:{
			File->private_data = &key_dev[1];
			ret =request_irq(IRQ_EINT2, handler_button, IRQT_FALLING, "button2",(key_dev+1));
		}break;
		case 2:{
			File->private_data = &key_dev[2];
			ret =request_irq(IRQ_EINT11, handler_button, IRQT_FALLING, "button3",(key_dev+2));
		}break;
		case 3:{
			File->private_data = &key_dev[3];
			ret =request_irq(IRQ_EINT19, handler_button, IRQT_FALLING, "button4",(key_dev+3));
		}break;
		default :break;

	}
	return 0;
}

//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t key_read(struct file * File, char __user * buff, size_t size, loff_t * loff)
{
	int count;
	char scan_val = 1;
	struct key_dev_t * key  = ( struct key_dev_t *) File->private_data;
	if(size != 1){
		return -1;
	}
	down_interruptible(&(key->sem));
	wait_event_interruptible(key_dev->button_wq,key->pin.stat);

	key->pin.stat = KEY_STAT_UP;
	scan_val =key->pin.pin_val;

	//printk(KERN_INFO"scan_val:%d",scan_val);
	count = copy_to_user(buff, (const void *)&scan_val, 1);
	up(&(key->sem));
	return count;
}


int key_release (struct inode * Inode, struct file * file)
{
	struct key_dev_t * key  = ( struct key_dev_t *) file->private_data;
	free_irq(IRQ_EINT0, &key);
	return 0;
}
/**
    *@author:         haobo.gao@qq.com
    *@parameter:
    *@return value:
    *@describe :
    *@BUG:
*/
unsigned int key_poll(struct file * file, struct poll_table_struct *pt)
{
	struct key_dev_t * dev = (struct key_dev_t *)file->private_data;
	unsigned int mask = 0;
	poll_wait(file,&(dev->button_wq),pt);
	if(dev->pin.stat == KEY_STAT_DOWN)
		mask |= POLLIN|POLLRDNORM;
	return mask;
}

int key_fasync(int fd, struct file * file, int mod)
{
	struct key_dev_t * dev = (struct key_dev_t *)file->private_data;
	return fasync_helper(fd, file, mod, &(dev->fasync_q));
}

//unsigned int (*poll) (struct file *, struct poll_table_struct *);
static struct file_operations key_ops={
	.open = key_open,
	.read = key_read,
	.release = key_release,
	.poll = key_poll,
	.fasync =key_fasync,
};




//we use mdev here
static struct class * chartest;
static struct class_device * keydev;

int __init key_set_up_cdev(struct key_dev_t * key_dev,int index)
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
	sema_init(&(key_dev->sem),1);
	//init the wait queue
	init_waitqueue_head(&key_dev->button_wq);
	return 0;
}

void set_key_gpio(void)
{
	ioaddr_gpfcon = (volatile unsigned long * )ioremap(0x56000050,16);
	ioaddr_gpgcon = (volatile unsigned long * )ioremap(0x56000060,16);
	ioaddr_gpfdat = (volatile unsigned long * )ioremap(0x56000054,16);
	ioaddr_gpgdat = (volatile unsigned long * )ioremap(0x56000064,16);

	key_dev[0].pin.pin_num = S3C2410_GPF0;
	key_dev[0].pin.pin_val = 0xf0;
	key_dev[0].pin.stat =KEY_STAT_UP;

	key_dev[1].pin.pin_num = S3C2410_GPF2;
	key_dev[1].pin.pin_val = 0xf2;
	key_dev[1].pin.stat =KEY_STAT_UP;

	key_dev[2].pin.pin_num = S3C2410_GPG3;
	key_dev[2].pin.pin_val = 0x73;
	key_dev[2].pin.stat =KEY_STAT_UP;

	key_dev[3].pin.pin_num = S3C2410_GPG11;
	key_dev[3].pin.pin_val = 0x11;
	key_dev[3].pin.stat =KEY_STAT_UP;
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
	set_key_gpio();
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
