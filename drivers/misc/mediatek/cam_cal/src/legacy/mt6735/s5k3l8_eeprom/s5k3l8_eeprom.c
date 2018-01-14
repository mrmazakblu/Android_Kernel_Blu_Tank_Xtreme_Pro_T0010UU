/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*
 * Driver for CAM_CAL
 *
 *
 */

#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "kd_camera_typedef.h"
#include "cam_cal.h"
#include "cam_cal_define.h"
#include "s5k3l8_eeprom.h"
//#include <asm/system.h>  // for SMP
#include <linux/dma-mapping.h>
#include <linux/module.h>

#define CAM_CALGETDLT_DEBUG
#define CAM_CAL_DEBUG
#ifdef CAM_CAL_DEBUG
#define CAM_CALDB printk
#else
#define CAM_CALDB(x,...)
#endif
extern void kdSetI2CSpeed(u16 i2cSpeed);
static DEFINE_SPINLOCK(g_CAM_CALLock); // for SMP
#define CAM_CAL_I2C_BUSNUM 0


/*******************************************************************************
*
********************************************************************************/
#define CAM_CAL_ICS_REVISION 1 //seanlin111208
/*******************************************************************************
*
********************************************************************************/
#define CAM_CAL_DRVNAME "CAM_CAL_DRV"
#define CAM_CAL_I2C_GROUP_ID 0
/*******************************************************************************
*
********************************************************************************/
static struct i2c_board_info __initdata kd_cam_cal_dev={ I2C_BOARD_INFO(CAM_CAL_DRVNAME, 0xB0>>1)};
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 *a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
static struct i2c_client * g_pstI2Cclient = NULL;

//81 is used for V4L driver
static dev_t g_CAM_CALdevno = MKDEV(CAM_CAL_DEV_MAJOR_NUMBER,0);
static struct cdev * g_pCAM_CAL_CharDrv = NULL;
//static spinlock_t g_CAM_CALLock;
//spin_lock(&g_CAM_CALLock);
//spin_unlock(&g_CAM_CALLock);

static struct class *CAM_CAL_class = NULL;
static atomic_t g_CAM_CALatomic;
//static DEFINE_SPINLOCK(kdcam_cal_drv_lock);
//spin_lock(&kdcam_cal_drv_lock);
//spin_unlock(&kdcam_cal_drv_lock);

#define OTP_SIZE 1388
extern unsigned char otp_data[OTP_SIZE] ;

int otp_flag=0;



static int iWriteData(unsigned int  ui4_offset, unsigned int  ui4_length, unsigned char * p_buff)
{
	return 0;
}

#if 0
static int iWriteCAM_CAL(u16 addr  , u32 Bytes, u8 pdata)
{
   int  ret = 0;
	int retry = 3;
    char puSendCmd[3] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,pdata};
	g_pstI2Cclient->addr = (S5K3L8OTP_DEVICE_ID>> 1);

	do {
        printk("[CAM_CAL][iWriteCAM_CAL] Write 0x%x=0x%x \n",addr, pdata);
		ret = i2c_master_send(g_pstI2Cclient, puSendCmd, 3);
        if (ret != 3) {
            printk("[CAM_CAL] I2C send failed!!\n");
        }
        else {
            return 0;
    	}
        mdelay(10);
    } while ((retry--) > 0);
	return -1;
}


static int iReadCAM_CAL(u16 addr, u32 len, u8 * pbuff)
{
    int  ret = 0;
    char puReadCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF)};
	g_pstI2Cclient->addr = (S5K3L8OTP_DEVICE_ID>> 1);

    
    ret = i2c_master_send(g_pstI2Cclient, puReadCmd, 2);

    if (ret != 2)
    {
        printk("[CAM_CAL] I2C send read address failed!! \n");
        return -1;
    }


    ret = i2c_master_recv(g_pstI2Cclient, (char *)pbuff, len);
	printk("[CAM_CAL][iReadCAM_CAL] Read 0x%x=0x%x \n", addr, pbuff[0]);
    if (ret != len)
    {
        printk("[CAM_CAL] I2C read data failed!! \n");
        return -1;
    }
	return 0;
}
#endif
#if 0
int iReadCAM_CAL_8(u8 addr, u8 * pbuff)
{
    int  ret = 0;
    char puReadCmd[1] = {(char)(addr)};

    spin_lock(&g_CAM_CALLock); //for SMP
    g_pstI2Cclient->addr = S5K3L8OTP_DEVICE_ID>>1;
	g_pstI2Cclient->timing=400;
	g_pstI2Cclient->addr = g_pstI2Cclient->addr & (I2C_MASK_FLAG | I2C_WR_FLAG);
    spin_unlock(&g_CAM_CALLock); // for SMP

    ret = i2c_master_send(g_pstI2Cclient, puReadCmd, 1);

    if (ret != 1)
    {
        printk("[CAM_CAL] I2C send read address failed!! \n");
	    printk("[CAMERA SENSOR] I2C send failed, addr = 0x%x, data = 0x%x !! \n", addr,  *pbuff );
        return -1;
    }

    ret = i2c_master_recv(g_pstI2Cclient, (char *)pbuff, 1);
	printk("[CAM_CAL][iReadCAM_CAL] Read 0x%x=0x%x \n", addr, pbuff[0]);
    if (ret != 1)
    {
        printk("[CAM_CAL] I2C read data failed!! \n");
        return -1;
    }
    return 0;
}



void S5K3L8_Read_LSC_Otp(kal_uint16 addr,unsigned int psize,unsigned char *pbuff)
{
	int i ;
	unsigned char temp = 0;
	if(otp_flag){
		for(i=0;i<psize;i++)
			pbuff[i]=otp_data[i];
	}
	else{
		for(i = 0; i<psize; i++)
		{
			iReadCAM_CAL_8(addr+i,&temp);
			printk("[CAM_CAL]addr+i = 0x%x,pbuff = 0x%x\n",(addr+i),temp );
			*(pbuff+i) = (unsigned char)temp;
		}  
	}
	otp_flag = 0;

}



void S5K3L8_Read_Otp(kal_uint16 addr,unsigned int pSize,unsigned char *pbuff)
{
	int i;
	unsigned char temp = 0;
	
	for(i = 0; i<pSize; i++)
		{
			iReadCAM_CAL_8(addr+i,&temp);
			printk("[CAM_CAL]addr+i = 0x%x,pbuff = 0x%x\n",(addr+i),temp );
			*(pbuff+i) =(unsigned char)temp;
		}

}


 int iReadData(kal_uint16 p_offset, unsigned int  p_len, unsigned char * p_buff)
{
   int  i = 0;

	if(p_len ==1)	//layout
    {
      S5K3L8_Read_Otp(p_offset,p_len,p_buff);
		printk("[cam_cal][xiaozhicai]layout check = %d ! \n",*p_buff);
   	}
	else if(p_len == 4)	//awb
    {      
		S5K3L8_Read_Otp(p_offset,p_len,p_buff);
   	}
	else if(p_len == 2 )	//af
    {
		S5K3L8_Read_Otp(p_offset,p_len,p_buff);
   	}
	else{		//lsc

		S5K3L8_Read_LSC_Otp(p_offset,p_len,p_buff);
	}
	
    for(i = 0;i<p_len;i++){
    printk( "[[CAM_CAL]]p_buff[%d]=%x\n", i,*(p_buff+i));
	}
    printk("[S24EEPORM] iReadData done\n" );
   return 0;
}
#else



static bool s5k3l8_read_eeprom_byte(u16 addr, u8* data)
{
    char pu_send_cmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    if(addr > S5K3L8_MAX_OFFSET)
        return false;

    if(iReadRegI2C(pu_send_cmd, 2, (u8*)data, 1, S5K3L8OTP_DEVICE_ID)<0)
        return false;
    return true;
}


int s5k3l8_read_eeprom(unsigned int addr, unsigned char *data, unsigned int size)
{
    int i;
    int offset = addr;
    int ret = 0;
    u8 flag[2] = {0};
    u32 check_sum = 0;
	kdSetI2CSpeed(100);
	msleep(150);

 
    for(i = 0; i < size; i++) {
		if (s5k3l8_read_eeprom_byte(offset, &data[i])) {
		CAM_CALDB("[S24CAM_CAL] read_eeprom 0x%0x 0x%x\n",offset, data[i]);
		offset += 1;
		ret += 1;
		check_sum += data[i];

		}else {
			break;
		}
    }

    check_sum = check_sum % 256;

    s5k3l8_read_eeprom_byte((offset), &flag[1]);
    CAM_CALDB("[S24CAM_CAL] read_eeprom 0x%0x %d. check_sum:%d.\n",(offset), flag[1], check_sum);

    return ret;
}

#endif


/*******************************************************************************
*
********************************************************************************/
#define NEW_UNLOCK_IOCTL
#ifndef NEW_UNLOCK_IOCTL
static int CAM_CAL_Ioctl(struct inode * a_pstInode,
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
#else
static long CAM_CAL_Ioctl(
    struct file *file,
    unsigned int a_u4Command,
    unsigned long a_u4Param
)
#endif
{
    int i4RetValue = 0;
    u8 * pBuff = NULL;
    u8 * pWorkingBuff = NULL;
    stCAM_CAL_INFO_STRUCT *ptempbuf;

#ifdef CAM_CALGETDLT_DEBUG
    struct timeval ktv1, ktv2;
    unsigned long TimeIntervalUS;
#endif
	CAM_CALDB("[S24CAM_CAL] CAM_CALIOC_S_WRITE:%x,CAM_CALIOC_G_READ:%x ,size of stCAM_CAL_INFO_STRUCT :%x \n",CAM_CALIOC_S_WRITE,CAM_CALIOC_G_READ,(sizeof(stCAM_CAL_INFO_STRUCT)));
	CAM_CALDB("[S24CAM_CAL] CAM_CALIOC_S_WRITE:%x,CAM_CALIOC_G_READ:%x \n",CAM_CALIOC_S_WRITE,CAM_CALIOC_G_READ);
#ifdef CONFIG_COMPAT
	CAM_CALDB("[S24CAM_CAL] COMPAT_CAM_CALIOC_S_WRITE:%x,COMPAT_CAM_CALIOC_G_READ:%x \n",COMPAT_CAM_CALIOC_S_WRITE,COMPAT_CAM_CALIOC_G_READ);
#endif

    if(_IOC_NONE == _IOC_DIR(a_u4Command))
    {
    }
    else
    {
		 CAM_CALDB("[S24CAM_CAL][CAM_CAL_Ioctl] ioctl data trans \n");
        pBuff = (u8 *)kmalloc(sizeof(stCAM_CAL_INFO_STRUCT),GFP_KERNEL);

        if(NULL == pBuff)
        {
            CAM_CALDB("[S24CAM_CAL][CAM_CAL_Ioctl] ioctl allocate mem failed\n");
            return -ENOMEM;
        }

        if(_IOC_WRITE & _IOC_DIR(a_u4Command))
        {
            if(copy_from_user((u8 *) pBuff , (u8 *) a_u4Param, sizeof(stCAM_CAL_INFO_STRUCT)))
            {    //get input structure address
                kfree(pBuff);
                CAM_CALDB("[S24CAM_CAL][CAM_CAL_Ioctl] copy from user failed\n");
                return -EFAULT;
            }
        }
    }

    ptempbuf = (stCAM_CAL_INFO_STRUCT *)pBuff;
    pWorkingBuff = (u8*)kmalloc(ptempbuf->u4Length,GFP_ATOMIC);
    if(NULL == pWorkingBuff)
    {
        kfree(pBuff);
        CAM_CALDB("[S24CAM_CAL] ioctl allocate mem failed\n");
        return -ENOMEM;
    }
     CAM_CALDB("[S24CAM_CAL] init Working buffer address 0x%8x  command is 0x%8x pu1Params :%s \n", (u32)pWorkingBuff, (u32)a_u4Command,(u8*)ptempbuf->pu1Params);
	i4RetValue = copy_from_user((u8*)pWorkingBuff ,  (u8*)ptempbuf->pu1Params, ptempbuf->u4Length);
	
    if(i4RetValue)
    {
        kfree(pBuff);
        kfree(pWorkingBuff);
        CAM_CALDB("[S24CAM_CAL] bufferlength: %d ,ioctl copy from user failed\n",ptempbuf->u4Length);
        return -EFAULT;
    }

    switch(a_u4Command)
    {
        case CAM_CALIOC_S_WRITE:
            CAM_CALDB("[S24CAM_CAL] Write CMD:%d \n",CAM_CALIOC_S_WRITE);
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv1);
#endif
            i4RetValue = iWriteData((u16)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Write data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif
            break;
        case CAM_CALIOC_G_READ:
            CAM_CALDB("[S24CAM_CAL] Read CMD:%d \n",CAM_CALIOC_G_READ);
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv1);
#endif
			
			otp_flag = 1;

            CAM_CALDB("[CAM_CAL] offset %d \n", ptempbuf->u4Offset);
            CAM_CALDB("[CAM_CAL] length %d \n", ptempbuf->u4Length);
            CAM_CALDB("[CAM_CAL] Before read Working buffer address 0x%8x \n", (u32)pWorkingBuff);
           // i4RetValue = iReadData((u16)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
			i4RetValue = s5k3l8_read_eeprom((u16)ptempbuf->u4Offset, pWorkingBuff, ptempbuf->u4Length);

            CAM_CALDB("[S24CAM_CAL] After read Working buffer data  0x%4x \n", *pWorkingBuff);


#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Read data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif

            break;
        default :
      	     CAM_CALDB("[S24CAM_CAL] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    if(_IOC_READ & _IOC_DIR(a_u4Command))
    {
        //copy data to user space buffer, keep other input paremeter unchange.
        CAM_CALDB("[S24CAM_CAL] to user length %d \n", ptempbuf->u4Length);
        CAM_CALDB("[S24CAM_CAL] to user  Working buffer address 0x%8x \n", (u32)pWorkingBuff);
        if(copy_to_user((u8 __user *) ptempbuf->pu1Params , (u8 *)pWorkingBuff , ptempbuf->u4Length))
        {
            kfree(pBuff);
            kfree(pWorkingBuff);
            CAM_CALDB("[S24CAM_CAL] ioctl copy to user failed\n");
            return -EFAULT;
        }
    }

    kfree(pBuff);
    kfree(pWorkingBuff);
    return i4RetValue;
}


static u32 g_u4Opened = 0;
//#define
//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
static int CAM_CAL_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    CAM_CALDB("[S24CAM_CAL] CAM_CAL_Open\n");
    spin_lock(&g_CAM_CALLock);
    if(g_u4Opened)
    {
        spin_unlock(&g_CAM_CALLock);
		CAM_CALDB("[S24CAM_CAL] Opened, return -EBUSY\n");
        return -EBUSY;
    }
    else
    {
        g_u4Opened = 1;
        atomic_set(&g_CAM_CALatomic,0);
    }
    spin_unlock(&g_CAM_CALLock);

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int CAM_CAL_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    spin_lock(&g_CAM_CALLock);

    g_u4Opened = 0;

    atomic_set(&g_CAM_CALatomic,0);

    spin_unlock(&g_CAM_CALLock);

    return 0;
}

static const struct file_operations g_stCAM_CAL_fops =
{
    .owner = THIS_MODULE,
    .open = CAM_CAL_Open,
    .release = CAM_CAL_Release,
    //.ioctl = CAM_CAL_Ioctl
    .unlocked_ioctl = CAM_CAL_Ioctl
};

#define CAM_CAL_DYNAMIC_ALLOCATE_DEVNO 1
inline static int RegisterCAM_CALCharDrv(void)
{
    struct device* CAM_CAL_device = NULL;

#if CAM_CAL_DYNAMIC_ALLOCATE_DEVNO
    if( alloc_chrdev_region(&g_CAM_CALdevno, 0, 1,CAM_CAL_DRVNAME) )
    {
        CAM_CALDB("[S24CAM_CAL] Allocate device no failed\n");

        return -EAGAIN;
    }
#else
    if( register_chrdev_region(  g_CAM_CALdevno , 1 , CAM_CAL_DRVNAME) )
    {
        CAM_CALDB("[S24CAM_CAL] Register device no failed\n");

        return -EAGAIN;
    }
#endif

    //Allocate driver
    g_pCAM_CAL_CharDrv = cdev_alloc();

    if(NULL == g_pCAM_CAL_CharDrv)
    {
        unregister_chrdev_region(g_CAM_CALdevno, 1);

        CAM_CALDB("[S24CAM_CAL] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
  
	cdev_init(g_pCAM_CAL_CharDrv, &g_stCAM_CAL_fops);

	g_pCAM_CAL_CharDrv->owner = THIS_MODULE;

	/* Add to system */
	if (cdev_add(g_pCAM_CAL_CharDrv, g_CAM_CALdevno, 1)) {
		CAM_CALDB(" Attatch file operation failed\n");

		unregister_chrdev_region(g_CAM_CALdevno, 1);

		return -EAGAIN;
	}

	CAM_CAL_class = class_create(THIS_MODULE, "CAM_CALdrv");
	if (IS_ERR(CAM_CAL_class)) {
		int ret = PTR_ERR(CAM_CAL_class);

		CAM_CALDB("Unable to create class, err = %d\n", ret);
		return ret;
	}
	CAM_CAL_device = device_create(CAM_CAL_class, NULL, g_CAM_CALdevno, NULL, CAM_CAL_DRVNAME);



    return 0;
}

inline static void UnregisterCAM_CALCharDrv(void)
{
    //Release char driver
    cdev_del(g_pCAM_CAL_CharDrv);

    unregister_chrdev_region(g_CAM_CALdevno, 1);

    device_destroy(CAM_CAL_class, g_CAM_CALdevno);
    class_destroy(CAM_CAL_class);
}


//////////////////////////////////////////////////////////////////////
#ifndef CAM_CAL_ICS_REVISION
static int CAM_CAL_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
#elif 0
static int CAM_CAL_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
#else
#endif
static int CAM_CAL_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int CAM_CAL_i2c_remove(struct i2c_client *);

static const struct i2c_device_id CAM_CAL_i2c_id[] = {{CAM_CAL_DRVNAME,0},{}};
#if 0 //test110314 Please use the same I2C Group ID as Sensor
static unsigned short force[] = {CAM_CAL_I2C_GROUP_ID, S5K3L8OTP_DEVICE_ID, I2C_CLIENT_END, I2C_CLIENT_END};
#else
//static unsigned short force[] = {IMG_SENSOR_I2C_GROUP_ID, OV5647OTP_DEVICE_ID, I2C_CLIENT_END, I2C_CLIENT_END};
#endif
//static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces,};


static struct i2c_driver CAM_CAL_i2c_driver = {
    .probe = CAM_CAL_i2c_probe,
    .remove = CAM_CAL_i2c_remove,
//   .detect = CAM_CAL_i2c_detect,
    .driver.name = CAM_CAL_DRVNAME,
    .id_table = CAM_CAL_i2c_id,
};

#ifndef CAM_CAL_ICS_REVISION
static int CAM_CAL_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, CAM_CAL_DRVNAME);
    return 0;
}
#endif
static int CAM_CAL_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
int i4RetValue = 0;
    CAM_CALDB("[S24CAM_CAL] Attach I2C \n");
//    spin_lock_init(&g_CAM_CALLock);

    //get sensor i2c client
    spin_lock(&g_CAM_CALLock); //for SMP
    g_pstI2Cclient = client;
    g_pstI2Cclient->addr = S5K3L8OTP_DEVICE_ID>>1;
    spin_unlock(&g_CAM_CALLock); // for SMP

    CAM_CALDB("[CAM_CAL] g_pstI2Cclient->addr = 0x%8x \n",g_pstI2Cclient->addr);
    //Register char driver
    i4RetValue = RegisterCAM_CALCharDrv();

    if(i4RetValue){
        CAM_CALDB("[S24CAM_CAL] register char device failed!\n");
        return i4RetValue;
    }


    CAM_CALDB("[S24CAM_CAL] Attached!! \n");
    return 0;
}

static int CAM_CAL_i2c_remove(struct i2c_client *client)
{
    return 0;
}

static int CAM_CAL_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&CAM_CAL_i2c_driver);
}

static int CAM_CAL_remove(struct platform_device *pdev)
{
    i2c_del_driver(&CAM_CAL_i2c_driver);
    return 0;
}

// platform structure
static struct platform_driver g_stCAM_CAL_Driver = {
    .probe		= CAM_CAL_probe,
    .remove	= CAM_CAL_remove,
    .driver		= {
        .name	= CAM_CAL_DRVNAME,
        .owner	= THIS_MODULE,
    }
};


static struct platform_device g_stCAM_CAL_Device = {
    .name = CAM_CAL_DRVNAME,
    .id = 0,
    .dev = {
    }
};

static int __init CAM_CAL_i2C_init(void)
{
    i2c_register_board_info(CAM_CAL_I2C_BUSNUM, &kd_cam_cal_dev, 1);
    if(platform_driver_register(&g_stCAM_CAL_Driver)){
        CAM_CALDB("failed to register S24CAM_CAL driver\n");
        return -ENODEV;
    }

    if (platform_device_register(&g_stCAM_CAL_Device))
    {
        CAM_CALDB("failed to register S24CAM_CAL driver, 2nd time\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit CAM_CAL_i2C_exit(void)
{
	platform_driver_unregister(&g_stCAM_CAL_Driver);
}

module_init(CAM_CAL_i2C_init);
module_exit(CAM_CAL_i2C_exit);

MODULE_DESCRIPTION("CAM_CAL driver");
MODULE_AUTHOR("Sean Lin <Sean.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");


