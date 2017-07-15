#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>

/* 480*272*3 */

static struct fb_ops fb_ops42440 = {
	.owner		= THIS_MODULE,
//	.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static struct fb_info *fd_info42440;

static int __init lcd_init(void)
{
	/* 1. allocate a  fb_info structure */
	fd_info42440 = framebuffer_alloc(0, NULL);

	/* 2. set the fucking parameters */
	strcpy(fd_info42440->fix.id, "2440lcd");
	fd_info42440->fix.smem_len = 272*480*16/8;
	fd_info42440->fix.type     = FB_TYPE_PACKED_PIXELS;
	fd_info42440->fix.visual   = FB_VISUAL_TRUECOLOR; /* TFT */
	fd_info42440->fix.line_length = 240*2;

	fd_info42440->var.xres           = 272;
	fd_info42440->var.yres           = 480;
	fd_info42440->var.xres_virtual   = 240;
	fd_info42440->var.yres_virtual   = 320;
	fd_info42440->var.bits_per_pixel = 16;

	/* RGB:565 */
	fd_info42440->var.red.offset     = 11;
	fd_info42440->var.red.length     = 5;

	fd_info42440->var.green.offset   = 5;
	fd_info42440->var.green.length   = 6;

	fd_info42440->var.blue.offset    = 0;
	fd_info42440->var.blue.length    = 5;

	fd_info42440->var.activate       = FB_ACTIVATE_NOW;



	return 0;
}

static void __exit lcd_exit(void)
{
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL v2");
