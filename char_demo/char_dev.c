/*@brief:
 * 	I write it for provide a demo for all char device driver purpose. which made it easy to start a new device driver.
 *you can use it just to replace the "char" with the device name you work with;
 * 																					haobo@: 2017Äê6ÔÂ5ÈÕ10:18:12
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/irq.h>


struct resource t;


#define NUMBEROF_CHAR_DEVICE 1


/*
 * module paramer related
 */
static dev_t dev_id;	//use this storge the device number.
static int major = 0;	//if there are no paramer send in.major would be zero.
module_param(major,int,S_IRUGO);



/*
 *@brief:	 this is a  struct to describe your own
 *	char device.
 */
struct char_dev_t {
	struct cdev dev;

};


//the global char device pointer
struct char_dev_t * char_dev_p;


//int (*open) (struct inode *, struct file *);
int char_dev_open (struct inode * inode, struct file * file)
{

	return 0;
}

//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t char_dev_read (struct file * file, char __user * user, size_t size, loff_t * loff)
{
	return 0;
}

//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
ssize_t char_dev_write (struct file * file, const char __user * user, size_t  size, loff_t * loff)
{
	return 0;
}


//int (*release) (struct inode *, struct file *);
int char_dev_release (struct inode * inode, struct file * file)
{

	return 0;
}

static struct file_operations char_dev_ops = {
	.open = char_dev_open,
	.read = char_dev_read,
	.write = char_dev_write,
	.release = char_dev_release,
};



/**
    *@author:       haobo.gao@qq.com
    *@parameter:    struct char_dev_t * char_dev  :
    *@parameter:    int index   :
    *@return value:	none
    *@describe :	this function is used to initialize the struct char_dev type's struct.
    *@BUG:			not found yet
*/
int set_up_char_dev(struct char_dev_t * char_dev,int index)
{
	int err;
	dev_t dev_num_ma_mi;
	//we made a complete device number here.minor device number is worked here.
	dev_num_ma_mi = MKDEV(major,index);

	//keydev = class_device_create(chartest, NULL,dev_num_ma_mi, NULL,"key%d",index);
	char_dev->dev.owner = THIS_MODULE;

	//bind the char_dev_ops to the cdev
	cdev_init(&char_dev->dev,&char_dev_ops);

	//add a char device to the system.
	err =  cdev_add(&char_dev->dev, dev_num_ma_mi, 1);
	if(err){
		printk(KERN_INFO"error to add_cdev!\n");
		return err;
	}
	return 0;
}


/**
    *@author:         haobo.gao@qq.com
    *@parameter:    none
    *@return value:
    *@describe :	do some initialize things when module load.
    *@describe 	The flow path likes this:
    *@describe 		1.obtain the device number
    *@describe 		2.allocate the device struct memory
    *@describe 		3.  set up char_dev_t structure
    *@BUG:			not found yet
*/
int __init char_dev_init()
{
	//get default device number
	int ret,
	i,
	dev_num = MKDEV(major,0);


	//1.obtain the device number.
	if(major)  {	//if major >= 1 it mains the major is send in by outside parameter. we using the values to made a device number as user want.
		//obtain the device number
		register_chrdev_region(&dev_num,NUMBEROF_CHAR_DEVICE,"char_dev name");
		major = MAJOR(dev_num);			//refresh the major value
	}else{			//if not it must be a wrong value send in or not send at all. we dynamic allocate it.
		//obtain the device number
		register_chrdev_region(&dev_num,NUMBEROF_CHAR_DEVICE,"char_dev name");
		major= MAJOR(dev_num);			//refresh the major value
	}
	if(dev_id < 0){					//if failure return the error number
		ret = dev_num;
		printk(KERN_INFO"register_chrdev_region:ERROR at return values:%d\n",ret);
		return ret;
	}
	dev_id = dev_num;

	//2.allocate the char_dev struct's memory
	char_dev_p = (struct char_dev_t*)kmalloc(sizeof(struct char_dev_t)* NUMBEROF_CHAR_DEVICE,GFP_KERNEL);
	if(char_dev_p == NULL){
		printk(KERN_INFO"FAILD MALLOC!\n");
		goto fail_malloc;
	}

	//3.set_up char_dev_t structure.
	for(i = 0;i<NUMBEROF_CHAR_DEVICE;i++){
		set_up_char_dev(&char_dev_p[i],i);
	}
	return 0;

fail_malloc:
		unregister_chrdev_region(dev_num, NUMBEROF_CHAR_DEVICE);
		return -ENOMEM;


}

/**
    *@author:         haobo.gao@qq.com
    *@parameter:    	none
    *@return value:
    *@describe :		free and clean the module when module exit;
    *@BUG:
*/

int __exit char_dev_exit()
{
	int i;
	unregister_chrdev_region(dev_id, NUMBEROF_CHAR_DEVICE);
	for(i = 0;i<NUMBEROF_CHAR_DEVICE;i++)
		cdev_del(&char_dev_p[i].dev);
	kfree(char_dev_p);
	return 0;
}


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("haobo.gao@qq.com");
MODULE_VERSION("Ver0.1");
MODULE_DESCRIPTION("The char device driver demo");
module_init(char_dev_init);
module_exit(char_dev_exit);
