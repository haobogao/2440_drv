#include <linux/fs.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/netlink.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <linux/string.h>
#include <linux/moduleparam.h>
#include "led.h"
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>





struct led_data_t * led;

 struct class * chartest_class;
 struct class_device * led_class_device;

#define LED_ON(x) 		*(led->vm_dat) &= ~(1<<(x+4));
#define LED_OFF(x) 		*(led->vm_dat) |= (1<<(x+4));



int led_open(struct inode * Inode,struct file * File)
{

	int minor = MINOR(Inode->i_cdev->dev);
	//led[minor]  open
//	*(led->vm_con) &= ~((0x3f)<<8);
//	*(led->vm_con) |= ((0x15)<<8);
	*(led->vm_con) &= ~( (0x3) << (led->led[minor].pin) * 2 );
	*(led->vm_con) |= ( (0x1) << (led->led[minor].pin) * 2 );
	File->private_data = (void *) &led->led[minor];
	return 0;
}

ssize_t led_read(struct file * File, char __user * buff, size_t size, loff_t * loff)
{
	return 0;
}

ssize_t led_write(struct file * File, const char __user * buff, size_t size, loff_t * loff)
{
	char buf[size+1];
	int count;
	struct led_t * _led = (struct led_t *) File->private_data;
	count =  copy_from_user((void *)buf,(void *) buff, size+1);
	printk(KERN_NOTICE "recive : %s form user space count:%d \n",buf,size+1);
	if(strcmp("on",buf) == 0){
		printk(KERN_NOTICE" on send in! led%d on \n",_led->minor);
		LED_ON(_led->minor);

		return 1;
	}
	if(strcmp("off",buf) == 0){
		printk(KERN_NOTICE"off send in! led%d off\n ",_led->minor);
		LED_OFF(_led->minor);

		return 0;
	}
	return 0;
}

int led_release (struct inode * inode, struct file * file)
{
	return 0;
}

//int (*open) (struct inode *, struct file *);
//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);

struct file_operations led_ops = {
		.open = led_open,
		.write = led_write,
		.read = led_read,
		.owner=THIS_MODULE,
		.release = led_release,
};

void led_setup_cdev(struct led_t * _led,int index)
{
		int err;
		_led->minor += index;
		_led->dev_num = MKDEV(led->major,_led->minor);

		led_class_device = class_device_create(chartest_class, NULL,_led->dev_num, NULL,"led%d",index);
		_led->led_dev.owner = THIS_MODULE;
	// cdev_init used for init the cdev and build the relationship between the file_operation and cdev.
		cdev_init(&_led->led_dev,&led_ops);
		//add a  char device in the system
		err =  cdev_add(&_led->led_dev, _led->dev_num, 1);

		if(err)
			printk(KERN_INFO"cdev_add: error\n");
}

void led_register(struct led_data_t * dev)
{
	int i;

	if(alloc_chrdev_region(&(dev->led[0].dev_num), 0, NUMOFLED, "led")!= 0 ){
		printk(KERN_NOTICE"alloc_chrdev_region: NOMEM");
		goto fail_region;
	}
	dev->major = MAJOR(dev->led[0].dev_num);

	chartest_class = class_create(THIS_MODULE, "chardev_test");

	for(i = 0;i<NUMOFLED;i++)
		led_setup_cdev(&(dev->led[i]),i);

fail_region:
	unregister_chrdev_region(dev->led[0].dev_num, NUMOFLED);
}


/***************************************************************************************************
 *@NOTE:
 * 			Following are platform reference operate which the probe function likes char devices driver's
 * 		modules initialize function.
 * 																haobo.gao@qq.com
 * ************************************************************************************************ */


int led_remove(struct platform_device * dev)
{
	int i;
	iounmap(led->vm_con);
	for(i = 0;i<NUMOFLED;i++)
		cdev_del(&led->led[i].led_dev);
	unregister_chrdev_region(led->led[0].dev_num,NUMOFLED);
    class_device_destroy(chartest_class,led->led[0].dev_num);
	class_destroy(chartest_class);
	return 0 ;
}


/**
    *@author:         haobo.gao@qq.com
    *@parameter:
    *@return value:
    *@describe :	Did the job about  :
    *@describe 					1>.get the data form led_device and set up some thing you need.
    *@describe 					2>.register the led char device.
    *@describe					3>.complete the reference address mmap.
    *@describe
    *@BUG:
*/
int led_probe(struct platform_device * dev)
{

	char i;
	led = (struct led_data_t *)dev->dev.platform_data;

#ifdef DEBUG_PRINT
	printk(KERN_NOTICE"led_probe: led probed!\n");
#endif


	//get and remap IO address


		led->vm_con = (unsigned int * )ioremap((led->reg_con),8);
		led->vm_dat = led->vm_con+1;

//	struct resource * res = platform_get_resource(dev,IORESOURCE_MEM,0);
//	res = platform_get_resource(dev,IORESOURCE_IRQ,0);

	//register char device
	//and set up cdev
	led_register(led);



	return 0;
}

struct platform_driver led_drv = {
		.remove =  led_remove,
		.probe =   led_probe,
		.driver = {
				.name = "led",
		},
};

static void __exit led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);

}

static int __init led_drv_init(void)
{
	platform_driver_register(&led_drv);
	return 0;
}


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("haobo.gao@qq.com");
MODULE_VERSION("LED Ver0.2");
MODULE_DESCRIPTION(" led driver reform 4 platform!");

module_init(led_drv_init);
module_exit(led_drv_exit);


