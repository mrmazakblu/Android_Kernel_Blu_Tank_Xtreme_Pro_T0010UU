/* 
 * drivers/input/touchscreen/CST2XX.c
 *
 * hynitron TouchScreen driver. 
 *
 * Copyright (c) 2015  hynitron
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
 * VERSION      	DATE			AUTHOR
 *  1.0		    2015-10-12		    Tim
 *
 * note: only support mulititouch
 */


#include "hyn_cst3_core.h"
#include "hyn_cst3_fw.h"

#define ANDROID_TOOL_SURPORT
#ifdef ANDROID_TOOL_SURPORT
int cst3xx_firmware_info(struct i2c_client * client);
int cst3xx_update_firmware(struct i2c_client * client, const unsigned char *pdata);
unsigned short g_unnormal_mode = 0;
unsigned short g_cst3xx_tx = 0;
unsigned short g_cst3xx_rx = 0;
#endif

#ifdef HYN_GESTURE
extern int hyn_Gesture_init(struct input_dev *input_dev);
extern void hyn_read_Gestruedata(void);
extern int hyn_gesture_sysfs(struct i2c_client * client);
struct input_dev *hyn_input_dev=NULL;
bool   gestrue_en=false;
bool   gestrue_lcd_flag=false;
struct wake_lock hyn_gesture_chrg_lock;
#endif 

//#define ICS_SLOT_REPORT
#define REPORT_XY_SWAP
#define SLEEP_CLEAR_POINT
//#define HIGH_SPEED_IIC_TRANSFER
#define TRANSACTION_LENGTH_LIMITED

#ifdef ICS_SLOT_REPORT
#include <linux/input/mt.h> // Protocol B, for input_mt_slot(), input_mt_init_slot()
#endif

#define TPD_PROXIMITY
#ifdef TPD_PROXIMITY
#include "hwmsensor.h"
#include "hwmsen_dev.h"
#include "sensors_io.h"
#include <linux/wakelock.h> 
u8 tpd_proximity_flag   = 0;     //flag whether start alps
u8 tpd_proximity_detect = 1;     //0-->close ; 1--> far away
#endif

//#define CONFIG_TP_ESD_PROTECT
#ifdef CONFIG_TP_ESD_PROTECT
#define SWITCH_ESD_OFF                  0
#define SWITCH_ESD_ON                   1
struct workqueue_struct *cst3xx_esd_workqueue;
#endif

#define GTP_RST_PORT    0
#define GTP_INT_PORT    1

#include <linux/hw_module_info.h>
static char fw_version[20] = "v";
static char tpd_chip_name[10] = "HYN";
static hw_module_info hw_info = {
	.type = HW_MODULE_TYPE_CTP,
	.id = 0x0,
	.priority = HW_MODULE_PRIORITY_CTP,
	.name = tpd_chip_name,
	.vendor = fw_version,
	.more = "already"
};


static int tpd_flag = 0;

static unsigned int touch_irq = 0;

static DECLARE_WAIT_QUEUE_HEAD(waiter);
extern struct tpd_device  *tpd;
static struct task_struct *thread = NULL;
static struct i2c_client *g_i2c_client = NULL; 


static unsigned char  report_flag = 0;
static unsigned char  key_index = 0xFF;

static unsigned int   g_cst3xx_ic_version  = 0;
static unsigned int   g_cst3xx_ic_checksum = 0;
static unsigned int   g_cst3xx_ic_type=0;


#define TPD_DEVICE_NAME			    "Hyn_device" //TPD_DEVICE
#define TPD_DRIVER_NAME			    "Hyn_driver"
#define TPD_MAX_FINGERS			    5
#define TPD_MAX_X				    720
#define TPD_MAX_Y				    1280
#define CST3XX_I2C_ADDR				0x38
#define I2C_BUS_NUMBER              0 //IIC bus num for mtk

#define CST3XX_DEBUG(fmt,arg...)  printk("HYN:[LINE=%d]"fmt,__LINE__, ##arg)
#define CST3XX_ERROR(fmt,arg...)  printk("HYN:[LINE=%d]"fmt,__LINE__, ##arg)

static int cst3xx_boot_update_fw(struct i2c_client   *client,  unsigned char *pdata);

static unsigned char *pcst3xx_update_firmware = (unsigned char *)cst3_fw ; //the updating firmware

#ifdef HIGH_SPEED_IIC_TRANSFER
int cst3xx_i2c_read(struct i2c_client *client, unsigned char *buf, int len) 
{ 
	struct i2c_msg msg; 
	int ret = -1; 
	int retries = 0; 
	
	msg.flags |= I2C_M_RD; 
	msg.addr   = client->addr;
	msg.len    = len; 
	msg.buf    = buf;	

	while (retries < 5) { 
		ret = i2c_transfer(client->adapter, &msg, 1); 
		if(ret == 1)
			break; 
		retries++; 
	} 
	
	return ret; 
} 


/*******************************************************
Function:
    read data from register.
Input:
    buf: first two byte is register addr, then read data store into buf
    len: length of data that to read
Output:
    success: number of messages
    fail:	negative errno
*******************************************************/
int cst3xx_i2c_read_register(struct i2c_client *client, unsigned char *buf, int len) 
{ 
	struct i2c_msg msgs[2]; 
	int ret = -1; 
	int retries = 0; 
	
	msgs[0].flags = client->flags & I2C_M_TEN;
	msgs[0].addr  = client->addr;  
	msgs[0].len   = 2;
	msgs[0].buf   = buf; 

	msgs[1].flags |= I2C_M_RD;
	msgs[1].addr   = client->addr; 
	msgs[1].len    = len; 
	msgs[1].buf    = buf;

	while (retries < 5) { 
		ret = i2c_transfer(client->adapter, msgs, 2); 
		if(ret == 2)
			break; 
		retries++; 
	} 
	
	return ret; 
} 

int cst3xx_i2c_write(struct i2c_client *client, unsigned char *buf, int len) 
{ 
	struct i2c_msg msg; 
	int ret = -1; 
	int retries = 0;

	msg.flags = client->flags & I2C_M_TEN; 
	msg.addr  = client->addr; 
	msg.len   = len; 
	msg.buf   = buf;		  
	  
	while (retries < 5) { 
		ret = i2c_transfer(client->adapter, &msg, 1); 
		if(ret == 1)
			break; 
		retries++; 
	} 	
	
	return ret; 
}

#else
int cst3xx_i2c_read(struct i2c_client *client, unsigned char *buf, int len) 
{ 
	int ret = -1; 
	int retries = 0; 
	
    client->timing  = 370;
    client->addr   |= I2C_ENEXT_FLAG;

	while (retries < 5) { 
		ret = i2c_master_recv(client, buf, len); 
		if(ret<=0) 
		    retries++;
        else
            break; 
	} 
	
	return ret; 
} 

int cst3xx_i2c_write(struct i2c_client *client, unsigned char *buf, int len) 
{ 
	int ret = -1; 
	int retries = 0; 
	
    client->timing  = 370;
    client->addr   |= I2C_ENEXT_FLAG;

	while (retries < 5) { 
		ret = i2c_master_send(client, buf, len); 
		if(ret<=0) 
		    retries++;
        else
            break; 
	} 
	
	return ret; 
}

int cst3xx_i2c_read_register(struct i2c_client *client, unsigned char *buf, int len) 
{ 
	int ret = -1; 
    
    ret = cst3xx_i2c_write(client, buf, 2);

    ret = cst3xx_i2c_read(client, buf, len);
	
    return ret; 
} 
#endif

static void cst3xx_reset_ic(unsigned int ms)
{
	tpd_gpio_output(GTP_RST_PORT, 0);
	msleep(100);
	tpd_gpio_output(GTP_RST_PORT, 1);
    msleep(ms);
}

static void cst3xx_sleep_ic(void)
{
	int retry = 0;
	int ret;
	unsigned char buf[4];

	buf[0] = 0xD1;
	buf[1] = 0x05;
	while (retry++ < 5) {
		ret = cst3xx_i2c_write(g_i2c_client, buf, 2);
		if (ret > 0)
			break;	
		mdelay(2);		
	}

    if(retry==5) CST3XX_ERROR("hyn go to sleep fail.ret:%d;\n", ret);
	
}


/*******************************************************
Function:
    test i2c communication
Input:
    client: i2c client
Output:

    success: big than 0
    fail:	negative
*******************************************************/
static int cst3xx_i2c_test(struct i2c_client *client)
{
	int retry = 0;
	int ret;
	unsigned char buf[4];

	buf[0] = 0xD1;
	buf[1] = 0x06;
	while (retry++ < 5) {
		ret = cst3xx_i2c_write(client, buf, 2);
		if (ret > 0)
			return ret;
		
		mdelay(2);		
	}

    if(retry==5) CST3XX_ERROR("hyn I2C TEST error.ret:%d;\n", ret);
	
	return ret;
}

#ifdef TPD_PROXIMITY
static int tpd_get_ps_value(void)
{
	return tpd_proximity_detect;  //send to OS to controll backlight on/off
}

static int tpd_enable_ps(int enable)
{
	u8 buf[4];
	
	if (enable) {
		//wake_lock(&ps_lock);
		buf[0] = 0xD0;
		buf[1] = 0x4B;
		buf[2] = 0x80;
		cst3xx_i2c_write(g_i2c_client, buf, 3);
		
		tpd_proximity_flag = 1;
		//add alps of function
		CST3XX_DEBUG("tpd-ps function is on\n");
	}
	else {
		tpd_proximity_flag = 0;
		//wake_unlock(&ps_lock);
		
		buf[0] = 0xD0;
		buf[1] = 0x4B;
		buf[2] = 0x00;
		cst3xx_i2c_write(g_i2c_client, buf, 3);
		
		CST3XX_DEBUG("tpd-ps function is off\n");
	}
	return 0;
}

static int tpd_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
        void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	struct hwm_sensor_data *sensor_data;

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int))) {
				CST3XX_ERROR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int))) {
				CST3XX_ERROR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else {
				value = *(int *)buff_in;
				if(value) {
				
					if((tpd_enable_ps(1) != 0)) {
						CST3XX_ERROR("enable ps fail: %d\n", err);
						return -1;
					}
				}
				else {
					if((tpd_enable_ps(0) != 0)) {
						CST3XX_ERROR("disable ps fail: %d\n", err);
						return -1;
					}
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(struct hwm_sensor_data))) {
				CST3XX_ERROR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else {
				sensor_data = (struct hwm_sensor_data *)buff_out;

				sensor_data->values[0] = tpd_get_ps_value();
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;

		default:
			CST3XX_DEBUG("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	return err;
}
#endif




static void cst3xx_touch_down(struct input_dev *input_dev,s32 id,s32 x,s32 y,s32 w)
{
    s32 temp_w = (w>>1);
	
#ifdef ICS_SLOT_REPORT
    input_mt_slot(input_dev, id);
    input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, 1);
    input_report_abs(input_dev, ABS_MT_TRACKING_ID, id);
    input_report_abs(input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, temp_w);
    input_report_abs(input_dev, ABS_MT_WIDTH_MAJOR, temp_w);
	input_report_abs(input_dev, ABS_MT_PRESSURE, temp_w);
#else
    input_report_key(input_dev, BTN_TOUCH, 1);
    input_report_abs(input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, temp_w);
    input_report_abs(input_dev, ABS_MT_WIDTH_MAJOR, temp_w);
    input_report_abs(input_dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(input_dev);
#endif

#if 0
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode()) {   
		tpd_button(x, y, 1);  
	}
#endif
}

static void cst3xx_touch_up(struct input_dev *input_dev, int id)
{
#ifdef ICS_SLOT_REPORT
    input_mt_slot(input_dev, id);
    input_report_abs(input_dev, ABS_MT_TRACKING_ID, -1);
    input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, 0);
#else
    input_report_key(input_dev, BTN_TOUCH, 0);
    input_mt_sync(input_dev);
#endif

#if 0
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode()) {   
	    tpd_button(0, 0, 0);  
	}
#endif
}

#ifdef ANDROID_TOOL_SURPORT   //debug tool support
#define CST3XX_PROC_DIR_NAME	"cst1xx_ts"
#define CST3XX_PROC_FILE_NAME	"cst1xx-update"
static struct proc_dir_entry *g_proc_dir, *g_update_file;
static int CMDIndex = 0;

static struct file *cst3xx_open_fw_file(char *path)
{
	struct file * filp = NULL;
	int ret;
	
	filp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(filp)) {
        ret = PTR_ERR(filp);
        return NULL;
    }
    filp->f_op->llseek(filp, 0, 0);
	
    return filp;
}

static void cst3xx_close_fw_file(struct file * filp)
{
	 if(filp)
	 filp_close(filp,NULL);
}

static int cst3xx_read_fw_file(unsigned char *filename, unsigned char *pdata, int *plen)
{
	struct file *fp;
	int size;
	int length;
	int ret = -1;

	if((pdata == NULL) || (strlen(filename) == 0)) {
		CST3XX_ERROR("file name is null.\n");
		return ret;
	}
	fp = cst3xx_open_fw_file(filename);
	if(fp == NULL) {		
        CST3XX_ERROR("Open bin file faild.path:%s.\n", filename);
		goto clean;
	}
	
	length = fp->f_op->llseek(fp, 0, SEEK_END); 
	fp->f_op->llseek(fp, 0, 0);	
	size = fp->f_op->read(fp, pdata, length, &fp->f_pos);
	if(size == length) {
    	ret = 0;
    	*plen = length;
	} 	

clean:
	cst3xx_close_fw_file(fp);
	return ret;
}

static int cst3xx_apk_fw_dowmload(struct i2c_client *client,
		unsigned char *pdata, int length) 
{ 
	int ret;

	pcst3xx_update_firmware=pdata;
	ret = cst3xx_update_firmware(g_i2c_client, pdata);
	if (ret < 0) {
		CST3XX_ERROR("online update fw failed.\n");
		cst3xx_reset_ic(10);
		return -1;
	}	

	return 0;
}

ssize_t cst3xx_proc_read_foobar(struct file *page,char __user *user_buf, size_t count, loff_t *data)
{	
	unsigned char buf[512];
	int len = 0;	
	int ret;

    CST3XX_DEBUG("cst3xx is entering cst3xx_proc_read_foobar.\n");
  
	if (CMDIndex == 0) {
		sprintf(buf,"Hynitron touchscreen driver 1.0\n");
		//strcpy(page,buf);	
		len = strlen(buf);
		ret = copy_to_user(user_buf,buf,len);
		
	}
	else if (CMDIndex == 1) {
		buf[0] = g_cst3xx_rx;
		buf[1] = g_cst3xx_tx;
		ret = copy_to_user(user_buf,buf,2);
    	len = 2;
	}
	if(CMDIndex == 2 || CMDIndex == 3) {		
		unsigned short rx,tx;
		int data_len;
		
		rx = g_cst3xx_rx;
		tx = g_cst3xx_tx;
		data_len = rx*tx*2 + 4 + (tx+rx)*2 + rx + rx;
		
		if(CMDIndex == 2) {
			buf[0] = 0xD1;
			buf[1] = 0x0D;
		}
		else {  
			buf[0] = 0xD1;
			buf[1] = 0x0A;
		}
				
		ret = i2c_master_send(g_i2c_client, buf, 2);  
		if(ret < 0) {			
			CST3XX_ERROR("Write command raw/diff mode failed.error:%d.\n", ret);
			goto END;
		}

		g_unnormal_mode = 1;
		mdelay(14);
		
 		//while(!gpio_get_value(CST2XX_INT_PORT));
		
		buf[0] = 0x80;
		buf[1] = 0x01;		
		ret = cst3xx_i2c_write(g_i2c_client, buf, 2);
		if(ret < 0) {				
			CST3XX_ERROR("Write command(0x8001) failed.error:%d.\n", ret);
			goto END;
		}		
		ret = cst3xx_i2c_read(g_i2c_client, &buf[2], data_len);
		if(ret < 0) {
			CST3XX_ERROR("Read raw/diff data failed.error:%d.\n", ret);
			goto END;
		}	

		mdelay(1);
		
		buf[0] = 0xD1;
		buf[1] = 0x08;		
		ret = cst3xx_i2c_write(g_i2c_client, buf, 2); 		
		if(ret < 0) {				
			CST3XX_ERROR("Write command normal mode failed.error:%d.\n", ret);
			goto END;
		}	
		
		buf[0] = rx;
		buf[1] = tx;	
		ret = copy_to_user(user_buf,buf,data_len + 2);
    	len = data_len + 2;
	}	

END:	
	g_unnormal_mode = 0;
	//enable i2c irq	/////////////*********** \D0\E8Ҫ\D0޸ĵ\C4	
	//if (g_ts_data->use_irq)
	//	cst3xx_irq_enable(g_ts_data);	
	
	CMDIndex = 0;	
	return len;
}

ssize_t cst3xx_proc_write_foobar(struct file *file, const char __user *buffer,size_t count, loff_t *data)
{
    unsigned char cmd[128];
    unsigned char *pdata = NULL;
	int len;
	int ret;
    int length = 24*1024;

	if (count > 128) 
		len = 128;
	else 
		len = count;

   CST3XX_DEBUG("cst3xx is entering cst3xx_proc_write_foobar .\n");
    
	if (copy_from_user(cmd, buffer, len))  {
		CST3XX_ERROR("copy data from user space failed.\n");
		return -EFAULT;
	}
	
	 CST3XX_DEBUG("cmd:%d......%d.......len:%d\r\n", cmd[0], cmd[1], len);
	
	if (cmd[0] == 0) {
	    pdata = kzalloc(sizeof(char)*length, GFP_KERNEL);
	    if(pdata == NULL) {
	        CST3XX_ERROR("zalloc GFP_KERNEL memory fail.\n");
	        return -ENOMEM;
	    }
		ret = cst3xx_read_fw_file(&cmd[1], pdata, &length);
	  	if(ret < 0) {
			if(pdata != NULL) {
				kfree(pdata);
				pdata = NULL;	
			}				
			return -EPERM;
	  	}
		
		ret = cst3xx_apk_fw_dowmload(g_i2c_client, pdata, length);
	  	if(ret < 0){
	        CST3XX_ERROR("update firmware failed.\n");
			if(pdata != NULL) {
				kfree(pdata);
				pdata = NULL;	
			}	
	        return -EPERM;
		}
        mdelay(50);
		
		cst3xx_firmware_info(g_i2c_client);    
		
		if(pdata != NULL) {
			kfree(pdata);
			pdata = NULL;	
		}
	}
	else if (cmd[0] == 2) {					
		//cst3xx_touch_release();		
		CMDIndex = cmd[1];			
	}			
	else if (cmd[0] == 3) {				
		CMDIndex = 0;		
	}	
			
	return count;
}

static const struct file_operations proc_tool_debug_fops = {
	.owner		= THIS_MODULE,
	.read	    = cst3xx_proc_read_foobar,
	.write		= cst3xx_proc_write_foobar, 
};



static int  cst3xx_proc_fs_init(void)

{

	int ret;

	

	g_proc_dir = proc_mkdir(CST3XX_PROC_DIR_NAME, NULL);

	if (g_proc_dir == NULL) {

		ret = -ENOMEM;

		goto out;

	}





   g_update_file = proc_create(CST3XX_PROC_FILE_NAME, 0777, g_proc_dir,&proc_tool_debug_fops);

   if (g_update_file == NULL)

   {

      ret = -ENOMEM;

      goto no_foo;

   }
	return 0;

no_foo:

	remove_proc_entry(CST3XX_PROC_FILE_NAME, g_proc_dir);

out:

	return ret;

}
#endif


#ifdef CONFIG_TP_ESD_PROTECT

static int esd_work_cycle = 200;
static struct delayed_work esd_check_work;
static int esd_running;
struct mutex esd_lock;
static void cst3xx_esd_check_func(struct work_struct *);


void cst3xx_init_esd_protect(void)
{
    esd_work_cycle = 2 * HZ;	/*HZ: clock ticks in 1 second generated by system*/
	CST3XX_DEBUG("Clock ticks for an esd cycle: %d", esd_work_cycle);
	INIT_DELAYED_WORK(&esd_check_work, cst3xx_esd_check_func);
	mutex_init(&esd_lock);


}
	
void cst3xx_esd_switch(s32 on);
{
	mutex_lock(&esd_lock);
	if (SWITCH_ESD_ON == on) {	/* switch on esd check */
		if (!esd_running) {
			esd_running = 1;
			CST3XX_DEBUG("Esd protector started!");
			queue_delayed_work(cst3xx_esd_workqueue, &esd_check_work, esd_work_cycle);
		}
	} else {		/* switch off esd check */
		if (esd_running) {
			esd_running = 0;
			CST3XX_DEBUG("Esd protector stopped!");
			cancel_delayed_work(&esd_check_work);
		}
	}
	mutex_unlock(&esd_lock);

}


static void cst3xx_esd_check_func(struct work_struct *work)
{
	
    int retry = 0;
	int ret;
	unsigned char buf[4];

	if (!esd_running) {
	CST3XX_DEBUG("Esd protector suspended!");
	return;
	}

	buf[0] = 0xD0;
	buf[1] = 0x4C;
	
	while(retry++ < 3) {
		ret = cst3xx_i2c_read_register(g_i2c_client, buf, 1);
		if (ret > 0) break;
		
		msleep(2);		
	}

    if((retry>3) || ((buf[0]!=226)&&(buf[0]!=237)&&(buf[0]!=240))) {
		
		//cst2xx_chip_init();
		
		//cst2xx_check_code(cst2xx_i2c);

		cst3xx_reset_ic(10);
    }
	
	mutex_lock(&esd_lock);
	if (esd_running)
		queue_delayed_work(cst3xx_esd_workqueue, &esd_check_work, esd_work_cycle);
	else
		CST3XX_DEBUG("Esd protector suspended!");
	mutex_unlock(&esd_lock);
}

#endif

/*******************************************************
Function:
    read checksum in bootloader mode
Input:
    client: i2c client
    strict: check checksum value
Output:
    success: 0
    fail:	-1
*******************************************************/

#define CST3XX_BIN_SIZE    (24*1024 + 24)

static int cst3xx_check_checksum(struct i2c_client * client)
{
	int ret;
	int i;
	unsigned int  checksum;
	unsigned int  bin_checksum;
	unsigned char buf[4];
	const unsigned char *pData;

	for(i=0; i<5; i++)
	{
		buf[0] = 0xA0;
		buf[1] = 0x00;
		ret = cst3xx_i2c_read_register(client, buf, 1);
		if(ret < 0)
		{
			msleep(2);
			continue;
		}

		if(buf[0]!=0)
			break;
		else
		msleep(2);
	}
    msleep(2);


    if(buf[0]==0x01)
	{
		buf[0] = 0xA0;
		buf[1] = 0x08;
		ret = cst3xx_i2c_read_register(client, buf, 4);
		
		if(ret < 0)	return -1;
		
		// read chip checksum
		checksum = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

        pData=(unsigned char  *)pcst3xx_update_firmware +24*1024+16;   //7*1024 +512
		bin_checksum = pData[0] + (pData[1]<<8) + (pData[2]<<16) + (pData[3]<<24);

        CST3XX_DEBUG("hyn the updated ic checksum is :0x%x. the updating firmware checksum is:0x%x------\n", checksum, bin_checksum);
    
        if(checksum!=bin_checksum)
		{
			CST3XX_ERROR("hyn check sum error.\n");		
			return -1;
			
		}
		
	}
	else
	{
		CST3XX_ERROR("hyn No checksum.\n");
		return -1;
	}	
	return 0;
}
static int cst3xx_into_program_mode(struct i2c_client * client)
{
	int ret;
	unsigned char buf[4];
	
	buf[0] = 0xA0;
	buf[1] = 0x01;	
	buf[2] = 0xAA;	//set cmd to enter program mode		
	ret = cst3xx_i2c_write(client, buf, 3);
	if (ret < 0)  return -1;

	mdelay(2);
	
	buf[0] = 0xA0;
	buf[1] = 0x02;	//check whether into program mode
	ret = cst3xx_i2c_read_register(client, buf, 1);
	if (ret < 0)  return -1;
	
	if (buf[0] != 0x55) return -1;
	
	return 0;
}

static int cst3xx_exit_program_mode(struct i2c_client * client)
{
	int ret;
	unsigned char buf[3];
	
	buf[0] = 0xA0;
	buf[1] = 0x06;
	buf[2] = 0xEE;
	ret = cst3xx_i2c_write(client, buf, 3);
	if (ret < 0)
		return -1;
	
	mdelay(10);	//wait for restart

	
	return 0;
}

static int cst3xx_erase_program_area(struct i2c_client * client)
{
	int ret;
	unsigned char buf[3];
	
	buf[0] = 0xA0;
	buf[1] = 0x02;	
	buf[2] = 0x00;		//set cmd to erase main area		
	ret = cst3xx_i2c_write(client, buf, 3);
	if (ret < 0) return -1;
	
	mdelay(5);
	
	buf[0] = 0xA0;
	buf[1] = 0x03;
	ret = cst3xx_i2c_read_register(client, buf, 1);
	if (ret < 0)  return -1;
	
	if (buf[0] != 0x55) return -1;

	return 0;
}

static int cst3xx_write_program_data(struct i2c_client * client,
		const unsigned char *pdata)
{
	int i, ret;
	unsigned char *i2c_buf;
	unsigned short eep_addr;
	int total_kbyte;
#ifdef TRANSACTION_LENGTH_LIMITED
	unsigned char temp_buf[8];
	unsigned short iic_addr;
	int  j;

#endif
	

	i2c_buf = kmalloc(sizeof(unsigned char)*(1024 + 2), GFP_KERNEL);
	if (i2c_buf == NULL) 
		return -1;
	
	//make sure fwbin len is N*1K
	//total_kbyte = len / 1024;
	total_kbyte = 24;
	for (i=0; i<total_kbyte; i++) {
		i2c_buf[0] = 0xA0;
		i2c_buf[1] = 0x14;
		eep_addr = i << 10;		//i * 1024
		i2c_buf[2] = eep_addr;
		i2c_buf[3] = eep_addr>>8;
		ret = cst3xx_i2c_write(client, i2c_buf, 4);
		if (ret < 0)
			goto error_out;




	
#ifdef TRANSACTION_LENGTH_LIMITED
		memcpy(i2c_buf, pdata + eep_addr, 1024);
		for(j=0; j<256; j++) {
			iic_addr = (j<<2);
    	temp_buf[0] = (iic_addr+0xA018)>>8;
    	temp_buf[1] = (iic_addr+0xA018)&0xFF;
		temp_buf[2] = i2c_buf[iic_addr+0];
		temp_buf[3] = i2c_buf[iic_addr+1];
		temp_buf[4] = i2c_buf[iic_addr+2];
		temp_buf[5] = i2c_buf[iic_addr+3];
    	ret = cst3xx_i2c_write(client, temp_buf, 6);
    		if (ret < 0)
    			goto error_out;		
		}
#else
		
		i2c_buf[0] = 0xA0;
		i2c_buf[1] = 0x18;
		memcpy(i2c_buf + 2, pdata + eep_addr, 1024);
		ret = cst3xx_i2c_write(client, i2c_buf, 1026);
		if (ret < 0)
			goto error_out;
#endif
		
		i2c_buf[0] = 0xA0;
		i2c_buf[1] = 0x04;
		i2c_buf[2] = 0xEE;
		ret = cst3xx_i2c_write(client, i2c_buf, 3);
		if (ret < 0)
			goto error_out;
		
		mdelay(60);	
		
		i2c_buf[0] = 0xA0;
		i2c_buf[1] = 0x05;
		ret = cst3xx_i2c_read_register(client, i2c_buf, 1);
		if (ret < 0)
			goto error_out;
		
		if (i2c_buf[0] != 0x55)
			goto error_out;

	}
	
	i2c_buf[0] = 0xA0;
	i2c_buf[1] = 0x03;
	i2c_buf[2] = 0x00;
	ret = cst3xx_i2c_write(client, i2c_buf, 3);
	if (ret < 0)
		goto error_out;
	
	mdelay(8);	
	
	if (i2c_buf != NULL) {
		kfree(i2c_buf);
		i2c_buf = NULL;
	}

	return 0;
	
error_out:
	if (i2c_buf != NULL) {
		kfree(i2c_buf);
		i2c_buf = NULL;
	}
	return -1;
}

int cst3xx_update_firmware(struct i2c_client * client, const unsigned char *pdata)
{
	int ret;
	int retry = 0;
	
	CST3XX_ERROR("----------upgrade cst3xx begain------------\n");
	
START_FLOW:	
	//g_update_flag = 1;
	
	//disable i2c irq	/////////////*********** \D0\E8要\D0薷牡\C4	
	
	cst3xx_reset_ic(10);

	ret = cst3xx_into_program_mode(client);
	if (ret < 0) {
		CST3XX_ERROR("[cst3xx]into program mode failed.\n");
		goto err_out;
	}

	ret = cst3xx_erase_program_area(client);
	if (ret) {
		CST3XX_ERROR("[cst3xx]erase main area failed.\n");
		goto err_out;
	}

	ret = cst3xx_write_program_data(client, pdata);
	if (ret < 0) {
		CST3XX_ERROR("[cst3xx]write program data into cstxxx failed.\n");
		goto err_out;
	}

    ret =cst3xx_check_checksum(client);
	if (ret < 0) {
		CST3XX_ERROR("[cst3xx] after write program cst3xx_check_checksum failed.\n");
		goto err_out;
	}

	ret = cst3xx_exit_program_mode(client);
	if (ret < 0) {
		CST3XX_ERROR("[cst3xx]exit program mode failed.\n");
		goto err_out;
	}


	cst3xx_reset_ic(10);
	
	CST3XX_ERROR("hyn----------cst3xx_update_firmware  end------------\n");
	
	return 0;
	
err_out:
	if (retry < 3) {
		retry++;
		goto START_FLOW;
	} 
	else {		
		return -1;
	}
}

static int cst3xx_update_judge( unsigned char *pdata, int strict)
{
	unsigned short ic_type, project_id;
	unsigned int fw_checksum, fw_version;
	const unsigned int *p;
	int i;
	unsigned char *pBuf;
		
	fw_checksum = 0x55;
	p = (const unsigned int *)pdata;
	for (i=0; i<(CST3XX_BIN_SIZE-4); i+=4) {
		fw_checksum += (*p);
		p++;
	}
	
	if (fw_checksum != (*p)) {
		CST3XX_ERROR("[cst3xx]calculated checksum error:0x%x not equal 0x%x.\n", fw_checksum, *p);
		return -1;	//bad fw, so do not update
	}
	
	pBuf = &pdata[CST3XX_BIN_SIZE-16];
	
	project_id = pBuf[1];
	project_id <<= 8;
	project_id |= pBuf[0];

	ic_type = pBuf[3];
	ic_type <<= 8;
	ic_type |= pBuf[2];

	fw_version = pBuf[7];
	fw_version <<= 8;
	fw_version |= pBuf[6];
	fw_version <<= 8;
	fw_version |= pBuf[5];
	fw_version <<= 8;
	fw_version |= pBuf[4];

	fw_checksum = pBuf[11];
	fw_checksum <<= 8;
	fw_checksum |= pBuf[10];
	fw_checksum <<= 8;
	fw_checksum |= pBuf[9];
	fw_checksum <<= 8;
	fw_checksum |= pBuf[8];	
	
	CST3XX_ERROR("[cst3xx]the updating firmware:project_id:0x%04x,ic type:0x%04x,version:0x%x,checksum:0x%x\n",
			project_id, ic_type, fw_version, fw_checksum);

	if (strict > 0) {
		
		if (g_cst3xx_ic_checksum != fw_checksum) {

			if (g_cst3xx_ic_version >= fw_version) {

				CST3XX_DEBUG("[cst3xx] the chip firmware:ic version(0x%x) is higher,no need to update.\n", g_cst3xx_ic_version);		
				return -1;	
			}		
		}
		else{

			CST3XX_DEBUG("[cst3xx]the updating checksum is same,no need to update(0x%x).\n",fw_checksum);
			return -1;	
		}
		
	}	
	
	return 0;
}


/*******************************************************
Function:
    get firmware version, ic type...
Input:
    client: i2c client
Output:
    success: 0
    fail:	-1
*******************************************************/
int cst3xx_firmware_info(struct i2c_client * client)
{
	int ret;
	unsigned char buf[28];

	/* go into abnormal mode before get hw info */
	buf[0] = 0xD1;
	buf[1] = 0x01;
	ret = cst3xx_i2c_write(client, buf, 2);
	if (ret < 0) 
	    return -1;
	mdelay(10);

    /* get ic type info */
	buf[0] = 0xD2;
	buf[1] = 0x04;
	ret = cst3xx_i2c_read_register(client, buf, 4);
	if (ret < 0) 
	    return -1;

    g_cst3xx_ic_type = buf[3]<<24 | buf[2]<<16 | buf[1]<<8 | buf[0];
    hw_info.id = g_cst3xx_ic_type >> 16;
	
	if(g_cst3xx_ic_type>>16 == 0x8c){
		sprintf(tpd_chip_name,"%s", "CST340");
	}
	else if(g_cst3xx_ic_type>>16 == 0x80){
		sprintf(tpd_chip_name,"%s", "CST328");
	}
	else if(g_cst3xx_ic_type>>16 == 0x94){
		sprintf(tpd_chip_name,"%s", "CST348");
	}
	else
	{
		CST3XX_ERROR(" [cst3xx] the chip ic don't have firmware. \n");
		return -1;
	}
	
		
	mdelay(10);

    /* get ic version info */
	buf[0] = 0xD2;
	buf[1] = 0x08;
	ret = cst3xx_i2c_read_register(client, buf, 8);
	if (ret < 0) 
	    return -1;

    g_cst3xx_ic_version = buf[3]<<24 | buf[2]<<16 | buf[1]<<8 | buf[0];
	g_cst3xx_ic_checksum = buf[3]<<24 | buf[2]<<16 | buf[1]<<8 | buf[0];
	sprintf(fw_version,"version 0x%x", g_cst3xx_ic_version);

	CST3XX_ERROR(" [cst3xx] the chip ic version:0x%x, checksum:0x%x\r\n",
		g_cst3xx_ic_version, g_cst3xx_ic_checksum);

	if(g_cst3xx_ic_version == 0xA5A5A5A5){
		CST3XX_ERROR(" [cst3xx] the chip ic don't have firmware. \n");
		return -1;
	}

    buf[0] = 0xD1;
	buf[1] = 0x09;
	ret = cst3xx_i2c_write(client, buf, 2);
	if (ret < 0) 
	    return -1;
    mdelay(5);
	
	return 0;
}

static int cst3xx_boot_update_fw(struct i2c_client   *client,  unsigned char *pdata)
{
	int ret;
	int retry = 0;
	int flag = 0;

	while (retry++ < 3) {
		ret = cst3xx_firmware_info(client);
		if (ret == 0) {
			flag = 1;
			break;
		}
	}

	if (flag == 1) {
		ret = cst3xx_update_judge(pdata, 1);
		if (ret < 0) {
			CST3XX_ERROR("[cst3xx] no need to update firmware.\n");
			return 0;
		}
	}
	
	ret = cst3xx_update_firmware(client, pdata);
	if (ret < 0){
		CST3XX_ERROR(" [cst3xx] update firmware failed.\n");
		return -1;
	}

    mdelay(50);

	ret = cst3xx_firmware_info(client);
	if (ret < 0) {
		CST3XX_ERROR(" [cst3xx] after update read version and checksum fail.\n");
		return -1;
	}

	return 0;
}


static int hyn_boot_update_fw(struct i2c_client * client)
{
	unsigned char *ptr_fw;
	ptr_fw = pcst3xx_update_firmware;	
	return cst3xx_boot_update_fw(client, ptr_fw);
}

static void cst3xx_touch_report(struct input_dev *input_dev)
{
	unsigned char buf[30];
	unsigned char i2c_buf[8];
	unsigned char key_status, key_id = 0, finger_id, sw;
	unsigned int  input_x = 0; 
	unsigned int  input_y = 0; 
	unsigned int  input_w = 0;
    unsigned char cnt_up, cnt_down;
	int   i, ret, idx; 
	int cnt, i2c_len;
#ifdef TRANSACTION_LENGTH_LIMITED
	int  len_1, len_2;
#endif

#ifdef TPD_PROXIMITY
	int err;
	struct hwm_sensor_data sensor_data;
	if (tpd_proximity_flag == 1) {
		buf[0] = 0xD0;
		buf[1] = 0x4B;
		err = cst3xx_i2c_read_register(g_i2c_client, buf, 1);
        if(err < 0) {
        	CST3XX_DEBUG("iic read proximity data failed.\n");
        	goto OUT_PROCESS;
        }
		
		if(buf[0]&0x7F) {
			tpd_proximity_detect = 0;  //close
		}
		else {
			tpd_proximity_detect = 1;  //far away
		}
		
		CST3XX_DEBUG("cst3xx ps change tpd_proximity_detect = %d  \n",tpd_proximity_detect);
		//map and store data to hwm_sensor_data
		sensor_data.values[0]    = tpd_get_ps_value();
		sensor_data.value_divide = 1;
		sensor_data.status       = SENSOR_STATUS_ACCURACY_MEDIUM;
		//let up layer to know
		if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data))) {
			CST3XX_DEBUG("call hwmsen_get_interrupt_data fail = %d\n", err);
		}
	}
#endif

#ifdef HYN_GESTURE
    if((gestrue_en == true)&&(gestrue_lcd_flag==true))
    {       		 
         hyn_read_Gestruedata();  
		 goto END; 	 
	}
#endif


    key_status = 0;


	buf[0] = 0xD0;
	buf[1] = 0x00;
	ret = cst3xx_i2c_read_register(g_i2c_client, buf, 7);
	if(ret < 0) {
		CST3XX_DEBUG("iic read touch point data failed.\n");
		goto OUT_PROCESS;
	}
		
	if(buf[6] != 0xAB) {
		//CST3XX_DEBUG("data is not valid..\r\n");
		goto OUT_PROCESS;
	}

	if(buf[5] == 0x80) {
		key_status = buf[0];
		key_id = buf[1];		
		goto KEY_PROCESS;
	} 
	
	cnt = buf[5] & 0x7F;
	if(cnt > TPD_MAX_FINGERS) goto OUT_PROCESS;
	else if(cnt==0)     goto CLR_POINT;
	
	if(cnt == 0x01) {
		goto FINGER_PROCESS;
	} 
	else {
		#ifdef TRANSACTION_LENGTH_LIMITED
		if((buf[5]&0x80) == 0x80) //key
		{
			i2c_len = (cnt - 1)*5 + 3;
			len_1   = i2c_len;
			for(idx=0; idx<i2c_len; idx+=6) {
			    i2c_buf[0] = 0xD0;
				i2c_buf[1] = 0x07+idx;
				
				if(len_1>=6) {
					len_2  = 6;
					len_1 -= 6;
				}
				else {
					len_2 = len_1;
					len_1 = 0;
				}
				
    			ret = cst3xx_i2c_read_register(g_i2c_client, i2c_buf, len_2);
    			if(ret < 0) goto OUT_PROCESS;

				for(i=0; i<len_2; i++) {
                   buf[5+idx+i] = i2c_buf[i];
				}
			}
			
			i2c_len   += 5;
			key_status = buf[i2c_len - 3];
			key_id     = buf[i2c_len - 2];
		} 
		else {			
			i2c_len = (cnt - 1)*5 + 1;
			len_1   = i2c_len;
			
			for(idx=0; idx<i2c_len; idx+=6) {
			    i2c_buf[0] = 0xD0;
				i2c_buf[1] = 0x07+idx;
				
				if(len_1>=6) {
					len_2  = 6;
					len_1 -= 6;
				}
				else {
					len_2 = len_1;
					len_1 = 0;
				}
				
    			ret = cst3xx_i2c_read_register(g_i2c_client, i2c_buf, len_2);
    			if (ret < 0) goto OUT_PROCESS;

				for(i=0; i<len_2; i++) {
                   buf[5+idx+i] = i2c_buf[i];
				}
			}			
			i2c_len += 5;
		}
		#else
		if ((buf[5]&0x80) == 0x80) {
			buf[5] = 0xD0;
			buf[6] = 0x07;
			i2c_len = (cnt - 1)*5 + 3;
			ret = cst3xx_i2c_read_register(g_i2c_client, &buf[5], i2c_len);
			if (ret < 0)
				goto OUT_PROCESS;
			i2c_len += 5;
			key_status = buf[i2c_len - 3];
			key_id = buf[i2c_len - 2];
		} 
		else {			
			buf[5] = 0xD0;
			buf[6] = 0x07;			
			i2c_len = (cnt - 1)*5 + 1;
			ret = cst3xx_i2c_read_register(g_i2c_client, &buf[5], i2c_len);
			if (ret < 0)
				goto OUT_PROCESS;
			i2c_len += 5;
		}
		#endif

		if (buf[i2c_len - 1] != 0xAB) {
			goto OUT_PROCESS;
		}
	}	

    //both key and point
	if((cnt>0)&&(key_status&0x80))  {
        if(report_flag==0xA5) goto KEY_PROCESS; 
	}
	
FINGER_PROCESS:

	i2c_buf[0] = 0xD0;
	i2c_buf[1] = 0x00;
	i2c_buf[2] = 0xAB;
	ret = cst3xx_i2c_write(g_i2c_client, i2c_buf, 3);
	if(ret < 0) {
		CST3XX_ERROR("hyn send read touch info ending failed.\r\n"); 
		cst3xx_reset_ic(20);
	}
	
	idx = 0;
    cnt_up = 0;
    cnt_down = 0;
	for (i = 0; i < cnt; i++) {
		
		input_x = (unsigned int)((buf[idx + 1] << 4) | ((buf[idx + 3] >> 4) & 0x0F));
		input_y = (unsigned int)((buf[idx + 2] << 4) | (buf[idx + 3] & 0x0F));	
		input_w = (unsigned int)(buf[idx + 4]);
		sw = (buf[idx] & 0x0F) >> 1;
		finger_id = (buf[idx] >> 4) & 0x0F;

        #if 0        	
		input_x = TPD_MAX_X - input_x;
		input_y = TPD_MAX_Y - input_y;

		#endif

	   
        //CST3XX_ERROR("Point x:%d, y:%d, id:%d, sw:%d. \n", input_x, input_y, finger_id, sw);

		if (sw == 0x03) {
			cst3xx_touch_down(input_dev, finger_id, input_x, input_y, input_w);
            cnt_down++;
        }
		else {
            cnt_up++;
            #ifdef ICS_SLOT_REPORT
			cst3xx_touch_up(input_dev, finger_id);
            #endif
        }
		idx += 5;
	}
    
    #ifndef ICS_SLOT_REPORT
    if((cnt_up>0) && (cnt_down==0))
        cst3xx_touch_up(input_dev, 0);
    #endif

	if(cnt_down==0)  report_flag = 0;
	else report_flag = 0xCA;

    input_sync(input_dev);
	goto END;

KEY_PROCESS:

	i2c_buf[0] = 0xD0;
	i2c_buf[1] = 0x00;
	i2c_buf[2] = 0xAB;
	ret = cst3xx_i2c_write(g_i2c_client, i2c_buf, 3);
	if (ret < 0) {
		CST3XX_ERROR("hyn send read touch info ending failed.\r\n"); 
		cst3xx_reset_ic(20);
	}

	if (tpd_dts_data.use_tpd_button) {
		
		if(key_status&0x80) {
			i = (key_id>>4)-1;
	        if((key_status&0x7F)==0x03) {
				if((i==key_index)||(key_index==0xFF)) {
	        		cst3xx_touch_down(input_dev, 0, tpd_dts_data.tpd_key_dim_local[i].key_x, tpd_dts_data.tpd_key_dim_local[i].key_y, 50);
	    			report_flag = 0xA5;
					key_index   = i;
				}
				else {
					
					key_index = 0xFF;
				}
			}
	    	else {
	            cst3xx_touch_up(input_dev, 0);
				report_flag = 0;	
				key_index = 0xFF;
	    	}
		}
	}
    
	input_sync(input_dev);
    goto END;

CLR_POINT:
#ifdef SLEEP_CLEAR_POINT
	#ifdef ICS_SLOT_REPORT
		for(i=0; i<=10; i++) {	
			input_mt_slot(input_dev, i);
			input_report_abs(input_dev, ABS_MT_TRACKING_ID, -1);
			input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, false);
		}
	#else
	    input_report_key(input_dev, BTN_TOUCH, 0);
		input_mt_sync(input_dev);
	#endif
		input_sync(input_dev);	
#endif		
		
OUT_PROCESS:
	buf[0] = 0xD0;
	buf[1] = 0x00;
	buf[2] = 0xAB;
	ret = cst3xx_i2c_write(g_i2c_client, buf, 3);
	if (ret < 0) {
		CST3XX_DEBUG("send read touch info ending failed.\n"); 
		cst3xx_reset_ic(20);
	}
	
END:	
	return;
}

static int cst3xx_touch_handler(void *unused)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };

    //CST3XX_DEBUG("===cst3xx_touch_handler2222\n");
	
	sched_setscheduler(current, SCHED_RR, &param);

	do {
        //enable_irq(touch_irq);
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(waiter, tpd_flag!=0);
		tpd_flag = 0;
        TPD_DEBUG_SET_TIME;
		set_current_state(TASK_RUNNING);

//        eint_flag = 0;
		cst3xx_touch_report(tpd->dev);
		
	} while (!kthread_should_stop());

	return 0;
}

static void cst3xx_ts_irq_handler(void) 
{
    //CST3XX_DEBUG("===[hyn]tpd irq interrupt===\n");
	//eint_flag = 1;
	tpd_flag  = 1;
	wake_up_interruptible(&waiter);
}
static int tpd_irq_registration(void)
{
	struct device_node *node = NULL;
	int ret = 0;
	u32 ints[2] = {0,0};

	tpd_gpio_as_int(GTP_INT_PORT);
    
	node = of_find_matching_node(node, touch_of_match);
	if (node) {
		/*touch_irq = gpio_to_irq(tpd_int_gpio_number);*/
		of_property_read_u32_array(node,"debounce", ints, ARRAY_SIZE(ints));
		gpio_set_debounce(ints[0], ints[1]);

		touch_irq = irq_of_parse_and_map(node, 0);
	       	if(touch_irq ==0)
			CST3XX_ERROR("*** Unable to irq_of_parse_and_map() ***\n");
	        	else
	        	{        
	    		ret = request_irq(touch_irq, (irq_handler_t)cst3xx_ts_irq_handler,IRQF_TRIGGER_RISING, "TOUCH_PANEL-eint_cst3xx", NULL);
	    		if (ret > 0)
	    		CST3XX_ERROR("tpd request_irq IRQ LINE NOT AVAILABLE!.\n");
	    }
	} 
    else
	{
		CST3XX_DEBUG("[%s] tpd request_irq can not find touch eint device node!.\n", __func__);
	}

	return 0;
}

#ifdef HYN_GESTURE

static ssize_t gesture_mode_show(struct class *class, struct class_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", gestrue_en == true ? "enable" : "disable");
}

static ssize_t gesture_mode_store(struct class *class, struct class_attribute *attr,const char *buf, size_t count)
{
	bool value;
	if(strtobool(buf, &value))
		return -EINVAL;

	//mutex_lock(&twd.lock);

	printk(" test %d\n",value);

	if(value)
		gestrue_en = true;
	else
		gestrue_en = false;

	printk(" test gestrue_en%d\n",gestrue_en);

	return count;
}
static struct class_attribute cls_attr[]={

    __ATTR(gesenable, 0664, gesture_mode_show, gesture_mode_store),
  
};
static int class_create_files(struct class *cls,struct class_attribute *attr,int size)
{
	int i,ret=0;
	for(i=0;i < size ;i++){
		ret=class_create_file(cls,attr + i);
		if(ret < 0)
			goto undo;
	}
	return 0;
undo:
	for (; i >= 0 ; i--)
		class_remove_file(cls, attr + i);
	return -ENODEV;
}


#endif






static int cst3xx_tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{	
	int ret;
	int rc;


#ifdef HYN_GESTURE
    int retval =0;
	struct class *cls;
#endif
	
#ifdef TPD_PROXIMITY
	struct hwmsen_object obj_ps;
#endif

    CST3XX_DEBUG("hyn is entering tpd_i2c_probe \n");

	if(client->addr != 0x1A)
	{
		client->addr = 0x1A;
		printk("%s:client->addr=%x\n",__func__,client->addr);
	}
	
	g_i2c_client = client;

	tpd_gpio_output(GTP_RST_PORT, 0);
	msleep(100);
	
	CST3XX_DEBUG("hyn Device Tree get regulator! \n");
	tpd->reg = regulator_get(tpd->tpd_dev, "vtouch");
	ret = regulator_set_voltage(tpd->reg, 2800000, 2800000);	/*set 2.8v*/
	if (ret) {
		CST3XX_ERROR("hyn regulator_set_voltage(%d) failed!\n", ret);
		return -1;
	}

    ret = regulator_enable(tpd->reg);
	if (ret != 0)
		CST3XX_ERROR("hyn Failed to enable reg-vgp2: %d\n", ret);
	
	msleep(100);
	tpd_gpio_output(GTP_RST_PORT, 1);	
	tpd_gpio_as_int(GTP_INT_PORT);	
	msleep(50);

    printk("%s:client->addr=%x\n",__func__,client->addr);

	ret = cst3xx_i2c_test(client);
	if (ret < 0) {
		CST3XX_ERROR("hyn i2c communication failed.now try to force update .\n");

		ret = cst3xx_update_firmware(client, pcst3xx_update_firmware);
		if (ret < 0){
			CST3XX_ERROR(" [cst3xx] update firmware failed.\n");
			return -1;
		}		 
	}

	
    msleep(20);

	rc = hyn_boot_update_fw(client);
	if(rc < 0){
		CST3XX_ERROR("hyn_boot_update_fw failed.\n");
		return -1;
	}
	else{

		hw_module_info_add(&hw_info); //add hw info
		hyn_input_dev=tpd->dev;

	}
	
#ifdef ANDROID_TOOL_SURPORT
	ret = cst3xx_proc_fs_init();
	if(ret < 0) {
		CST3XX_ERROR("hyn create cst3xx proc fs failed.\n");
	}
#endif

	tpd_irq_registration();
	
    tpd_load_status = 1;

    disable_irq(touch_irq);
	thread = kthread_run(cst3xx_touch_handler, 0, TPD_DEVICE_NAME);
	if (IS_ERR(thread)) {
		ret = PTR_ERR(thread);
		CST3XX_ERROR("hyn create touch event handler thread failed: %d.\n", ret);
	}
        enable_irq(touch_irq);


#ifdef HYN_GESTURE

 		wake_lock_init(&hyn_gesture_chrg_lock, WAKE_LOCK_SUSPEND, "gesture_wake_lock");
        retval = hyn_Gesture_init(tpd->dev);;
		if (retval)
		{
			CST3XX_ERROR("Create hyn_Gesture_init failed!\n.");
		}		
#endif

		
#ifdef CONFIG_TP_ESD_PROTECT
	cst3xx_esd_workqueue = create_singlethread_workqueue("cst2xx_esd_workqueue");
	if (cst3xx_esd_workqueue == NULL)
		CST3XX_ERROR("create cst2xx_esd_workqueue failed!");

#endif


#ifdef TPD_PROXIMITY
{
	int err = 0;
	hwmsen_detach(ID_PROXIMITY);
	obj_ps.self    = NULL;
	obj_ps.polling = 0;//interrupt mode
	//obj_ps.polling = 1;//need to confirm what mode is!!!
	obj_ps.sensor_operate = tpd_ps_operate;
	if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps))) {
		CST3XX_ERROR("ID_PROXIMITY attach fail = %d\n", err);
	}		
	//gsl_gain_psensor_data(g_i2c_client);
	//wake_lock_init(&ps_lock, WAKE_LOCK_SUSPEND, "ps wakelock");
}
#endif



#ifdef CONFIG_TP_ESD_PROTECT

	cst3xx_init_esd_protect();
	cst3xx_esd_switch(SWITCH_ESD_ON);

#endif

#ifdef HYN_GESTURE
		hyn_gesture_sysfs(client);
       
	    gestrue_en = false;
		gestrue_lcd_flag=false;
		cls = class_create(THIS_MODULE,"syna");
		if(cls){
	        retval=class_create_files(cls,cls_attr,ARRAY_SIZE(cls_attr));
			if(retval)
				CST3XX_ERROR("[TOUCH] creat class file failed.\n");
				//goto err_sysfs;
		}
#endif

    

	CST3XX_DEBUG("hyn tpd_i2c_probe end .\n");
	return 0;
}

static int cst3xx_tpd_remove(struct i2c_client *client)
{
	CST3XX_DEBUG("cst3xx removed.\n");

#ifdef HYN_GESTURE
	wake_lock_destroy(&hyn_gesture_chrg_lock);
#endif
	gpio_free(GTP_RST_PORT);
	gpio_free(GTP_INT_PORT);
	
	return 0;
}

/*
static int cst3xx_tpd_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE_NAME);
	return 0;
}
*/

static const struct i2c_device_id cst3xx_tpd_id[] = {{TPD_DEVICE_NAME,0},{}};
/*
static struct i2c_board_info __initdata cst3xx_i2c_tpd = { 
	I2C_BOARD_INFO(TPD_DEVICE_NAME, CST3XX_I2C_ADDR)
};
*/
//static unsigned short force[] = { 0, CST3XX_I2C_ADDR, I2C_CLIENT_END, I2C_CLIENT_END };
//static const unsigned short *const forces[] = { force, NULL };

static const struct of_device_id tpd_of_match[] = {
	{.compatible = "mediatek,CAP_TOUCH"},
	{},
};

static struct i2c_driver cst3xx_ts_driver = {
  .driver = {
    .name = TPD_DEVICE_NAME,
	.of_match_table = of_match_ptr(tpd_of_match),
  },
  .probe    = cst3xx_tpd_probe,
  .remove   = cst3xx_tpd_remove,
  .id_table = cst3xx_tpd_id,
//  .detect   = cst3xx_tpd_detect,
  //.address_list = (const unsigned short *)forces,
};
static int cst3xx_local_init(void)
{

	CST3XX_DEBUG("hyn is entering cst3xx_local_init (Built %s @ %s)\n", __DATE__, __TIME__);

	if (i2c_add_driver(&cst3xx_ts_driver) != 0) {
		CST3XX_ERROR("hyn unable to add i2c driver.\n");
		return -1;
	}

	if(tpd_load_status == 0)  {
		i2c_del_driver(&cst3xx_ts_driver);
		return -1;
	}
	else {
        #ifdef ICS_SLOT_REPORT
        clear_bit(BTN_TOUCH, tpd->dev->keybit);
		input_set_abs_params(tpd->dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_WIDTH_MINOR, 0, 15, 0, 0);
	    input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, (TPD_MAX_FINGERS - 1), 0, 0);
 //  	input_mt_init_slots(tpd->dev, 10);
        #endif
	}

	
	if (tpd_dts_data.use_tpd_button) {
		/*initialize tpd button data*/
		tpd_button_setting(tpd_dts_data.tpd_key_num, tpd_dts_data.tpd_key_local,
		tpd_dts_data.tpd_key_dim_local);
	}


	tpd_type_cap = 1;
	
	CST3XX_DEBUG("hyn is end %s, %d\n", __FUNCTION__, __LINE__);
	
	return 0;
}
/*
static void cst3xx_enter_sleep(struct i2c_client *client)
{
	int ret;
	int retry = 0;
	unsigned char buf[2];

    buf[0] = 0xD1;
	buf[1] = 0x05;
	while (retry++ < 5) {
		ret = cst3xx_i2c_write(client, buf, 2);
		if (ret > 0)
			return;
		mdelay(2);
	}
	
	return;
}

*/


static void cst3xx_resume(struct device *h)
{	
 #ifdef ICS_SLOT_REPORT
	int idx;
#endif	
    int ret=0;
	CST3XX_DEBUG("cst3xx wake up.\n");	

#ifdef TPD_PROXIMITY
		if (tpd_proximity_flag == 1) {
		    //tpd_enable_ps(1);
		    return ;
		}	
#endif

#ifdef SLEEP_CLEAR_POINT
#ifdef ICS_SLOT_REPORT
	for(idx=0; idx<=10; idx++) {	
		input_mt_slot(tpd->dev, idx);
		input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
		input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
	}
#else
    input_report_key(tpd->dev, BTN_TOUCH, 0);
	input_mt_sync(tpd->dev);
#endif
	input_sync(tpd->dev);	
#endif

#ifdef HYN_GESTURE
    printk(" func=%s,LINE=%d,gestrue_en=%d\n",__func__,__LINE__,gestrue_en);
    if(gestrue_en == true){

		u8 buf[3];
        //close gesture detect
        buf[0] = 0xD0;
		buf[1] = 0x4C;
		buf[2] = 0x00;
		cst3xx_i2c_write(g_i2c_client, buf, 3);
   		
		CST3XX_DEBUG("tpd-gesture detect is closed \n");

		gestrue_lcd_flag=false;
		wake_unlock(&hyn_gesture_chrg_lock);
    }
	else
#endif	
	{
		enable_irq(touch_irq);
	}
 	msleep(30);
	cst3xx_reset_ic(30);
    msleep(200);
	ret = hyn_boot_update_fw(g_i2c_client);
	if(ret < 0){
		CST3XX_ERROR("hyn_boot_update_fw failed.\n");
		//return -1;
	}

#ifdef CONFIG_TP_ESD_PROTECT
	cst3xx_esd_switch(SWITCH_ESD_ON);
#endif
	
	CST3XX_DEBUG("cst3xx wake up done.\n");
}


static void cst3xx_suspend(struct device *h)
{
#ifdef ICS_SLOT_REPORT
	int idx;
#endif
	
	CST3XX_DEBUG("cst3xx enter sleep.\n");

#ifdef TPD_PROXIMITY
	if (tpd_proximity_flag == 1) {
		return ;
	}
#endif

#ifdef HYN_GESTURE
	printk(" func=%s,LINE=%d,gestrue_en=%d\n",__func__,__LINE__,gestrue_en);
    if(gestrue_en == true){
		u8 buf[4];
    
        buf[0] = 0xD0;
		buf[1] = 0x4C;
		buf[2] = 0x80;
		cst3xx_i2c_write(g_i2c_client, buf, 3);		
		CST3XX_DEBUG("tpd-gesture detect is opened \n");
		
		gestrue_lcd_flag=true;
		msleep(10);	
        return;
    }
#endif

#ifdef CONFIG_TP_ESD_PROTECT
	cst3xx_esd_switch(SWITCH_ESD_OFF);
#endif

    disable_irq(touch_irq);
//	tpd_gpio_output(GTP_RST_PORT, 0);
	cst3xx_sleep_ic();
  

#ifdef SLEEP_CLEAR_POINT
	#ifdef ICS_SLOT_REPORT
		for(idx=0; idx<=10; idx++) {	
			input_mt_slot(tpd->dev, idx);
			input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
			input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
		}
	#else
	    input_report_key(tpd->dev, BTN_TOUCH, 0);
		input_mt_sync(tpd->dev);
	#endif
		input_sync(tpd->dev);	
#endif	
	
	CST3XX_DEBUG("cst3xx enter sleep done.\n");
}

static struct tpd_driver_t cst3xx_ts_device = {
	.tpd_device_name = "cst3xx",
	.tpd_local_init = cst3xx_local_init,
	.suspend = cst3xx_suspend,
	.resume = cst3xx_resume,
};

/* called when loaded into kernel */
static int __init cst3xx_ts_init(void)
{
	CST3XX_DEBUG("hyn is entering cst3xx_ts_init.\n");
	//i2c_register_board_info(I2C_BUS_NUMBER, &cst3xx_i2c_tpd, 1);
	tpd_get_dts_info();
	if (tpd_driver_add(&cst3xx_ts_device) < 0)
		CST3XX_DEBUG("add cst3xx driver failed.\n");
	else
	CST3XX_DEBUG("hyn add cst3xx ---------------------driver ok ok\n");

	return 0;
}

/* should never be called */
static void __exit cst3xx_ts_exit(void)
{
	CST3XX_DEBUG("hyn is entering cst3xx_ts_exit.\n");
	tpd_driver_remove(&cst3xx_ts_device);
}

module_init(cst3xx_ts_init);
module_exit(cst3xx_ts_exit);

