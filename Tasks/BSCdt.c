/************************************************************
Copyright (C), ����Ӣ����Ƽ��������޹�˾, ����һ��Ȩ��
�� �� ����BSCdt.c
��    ����1.00
�������ڣ�2012-02-29
��    �ߣ�������
����������CDT��Լʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-02-29  1.00     ����
***********************************************************/


#include <string.h>
#include "BSCdt.h"
#include "Backstage.h"

#include "../Drivers/uart_device.h"
#include "../Drivers/Delay.h"


//#define DEBUG

#ifdef DEBUG
	#define DPRINT(fmt, args...) u32_usb_debug_print(fmt, ##args)
#else
	#define DPRINT(fmt, args...)
#endif


#define CDT_YK_BYTE_SIZE     30

#define	CDT_DEST        0x01
#define CDT_CTRL        0x71
#define CDT_A_FRAME	    0x61
#define CDT_B_FRAME	    0xC2
#define CDT_C_FRAME	    0xB3
#define CDT_YX_FRAME	0xF4

#define CDT_YK_FUNC_SELECT   0xE0
#define CDT_YK_FUNC_RETRUN   0xE1
#define CDT_YK_FUNC_EXECUTE  0xE2
#define CDT_YK_FUNC_CANCEL   0xE3

#define CDT_YK_OP_SWITCH_ON  0xCC
#define CDT_YK_OP_SWITCH_OFF 0x33
#define CDT_YK_OP_ERROR      0xFF
#define CDT_YK_OP_EXECUTE    0xAA
#define CDT_YK_OP_CANCEL     0x55

#define CDT_BAT_CTRL_WAY     0x01
#define CDT_BAT_CTRL         0x02


#define BS_CDT_MAJOR_YC_MAX_SIZE          86     //��Ҫң�����ļĴ�������
#define BS_CDT_MINOR_YC_MAX_SIZE          86     //��Ҫң�����ļĴ�������
#define BS_CDT_GENERAL_YC_MAX_SIZE        84     //һ��ң�����ļĴ�������
#define BS_CDT_YX_MAX_SIZE                32     //ң�����ļĴ�������

#define BS_CDT_MAJOR_YC_FUNC_START_CODE   0      //��Ҫң�⹦������ʼֵ
#define BS_CDT_MINOR_YC_FUNC_START_CODE   43     //��Ҫң�⹦������ʼֵ
#define BS_CDT_GENERAL_YC_FUNC_START_CODE 86     //һ��ң�⹦������ʼֵ
#define BS_CDT_YX_FUNC_START_CODE         0xF0   //ң�Ź�������ʼֵ


typedef enum
{
	CDT_YC_MAJOR,
	CDT_YC_MINOR,
	CDT_YC_GENERAL,
	CDT_YX,
	CDT_YK
}CDT_STATUS_E;

typedef struct
{
	U8_T func_code;
	U8_T operation;
	U8_T ordinal;
}CDT_CMD_T;

static CDT_STATUS_E e_cdt_status = CDT_YC_MAJOR;

static U8_T u8_cdt_crc(U8_T *buf, U32_T len)
{
	U8_T q, r;
	U8_T crc;

	crc = 0;
	q = 0;
	r = 0;

	for(q=0; q<len; q++)
	{
		crc ^= buf[q];

		for(r=0; r<8; r++)
		{
			if( crc & 0x80 )
			{
				crc <<= 1;
				crc ^= 0x07;
			}
			else
				crc <<= 1;
		}
	}
	crc = (~crc);
	return crc;
}

static int cdt_unpacket(U8_T addr, CDT_CMD_T *command)
{
	U32_T n, h, num, valid;
	S32_T ret = -1;
	U32_T i;

	n = g_u8_bs_recv_len;
	h = 0;

	/*CDT��Լ������֡������12���ֽ�,����ͷβ��־�Ա�ȡ��һ���������ݰ�*/
	while (n > 12) {
		/*�����ݰ��в��Ұ�ͷ*/
		if(!((g_u8_bs_recv_buf[h]== 0xEB) 	/*�����ֲ����ܵ���EBH*/
		&& (g_u8_bs_recv_buf[h+1] == 0x90)	/*��ֹ���� EB 90 EB 90 EB 90*/
		&& (g_u8_bs_recv_buf[h+2] == 0xEB) 	/*EB 90 EB 90 EB 90�����*/
		&& (g_u8_bs_recv_buf[h+3] == 0x90)
		&& (g_u8_bs_recv_buf[h+4] == 0xEB) 
		&& (g_u8_bs_recv_buf[h+5] == 0x90)
		&& (g_u8_bs_recv_buf[h+6] != 0xEB))) {
			h++;
			n--;
			continue;
		}

		/*�趨����*/
		if((g_u8_bs_recv_buf[h+6] != 0x71) || (g_u8_bs_recv_buf[h+8] != 3)) {          /*�����趨���֡��Ч*/
			h += 7;
			n -= 7;
			continue;
		}
		
		/*�ӹ�����������ȡ�ñ�����ַ*/
		if(addr != g_u8_bs_recv_buf[h+10]) {        /* ��ַ������֡��Ч */
			h += 7;
			n -= 7;
			continue;
		}
		
		num = g_u8_bs_recv_buf[h+8];/* ����֡��3����Ϣ�� */
		/*���յĳ���С������֡����*/
		if((num*6+12) >  n)
			break;
		
		valid = 1;
		
		for(i=0; i<(num+1); i++) {
			if(g_u8_bs_recv_buf[h+11+i*6] != u8_cdt_crc(&g_u8_bs_recv_buf[h+6+i*6], 5)) /*У���֡��Ч*/
				valid = 0;
		}

		/*3����Ϣ�ֱ�����ͬ������֡��Ч*/
		for(i=0; i<6; i++) {
			if((g_u8_bs_recv_buf[h+12+i] != g_u8_bs_recv_buf[h+18+i])
			|| (g_u8_bs_recv_buf[h+12+i] != g_u8_bs_recv_buf[h+24+i]))
				valid = 0;
		}
		
		if ((g_u8_bs_recv_buf[h+13]!=g_u8_bs_recv_buf[h+15]) || (g_u8_bs_recv_buf[h+14]!=g_u8_bs_recv_buf[h+16]))
			valid = 0;
		
		if (!( ( (g_u8_bs_recv_buf[h+7]==CDT_A_FRAME) && (g_u8_bs_recv_buf[h+12]==CDT_YK_FUNC_SELECT) ) ||
				( (g_u8_bs_recv_buf[h+7]==CDT_B_FRAME) && (g_u8_bs_recv_buf[h+12]==CDT_YK_FUNC_EXECUTE) ) ||
				( (g_u8_bs_recv_buf[h+7]==CDT_C_FRAME) && (g_u8_bs_recv_buf[h+12]==CDT_YK_FUNC_CANCEL) ) ) ) {
			valid = 0;
		}

		if(valid == 0) {/* ֡��Ч��������֡ */
			h += 7;
			n -= 7;
			continue;
		}
		
		command->func_code = g_u8_bs_recv_buf[h+12];
		command->operation = g_u8_bs_recv_buf[h+13];
		command->ordinal = g_u8_bs_recv_buf[h+14];
		DPRINT("cdt, func_code=%x, operation=%x, ordinal=%x\n", command->func_code, command->operation, command->ordinal);
		h += CDT_YK_BYTE_SIZE;
		n -= CDT_YK_BYTE_SIZE;
		ret = 0;
		break;
	}
	
	if (h > 0)
	{
		for (i=0; i<n; i++)
		{
			g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
		}
		g_u8_bs_recv_len = n;
	}
	
	return ret;
}

static void cdt_yk_return(const CDT_CMD_T *command)
{
	U32_T i;

	memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));

	/* ���ͬ���� */
	g_u8_bs_send_buf[0] = 0xEB;
	g_u8_bs_send_buf[1] = 0x90;
	g_u8_bs_send_buf[2] = 0xEB;
	g_u8_bs_send_buf[3] = 0x90;
	g_u8_bs_send_buf[4] = 0xEB;
	g_u8_bs_send_buf[5] = 0x90;

	/*�����ִ��*/
	g_u8_bs_send_buf[6] = CDT_CTRL;	           /*�����ֽ�*/
	g_u8_bs_send_buf[7] = CDT_A_FRAME;	       /*֡���*/
	g_u8_bs_send_buf[8] = 3;		               /*���͵���Ϣ�ָ�������6Ϊ������*/
	g_u8_bs_send_buf[9] = g_u8_addr;	
	g_u8_bs_send_buf[10] = CDT_DEST;	           /*Ŀ�ĵ�ַ*/
	g_u8_bs_send_buf[11] = u8_cdt_crc(&g_u8_bs_send_buf[6], 5); /*CRCУ��*/

	for (i=0; i<3; i++) {
		g_u8_bs_send_buf[12+i*6] = CDT_YK_FUNC_RETRUN;
		g_u8_bs_send_buf[13+i*6] = command->operation;
		g_u8_bs_send_buf[14+i*6] = command->ordinal;
		g_u8_bs_send_buf[15+i*6] = command->operation;
		g_u8_bs_send_buf[16+i*6] = command->ordinal;
		g_u8_bs_send_buf[17+i*6] = u8_cdt_crc(&g_u8_bs_send_buf[12+i*6], 5);
	}

	v_bs_send_data(g_u8_bs_send_buf, CDT_YK_BYTE_SIZE);
}

#define TIMEOUT_TICK 5   //�����ڳ�ʱֵ

static void v_cdt_recv(void)
{
	S32_T cnt = 0;
	U32_T time, time_cnt, time_out;
	CDT_CMD_T comm_select, comm_buf;

	time = u32_bs_calculate_time(CDT_YK_BYTE_SIZE, g_e_baud)+850;     //��ֹ��������,������ʱ�ȴ�850ms��ʱ��

	while (1) {
		time_out = time / (TIMEOUT_TICK * 10);
		time_cnt = 0;

		while(time_cnt < time_out)
		{
			os_evt_set(BACKSTAGE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			
			os_dly_wait(TIMEOUT_TICK);
			time_cnt++;
			
			cnt = Uart_recv(BS_COM_PORT_NUM, g_u8_bs_recv_buf+g_u8_bs_recv_len, CDT_YK_BYTE_SIZE);
			if (cnt < 0)
				continue;
			
			g_u8_bs_recv_len += cnt;
			
			if (cdt_unpacket(g_u8_addr, &comm_buf) == 0)
			{
				if (comm_buf.func_code == CDT_YK_FUNC_SELECT)
				{
					comm_select = comm_buf;
					e_cdt_status = CDT_YK;
					cdt_yk_return(&comm_select);
					time = 30 * 1000;
					break;
				}
				else if (comm_buf.func_code == CDT_YK_FUNC_EXECUTE)
				{
					if ((e_cdt_status==CDT_YK) && (comm_select.ordinal==comm_buf.ordinal) && (comm_select.ordinal<=g_u16_bs_yk_size))
					{
						DPRINT("cdt execute, ordianl=%x, operation=%x\n", comm_select.ordinal, comm_select.operation);
						//ִ�ж���
						if(comm_select.operation == 0x33)
							s32_bs_yk_handle(comm_select.ordinal-1, 1);   //��Ŵ�1��ʼ
						else if(comm_select.operation == 0xCC)
							s32_bs_yk_handle(comm_select.ordinal-1, 0);
					}
					
					goto out;
				}
				else {
					DPRINT("cdt cancel, ordianl=%x, operation=%x\n", comm_select.ordinal, comm_select.operation);
					goto out;
				}
			}
		}
		
		if (time_cnt >= time_out)
			break;
	}

out:
	if (e_cdt_status == CDT_YK)
		e_cdt_status = CDT_YC_MAJOR;
}

static void v_cdt_send(void)
{
	U16_T  *pu16_buf;
	U16_T u16_size;
	U8_T  n,function_code = 0;
	U32_T i, j;
	
	memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));

	switch (e_cdt_status) {
		case CDT_YC_MAJOR:
			g_u8_bs_send_buf[7] = CDT_A_FRAME;          /*֡���*/
			function_code = BS_CDT_MAJOR_YC_FUNC_START_CODE;
			pu16_buf = g_u16_bs_cdt_yc;
			
			if (g_u16_bs_cdt_yc_size > BS_CDT_MAJOR_YC_MAX_SIZE)
			{
				e_cdt_status = CDT_YC_MINOR;           //ң��������������Ҫң�������������л�����Ҫң��״̬
				u16_size = BS_CDT_MAJOR_YC_MAX_SIZE;
			}	
			else
			{
				e_cdt_status = CDT_YX;                //ң����������������Ҫң�������������л���ң��״̬
				u16_size = g_u16_bs_cdt_yc_size;
			}
			
			break;
			
		case CDT_YC_MINOR:
			g_u8_bs_send_buf[7] = CDT_B_FRAME;          /*֡���*/
			function_code = BS_CDT_MINOR_YC_FUNC_START_CODE;
			pu16_buf = g_u16_bs_cdt_yc + BS_CDT_MAJOR_YC_MAX_SIZE;
			
			if (g_u16_bs_cdt_yc_size > (BS_CDT_MAJOR_YC_MAX_SIZE + BS_CDT_MINOR_YC_MAX_SIZE))
			{
				e_cdt_status = CDT_YC_GENERAL;         //ң��������������Ҫң�⡢��Ҫң����������֮�ͣ��л���һ��ң��״̬
				u16_size = BS_CDT_MINOR_YC_MAX_SIZE;
			}	
			else
			{
				e_cdt_status = CDT_YX;                 //ң��������������Ҫң�⡢��Ҫң����������֮�ͣ��л���ң��״̬ 
				u16_size = g_u16_bs_cdt_yc_size - BS_CDT_MAJOR_YC_MAX_SIZE;
			}
			
			break;
			
		case CDT_YC_GENERAL:
			g_u8_bs_send_buf[7] = CDT_C_FRAME;         /*֡���*/
			function_code = BS_CDT_GENERAL_YC_FUNC_START_CODE;
			pu16_buf = g_u16_bs_cdt_yc + BS_CDT_MAJOR_YC_MAX_SIZE + BS_CDT_MINOR_YC_MAX_SIZE;
			e_cdt_status = CDT_YX;
			
			if (g_u16_bs_cdt_yc_size > (BS_CDT_MAJOR_YC_MAX_SIZE + BS_CDT_MINOR_YC_MAX_SIZE + BS_CDT_GENERAL_YC_MAX_SIZE))
				u16_size = BS_CDT_GENERAL_YC_MAX_SIZE;	
			else
				u16_size = g_u16_bs_cdt_yc_size - BS_CDT_MAJOR_YC_MAX_SIZE - BS_CDT_MINOR_YC_MAX_SIZE;
			
			break;
			
		case CDT_YX:
			g_u8_bs_send_buf[7] = CDT_YX_FRAME;        /*֡���*/
			function_code = BS_CDT_YX_FUNC_START_CODE;
			pu16_buf = g_u16_bs_yx;
			e_cdt_status = CDT_YC_MAJOR;
			
			if (g_u16_bs_yx_size > BS_CDT_YX_MAX_SIZE)
				u16_size = BS_CDT_YX_MAX_SIZE;
			else
				u16_size = g_u16_bs_yx_size;
			
			break;
			
		default:
			return;
	}
	
	/* ���ͬ���� */
	g_u8_bs_send_buf[0] = 0xEB;
	g_u8_bs_send_buf[1] = 0x90;
	g_u8_bs_send_buf[2] = 0xEB;
	g_u8_bs_send_buf[3] = 0x90;
	g_u8_bs_send_buf[4] = 0xEB;
	g_u8_bs_send_buf[5] = 0x90;
	
	n = (u16_size + 1) / 2;
	if (n > ((BS_SEND_BUF_SIZE-12)/6))
		n = (BS_SEND_BUF_SIZE-12)/6;

	/*�����ִ��*/
	g_u8_bs_send_buf[6] = CDT_CTRL;	           /*�����ֽ�*/
	g_u8_bs_send_buf[8] = n;		               /*���͵���Ϣ�ָ�������6Ϊ������*/
	g_u8_bs_send_buf[9] = g_u8_addr;	
	g_u8_bs_send_buf[10] = CDT_DEST;	           /*Ŀ�ĵ�ַ*/
	g_u8_bs_send_buf[11] = u8_cdt_crc(&g_u8_bs_send_buf[6], 5); /*CRCУ��*/

    for (i=0, j=0; i<n; i++, j+=2) {
		g_u8_bs_send_buf[12+i*6] = (function_code + i);
		g_u8_bs_send_buf[13+i*6] = (U8_T)(pu16_buf[j]);
		g_u8_bs_send_buf[14+i*6] = (U8_T)(pu16_buf[j]>>8);
		g_u8_bs_send_buf[15+i*6] = (U8_T)(pu16_buf[j+1]);
		g_u8_bs_send_buf[16+i*6] = (U8_T)(pu16_buf[j+1]>>8);
		g_u8_bs_send_buf[17+i*6] = u8_cdt_crc(&g_u8_bs_send_buf[12+i*6], 5);
	}

	v_bs_send_data(g_u8_bs_send_buf, n*6+12);
}

void v_cdt_run(void)
{
	v_cdt_send();
    os_evt_set(BACKSTAGE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
	v_cdt_recv();
}


