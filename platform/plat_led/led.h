#ifndef _LED_H
#define _LED_H

#include <linux/cdev.h>



#define NUMOFLED 3

#define DEBUG_PRINT
struct led_t {
	char  pin;
	char  stat;
	struct cdev led_dev;
	int minor;
	dev_t  dev_num;
};

struct led_data_t {
	struct led_t led[NUMOFLED];
	char current_led;
	int major;
	unsigned int *  vm_con;
	unsigned int *  vm_dat;
	unsigned int  reg_con;
	unsigned int  reg_dat;
};



#endif
