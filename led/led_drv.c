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

#define LED_MAJOR 120
#define LED_MINOR 0

#define LED_ON 		*ioaddr_gpfdat &= ~(1<<4|1<<5|1<<6);
#define LED_OFF 	*ioaddr_gpfdat |= (1<<4|1<<5|1<<6);
static int major,minor = LED_MINOR;
static dev_t dev_num;
static int led_num = 4;
module_param(major,int,S_IRUGO);
/*
 here we want drive the led 1,2,4 on the 2440board
 the hardware on 2440 as following :
 	 	 led 1  ------- GPF4
 	 	 led 2  ------- GPF5
 	 	 led 4 -------- GPF6
		GPFCON ---- 0x56000050
		GPFDAT ---- 0x56000054
*/

volatile unsigned long * ioaddr_gpfcon = NULL;
volatile unsigned long * ioaddr_gpfdat = NULL;


#define DEBUG_PRINT

struct led_dev_t{
	struct cdev ledev;
};

static struct led_dev_t * led_dev;





/*in open function
 * we configure the led port to
 * output mode.
 */
int led_open(struct inode * Inode,struct file * File)
{
	*ioaddr_gpfcon &= ~((0x3f)<<8);
	*ioaddr_gpfcon |= ((0x15)<<8);

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
	count =  copy_from_user((void *)buf,(void *) buff, size+1);
	printk(KERN_NOTICE "recive : %s form user space count:%d \n",buf,size+1);
	if(strcmp("on",buf) == 0){
		printk(KERN_NOTICE"in off driver");
		LED_ON;
		return 1;
	}
	if(strcmp("off",buf) == 0){
		printk(KERN_NOTICE"in on driver");
		LED_OFF;
		return 0;
	}
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
};



static struct class * chartest;
static struct class_device * leddev;

/*
 * This function is for the led_dev_t 's set up
 */
void led_setup_cdev(struct led_dev_t * led_dev,int index)
{
	int err;
	dev_t dev_num_ma_mi;
	char minor_str[2],devname[4] = "led";
	minor_str[0] = '0'+index;
	minor_str[1] = '\0';
	dev_num_ma_mi = MKDEV(major,minor+index);
	//leddev = class_device_create(chartest, NULL,dev_num, NULL,"led%d");
	leddev = class_device_create(chartest, NULL,dev_num_ma_mi, NULL,strncat(devname,minor_str,2),0);
	led_dev->ledev.owner = THIS_MODULE;
// cdev_init used for init the cdev and build the relationship between the file_operation and cdev.
	cdev_init(&led_dev->ledev,&led_ops);
	//add a  char device in the system
	err =  cdev_add(&led_dev->ledev, dev_num, 1);
	if(err)
		printk(KERN_INFO"cdev_add: error\n");
}

int __init led_init(void)
{
	int ret,num;
	dev_num =MKDEV(major,minor);
	printk(KERN_INFO"marjor = %d",major);
	if(major){
		register_chrdev_region(dev_num, 4, "led");
	}
	else {
		alloc_chrdev_region(&dev_num, 0, 4, "led");
		major = MAJOR(dev_num);

	}
	//use mdev auto create /dev/led
	chartest = class_create(THIS_MODULE, "chardev_test");

	led_dev =kzalloc(sizeof(struct led_dev_t)*led_num, GFP_KERNEL);
	if(!led_dev){
		ret = -ENOMEM;
		goto fail_malloc ;
	}
	for(num = 0;num<led_num;num++)
		led_setup_cdev(led_dev+num,num);
	ioaddr_gpfcon = (volatile unsigned long * )ioremap(0x56000050,16);
	ioaddr_gpfdat = (volatile unsigned long * )ioremap(0x56000054,16);
# ifdef DEBUG_PRINT
	printk(KERN_NOTICE"led init ok");
# endif
	return 0;
fail_malloc:
	unregister_chrdev_region(dev_num, 4);

	return ret;
   // register_chrdev(major,"led_drv",&led_ops);
}


void __exit led_exit(void)
{
	int num;

	iounmap((unsigned long *)0x56000050);
	iounmap((unsigned long *)0x56000054);
	unregister_chrdev_region(MKDEV(major,0), 4);
	for(num=0;num<led_num;num++)
		cdev_del(&(led_dev+num)->ledev);
	kfree(led_dev);
    class_device_destroy(chartest, dev_num);
	class_destroy(chartest);

# ifdef DEBUG_PRINT
	printk(KERN_NOTICE"led module exit ok");
# endif
}
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("haobo.gao@qq.com");
MODULE_VERSION("LED Ver0.1");
MODULE_DESCRIPTION("The first led driver in board!");

module_init(led_init);
module_exit(led_exit);
