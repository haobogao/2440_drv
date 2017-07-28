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
#include <linux/fs.h>
#include <linux/io.h>





/* 480*272*3 */


//Vertical back porch is the number of inactive lines at the start of a frame, after vertical synchronization period.
#define LCD_VBPD 		2<<24
#define LCD_LINEVAL		(272-1)<<14
#define LCD_VFPD		2<<6
#define LCD_VSPW		10

struct reg_addr_t{
	/* prefix "phy" means physical register address while the "vm" means virtual address
	 *  which indicate physical address after kernel mmap  */
	unsigned int vir;
	unsigned int phy;
};



/*
 	* set gpio
 		*
 		* gpb0 ---> LCD background led
 		*
 		* VD[0:23] -- GPD[0-15] GPC[8-15]
 		*
 		* GPG4 ------LCD_PWREN
 		* LEND  -----GPC0
 		* VCLK	-----GPC1
 		* VLINE -----GPC2
 		* VFRAME --- GPC3
 		* VM/VDEN ---- GPC4
 		* LCD_LPCOE----GPC5
 		* LCD_LPCREV -----GPC6
 		* LCD_LPCREVB -----GPC7

*/
struct lcd_port_t{

	struct reg_addr_t gpc_con;
	struct reg_addr_t gpc_dat;

	struct reg_addr_t gpd_con;
	struct reg_addr_t gpd_dat;

	struct reg_addr_t gpb_con;
	struct reg_addr_t gpb_dat;

	struct reg_addr_t gpg_con;
	struct reg_addr_t gpg_dat;

};

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
	 struct  lcd_port_t * lcd_port;
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



#define ON 	1
#define OFF 0


static struct s3c2440_lcd_struct lcd;

static inline void lcd_background_led_switch(char stat)
{
	if(stat == ON){
		lcd.lcd_port->gpb_dat.vir |= (0x01 << 0);
	}else if(stat == OFF){
		lcd.lcd_port->gpb_dat.vir &= ~(0x01 << 0);
	}else
		printk(KERN_NOTICE"@ lcd_drv.c:lcd_background_led_switch dummy parameter!");
}

static inline void lcd_switch(char stat)
{
	if(stat == ON){
		lcd.lcd_regs.LCDCON1 |= (0x01 << 0);
		lcd.lcd_regs.LCDCON5 |= (0x01<<3);
	}else if(stat == OFF){
		lcd.lcd_regs.LCDCON1 &= ~(0x01 << 0);
		lcd.lcd_regs.LCDCON5 &= ~(0x01<<3);
	}else
		printk(KERN_NOTICE"@ lcd_drv.c:lcd_switch dummy parameter!");
}


static int __init lcd_init(void)
{
	/* 1. allocate a  fb_info structure */
	lcd.fd_info42440 = framebuffer_alloc(0, NULL);

	/* 2. set the fucking parameters */
		/*set  fix*/
	strcpy(lcd.fd_info42440->fix.id, "2440lcd");
	lcd.fd_info42440->fix.smem_len = 272*480*16/8;
	lcd.fd_info42440->fix.type     = FB_TYPE_PACKED_PIXELS;
	lcd.fd_info42440->fix.visual   = FB_VISUAL_TRUECOLOR;

	/*set var	*/
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


	/*
	 	* set gpio
	 		*
	 		* gpb0 ---> LCD background led
	 		*
	 		* VD[0:23] -- GPD[0-15] GPC[8-15]
	 		*
	 		* GPG4 ------LCD_PWREN
	 		* LEND  -----GPC0
	 		* VCLK	-----GPC1
	 		* VLINE -----GPC2
	 		* VFRAME --- GPC3
	 		* VM/VDEN ---- GPC4
	 		* LCD_LPCOE----GPC5
	 		* LCD_LPCREV -----GPC6
	 		* LCD_LPCREVB -----GPC7

	*/



		//set port physical address
	lcd.lcd_port->gpb_con.phy =0x56000010;
	lcd.lcd_port->gpb_dat.phy =0x56000014;
	lcd.lcd_port->gpc_con.phy =0x56000020;
	lcd.lcd_port->gpc_dat.phy =0x56000024;
	lcd.lcd_port->gpd_con.phy =0x56000030;
	lcd.lcd_port->gpd_dat.phy =0x56000034;
	lcd.lcd_port->gpg_con.phy = 0x56000060;
	lcd.lcd_port->gpg_dat.phy = 0x56000064;

		//mmap all regs
	lcd.lcd_port->gpb_con.vir = ioremap(lcd.lcd_port->gpb_con.phy,16);
	lcd.lcd_port->gpb_dat.vir = ioremap(lcd.lcd_port->gpb_dat.phy,16);
	lcd.lcd_port->gpc_con.vir = ioremap(lcd.lcd_port->gpc_con.phy,16);
	lcd.lcd_port->gpc_dat.vir = ioremap(lcd.lcd_port->gpc_dat.phy,16);
	lcd.lcd_port->gpd_con.vir = ioremap(lcd.lcd_port->gpd_con.phy,16);
	lcd.lcd_port->gpd_dat.vir = ioremap(lcd.lcd_port->gpd_dat.phy,16);
	lcd.lcd_port->gpg_con.vir = ioremap(lcd.lcd_port->gpg_con.phy,16);
	lcd.lcd_port->gpg_dat.vir = ioremap(lcd.lcd_port->gpg_dat.phy,16);
	lcd.lcd_regs = ioremap(lcd.lcd_regs,sizeof(struct s3c2440_lcd_regs));

	//set background led
	//configure gpb  to output mode
	lcd.lcd_port->gpb_con.vir &= ~(3<<0);			//lowest two bit set to zero
	lcd.lcd_port->gpb_con.vir |= 0x03;

	//set gpg4 to LCD_POWEN .output value of LCD_PWREN pin is controlled by ENVID
	lcd.lcd_port->gpg_con.vir &= ~(3<< (2*4) );
	lcd.lcd_port->gpg_con.vir |=	3<<(2*4);

	/*set HW reg*/
	lcd.lcd_regs.LCDCON1 =4<<8|0x03<<5|0x0c<<1;
	lcd.lcd_regs.LCDCON2 = LCD_LINEVAL|LCD_VBPD|LCD_VFPD|LCD_VSPW;
	lcd.lcd_regs.LCDCON3 = 2<<19|479<<8|2;
	lcd.lcd_regs.LCDCON4 = 41;
	lcd.lcd_regs.LCDCON5 = 1<<11;

	/*allocate the frame buff*/
	lcd.fd_info42440->screen_base = dma_alloc_writecombine(NULL, lcd.fd_info42440->fix.smem_len , lcd.fd_info42440->fix.smem_start , GFP_KERNEL);
	lcd.lcd_regs.LCDSADDR1 = ( lcd.fd_info42440->fix.smem_start >>1 )  & ~(0x03<<30);
//	LCDBASEL = LCDBASEU + (PAGEWIDTH + OFFSIZE) ¡Á (LINEVAL + 1)
	lcd.lcd_regs.LCDSADDR2 = ( ( (lcd.fd_info42440->fix.smem_start + lcd.fd_info42440->fix.smem_len ) >> 1 + 1 )  & 0x1fffff );
	lcd.lcd_regs.LCDSADDR3 = 272*16/16;
	lcd_switch(ON);
	lcd_background_led_switch(ON);	//open the lcd light
	register_framebuffer(lcd.fd_info42440);
	return 0;
}

static void __exit lcd_exit(void)
{
	unregister_framebuffer(lcd.fd_info42440);
	dma_free_writecombine(NULL,lcd.fd_info42440->fix.smem_len,lcd.fd_info42440->screen_base ,lcd.fd_info42440->fix.smem_start);
	framebuffer_release(lcd.fd_info42440);
	iounmap(lcd.lcd_port->gpb_con.vir);
	iounmap(lcd.lcd_port->gpb_dat.vir);
	iounmap(lcd.lcd_port->gpc_con.vir);
	iounmap(lcd.lcd_port->gpc_dat.vir);
	iounmap(lcd.lcd_port->gpg_con.vir);
	iounmap(lcd.lcd_port->gpg_dat.vir);
	iounmap(lcd.lcd_port->gpd_con.vir);
	iounmap(lcd.lcd_port->gpd_dat.vir);
	lcd_background_led_switch(OFF);
	lcd_switch(OFF);

}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL v2");
