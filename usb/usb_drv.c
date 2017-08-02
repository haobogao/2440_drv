/*
 * usb_drv.c
 *
 *  Created on: 2017Äê8ÔÂ2ÈÕ
 *      Author: haobo
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb_gadget.h>

	struct urb;
int __init usb_init(void)
{
	URB_SHORT_NOT_OK


	return 0;
}


void __exit usb_exit(void)
{
	;
}

module_init(usb_init);
module_exit(usb_exit);
