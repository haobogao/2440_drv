/*
 * led_dev.c
 *
 *  Created on: 2017Äê6ÔÂ26ÈÕ
 *      Author: haobo
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "led.h"
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/list.h>



struct led_data_t * led_data;




struct resource led_res[]={
	[0] ={
		.start = 0x56000050,
		.end = 0x56000050+8-1,
		.flags = IORESOURCE_MEM,
	},

	[1] = {
		.start =4,
		.end = 4,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start =5,
		.end = 5,
		.flags = IORESOURCE_IRQ,
	},
	[3] = {
		.start =6,
		.end = 6,
		.flags = IORESOURCE_IRQ,
	},

} ;

void led_release(struct device * dev)
{
	;
}
struct platform_device led_dev = {
	.name = "led",
	.id = -1,
	.num_resources = ARRAY_SIZE(led_res),
	.resource = led_res,
};


void set_up_led_data(void )
{
	int i;
	led_data = (struct led_data_t *) kzalloc(sizeof(struct led_data_t),GFP_KERNEL);
	if(!led_data){
		printk(KERN_NOTICE"set_up_led_data: ENOMEM");
	}
#ifdef DEBUG_PRINT
	printk(KERN_NOTICE"set_up_led_data: led_data alloced!\n");
#endif

	led_data->current_led = 0;
	led_data->major = 0;
	led_data->reg_con=(0x56000050);
#ifdef DEBUG_PRINT
	printk(KERN_NOTICE"set_up_led_data: reg_con \n");
#endif

	for (i = 0;i<NUMOFLED;i++){
		led_data->led[i].pin = 4+i;
		led_data->led[i].stat = 0;
		led_data->led[i].minor = 0;
		led_data->led[i].dev_num = 0;
	}
#ifdef DEBUG_PRINT
	printk(KERN_NOTICE"set_up_led_data: lafter for;\n");
#endif

		led_dev.dev.release =led_release;
#ifdef DEBUG_PRINT
	printk(KERN_NOTICE"set_up_led_data: release ok!\n");
#endif

		led_dev.dev.platform_data = led_data;
#ifdef DEBUG_PRINT
	printk(KERN_NOTICE"set_up_led_data: end func!\n");
#endif


}


int __init led_plat_dev_init(void )
{
	set_up_led_data();
	platform_device_register(&led_dev);
	return 0 ;
}

void __exit led_plat_dev_exit(void)
{
	kfree(led_data);
	platform_device_unregister(&led_dev);
}

module_init(led_plat_dev_init);
module_exit(led_plat_dev_exit);
MODULE_LICENSE("GPL v2");
