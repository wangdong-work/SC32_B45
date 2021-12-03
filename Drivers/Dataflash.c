/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Dataflash.c
版    本：1.00
创建日期：2012-03-12
作    者：郭数理
功能描述：AT45DB08驱动实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-12  1.00     创建
**************************************************************/

#include <LPC17xx.h>
#include <rtl.h>
#include <string.h>
#include "Dataflash.h"
#include "Delay.h"


#define DATAFLASH_WRITE_VERIFY

#define SSP_SR_TFE 0x01
#define SSP_SR_TNF 0x02
#define SSP_SR_RNE 0x04
#define SSP_SR_RFF 0x08
#define SSP_SR_BSY 0x10
#define SSP_FIFO_SIZE 8

#define PAGE_SIZE 264
#define PAGE_OFFSET 9


/* Status Register Read */
#define OP_READ_STATUS		0xD7

/* Continuous Array Read */
#define OP_READ_CONTINUOUS	0x03

/* Main Memory Page to Buffer Transfer */
#define OP_TRANSFER_BUF1	0x53
#define OP_TRANSFER_BUF2	0x55

/* write to buffer, then write-erase to flash */
#define OP_PROGRAM_VIA_BUF1	0x82
#define OP_PROGRAM_VIA_BUF2	0x85

/* compare buffer to flash */
#define OP_COMPARE_BUF1		0x60
#define OP_COMPARE_BUF2		0x61

#define SSEL_LOW() (LPC_GPIO1->FIOCLR = (1<<21))
#define SSEL_HIGH() (LPC_GPIO1->FIOSET = (1<<21))


static OS_MUT m_mut_dataflash_mutex;

/*************************************************************
函数名称: v_flash_ssp_init		           				
函数功能: 配置与dataflash相关的spi口						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_flash_ssp_init(void)
{
	U32_T reg;

	reg = LPC_GPIO1->FIODIR;  // SSEL0每发送一个数据都会拉高片选，不符合dataflash的时序要求，所以把片选设置为GPIO输出，软件控制片选引脚的电平
	reg |= (1 << 21);
	LPC_GPIO1->FIODIR = reg;

	LPC_SSP0->CPSR = 0x00000002;     // CPSDVSR=2
	LPC_SSP0->IMSC = 0;              // Disable all interrupt
	LPC_SSP0->ICR = 0x00000003;      // Clear interrupt flags
	LPC_SSP0->DMACR = 0;             // Disable DMA
	LPC_SSP0->CR0 = 0x00000007;      // 8bit data frame, spi mode 0
	LPC_SSP0->CR1 = 0x00000002;		 // enable spi
}

/*************************************************************
函数名称: v_flash_ssp_write		           				
函数功能: 通过spi口发送数据						
输入参数: pu8_tx     -- 指向要发送的数据区
          u32_tx_len --	要发送数据的长度      		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_flash_ssp_write(const U8_T *pu8_tx, U32_T u32_tx_len)
{
	U32_T i;
	U8_T buf;

	SSEL_LOW();

	for (i=0; i<u32_tx_len; )                            //发送所有的数据，丢弃接收到的数据，不考虑溢出问题
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx++;
			i++;
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)             //等待数据发送完成
	{
		if (i++ > 100000)                                //如果出现某种错误，不能正常退出，则超时1s退出
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)             //清空接收FIFO，丢弃接收到的数据
	{
		buf = LPC_SSP0->DR;
		if (++i > SSP_FIFO_SIZE+1)                       //最多只有SSP_FIFO_SIZE个数据
		{
			break;
		}
	}
	LPC_SSP0->ICR = 0x00000003;                          //清除相关的中断标志，因为没有用中断，所以此语句可以被屏蔽
	buf++;                                               //无用语句，消除变量末用警告

	SSEL_HIGH();
}

/*************************************************************
函数名称: v_flash_ssp_write_two_blocks		           				
函数功能: 通过spi口发送两块数据						
输入参数: pu8_tx1     -- 指向要发送的第一块数据区
          u32_tx1_len -- 第一块数据的长度
		  pu8_tx2     -- 指向要发送的第二块数据区
          u32_tx2_len -- 第二块数据的长度      		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_flash_ssp_write_two_blocks(const U8_T *pu8_tx1, U32_T u32_tx1_len, const U8_T *pu8_tx2, U32_T u32_tx2_len)
{
	U32_T i;
	U32_T buf = 0;

	SSEL_LOW();

	for (i=0; i<u32_tx1_len; )                               //发送tx1的数据，丢弃接收到的数据，不考虑溢出问题
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx1++;
			i++;
		}
	}

	for (i=0; i<u32_tx2_len; )                               //发送tx2的数据，丢弃接收到的数据，不考虑溢出问题
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx2++;
			i++;
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)             //等待数据发送完成
	{
		if (i++ > 100000)                                //如果出现某种错误，不能正常退出，则超时1s退出
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)             //清空接收FIFO，丢弃接收到的数据
	{
		buf = LPC_SSP0->DR;
		if (++i > SSP_FIFO_SIZE+1)                       //最多只有SSP_FIFO_SIZE个数据
		{
			break;
		}
	}
	LPC_SSP0->ICR = 0x00000003;		                     //清除相关的中断标志，因为没有用中断，所以此语句可以被屏蔽
	buf++;												 //无用语句，消除变量末用警告

	SSEL_HIGH();
}

/*************************************************************
函数名称: v_flash_ssp_write_then_read		           				
函数功能: 通过spi口发送两块数据						
输入参数: pu8_tx     -- 指向要发送的数据区
          u32_tx_len --	要发送数据的长度
		  u32_rx_len -- 接收数据缓冲区的长度
输出参数: pu8_rx     -- 指向接收缓冲区，用于保存接收到的数据，调用者需为其分配至少u32_rx_len个字节的空间
返回值  ：无														   				
**************************************************************/
static void v_flash_ssp_write_then_read(const U8_T *pu8_tx, U32_T u32_tx_len, U8_T *pu8_rx, U32_T u32_rx_len)
{
	U8_T *buf = pu8_rx;
	U32_T i, cnt;
	U32_T remaining = u32_rx_len;

	SSEL_LOW();

	for (i=0; i<u32_tx_len; )                                //发送所有的数据，丢弃接收到的数据，不考虑溢出问题
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx++;
			i++;
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)              //等待数据发送完成
	{
		if (i++ > 100000)                                 //如果出现某种错误，不能正常退出，则超时1s退出
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)             //清空接收FIFO，丢弃接收到的数据
	{
		buf[0] = LPC_SSP0->DR;
		if (++i > SSP_FIFO_SIZE+1)                       //最多只有SSP_FIFO_SIZE个数据
		{
			break;
		}
	}
	LPC_SSP0->ICR = 0x00000003;                          //清除相关的中断标志，因为没有用中断，所以此语句可以被屏蔽

	while (remaining != 0)
	{
		if (remaining > SSP_FIFO_SIZE)
		{
			cnt = SSP_FIFO_SIZE;
			remaining -= SSP_FIFO_SIZE;
		}
		else
		{
			cnt = remaining;
			remaining = 0;
		}

		for (i=0; i<cnt; )                               //由于FIFO只有SSP_FIFO_SIZE个字节，所以每次最多发送SSP_FIFO_SIZE个字节
		{
			if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
			{
				LPC_SSP0->DR = 0;			             //只是为了读取数据，发送的字节可以是任意数据，这里发送0
				i++;
			}
		}
		
		i = 0;
		while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)         //等待数据发送完成
		{
			if (i++ > 100000)                            //如果出现某种错误，不能正常退出，则超时1s退出
			{
				break;
			}
			else
			{
				v_delay_udelay(10);
			}
		}

		i = 0;
		while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)
		{
			*buf++ = LPC_SSP0->DR;
			if (++i >= cnt)
			{
				break;
			}
		}
	}

	SSEL_HIGH();
}

/*************************************************************
函数名称: u8_flash_dataflash_waitready		           				
函数功能: 查询dataflash的状态，判断操作是否完成						
输入参数: 无      		   				
输出参数: 无
返回值  ：返回dataflash的状态值，出错返回0														   				
**************************************************************/
static U8_T u8_flash_dataflash_waitready(void)
{
	U8_T	status;
	U8_T command = OP_READ_STATUS;
	U32_T i = 0;

	while (1)
	{
		v_flash_ssp_write_then_read(&command, 1, &status, 1);

		if (status & (1 << 7))	/* RDY/nBSY */
		{
			return status;
		}

		if (i++ > 100000)                  //如果出现某种错误，不能正常退出，则超时1s退出
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	return 0;
}

/*************************************************************
函数名称: v_flash_dataflash_init
函数功能: 初始化dataflash, 配置与dataflash相关的spi口
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_flash_dataflash_init(void)
{
	U32_T reg;

	reg = LPC_GPIO2->FIODIR;  //P2.2为控制字库和型号烧写是否允许的引脚，设为输入状态，高电平允许烧写字库和型号，低电平不允许烧写字库和型号
	reg &= ~(1 << 2);
	LPC_GPIO2->FIODIR = reg;
	
	v_flash_ssp_init();
}

/*************************************************************
函数名称: u8_flash_read_font_task_pin
函数功能: 读取字库和型号烧写控制引脚的状态
输入参数: 无
输出参数: 无
返回值  ：1：允许烧写；0：不允许烧写
**************************************************************/
U8_T u8_flash_read_font_task_pin(void)
{
	if ((LPC_GPIO2->FIOPIN & 0x0004) != 0)
		return 1;
	else
		return 0;
}

/*************************************************************
函数名称: v_flash_dataflash_init_mutex		           				
函数功能: 初始化dataflash相关的互斥量，在RTX启动之后调用，在第一个任务的开头部分调用						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_flash_dataflash_init_mutex(void)
{
	os_mut_init(m_mut_dataflash_mutex);             //初始化互斥量
}

/*************************************************************
函数名称: s32_flash_dataflash_read		           				
函数功能: 读取dataflash数据						
输入参数: u32_from -- 要读取的地址
		  u32_len  -- 要读取的长度        		   				
输出参数: pu8_buf  -- 返回读取到的内容，调用者需为其分配足够的空间（长度不小于len)
返回值  ：返回0表示成功，若返回-1则表示出错          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_read(U32_T u32_from, U8_T *pu8_buf, U32_T u32_len)
{
	U32_T addr;
	U8_T tx_buf[4];

	if (u32_len == 0)
		return 0;

	if (u32_from + u32_len > DATAFLASH_SIZE)
		return -1;

	os_mut_wait(m_mut_dataflash_mutex, 0xffff);     //操作前获取互斥量

	addr = ((u32_from / PAGE_SIZE) << PAGE_OFFSET) + (u32_from % PAGE_SIZE);

	tx_buf[0] = OP_READ_CONTINUOUS;
	tx_buf[1] = (U8_T)(addr >> 16);
	tx_buf[2] = (U8_T)(addr >> 8);
	tx_buf[3] = (U8_T)addr;

	v_flash_ssp_write_then_read(tx_buf, 4, pu8_buf, u32_len);

	os_mut_release(m_mut_dataflash_mutex);         //操作后释放互斥量       

	return 0;
}

/*************************************************************
函数名称: s32_flash_dataflash_write		           				
函数功能: 写数据到dataflash						
输入参数: u32_to -- 要写入的地址
		  u32_len  -- 要写入数据的长度        		   				
          pu8_buf  -- 指向要写入的数据
输出参数: 无
返回值  ：返回0表示成功，若返回-1则表示出错          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_write(U32_T u32_to, U8_T *pu8_buf, U32_T u32_len)
{
	S32_T status = 0;
	U32_T page_addr, page_offset, addr;
	U32_T remaining = u32_len;
	U32_T write_len;
	U8_T tx_buf[4];
	U8_T *write_buf = pu8_buf;

	if (u32_len == 0)
		return 0;

	if (u32_to + u32_len > DATAFLASH_SIZE)
		return -1;

	os_mut_wait(m_mut_dataflash_mutex, 0xffff);     //操作前获取互斥量

	page_addr = (u32_to / PAGE_SIZE);
	page_offset = (u32_to % PAGE_SIZE);

	if (page_offset + u32_len > PAGE_SIZE)
		write_len = PAGE_SIZE - page_offset;
	else
		write_len = u32_len;

	while (remaining != 0)
	{
		addr = (page_addr << PAGE_OFFSET);

		if (write_len != PAGE_SIZE)
		{
			tx_buf[0] = OP_TRANSFER_BUF1;
			tx_buf[1] = (U8_T)(addr >> 16);
			tx_buf[2] = (U8_T)(addr >> 8);
			tx_buf[3] = (U8_T)addr;

			v_flash_ssp_write(tx_buf, 4);
			u8_flash_dataflash_waitready();
		}

		addr += page_offset;
		tx_buf[0] = OP_PROGRAM_VIA_BUF1;
		tx_buf[1] = (U8_T)(addr >> 16);
		tx_buf[2] = (U8_T)(addr >> 8);
		tx_buf[3] = (U8_T)addr;
		v_flash_ssp_write_two_blocks(tx_buf, 4, write_buf, write_len);
		u8_flash_dataflash_waitready();

#ifdef DATAFLASH_WRITE_VERIFY
		addr = (page_addr << PAGE_OFFSET);	
		tx_buf[0] = OP_COMPARE_BUF1;
		tx_buf[1] = (U8_T)(addr >> 16);
		tx_buf[2] = (U8_T)(addr >> 8);
		tx_buf[3] = (U8_T)addr;

		v_flash_ssp_write(tx_buf, 4);
		if ((u8_flash_dataflash_waitready() & 0x40) != 0)
			status = -1;
#endif

		remaining -= write_len;
		page_addr++;
		page_offset = 0;
		write_buf += write_len;

		if (remaining > PAGE_SIZE)
			write_len = PAGE_SIZE;
		else
			write_len = remaining;
	}

	os_mut_release(m_mut_dataflash_mutex);         //操作后释放互斥量

	return status;
}
