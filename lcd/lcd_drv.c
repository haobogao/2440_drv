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



//Vertical back porch is the number of inactive lines at the start of a frame, after vertical synchronization period.
#define LCD_VBPD 		2<<24
#define LCD_LINEVAL		(272-1)<<14
#define LCD_VFPD		2<<6
#define LCD_VSPW		10
/*board regs*/
static struct s3c2440_lcd_regs{
	/*lcdcon1  */
	unsigned int LCDCON1;
	unsigned int LCDCON2;		//0X4D000004
	unsigned int LCDCON3;
	unsigned int LCDCON4;		//0X4D00000C
	unsigned int LCDCON5;		//0X4D000010
	unsigned int LCDSADDR1;		//0X4D000014
	unsigned int LCDSADDR2;		//0X4D000018
	unsigned int LCDSADDR3;		//0X4D00001C
	unsigned int REDLUT;		//0X4D000020
	unsigned int GREENLUT;		//0X4D000024
	unsigned int BLUELUT;		//0X4D000028
	unsigned int dummy[0x4c-0x28-0x4];		//useless
    unsigned int DITHMODE;		//0X4D00004C
    unsigned int  TPAL;			//0X4D000050
    unsigned int LCDINTPND;		//0X4D000054
    unsigned int LCDSRCPND;		//0X4D000058
    unsigned int LCDINTMSK;		//0X4D00005C
    unsigned int TCONSEL;		//0X4D000060

};


static struct s3c2440_lcd_struct {
	 struct fb_info *fd_info42440;
	 struct s3c2440_lcd_regs lcd_regs;
	 struct fb_ops fb_ops42440 = {
	 	.owner		= THIS_MODULE,
	 //	.fb_setcolreg	= atmel_lcdfb_setcolreg,
	 	.fb_fillrect	= cfb_fillrect,
	 	.fb_copyarea	= cfb_copyarea,
	 	.fb_imageblit	= cfb_imageblit,
	 };
};

static struct s3c2440_lcd_struct lcd;
static int __init lcd_init(void)
{
	/* 1. allocate a  fb_info structure */
	lcd.fd_info42440 = framebuffer_alloc(0, NULL);

	/* 2. set the fucking parameters */
	strcpy(lcd.fd_info42440->fix.id, "2440lcd");
	lcd.fd_info42440->fix.smem_len = 272*480*16/8;
	lcd.fd_info42440->fix.type     = FB_TYPE_PACKED_PIXELS;
	lcd.fd_info42440->fix.visual   = FB_VISUAL_TRUECOLOR;

	lcd.fd_info42440->var.xres           = 480;
	lcd.fd_info42440->var.yres           = 272;
	lcd.fd_info42440->var.xres_virtual   = 480;
	lcd.fd_info42440->var.yres_virtual   = 272;
	lcd.fd_info42440->var.bits_per_pixel = 16;

	/* RGB:565 */
	lcd.fd_info42440->var.red.offset     = 11;
	lcd.fd_info42440->var.red.length     = 5;

	lcd.fd_info42440->var.green.offset   = 5;
	lcd.fd_info42440->var.green.length   = 6;

	lcd.fd_info42440->var.blue.offset    = 0;
	lcd.fd_info42440->var.blue.length    = 5;

	lcd.fd_info42440->var.activate       = FB_ACTIVATE_NOW;
	lcd.fd_info42440->var;

	/*set HW reg*/
	lcd.lcd_regs.LCDCON1 =4<<8|0x03<<5|0x0c<<1;
	lcd.lcd_regs.LCDCON2 = VBPD;

	return 0;
}

static void __exit lcd_exit(void)
{
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL v2");
