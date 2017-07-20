/*										A simple block device driver test
 * 			write at the meanwhile read the LDD3, test used code;
 * 																					by haobo.gao
 * 																					2017年7月20日21:20:56
 *
 */
#include <linux/blkdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/bio.h>

#define CUR_INVALID_DELAY   30*HZ 		//clean the disk after how many second you close the device.
#define CUR_NR_DISK 4				//we assumed that there are four disk under this driver's control

#define CUR_SECTOR_SIZE	 512		//the sector size

#define CUR_NR_SECTOR	 1024		//how many sector are there

#define CUR_DISK_MINORS		16
// defined to make it easy to using the related data structure in this driver;
struct test_blk_t{
	unsigned char 			*data;
	unsigned int 			major;
	char 					name[20] ;
	struct gendisk  		*disk;
	struct timer_list 		 timer;
	struct request_queue    *queue;
	short 					flag_media_changed;
	spinlock_t               spinlock;
	int 					 size;
	unsigned int			user_count;	//user used count,when it decreased to zero.this device will be freed
};

static struct test_blk_t * test_blk;

static int test_blk_open (struct inode * inode, struct file * file)
{
	struct test_blk_t * dev = inode->i_bdev->bd_disk->private_data;
	/*
	 	 *@ when open this device kill the timer,
	 	 *@ add a timer while close this device.
	 	 *@ Doing this way is for make it like a remove able block device that when you close this
	 	 *@ more than 30 second,it will be assumed to be removed.
	 */
	del_timer_sync( &( dev->timer ) );
	file->private_data = dev;
	spin_lock( &(dev->spinlock) );
	if(!dev->user_count)
		check_disk_change(inode->i_bdev);
	dev->user_count++;
	spin_unlock( &(dev->spinlock) );


	return 0;
}

static int test_blk_release(struct inode *inode, struct file * file)
{
	struct test_blk_t * dev = inode->i_bdev->bd_disk->private_data;

	spin_lock( &(dev->spinlock) );
	dev->user_count--;
	if(dev->user_count == 0){
		dev->timer.expires =jiffies + CUR_INVALID_DELAY;
		add_timer( &(dev->timer) );
	}
	spin_unlock( &(dev->spinlock) );

	return 0;
}

struct block_device_operations blk_dev_ops = {
		.open = test_blk_open,
		.release = test_blk_release,
};


/*
 	 *@this function was binded at request queue
 */
void test_blk_request(request_queue_t q)
{
	;
}
/*
 	 *@ timer count come function
 */
void timer_function(unsigned long par)
{

}

//set up the struct test_blk_t type structure
void set_up_test_blk(struct test_blk_t * test_blk,int index)
{
	/*
	 	 *@  Get memory for data and initialize the size  .
	 */
	memset(test_blk,0,sizeof( struct test_blk_t ) );
	test_blk->size = CUR_NR_SECTOR * CUR_SECTOR_SIZE;
	test_blk->data = (unsigned char *)vmalloc( test_blk->size);
	if(test_blk->data == NULL){
			printk(KERN_NOTICE"vmalloc ERROR: failed to allocate memory\n");
			return ;
	}

	//initial the spinlock
	spin_lock_init( &( test_blk->spinlock ) );


	/*
	 	 *@  initial a timer :
	 	 *@  	when this timer count come to end, the device while be 'invalidates'
	 */
	init_timer( &( test_blk->timer ) );
	test_blk->timer.function = timer_function;
	test_blk->timer.data  = (unsigned long)test_blk;

	/*
	 	 *@ initial a request  queue
	 */
	test_blk->queue = blk_init_queue( test_blk_request, &( test_blk->spinlock ) );
	if(test_blk->queue == NULL){
		printk(KERN_NOTICE"NOTICE:failed allocate MEM @ blk_init_queue\n");
		goto failed_vfree;
	}

	/*
	 	 *@ set the hardware sector size
	 */
	blk_queue_hardsect_size(test_blk->queue, CUR_SECTOR_SIZE);


	test_blk->queue->queuedata = test_blk;

	test_blk->disk = alloc_disk(CUR_DISK_MINORS);
	if(test_blk->disk == NULL){
		printk(KERN_NOTICE"ERR :allocate the disk!\n");
		goto failed_vfree;
	}
	test_blk->disk->major = test_blk->major;
	// every minor has max 16 part, so use index * 16
	test_blk->disk->first_minor = index*CUR_DISK_MINORS;
	test_blk->disk->fops = &blk_dev_ops;
	test_blk->disk->queue = test_blk->queue;
	test_blk->disk->private_data = test_blk;
	snprintf( test_blk->disk->disk_name,32,"disk :%c",'A'+index);
	set_capacity(test_blk->disk,CUR_NR_SECTOR*( CUR_SECTOR_SIZE / 512) );
	//add_disk
	add_disk(test_blk->disk);
failed_vfree:
	vfree(test_blk->data);
}


int __init test_block_init(void)
{
	int i;

	//allocate the device memory

	test_blk = (struct test_blk_t *)kmalloc(sizeof(struct test_blk_t)*CUR_NR_DISK,GFP_KERNEL);
	if(NULL == test_blk){
		printk(KERN_INFO"ERR: register_blkdev");
		goto faild_register;
	}

	//set name

	for(i = 0;i < CUR_NR_DISK;i++)
		strcpy((test_blk+i)->name,"test_blk");

	//register a block device

	test_blk->major = register_blkdev(0, test_blk->name );
	if(test_blk->major <= 0){
		printk(KERN_INFO"ERR: register_blkdev");
		goto faild_register;
	}

	//set every structure

	for(i = 0;i < CUR_NR_DISK;i++)
		set_up_test_blk((test_blk+i),i);



	return 0;

faild_register:
		unregister_blkdev(test_blk->major,test_blk->name);
		return -1;
}



void __exit test_block_exit(void)
{
	int i;
	unregister_blkdev(test_blk->major,test_blk->name);
	for ( i = 0 ;i < CUR_NR_DISK; i++ ){
		vfree( (test_blk+i )->data );
	}
}

module_init(test_block_init);
module_exit(test_block_exit);
MODULE_LICENSE("GPL v2");
