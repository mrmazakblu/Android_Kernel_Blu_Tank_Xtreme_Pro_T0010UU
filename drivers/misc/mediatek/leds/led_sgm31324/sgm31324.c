/*
 * gradual_led - REALLY gradual_led memory mapping demonstration.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include "mt-plat/mtgpio.h"
#include <mt-plat/mt_gpio.h>
#include <mt-plat/mt_gpio_core.h>
#include <linux/err.h>
#include <linux/major.h>
//
#include "sgm31324.h"
#define OPEN_FLASH_DEBUG
#ifdef OPEN_FLASH_DEBUG
#define LED_DEBUG 	printk
#else
#define LED_DEBUG(format, ...)
#endif
unsigned char SN_IIC_WriteReg(unsigned char bRegAddr, unsigned char bRegData);
struct i2c_client * i2c_connect_client = NULL;
static struct class *g_sgm31324;
static struct device *g_sgm31324_dev;
static int g_major;
static char s_led_rgb_mask =0x0;
#define LED_RED_TYPE    0x10
#define LED_GREEN_TYPE  0x04
#define LED_BLUE_TYPE   0x01
#define LED_Enable_On   0x40

bool sgm31324_flag = false;

extern void i2c_eeprom_write_byte(uint32_t address, uint8_t wr_data);
//
unsigned char SN_IIC_WriteReg(unsigned char bRegAddr, unsigned char bRegData)
{
	u8 databuf[10];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);
	databuf[0] = bRegAddr;
	databuf[1] = bRegData;

	res = i2c_master_send(i2c_connect_client, databuf, 2);

	if(res != 2)
	{
		printk("cuizhaojun:I2C send failed addr = 0x%x, data = 0x%x !!\n",databuf[0],databuf[1]);
		return -1;
	}

		//printk("cuizhaojun:I2C send data! addr = 0x%x, data = 0x%x !!\n",databuf[0],databuf[1]);

	return 0;
}
//

void  Disable_Auto_blink(void)
{
  SN_IIC_WriteReg(0x09, 0x06);// reg9[0] bit 0 = 0;
  SN_IIC_WriteReg(0x04, 0x00);
}

void  enable_Auto_blink(void)
{
  SN_IIC_WriteReg(0x09, 0xAF);// reg9[0] bit 0 = 0;
  SN_IIC_WriteReg(0x04, 0x82);
}


void led_rgb_all_off(void)
{
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	SN_IIC_WriteReg(0x04, 0x00);//initialization LED off
	SN_IIC_WriteReg(0x06, 0x00);//set current is 0.125mA
	SN_IIC_WriteReg(0x07, 0x00);//set current is 0.125mA
	SN_IIC_WriteReg(0x08, 0x00);//set current is 0.125mA
	SN_IIC_WriteReg(0x04, 0x00);//initialization LED off
	s_led_rgb_mask=0;
}
void led_blue_on(void){
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
        SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
	SN_IIC_WriteReg( 0x06, 0x77);//set current is 15mA
        SN_IIC_WriteReg( 0x00, 0x00);//mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x04, 0x41);//turn om led
}

void led_green_on(void){
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
        SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
	SN_IIC_WriteReg( 0x07, 0x77);//set current is 15mA
        SN_IIC_WriteReg( 0x00, 0x00);//mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x04, 0x44);//turn om led
}
//
void led_red_on(void){
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
        SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
	SN_IIC_WriteReg( 0x08, 0x77);//set current is 15mA
        SN_IIC_WriteReg( 0x00, 0x00);//mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x04, 0x50);//turn om led
}
//
void led_rgb_red_on(int level)
{
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	s_led_rgb_mask|=LED_RED_TYPE + LED_Enable_On;
	SN_IIC_WriteReg( 0x08, level);//set current is 15mA
	SN_IIC_WriteReg( 0x00, 0x00);//mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x04, s_led_rgb_mask);//turn on led

}
//
void led_rgb_green_on(int level)
{
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	s_led_rgb_mask|=LED_GREEN_TYPE + LED_Enable_On;
	SN_IIC_WriteReg( 0x07, level);//set current is 15mA
	SN_IIC_WriteReg( 0x00, 0x00);//mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x04, s_led_rgb_mask);//turn on led
}
//
void led_rgb_blue_on(int level)
{
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	s_led_rgb_mask|=LED_BLUE_TYPE + LED_Enable_On;
	SN_IIC_WriteReg( 0x06, level);//set current is 15mA
	SN_IIC_WriteReg( 0x00, 0x00);//mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x04, s_led_rgb_mask);//turn on led

}
void led_rgb_on(char type,int level)
{
	switch(type)
	{
	case 0:
		led_rgb_red_on(level);
		break;
	case 1:
		led_rgb_green_on(level);
		break;
	case 2:
		led_rgb_blue_on(level);
		break;
	default:
		break;
	}
}
void led_rgb_on_demo(void)
{
	unsigned char led_type_temp;
	unsigned char brigntness_max=0x77;
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	led_type_temp=0;//red
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);
	//
	led_type_temp=1;//green
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);
	//led_rgb_all_off();
	//
	led_type_temp=2;//blue
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);

	//red+green
	led_type_temp=0;//
	led_rgb_on(led_type_temp,brigntness_max);
	led_type_temp=1;//
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);

	//red+blue
	led_type_temp=0;//
	led_rgb_on(led_type_temp,brigntness_max);
	led_type_temp=2;//
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);

	//green+blue
	led_type_temp=1;//
	led_rgb_on(led_type_temp,brigntness_max);
	led_type_temp=2;//
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);

	//red+ green +blue
	led_type_temp=0;//
	led_rgb_on(led_type_temp,brigntness_max);
	led_type_temp=1;//
	led_rgb_on(led_type_temp,brigntness_max);
	led_type_temp=2;//
	led_rgb_on(led_type_temp,brigntness_max);
	mdelay(1000);
	led_rgb_all_off();
	mdelay(1000);
}
void led_rgb_red_breath(unsigned int level,unsigned char risetime,unsigned char period){
	/*
	 * Blue flash time period: 2.5s, rise/fall 1s, sleep 0.5s
	 * reg5 = 0xaa, reg1 = 0x12
	 */
	unsigned char breath_mask=0x20;
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	s_led_rgb_mask|=LED_RED_TYPE;
	if(s_led_rgb_mask&LED_RED_TYPE)
	{
		breath_mask|=0x20 + LED_Enable_On;
	}
	else if (s_led_rgb_mask&LED_GREEN_TYPE)
	{
		breath_mask|=0x08 + LED_Enable_On;
	}
	else if (s_led_rgb_mask&LED_BLUE_TYPE)
	{
		breath_mask|=0x02 + LED_Enable_On;
	}
	SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
        SN_IIC_WriteReg( 0x00, 0x20);// mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x08, level);//set current is 15mA
	SN_IIC_WriteReg( 0x05, risetime);//rase time
	SN_IIC_WriteReg( 0x01, period);//dry flash period
	SN_IIC_WriteReg( 0x02, 0x00);//reset internal counter
	SN_IIC_WriteReg( 0x04, breath_mask);//allocate led1 to timer1
	SN_IIC_WriteReg( 0x02, 0x56);//led flashing(curerent ramp-up and down countinuously)
}
//
void led_rgb_green_breath(unsigned int level,unsigned char risetime,unsigned char period)
{
	/*
	 * Green flash time period: 2.5s, rise/fall 1s, sleep 0.5s
	 * reg5 = 0xaa, reg1 = 0x12
	 */
	unsigned char breath_mask=0x08;
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	s_led_rgb_mask|=LED_GREEN_TYPE;
	if(s_led_rgb_mask&LED_RED_TYPE)
	{
		breath_mask|=0x20 + LED_Enable_On;
	}
	else if (s_led_rgb_mask&LED_GREEN_TYPE)
	{
		breath_mask|=0x08 + LED_Enable_On;
	}
	else if (s_led_rgb_mask&LED_BLUE_TYPE)
	{
		breath_mask|=0x02 + LED_Enable_On;
	}
	SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
        SN_IIC_WriteReg( 0x00, 0x20);// mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x07, level);//set current is 15mA
	SN_IIC_WriteReg( 0x05, risetime);//rase time
	SN_IIC_WriteReg( 0x01, period);//dry flash period
	SN_IIC_WriteReg( 0x02, 0x00);//reset internal counter
	SN_IIC_WriteReg( 0x04, breath_mask);//allocate led1 to timer1
	SN_IIC_WriteReg( 0x02, 0x56);//led flashing(curerent ramp-up and down countinuously)
}

void led_rgb_blue_breath(unsigned int level,unsigned char risetime,unsigned char period)
{
	/*
	 * RED flash time period: 2.5s, rise/fall 1s, sleep 0.5s
	 * reg5 = 0xaa, reg1 = 0x12
	 */
	unsigned char breath_mask=0x02;
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	s_led_rgb_mask|=LED_BLUE_TYPE;
	if(s_led_rgb_mask&LED_RED_TYPE)
	{
		breath_mask|=0x20 + LED_Enable_On;
	}
	else if (s_led_rgb_mask&LED_GREEN_TYPE)
	{
		breath_mask|=0x08 + LED_Enable_On;
	}
	else if (s_led_rgb_mask&LED_BLUE_TYPE)
	{
		breath_mask|=0x02 + LED_Enable_On;
	}
	SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
        SN_IIC_WriteReg( 0x00, 0x20);// mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x06, level);//set current is 15mA
	SN_IIC_WriteReg( 0x05, risetime);//rase time
	SN_IIC_WriteReg( 0x01, period);//dry flash period
	SN_IIC_WriteReg( 0x02, 0x00);//reset internal counter
	SN_IIC_WriteReg( 0x04, breath_mask);//allocate led1 to timer1
	SN_IIC_WriteReg( 0x02, 0x56);//led flashing(curerent ramp-up and down countinuously)
}
//
void led_rgb_breath(char type,unsigned int level,unsigned char risetime,unsigned char period)
{
	switch(type)
	{
	case 0:
		led_rgb_red_breath(level,risetime,period);
		break;
	case 1:
		led_rgb_green_breath(level,risetime,period);
		break;
	case 2:
		led_rgb_blue_breath(level,risetime,period);
		break;
	default:
		break;
	}
}
//
void led_red_breath(void)
{
	/*
	 * Blue flash time period: 2.5s, rise/fall 1s, sleep 0.5s
	 * reg5 = 0xaa, reg1 = 0x12
	 */
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
        SN_IIC_WriteReg( 0x00, 0x20);// mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x08, 0x77);//set current is 15mA
	SN_IIC_WriteReg( 0x05, 0xaa);//rase time
	SN_IIC_WriteReg( 0x01, 0x12);//dry flash period
	SN_IIC_WriteReg( 0x02, 0x00);//reset internal counter
	SN_IIC_WriteReg( 0x04, 0x60);//allocate led1 to timer1
	SN_IIC_WriteReg( 0x02, 0x56);//led flashing(curerent ramp-up and down countinuously)
}
//
void led_green_breath(void)
{
	/*
	 * Green flash time period: 2.5s, rise/fall 1s, sleep 0.5s
	 * reg5 = 0xaa, reg1 = 0x12
	 */
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
        SN_IIC_WriteReg( 0x00, 0x20);// mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x07, 0x77);//set current is 15mA
	SN_IIC_WriteReg( 0x05, 0xaa);//rase time
	SN_IIC_WriteReg( 0x01, 0x12);//dry flash period
	SN_IIC_WriteReg( 0x02, 0x00);//reset internal counter
	SN_IIC_WriteReg( 0x04, 0x48);//allocate led1 to timer1
	SN_IIC_WriteReg( 0x02, 0x56);//led flashing(curerent ramp-up and down countinuously)
}

void led_blue_breath(void){
	/*
	 * RED flash time period: 2.5s, rise/fall 1s, sleep 0.5s
	 * reg5 = 0xaa, reg1 = 0x12
	 */
    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	SN_IIC_WriteReg( 0x04, 0x00);// initialization LED off
        SN_IIC_WriteReg( 0x00, 0x20);// mode set---IC work when both SCL and SDA goes high
	SN_IIC_WriteReg( 0x06, 0x77);//set current is 15mA
	SN_IIC_WriteReg( 0x05, 0xaa);//rase time
	SN_IIC_WriteReg( 0x01, 0x12);//dry flash period
	SN_IIC_WriteReg( 0x02, 0x00);//reset internal counter
	SN_IIC_WriteReg( 0x04, 0x42);//allocate led1 to timer1
	SN_IIC_WriteReg( 0x02, 0x56);//led flashing(curerent ramp-up and down countinuously)
}
void led_rgb_breath_demo(void)
{
	unsigned char led_type_temp;
	unsigned char brigntness=0x77;
	unsigned char risetime_t=0xaa;
	unsigned char period_t =0x12;

    LED_DEBUG("cuizhaojun:func==%s,line ==%d\n",__func__,__LINE__);
	//
	led_type_temp=0;//red
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();
	//
	led_type_temp=1;//green
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();
	//
	led_type_temp=2;//blue
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();

	led_type_temp=0;//red+green
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	led_type_temp=1;//
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();
	//
	led_type_temp=0;//red+blue
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	led_type_temp=2;//
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();
	//

	led_type_temp=1;//green+blue
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	led_type_temp=2;//
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();

	led_type_temp=0;//red+green+blue
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	led_type_temp=1;//
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	led_type_temp=2;//
	led_rgb_breath(led_type_temp,brigntness,risetime_t,period_t);
	mdelay(5000);
	mdelay(5000);
	led_rgb_all_off();
}
static ssize_t led_on_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return -EINVAL;
}

static ssize_t led_on_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	ssize_t ret = -EINVAL;
	unsigned int rgb_type_temp;
	ret = kstrtouint(buf, 10, &rgb_type_temp);
	if (ret==0){
		printk("rgb_type_temp = %d\n", rgb_type_temp);
		switch (rgb_type_temp)
		{
		case 1://red
			led_red_on();
			break;
		case 2://green
			led_green_on();
			break;
		case 3://blue
			led_blue_on();
			break;
		case 4://
			led_rgb_on_demo();
			break;
		case 5:
			led_rgb_all_off();
			break;
		}
		return n;
	}
	printk("rgb_type_temp store fail \n");
	return 0;
}
static ssize_t led_breath_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return -EINVAL;
}

static ssize_t led_breath_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	ssize_t ret = -EINVAL;
	unsigned int rgb_type_temp;
	ret = kstrtouint(buf, 10, &rgb_type_temp);
	if (ret==0){
		printk("rgb_type_temp = %d\n", rgb_type_temp);
		switch (rgb_type_temp)
		{
		case 1://red
			led_red_breath();
			break;
		case 2://green
			led_green_breath();
			break;
		case 3://blue
			led_blue_breath();
			break;
		case 4://
			led_rgb_breath_demo();
			break;
		case 5://
			led_rgb_all_off();
			break;
		}
		return n;
	}
	printk("rgb_type_temp store fail \n");
	return 0;
}
//
static struct file_operations sgm31324_fops = {
	.owner = THIS_MODULE,
};
static DEVICE_ATTR(led_on, 0664, led_on_show, led_on_store);
static DEVICE_ATTR(led_breath, 0664, led_breath_show, led_breath_store);
static int sgm31324_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	int result=0;
	//
	LED_DEBUG("sgm31324_i2c_probe entry --yangcl\n");
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		dev_err(&client->dev, "gradual_led driver: client is not i2c capable.\n");
		result = -ENODEV;
		goto i2c_functioality_failed;
	}
	//
	i2c_connect_client = client;
	//
	g_major = register_chrdev(0, "sgm31324", &sgm31324_fops);
	g_sgm31324 = class_create(THIS_MODULE, "sgm31324");
	g_sgm31324_dev = device_create(g_sgm31324, NULL, MKDEV(g_major, 0), NULL, "sgm31324");
	if (device_create_file(g_sgm31324_dev, &dev_attr_led_on) < 0)
	pr_err("Failed to create device file(%s)!\n", dev_attr_led_on.attr.name);
	if (device_create_file(g_sgm31324_dev, &dev_attr_led_breath) < 0)
	pr_err("Failed to create device file(%s)!\n", dev_attr_led_breath.attr.name);
	//
	SN_IIC_WriteReg(0x06, 0x00);//set current is 0.125mA
	SN_IIC_WriteReg(0x04, 0x00);//turn on leds

    Disable_Auto_blink();

    sgm31324_flag = true;

	return 0;
	i2c_functioality_failed:
	dev_err(&client->dev,"sgm31324_led driver init failed.\n");
	return result;
}
//
static int sgm31324_i2c_remove(struct i2c_client *client)
{
    i2c_connect_client = NULL;
    i2c_unregister_device(client);
    return 0;
}
//
static const struct i2c_device_id sgm31324_i2c_id[] = {
    { SGM31324_I2C_NAME, 0 },
    { },
};
//
struct of_device_id led_rgb_of_match[] = {
	{ .compatible = "mediatek,i2c_led_rgb", },
};
/*----------------------------------------------------------------------------*/

static struct i2c_driver sgm31324_i2c_driver = {
    .driver = {
        .name = SGM31324_I2C_NAME,
		.of_match_table =  led_rgb_of_match,
    },
    .probe    =  sgm31324_i2c_probe,
    .remove   =  sgm31324_i2c_remove,
    .id_table =  sgm31324_i2c_id,
};


/*----------------------------------------------------------------------------*/
static int __init sgm31324_init(void)
{
    int  ret = 0;

    LED_DEBUG(" sgm31324_init ----yangcl\n");
    if(i2c_add_driver(&sgm31324_i2c_driver))
    {
	LED_DEBUG("sgm31324 add driver error\n");
	return -1;
    }
    return ret;

}

static void __exit gradual_led_exit(void)
{
    i2c_del_driver(&sgm31324_i2c_driver);
    return;
}

late_initcall_sync(sgm31324_init);
module_exit(gradual_led_exit);

MODULE_AUTHOR("DROI DRIVER");
MODULE_DESCRIPTION("led rgb sgm31324 driver");
MODULE_LICENSE("GPL");

