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
#include <asm/atomic.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/string.h>

volatile unsigned long * ioaddr_gpfcon = NULL;
volatile unsigned long * ioaddr_gpgcon = NULL;
volatile unsigned long * ioaddr_gpfdat = NULL;
volatile unsigned long * ioaddr_gpgdat = NULL;



static int count_button = 4;


struct pin_t{
	unsigned int pin_num;
	unsigned char pin_val;
	unsigned char stat;
	int irq;
	char  name[6] ;
};

struct key_input_dev_t {
	struct input_dev * input_key;
	struct pin_t pin[4];
	struct timer_list shake_handler;
	int current_key;
	char input_name[20];
};
static struct key_input_dev_t * key_input_dev;


//void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
void shake_handler_func(unsigned long data)
{
	struct key_input_dev_t * dev = (struct key_input_dev_t *) data;
	int pinval;

	pinval =  s3c2410_gpio_getpin(dev->pin[dev->current_key].pin_num);
//	printk(KERN_INFO"Timer: weakup is Ok!\n");
	if(pinval){
		input_event(dev->input_key,EV_KEY, dev->pin[dev->current_key].pin_val, 0);
		input_sync(dev->input_key);

	}else{
		input_event(dev->input_key,EV_KEY, dev->pin[dev->current_key].pin_val, 1);
		input_sync(dev->input_key);
	}
}


static irqreturn_t handler_button(int irq,void * devid)
{
	struct key_input_dev_t * p =(struct key_input_dev_t *)devid;

	if(IRQ_EINT0 == irq){
		mod_timer(&(p->shake_handler),jiffies+HZ/100);
		p->current_key = 0;
	}
	if( IRQ_EINT2 == irq){
		mod_timer(&(p->shake_handler),jiffies+HZ/100);
		p->current_key = 1;
	}
	if( IRQ_EINT11 == irq){
		mod_timer(&(p->shake_handler),jiffies+HZ/100);
		p->current_key = 2;
	}
	if(IRQ_EINT19 == irq){
		mod_timer(&(p->shake_handler),jiffies+HZ/100);
		p->current_key = 3;
	}
	return IRQ_HANDLED;
}




void set_inputkey_gpio(void)
{
	ioaddr_gpfcon = (volatile unsigned long * )ioremap(0x56000050,16);
	ioaddr_gpgcon = (volatile unsigned long * )ioremap(0x56000060,16);
	ioaddr_gpfdat = (volatile unsigned long * )ioremap(0x56000054,16);
	ioaddr_gpgdat = (volatile unsigned long * )ioremap(0x56000064,16);

	strcpy( key_input_dev->input_name,"key_input");

	key_input_dev->pin[0].pin_num = S3C2410_GPF0;
	key_input_dev->pin[0].pin_val = KEY_L;

	key_input_dev->pin[0].irq = IRQ_EINT0;
	strcpy(key_input_dev->pin[0].name,"key0");

	key_input_dev->pin[1].pin_num = S3C2410_GPF2;
	key_input_dev->pin[1].pin_val = KEY_S;

	key_input_dev->pin[1].irq = IRQ_EINT2;
	strcpy(key_input_dev->pin[1].name,"key1");

	key_input_dev->pin[2].pin_num = S3C2410_GPG3;
	key_input_dev->pin[2].pin_val = KEY_ENTER;

	key_input_dev->pin[2].irq = IRQ_EINT11;
	strcpy(key_input_dev->pin[2].name,"key2");

	key_input_dev->pin[3].pin_num = S3C2410_GPG11;
	key_input_dev->pin[3].pin_val = KEY_LEFTSHIFT;

	key_input_dev->pin[3].irq = IRQ_EINT19;
	strcpy(key_input_dev->pin[3].name,"key3");
}

int __init key_input_init(void)
{
	int i,ret;
	key_input_dev =kzalloc(sizeof(struct key_input_dev_t)*count_button, GFP_KERNEL);
	 key_input_dev->input_key = input_allocate_device();
	 set_inputkey_gpio();
	 key_input_dev->input_key->name = key_input_dev->input_name ;
	 //set this structure
	 set_bit(EV_KEY, key_input_dev->input_key->evbit);
	 set_bit(EV_REP,key_input_dev->input_key->evbit);

		set_bit(KEY_L, key_input_dev->input_key->keybit);
		set_bit(KEY_S, key_input_dev->input_key->keybit);
		set_bit(KEY_ENTER, key_input_dev->input_key->keybit);
		set_bit(KEY_LEFTSHIFT, key_input_dev->input_key->keybit);
	/*register */
	input_register_device(key_input_dev->input_key);
	for(i = 0;i <count_button;i++)
		ret =request_irq(key_input_dev->pin[i].irq, handler_button, IRQT_BOTHEDGE,key_input_dev->pin[i].name,(key_input_dev));

	init_timer(&(key_input_dev->shake_handler));
	key_input_dev->shake_handler.function = shake_handler_func;
	key_input_dev->shake_handler.data =(unsigned long) key_input_dev;
	add_timer(&(key_input_dev->shake_handler));
	return 0;
}

void __exit key_input_exit(void)
{
	int i;
	if(ioaddr_gpgcon)
		iounmap(ioaddr_gpgcon);
	if(ioaddr_gpfcon)
		iounmap(ioaddr_gpfcon);
	if(ioaddr_gpgdat)
		iounmap(ioaddr_gpgdat);
	if(ioaddr_gpgdat)
		iounmap(ioaddr_gpgdat);
	input_unregister_device(key_input_dev->input_key);
	input_free_device(key_input_dev->input_key);
	del_timer(&(key_input_dev->shake_handler));
	 for(i = 0;i <count_button;i++)
	 	free_irq(key_input_dev->pin[i].irq,(void *)key_input_dev);

	 kfree(key_input_dev);
}





MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("haobo.gao@qq.com");
MODULE_VERSION("key Ver0.1");
MODULE_DESCRIPTION("The 2440 key driver in board!");
module_init(key_input_init);
module_exit(key_input_exit);





