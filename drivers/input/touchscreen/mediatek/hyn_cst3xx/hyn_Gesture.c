/*
 *
 * FocalTech TouchScreen driver.
 *
 * Copyright (c) 2010-2015, Focaltech Ltd. All rights reserved.
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

 /*******************************************************************************
*
* File Name: hyn_Gesture.c
*
* Author: qi wu
*
* Created: 2015-01-29
*
* Modify by mshl on 2015-03-20
*
* Abstract:
*
* Reference:
*
*******************************************************************************/

/*******************************************************************************
* 1.Included header files
*******************************************************************************/
#include "hyn_cst3_core.h"

#ifdef HYN_GESTURE

extern int cst3xx_i2c_read(struct i2c_client *client, unsigned char *buf, int len);
extern int cst3xx_i2c_write(struct i2c_client *client, unsigned char *buf, int len);
extern int cst3xx_i2c_read_register(struct i2c_client *client, unsigned char *buf, int len) ;

/*******************************************************************************
* Private constant and macro definitions using #define
*******************************************************************************/

#define GESTURE_RIGHT								0x01
#define GESTURE_UP		    						0x02
#define GESTURE_LEFT								0x03
#define GESTURE_DOWN								0x04
#define GESTURE_O		    						0x05
#define GESTURE_O_1		    						0x14
#define GESTURE_O_2		    						0x16
#define GESTURE_C		    					    0x06
#define GESTURE_C_1		    					    0x0E
#define GESTURE_E		    						0x07
#define GESTURE_E_1		    						0x11
#define GESTURE_E_2	    						    0x13
#define GESTURE_M		   	 						0x08
#define GESTURE_M_1	   	 						    0x09
#define GESTURE_M_2	   	 						    0x0F
#define GESTURE_M_3	   	 						    0x10
#define GESTURE_M_4	   	 						    0x15
#define GESTURE_W		    						0x0A
#define GESTURE_S		    						0x0C
#define GESTURE_S_1		    						0x12
#define GESTURE_V		    						0x0B
#define GESTURE_Z		    						0x0D
#define GESTURE_DOUBLECLICK						    0x20

#define KEY_GESTURE_U 						KEY_U
#define KEY_GESTURE_UP 						KEY_UP
#define KEY_GESTURE_DOWN 					KEY_DOWN
#define KEY_GESTURE_LEFT 					KEY_LEFT
#define KEY_GESTURE_RIGHT 					KEY_RIGHT
#define KEY_GESTURE_O 						KEY_O
#define KEY_GESTURE_E 						KEY_E
#define KEY_GESTURE_M 						KEY_M
#define KEY_GESTURE_W 						KEY_W
#define KEY_GESTURE_S 						KEY_S
#define KEY_GESTURE_V 						KEY_V
#define KEY_GESTURE_Z 						KEY_Z
#define KEY_GESTURE_C 						KEY_C


#define TS_WAKE_LOCK_TIMEOUT		(5 * HZ)
unsigned short hyn_gesture_id=0;


extern struct wake_lock gesture_chrg_lock;
extern struct input_dev *hyn_input_dev;	
static struct i2c_client *client01;


/*******************************************************************************
* Static variables
*******************************************************************************/


/*******************************************************************************
* Global variable or extern global variabls/functions
*******************************************************************************/

/*******************************************************************************
* Static function prototypes
*******************************************************************************/


/*******************************************************************************
*   Name: hyn_Gesture_init
*  Brief:
*  Input:
* Output: None
* Return: None
*******************************************************************************/
 struct hyn_gesture_entry {
	 struct attribute attr;
	 ssize_t (*show)(struct kobject *kobj, char *page);
	 ssize_t (*store)(struct kobject *kobj, const char *page, size_t size);
 };
 
 struct hyn_gesture_sysobj {
	 struct kobject kobj;
	 atomic_t enable;
 } gesture_hyn_sysobj = {
	 .enable = ATOMIC_INIT(0),
 };

int hyn_Gesture_init(struct input_dev *input_dev)
{
     
	input_set_capability(input_dev, EV_KEY, KEY_POWER);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_U);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_UP);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_DOWN);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_LEFT);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_RIGHT);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_O);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_E);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_M);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_W);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_S);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_V);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_Z);
	input_set_capability(input_dev, EV_KEY, KEY_GESTURE_C);
	


	__set_bit(KEY_GESTURE_RIGHT, input_dev->keybit);
	__set_bit(KEY_GESTURE_LEFT, input_dev->keybit);
	__set_bit(KEY_GESTURE_UP, input_dev->keybit);
	__set_bit(KEY_GESTURE_DOWN, input_dev->keybit);
	__set_bit(KEY_GESTURE_U, input_dev->keybit);
	__set_bit(KEY_GESTURE_O, input_dev->keybit);
	__set_bit(KEY_GESTURE_E, input_dev->keybit);
	__set_bit(KEY_GESTURE_M, input_dev->keybit);
	__set_bit(KEY_GESTURE_W, input_dev->keybit);
	__set_bit(KEY_GESTURE_S, input_dev->keybit);
	__set_bit(KEY_GESTURE_V, input_dev->keybit);
	__set_bit(KEY_GESTURE_Z, input_dev->keybit);
	__set_bit(KEY_GESTURE_C, input_dev->keybit);

	return 0;
}

/*******************************************************************************
*   Name: hyn_check_gesture
*  Brief:
*  Input:
* Output: None
* Return: None
*******************************************************************************/
static void hyn_check_gesture(struct input_dev *input_dev)
{
    wake_lock_timeout(&gesture_chrg_lock,TS_WAKE_LOCK_TIMEOUT);
	switch(hyn_gesture_id)
	{
	        case GESTURE_LEFT:
	                input_report_key(input_dev, KEY_GESTURE_LEFT, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_LEFT, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_RIGHT:
	                input_report_key(input_dev, KEY_GESTURE_RIGHT, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_RIGHT, 0);
	                input_sync(input_dev);
			    break;
	        case GESTURE_UP:
	                input_report_key(input_dev, KEY_GESTURE_UP, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_UP, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_DOWN:
	                input_report_key(input_dev, KEY_GESTURE_DOWN, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_DOWN, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_DOUBLECLICK:
	                input_report_key(input_dev, KEY_GESTURE_U, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_U, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_O:
			case GESTURE_O_1:
			case GESTURE_O_2:
	                input_report_key(input_dev, KEY_GESTURE_O, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_O, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_W:
	                input_report_key(input_dev, KEY_GESTURE_W, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_W, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_M:
			case GESTURE_M_1:
			case GESTURE_M_2:
			case GESTURE_M_3:
			case GESTURE_M_4:				
	                input_report_key(input_dev, KEY_GESTURE_M, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_M, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_E:
			case GESTURE_E_1:
			case GESTURE_E_2:
	                input_report_key(input_dev, KEY_GESTURE_E, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_E, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_S:
			case GESTURE_S_1:
	                input_report_key(input_dev, KEY_GESTURE_S, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_S, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_V:
	                input_report_key(input_dev, KEY_GESTURE_V, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_V, 0);
	                input_sync(input_dev);
	                break;
	        case GESTURE_Z:
	                input_report_key(input_dev, KEY_GESTURE_Z, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_Z, 0);
	                input_sync(input_dev);
	                break;
			case GESTURE_C:
			case GESTURE_C_1:			
	                input_report_key(input_dev, KEY_GESTURE_C, 1);
	                input_sync(input_dev);
	                input_report_key(input_dev, KEY_GESTURE_C, 0);
	                input_sync(input_dev);
	                break;
		
	    	default:

	                break;
	}

}

 /************************************************************************
* Name: hyn_read_Gestruedata
* Brief: read data from TP register
* Input: no
* Output: no
* Return: fail <0
***********************************************************************/
void hyn_read_Gestruedata(void)        //read data from fw.Drawdata[i] sent to mobile************************************************
{
    u8 buf[4];
	int ret;

	buf[0] = 0xD0;
	buf[1] = 0x4C;
	ret = cst3xx_i2c_read_register(client01, buf, 1);
    if(ret < 0)
    {
    	printk("iic read gesture flag failed.\n");   
    }
	hyn_gesture_id=buf[0]&0x7F;

	printk("[HYN_GESTURE] hyn_gesture_id =%d \n",hyn_gesture_id);

	hyn_check_gesture(hyn_input_dev);
	
}
 /**********************************************************
????
*********************************************************/

 static ssize_t hyn_gesture_attr_show(struct kobject *kobj, struct attribute *attr, char *buffer)
 {
	 struct hyn_gesture_entry *entry = container_of(attr, struct hyn_gesture_entry, attr);
	 return entry->show(kobj, buffer);
 }
 static ssize_t hyn_gesture_attr_store(struct kobject *kobj, struct attribute *attr, const char *buffer, size_t size)
 {
	 struct hyn_gesture_entry *entry = container_of(attr, struct hyn_gesture_entry, attr);
	 return entry->store(kobj, buffer, size);
 }

 static ssize_t hyn_gesture_enable_show(struct kobject *kobj, char *buffer)
 {
	 int buf_len=0;
	  buf_len += sprintf(buffer, "%d",tpd->gesture_enable);
	 return buf_len;
 }
 static ssize_t hyn_gesture_enable_store(struct kobject *kobj, const char *buffer, size_t size)
 {
	 int res = sscanf(buffer, "%d", &tpd->gesture_enable);
	 if (res != 1) {
		 printk("hyn_gesture_enable_store input string :'%s' \n",buffer);
	 } else {
		 printk("[hyn_gesture_enable_store] tpd->gesture_enable %d\n", tpd->gesture_enable);
	 }
	 return size;
 }

 static ssize_t hyn_gesture_debug_show(struct kobject *kobj, char *buffer)
 {
	 ssize_t len = 0;
	 //memcpy(buffer,hyn_gesture_id,1);
		 	
	 //printk("hyn_gesture_debug_show--end %d\n", hyn_gesture_id);

	 return len;
 }
 static ssize_t hyn_gesture_debug_store(struct kobject *kobj, const char *buffer, size_t size)
 {
	/* if(sscanf(buffer, "%c", &proc_operate_mode) == 1)
	 {
		 printk("hyn_gesture_debug_store success %d size=%d\n",proc_operate_mode,size);
		 printk("hyn_gesture_debug_store input string :'%s' \n",buffer);
	 }
	 else
	 {
		 printk("hyn_gesture_debug_store error format :'%s'\n",buffer);
		 proc_operate_mode=0xff;
		 return -EFAULT;
	 }
	 switch (proc_operate_mode) {
	 default:
		 break;
	 }*/

	 return size;
 }
 
 void hyn_gesture_sysfs_release(struct kobject *kobj)
 {
	 struct hyn_gesture_sysobj		 *ge_sysfs;

	 ge_sysfs=container_of(kobj, struct hyn_gesture_sysobj, kobj);

	 kfree(ge_sysfs);
 }

 /*---------------------------------------------------------------------------*/
 struct sysfs_ops hyn_gesture_sysfs_ops = {
	 .show	 = hyn_gesture_attr_show,
	 .store  = hyn_gesture_attr_store,
 };
 /*---------------------------------------------------------------------------*/
 /*---------------------------------------------------------------------------*/

 struct hyn_gesture_entry hyn_gesture_enable_entry = {
	 { .name = "gesture_enable", .mode = 0664 },
	 hyn_gesture_enable_show,
	 hyn_gesture_enable_store,
 };

 /*---------------------------------------------------------------------------*/
 struct hyn_gesture_entry hyn_gesture_debug_entry = {
	 { .name = "debug", .mode = 0664},
	 hyn_gesture_debug_show,
	 hyn_gesture_debug_store,
 };
 /*---------------------------------------------------------------------------*/
 struct attribute *hyn_gesture_attributes[] = {
	 &hyn_gesture_enable_entry.attr,  /*enable setting*/
	 &hyn_gesture_debug_entry.attr,
	 NULL,
 };
 /*---------------------------------------------------------------------------*/
 struct kobj_type hyn_gesture_ktype = {
	 .sysfs_ops = &hyn_gesture_sysfs_ops,
	 .release =hyn_gesture_sysfs_release,
	 .default_attrs = hyn_gesture_attributes,
 };
 /*---------------------------------------------------------------------------*/

 /*---------------------------------------------------------------------------*/
 int hyn_gesture_sysfs(struct i2c_client * client)
 {
	 struct hyn_gesture_sysobj *obj = &gesture_hyn_sysobj;

	 memset(&obj->kobj, 0x00, sizeof(obj->kobj));

	 atomic_set(&obj->enable, 0);

	 obj->kobj.parent = kernel_kobj;
	 if (kobject_init_and_add(&obj->kobj, &hyn_gesture_ktype, NULL, "gesture")) {
		 kobject_put(&obj->kobj);
		 printk("error hyn_gesture_sysfs ");
		 return -ENOMEM;
	 }
	 kobject_uevent(&obj->kobj, KOBJ_ADD);
	 client01 = client;
	 return 0;
 }

#endif
