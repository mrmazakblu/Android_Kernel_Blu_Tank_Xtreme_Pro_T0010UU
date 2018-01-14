/*
 * drivers/misc/tfa98xx.c
 *
 * Copyright (c) 2014, WPI (World Peace Industrial Group).  All rights reserved.
 * Author: Nick Li
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <sound/initval.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

//For MTK platform
#include <linux/dma-mapping.h>


/***************** DEBUG ****************/
//#define pr_debug printk
//#define pr_warning printk

/* print the bytes of I2C read and write, should be closed when debugging is finished.. */
#define TEST_DEBUG

/* print the log message, should be closed when debugging is finished.. */
#define TFA98XX_DEBUG

//for MTK I2C burst
#define I2C_USE_DMA

//for SPRD/ROCKCHIP
//#define I2C_BUS_NUM_STATIC_ALLOC

/***************** DEBUG ****************/

#ifdef TFA98XX_DEBUG
#define PRINT_LOG printk
#else
#define PRINT_LOG(...) 
#endif

struct tfa98xx_dev	
{
	struct mutex		lock;
	struct i2c_client	*client;
	struct miscdevice	tfa98xx_device;
	bool deviceInit;
};

static u8 *I2CDMABuf_va = NULL;
static dma_addr_t I2CDMABuf_pa = 0;
static struct i2c_client *g_client = NULL;

static struct tfa98xx_dev tfa98xx[2];
static struct tfa98xx_dev *tfa98xx_dev = NULL;

/* tfa98xx I2C defination ++ */
#define TFA98XX_I2C_NAME   "i2c_tfa9890"
#define tfa9890_dev_r      "tfa9890r"
#define tfa9890_dev_l      "tfa9890l"

#define TFA98XX_I2C_ADDR_L    0x34
#define TFA98XX_I2C_ADDR_R    0x34

#define I2C_STATIC_BUS_NUM        2

#define MAX_BUFFER_SIZE	512
/* tfa98xx I2C defination -- */

#define TFA98XX_REVISIONNUMBER 	0x03
#define TFA9890_REV 					0x80
#define TFA9887_REV 					0x12

static struct i2c_board_info  tfa98xx_i2c_boardinfo[] = {
//	{I2C_BOARD_INFO(tfa9890_dev_r, TFA98XX_I2C_ADDR_L)}, 
	{I2C_BOARD_INFO(tfa9890_dev_l, TFA98XX_I2C_ADDR_R)}, 
};
#if 0
void tfa98xx_i2s_init(void)
{
    printk("Set GPIO for AFE I2S output to external DAC \n");
    mt_set_gpio_mode(GPIO_I2S0_WS_PIN, GPIO_I2S0_WS_PIN_M_I2S1_LRCK);
    mt_set_gpio_mode(GPIO_I2S1_CK_PIN, GPIO_I2S1_CK_PIN_M_I2S1_BCK);
    mt_set_gpio_mode(GPIO_I2S1_DAT_PIN, GPIO_I2S1_DAT_PIN_M_I2S1_DO);
}
#endif
//////////////////////// i2c R/W ////////////////////////////
#ifdef I2C_USE_DMA
static int tfa_i2c_write(struct i2c_client *client, const uint8_t *buf, int len)
{
	int i = 0;
	for(i = 0 ; i < len; i++)
	{
		I2CDMABuf_va[i] = buf[i];
	}

	if(len < 8)
	{
		client->addr = (client->addr & I2C_MASK_FLAG) | I2C_ENEXT_FLAG;
		return i2c_master_send(client, buf, len);
	}
	else
	{
		client->addr = (client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
		client->addr = client->addr | I2C_ENEXT_FLAG;
		return i2c_master_send(client, (unsigned char *)I2CDMABuf_pa, len);
	}    
}

static int tfa_i2c_read(struct i2c_client *client, uint8_t *buf, int len)
{
	int i = 0, ret = 0;

	if(len < 8)
	{
		client->addr = (client->addr & I2C_MASK_FLAG) | I2C_ENEXT_FLAG;
		return i2c_master_recv(client, buf, len);
	}
	else
	{
		client->addr = (client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
		client->addr = client->addr | I2C_ENEXT_FLAG;
		ret = i2c_master_recv(client, (unsigned char *)I2CDMABuf_pa, len);

		if(ret < 0)
		{
			printk("%s: i2c_master_recv(len = %d) returned %d.\n", __func__, len, ret);
			return ret;
		}

		for(i = 0; i < len; i++)
		{
			buf[i] = I2CDMABuf_va[i];
		}
	}
	client->addr = client->addr & I2C_MASK_FLAG;
	return ret;
}
#endif

static ssize_t tfa98xx_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	char tmp[MAX_BUFFER_SIZE];
	int ret;
#ifdef TEST_DEBUG
	int i;
#endif

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (tfa98xx_dev == NULL){
		 PRINT_LOG("The i2c slave address is not exist\n");
		 return -ENODEV;
	}
#ifdef I2C_USE_DMA
	//tfa98xx_dev->client->addr = tfa98xx_dev->client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG | I2C_ENEXT_FLAG;
	mutex_lock(&tfa98xx_dev->lock);
	ret = tfa_i2c_read(tfa98xx_dev->client, tmp, count);
	mutex_unlock(&tfa98xx_dev->lock);
	tfa98xx_dev->client->addr = tfa98xx_dev->client->addr & I2C_MASK_FLAG;
	if(ret <= 0)
	{
		printk("[mtk-tfa] i2c read communcate error: 0x%x\n", ret);
		return ret;
	}
#else
	/* Read data */
	mutex_lock(&tfa98xx_dev->lock);
	ret = i2c_master_recv(tfa98xx_dev->client, tmp, count);
	mutex_unlock(&tfa98xx_dev->lock);

	if (ret < 0) 
	{
		printk("%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
#endif
	if (ret > count) 
	{
		printk("%s: received too many bytes from i2c (%d)\n", __func__, ret);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) 
	{
		pr_warning("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}

#ifdef TEST_DEBUG
	PRINT_LOG("Read from TFA98XX:");
	for(i = 0; i < ret; i++)
	{
		PRINT_LOG(" %02X", tmp[i]);
	}
	PRINT_LOG("\n");
#endif

	return ret;
}

static ssize_t tfa98xx_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	char tmp[MAX_BUFFER_SIZE];
	int ret;
#ifdef TEST_DEBUG
	int i;
#endif

	if (count > MAX_BUFFER_SIZE)
	{
		count = MAX_BUFFER_SIZE;
	}
	if (copy_from_user(tmp, buf, count)) 
	{
		printk("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	if (tfa98xx_dev == NULL){
		 PRINT_LOG("%s : The i2c slave address is not exist\n", __func__);
		 return -ENODEV;
	}
#ifdef I2C_USE_DMA
	//tfa98xx_dev->client->addr = tfa98xx_dev->client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG | I2C_ENEXT_FLAG;
	mutex_lock(&tfa98xx_dev->lock);
	ret = tfa_i2c_write(tfa98xx_dev->client, tmp, count);
	mutex_unlock(&tfa98xx_dev->lock);
	tfa98xx_dev->client->addr = tfa98xx_dev->client->addr & I2C_MASK_FLAG;
	//if (ret != strlen(tmp))
	if(ret <=0 )
	{
		printk("[mtk-tfa] i2c write communcate error: 0x%x\n", ret);
		return ret;
	}

#else
	/* Write data */
	mutex_lock(&tfa98xx_dev->lock);
	ret = i2c_master_send(tfa98xx_dev->client, tmp, count);
	mutex_unlock(&tfa98xx_dev->lock);
	if (ret != count) 
	{
		printk("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}
#endif

#ifdef TEST_DEBUG
	PRINT_LOG("Write to TFA98XX:");
	for(i = 0; i < count; i++)
	{
		PRINT_LOG(" %02X", tmp[i]);
	}
	PRINT_LOG("\n");
#endif	
	return ret;
}
//////////////////////// i2c R/W ////////////////////////////

static int tfa98xx_dev_open(struct inode *inode, struct file *filp)
{

	pr_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

	return 0;
}
/*
static int i2cdev_check(struct device *dev, void *addrp)
{
	struct i2c_client *client = i2c_verify_client(dev);

	if (!client || client->addr != *(unsigned int *)addrp)
		return 0;

	return dev->driver ? -EBUSY : 0;
}
*/
/* This address checking function differs from the one in i2c-core
   in that it considers an address with a registered device, but no
   driver bound to it, as NOT busy. */
/*
static int i2cdev_check_addr(struct i2c_adapter *adapter, unsigned int addr)
{
	return device_for_each_child(&adapter->dev, &addr, i2cdev_check);
}
*/
static long tfa98xx_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	//void __user *argp = (void __user *)arg;
	//int mode = 0;
	int buf;
	int i = 0;;
#if 1
	switch (cmd) 
	{
		/* This part do not work ok now , it is used for stereo application. */
		case I2C_SLAVE:
			buf = arg;	
			//printk("nxp:slave addr = 0x%2x,array=%ld\n",buf,ARRAY_SIZE(tfa98xx));
			tfa98xx_dev = NULL;
			for (i=0;i<ARRAY_SIZE(tfa98xx);i++){
				if (tfa98xx[i].client == NULL)
					continue;
				if ((tfa98xx[i].client->addr&I2C_MASK_FLAG) == buf){
					tfa98xx_dev = &tfa98xx[i];
					//printk("nxp:OK:slave addr  = 0x%2x\n",tfa98xx[i].client->addr&I2C_MASK_FLAG);
					break;
				}
			}
			break;
		case I2C_SLAVE_FORCE:
			break;
		default:
			printk("%s bad ioctl %u\n", __func__, cmd);
			return -EINVAL;
	}
#endif

	return 0;
}

static const struct file_operations tfa98xx_dev_fops = 
{
	.owner	= THIS_MODULE,
	.open	= tfa98xx_dev_open,
	.unlocked_ioctl = tfa98xx_dev_ioctl,
	.llseek	= no_llseek,
	.read	= tfa98xx_dev_read,
	.write	= tfa98xx_dev_write,
};

#if 1
static ssize_t tfa_mtp_show(struct class *class, struct class_attribute *attr,
		char *buf)
{
	int tmp_value= 0;
	printk("songqi:enter tfa_mtp_store\n ");
	//just clear mtp

	//printk("%s %d\n",__func__,__LINE__);
	//int trys = 0;
	//int ret = 0;
	//int smart_value = 1;
	//	i2c_master_recv(tfa98xx->client, mtp_value, 1);
	//printk("%s mtp_value[0]:0x%x 0x%x\n",__func__,mtp_value[0],mtp_value[1]);
	
	g_client->addr = (g_client->addr & I2C_MASK_FLAG) | I2C_ENEXT_FLAG;
	//	if((0x01 & mtp_value[0]) && (0x02 & mtp_value[0]))
	{


		i2c_smbus_write_word_data(g_client, 0x0b,0x5a00);
		i2c_smbus_write_word_data(g_client, 0x80,0x0100);	
		i2c_smbus_write_word_data(g_client, 0x62,0x0008);
	}
	msleep(2000);
	tmp_value = i2c_smbus_read_word_data(g_client, 0x80);
	printk("songqi:tmp_value = 0x%x\n",tmp_value);
	//printk("songqi:smart_value = 0x%x\n",smart_value);	
/*	
	if(0 == tmp_value)
	{
		smart_value = 0;
		//printk("songqi:smart_value(2) = 0x%x\n",smart_value);
	}
*/
	//printk("%s tmp_value[0]:0x%x 0x%x\n",__func__,tmp_value[0],tmp_value[1]);
	/*
	   do
	   {
	   mtp_value[0] = 0x00;
	   i2c_master_recv(tfa98xx->client, mtp_value, 1);	
	   if((mtp_value[0] & 0x100) == 0x100)
	   {
	   trys ++;
	   msleep(1000);
	   }
	   else
	   break;
	   }while(trys<100);
	   */

	//return printk("%d\n", smart_value);
	return snprintf(buf, PAGE_SIZE, "%s\n", tmp_value ? "success":"fail");
}

static ssize_t tfa_mtp_store(struct class *class, struct class_attribute *attr,const char *buf, size_t size)
{
	//just clear mtp

	//printk("%s %d\n",__func__,__LINE__);
	//int trys = 0;
	//int ret = 0;
	//int tmp_value = 0;
	//	i2c_master_recv(tfa98xx->client, mtp_value, 1);
	//printk("%s mtp_value[0]:0x%x 0x%x\n",__func__,mtp_value[0],mtp_value[1]);
	//	tmp_value = i2c_smbus_read_word_data(g_client, 0x80);
	g_client->addr = (g_client->addr & I2C_MASK_FLAG) | I2C_ENEXT_FLAG;
	//	if((0x01 & mtp_value[0]) && (0x02 & mtp_value[0]))
	{


		i2c_smbus_write_word_data(g_client, 0x0b,0x5a00);
		i2c_smbus_write_word_data(g_client, 0x80,0x0100);	
		i2c_smbus_write_word_data(g_client, 0x62,0x0008);
	}
	msleep(2000);
	/*
	   do
	   {
	   mtp_value[0] = 0x00;
	   i2c_master_recv(tfa98xx->client, mtp_value, 1);	
	   if((mtp_value[0] & 0x100) == 0x100)
	   {
	   trys ++;
	   msleep(1000);
	   }
	   else
	   break;
	   }while(trys<100);
	   */

	return size;
}
#endif
static struct class_attribute cls_attr[]={
	__ATTR(tfa_mtp,0664,tfa_mtp_show,tfa_mtp_store),
	};
static struct class *tfa_class = NULL;
static int tfa98xx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	int ret_value=-1;
	int tempvalue1=0;
	int rev_value=0;
	int index = id->driver_data;
	//char tmp[MAX_BUFFER_SIZE];

	printk("%s : Smart Audio tfa98xx probe start %s\n", __func__, id->name);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk("%s : need I2C_FUNC_I2C\n", __func__);
		return  -ENODEV;
	}

	client->dev.coherent_dma_mask = 0xffffffff;
	tfa98xx[index].client   = client;
			tfa98xx_dev = &tfa98xx[index];
	g_client = client;

	/* init mutex and queues */
	mutex_init(&tfa98xx[index].lock);

#ifdef I2C_USE_DMA
	//msleep(50);
	if(I2CDMABuf_va == NULL) {
		I2CDMABuf_va = (u8 *)dma_alloc_coherent(&(client->dev), 4096, &I2CDMABuf_pa, GFP_KERNEL);
		if(!I2CDMABuf_va)
		{
			printk("Allocate TFA98XX DMA I2C Buffer failed!\n");
			return -1;
		}
	}
#if 0		//for i2c test
	tfa98xx->client->addr = tfa98xx->client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG | I2C_ENEXT_FLAG;
	ret = tfa_i2c_write(tfa98xx->client, cmdbuf, 10);
	tfa98xx->client->addr = tfa98xx->client->addr & I2C_MASK_FLAG;
	if (ret != sizeof(cmdbuf))
	{
		TPD_DEBUG("[mtk-tfa] i2c write communcate error: 0x%x\n", ret);
		return -1;
	}
#endif
#endif

	tempvalue1 = i2c_smbus_read_word_data(client, TFA98XX_REVISIONNUMBER);
	rev_value = ((tempvalue1 & 0x00FF)<< 8) | ((tempvalue1 & 0xFF00)>> 8);
	rev_value = rev_value & 0xFFFF;

	PRINT_LOG("tfa98xx_i2c_probe:rev_value=0x%x\n", rev_value);

	if (rev_value == TFA9887_REV) 
	{
		printk("NXP Device detected!\nTFA9887 registered I2C driver!\n");
	}
	else if(rev_value == TFA9890_REV)
	{
		printk("NXP Device detected!\nTFA9890 registered I2C driver!\n");
	}
	else
	{
		printk("NXP Device not found, i2c error %d \n", rev_value);
		//error = -1;
		goto i2c_error;
	}  
	if (tfa_class)
		goto device_finish;

	tfa_class = class_create(THIS_MODULE, "tfa_se");
	ret_value=class_create_file(tfa_class, cls_attr);
	tfa98xx[index].tfa98xx_device.minor = MISC_DYNAMIC_MINOR;
	tfa98xx[index].tfa98xx_device.name = TFA98XX_I2C_NAME;
	tfa98xx[index].tfa98xx_device.fops = &tfa98xx_dev_fops;


	ret = misc_register(&tfa98xx[index].tfa98xx_device);
	if (ret) 
	{
		printk("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}
device_finish:
	mutex_lock(&tfa98xx[index].lock);
	tfa98xx[index].deviceInit = true;
	mutex_unlock(&tfa98xx[index].lock);

	//climax --slave=0x34 -r 0x4 -w 0x780b -r 0x9 -w 0x8219 -r 0x9 -w 0x8618 
#if 0  //tmp
	//*
	   client->addr = client->addr & I2C_MASK_FLAG | I2C_ENEXT_FLAG;
	   tmp[0] = 0x04;
	   tmp[1] = 0x78;
	   tmp[2] = 0x0b;
	   i2c_master_send(client, tmp, 3);	

	   tmp[0] = 0x09;
	   tmp[1] = 0x82;
	   tmp[2] = 0x19;
	   i2c_master_send(client, tmp, 3);	

	   tmp[0] = 0x09;
	   tmp[1] = 0x86;
	   tmp[2] = 0x18;
	   i2c_master_send(client, tmp, 3);	
	   //*/

	tmp[0] = 0x07;
	tmp[1] = 0x8f;
	tmp[2] = 0xe6;
	i2c_master_send(client, tmp, 3);	
#endif
	printk("Tfa98xx probe success!\n");
	return 0;

err_misc_register:
	misc_deregister(&tfa98xx[index].tfa98xx_device);
i2c_error:
	mutex_destroy(&tfa98xx[index].lock);
//err_exit:
	return ret;
}
static int tfa98xx_i2c_remove(struct i2c_client *client)
{
	int index=-1;
	PRINT_LOG("Enter %s.  %d\n", __FUNCTION__, __LINE__);
	index = (client->addr == TFA98XX_I2C_ADDR_L)? 0:1;
#ifdef I2C_USE_DMA
	if(I2CDMABuf_va)
	{
		dma_free_coherent(NULL, 4096, I2CDMABuf_va, I2CDMABuf_pa);
		I2CDMABuf_va = NULL;
		I2CDMABuf_pa = 0;
	}
#endif
	misc_deregister(&tfa98xx[index].tfa98xx_device);
	mutex_destroy(&tfa98xx[index].lock);
	return 0;
}

static void tfa98xx_i2c_shutdown(struct i2c_client *i2c)
{
	PRINT_LOG("Enter %s. +  %4d\n", __FUNCTION__, __LINE__);
}

#undef CONFIG_OF
#ifdef CONFIG_OF
static const struct of_device_id tfa98xx_of_match[] = {
	{ .compatible = "nxp,i2c_tfa98xx", },
	{},
};
MODULE_DEVICE_TABLE(of, tfa98xx_of_match);
#endif
static const struct i2c_device_id tfa98xx_i2c_id[] = {
//	{ tfa9890_dev_r, 0 },
	{ tfa9890_dev_l, 0 },
	{ }
};
static struct i2c_driver tfa98xx_i2c_driver = {
	.driver = {
		.name = TFA98XX_I2C_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = tfa98xx_of_match,
#endif
	},
	.probe =    tfa98xx_i2c_probe,
	.remove =   tfa98xx_i2c_remove,
	.id_table = tfa98xx_i2c_id,
	.shutdown = tfa98xx_i2c_shutdown,
};

static int smart_pa_probe(struct platform_device *pdev) 
{
	int ret = 0;
	PRINT_LOG("Loading tfa98xx probe\n");
	ret = i2c_add_driver(&tfa98xx_i2c_driver);
	if(ret != 0)
	{
		printk(KERN_ERR "Failed to register tfa98xx I2C driver: %d\n",ret);
		return -ENODEV;
	}
	return 0;
}

static int smart_pa_remove(struct platform_device *pdev)
{   
	i2c_del_driver(&tfa98xx_i2c_driver);
	return 0;
}

static struct platform_driver smart_pa_driver = {
	.probe      = smart_pa_probe,
	.remove     = smart_pa_remove,    
	.driver     = {
		.name  = "smart_pa",
	}
};

static int __init tfa98xx_init(void)
{
	int ret = 0;
	struct platform_device *tfa98xx_device;
	PRINT_LOG("Loading tfa98xx driver\n");
	ret = i2c_register_board_info(I2C_STATIC_BUS_NUM, tfa98xx_i2c_boardinfo, ARRAY_SIZE(tfa98xx_i2c_boardinfo));
	if (ret < 0) {
		printk("i2c_register_board_info error %d\n", ret);
		return -ENODEV;
	}



//	tfa98xx_i2s_init();
	tfa98xx_device = platform_device_alloc("smart_pa", -1);
	if (tfa98xx_device == NULL) {
		PRINT_LOG("platform alloc error\n");
		return -ENOMEM;
	}
	if (platform_device_add(tfa98xx_device)){
		platform_device_put(tfa98xx_device);
		return -ENODEV;
	}
	if (platform_driver_register(&smart_pa_driver))
	{
		return -ENODEV;
	}

	return ret;
}

arch_initcall(tfa98xx_init);

static void __exit tfa98xx_exit(void)
{
    PRINT_LOG("tfa98xx_exit\n");
}
module_exit(tfa98xx_exit);


MODULE_AUTHOR("Nick Li <nick.li@wpi-group.com>");
MODULE_DESCRIPTION("TFA98XX Audio Codec I2C driver");
MODULE_LICENSE("GPL");
