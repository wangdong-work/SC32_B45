/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Dataflash.c
��    ����1.00
�������ڣ�2012-03-12
��    �ߣ�������
����������AT45DB08����ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-12  1.00     ����
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
��������: v_flash_ssp_init		           				
��������: ������dataflash��ص�spi��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_flash_ssp_init(void)
{
	U32_T reg;

	reg = LPC_GPIO1->FIODIR;  // SSEL0ÿ����һ�����ݶ�������Ƭѡ��������dataflash��ʱ��Ҫ�����԰�Ƭѡ����ΪGPIO������������Ƭѡ���ŵĵ�ƽ
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
��������: v_flash_ssp_write		           				
��������: ͨ��spi�ڷ�������						
�������: pu8_tx     -- ָ��Ҫ���͵�������
          u32_tx_len --	Ҫ�������ݵĳ���      		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_flash_ssp_write(const U8_T *pu8_tx, U32_T u32_tx_len)
{
	U32_T i;
	U8_T buf;

	SSEL_LOW();

	for (i=0; i<u32_tx_len; )                            //�������е����ݣ��������յ������ݣ��������������
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx++;
			i++;
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)             //�ȴ����ݷ������
	{
		if (i++ > 100000)                                //�������ĳ�ִ��󣬲��������˳�����ʱ1s�˳�
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)             //��ս���FIFO���������յ�������
	{
		buf = LPC_SSP0->DR;
		if (++i > SSP_FIFO_SIZE+1)                       //���ֻ��SSP_FIFO_SIZE������
		{
			break;
		}
	}
	LPC_SSP0->ICR = 0x00000003;                          //�����ص��жϱ�־����Ϊû�����жϣ����Դ������Ա�����
	buf++;                                               //������䣬��������ĩ�þ���

	SSEL_HIGH();
}

/*************************************************************
��������: v_flash_ssp_write_two_blocks		           				
��������: ͨ��spi�ڷ�����������						
�������: pu8_tx1     -- ָ��Ҫ���͵ĵ�һ��������
          u32_tx1_len -- ��һ�����ݵĳ���
		  pu8_tx2     -- ָ��Ҫ���͵ĵڶ���������
          u32_tx2_len -- �ڶ������ݵĳ���      		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_flash_ssp_write_two_blocks(const U8_T *pu8_tx1, U32_T u32_tx1_len, const U8_T *pu8_tx2, U32_T u32_tx2_len)
{
	U32_T i;
	U32_T buf = 0;

	SSEL_LOW();

	for (i=0; i<u32_tx1_len; )                               //����tx1�����ݣ��������յ������ݣ��������������
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx1++;
			i++;
		}
	}

	for (i=0; i<u32_tx2_len; )                               //����tx2�����ݣ��������յ������ݣ��������������
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx2++;
			i++;
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)             //�ȴ����ݷ������
	{
		if (i++ > 100000)                                //�������ĳ�ִ��󣬲��������˳�����ʱ1s�˳�
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)             //��ս���FIFO���������յ�������
	{
		buf = LPC_SSP0->DR;
		if (++i > SSP_FIFO_SIZE+1)                       //���ֻ��SSP_FIFO_SIZE������
		{
			break;
		}
	}
	LPC_SSP0->ICR = 0x00000003;		                     //�����ص��жϱ�־����Ϊû�����жϣ����Դ������Ա�����
	buf++;												 //������䣬��������ĩ�þ���

	SSEL_HIGH();
}

/*************************************************************
��������: v_flash_ssp_write_then_read		           				
��������: ͨ��spi�ڷ�����������						
�������: pu8_tx     -- ָ��Ҫ���͵�������
          u32_tx_len --	Ҫ�������ݵĳ���
		  u32_rx_len -- �������ݻ������ĳ���
�������: pu8_rx     -- ָ����ջ����������ڱ�����յ������ݣ���������Ϊ���������u32_rx_len���ֽڵĿռ�
����ֵ  ����														   				
**************************************************************/
static void v_flash_ssp_write_then_read(const U8_T *pu8_tx, U32_T u32_tx_len, U8_T *pu8_rx, U32_T u32_rx_len)
{
	U8_T *buf = pu8_rx;
	U32_T i, cnt;
	U32_T remaining = u32_rx_len;

	SSEL_LOW();

	for (i=0; i<u32_tx_len; )                                //�������е����ݣ��������յ������ݣ��������������
	{
		if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
		{
			LPC_SSP0->DR = *pu8_tx++;
			i++;
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)              //�ȴ����ݷ������
	{
		if (i++ > 100000)                                 //�������ĳ�ִ��󣬲��������˳�����ʱ1s�˳�
		{
			break;
		}
		else
		{
			v_delay_udelay(10);
		}
	}

	i = 0;
	while ((LPC_SSP0->SR & SSP_SR_RNE) != 0)             //��ս���FIFO���������յ�������
	{
		buf[0] = LPC_SSP0->DR;
		if (++i > SSP_FIFO_SIZE+1)                       //���ֻ��SSP_FIFO_SIZE������
		{
			break;
		}
	}
	LPC_SSP0->ICR = 0x00000003;                          //�����ص��жϱ�־����Ϊû�����жϣ����Դ������Ա�����

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

		for (i=0; i<cnt; )                               //����FIFOֻ��SSP_FIFO_SIZE���ֽڣ�����ÿ����෢��SSP_FIFO_SIZE���ֽ�
		{
			if ((LPC_SSP0->SR & SSP_SR_TNF) != 0)
			{
				LPC_SSP0->DR = 0;			             //ֻ��Ϊ�˶�ȡ���ݣ����͵��ֽڿ������������ݣ����﷢��0
				i++;
			}
		}
		
		i = 0;
		while ((LPC_SSP0->SR & SSP_SR_BSY) != 0)         //�ȴ����ݷ������
		{
			if (i++ > 100000)                            //�������ĳ�ִ��󣬲��������˳�����ʱ1s�˳�
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
��������: u8_flash_dataflash_waitready		           				
��������: ��ѯdataflash��״̬���жϲ����Ƿ����						
�������: ��      		   				
�������: ��
����ֵ  ������dataflash��״ֵ̬��������0														   				
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

		if (i++ > 100000)                  //�������ĳ�ִ��󣬲��������˳�����ʱ1s�˳�
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
��������: v_flash_dataflash_init
��������: ��ʼ��dataflash, ������dataflash��ص�spi��
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_flash_dataflash_init(void)
{
	U32_T reg;

	reg = LPC_GPIO2->FIODIR;  //P2.2Ϊ�����ֿ���ͺ���д�Ƿ���������ţ���Ϊ����״̬���ߵ�ƽ������д�ֿ���ͺţ��͵�ƽ��������д�ֿ���ͺ�
	reg &= ~(1 << 2);
	LPC_GPIO2->FIODIR = reg;
	
	v_flash_ssp_init();
}

/*************************************************************
��������: u8_flash_read_font_task_pin
��������: ��ȡ�ֿ���ͺ���д�������ŵ�״̬
�������: ��
�������: ��
����ֵ  ��1��������д��0����������д
**************************************************************/
U8_T u8_flash_read_font_task_pin(void)
{
	if ((LPC_GPIO2->FIOPIN & 0x0004) != 0)
		return 1;
	else
		return 0;
}

/*************************************************************
��������: v_flash_dataflash_init_mutex		           				
��������: ��ʼ��dataflash��صĻ���������RTX����֮����ã��ڵ�һ������Ŀ�ͷ���ֵ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_flash_dataflash_init_mutex(void)
{
	os_mut_init(m_mut_dataflash_mutex);             //��ʼ��������
}

/*************************************************************
��������: s32_flash_dataflash_read		           				
��������: ��ȡdataflash����						
�������: u32_from -- Ҫ��ȡ�ĵ�ַ
		  u32_len  -- Ҫ��ȡ�ĳ���        		   				
�������: pu8_buf  -- ���ض�ȡ�������ݣ���������Ϊ������㹻�Ŀռ䣨���Ȳ�С��len)
����ֵ  ������0��ʾ�ɹ���������-1���ʾ����          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_read(U32_T u32_from, U8_T *pu8_buf, U32_T u32_len)
{
	U32_T addr;
	U8_T tx_buf[4];

	if (u32_len == 0)
		return 0;

	if (u32_from + u32_len > DATAFLASH_SIZE)
		return -1;

	os_mut_wait(m_mut_dataflash_mutex, 0xffff);     //����ǰ��ȡ������

	addr = ((u32_from / PAGE_SIZE) << PAGE_OFFSET) + (u32_from % PAGE_SIZE);

	tx_buf[0] = OP_READ_CONTINUOUS;
	tx_buf[1] = (U8_T)(addr >> 16);
	tx_buf[2] = (U8_T)(addr >> 8);
	tx_buf[3] = (U8_T)addr;

	v_flash_ssp_write_then_read(tx_buf, 4, pu8_buf, u32_len);

	os_mut_release(m_mut_dataflash_mutex);         //�������ͷŻ�����       

	return 0;
}

/*************************************************************
��������: s32_flash_dataflash_write		           				
��������: д���ݵ�dataflash						
�������: u32_to -- Ҫд��ĵ�ַ
		  u32_len  -- Ҫд�����ݵĳ���        		   				
          pu8_buf  -- ָ��Ҫд�������
�������: ��
����ֵ  ������0��ʾ�ɹ���������-1���ʾ����          		   															   				
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

	os_mut_wait(m_mut_dataflash_mutex, 0xffff);     //����ǰ��ȡ������

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

	os_mut_release(m_mut_dataflash_mutex);         //�������ͷŻ�����

	return status;
}
