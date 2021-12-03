/************************************************************
Copyright (C), 深圳英可瑞科技开发有限公司, 保留一切权利
文 件 名：BSCdt.c
版    本：1.00
创建日期：2012-02-29
作    者：郭数理
功能描述：CDT规约实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-02-29  1.00     创建
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


#define BS_CDT_MAJOR_YC_MAX_SIZE          86     //重要遥测最大的寄存器数量
#define BS_CDT_MINOR_YC_MAX_SIZE          86     //次要遥测最大的寄存器数量
#define BS_CDT_GENERAL_YC_MAX_SIZE        84     //一般遥测最大的寄存器数量
#define BS_CDT_YX_MAX_SIZE                32     //遥信最大的寄存器数量

#define BS_CDT_MAJOR_YC_FUNC_START_CODE   0      //重要遥测功能码起始值
#define BS_CDT_MINOR_YC_FUNC_START_CODE   43     //次要遥测功能码起始值
#define BS_CDT_GENERAL_YC_FUNC_START_CODE 86     //一般遥测功能码起始值
#define BS_CDT_YX_FUNC_START_CODE         0xF0   //遥信功能码起始值


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

	/*CDT规约的数据帧至少有12个字节,查找头尾标志以便取得一完整的数据包*/
	while (n > 12) {
		/*在数据包中查找包头*/
		if(!((g_u8_bs_recv_buf[h]== 0xEB) 	/*控制字不可能等于EBH*/
		&& (g_u8_bs_recv_buf[h+1] == 0x90)	/*防止出现 EB 90 EB 90 EB 90*/
		&& (g_u8_bs_recv_buf[h+2] == 0xEB) 	/*EB 90 EB 90 EB 90的情况*/
		&& (g_u8_bs_recv_buf[h+3] == 0x90)
		&& (g_u8_bs_recv_buf[h+4] == 0xEB) 
		&& (g_u8_bs_recv_buf[h+5] == 0x90)
		&& (g_u8_bs_recv_buf[h+6] != 0xEB))) {
			h++;
			n--;
			continue;
		}

		/*设定命令*/
		if((g_u8_bs_recv_buf[h+6] != 0x71) || (g_u8_bs_recv_buf[h+8] != 3)) {          /*不是设定命令，帧无效*/
			h += 7;
			n -= 7;
			continue;
		}
		
		/*从公共数据区中取得本机地址*/
		if(addr != g_u8_bs_recv_buf[h+10]) {        /* 地址不符，帧无效 */
			h += 7;
			n -= 7;
			continue;
		}
		
		num = g_u8_bs_recv_buf[h+8];/* 控制帧，3个信息字 */
		/*接收的长度小于数据帧长度*/
		if((num*6+12) >  n)
			break;
		
		valid = 1;
		
		for(i=0; i<(num+1); i++) {
			if(g_u8_bs_recv_buf[h+11+i*6] != u8_cdt_crc(&g_u8_bs_recv_buf[h+6+i*6], 5)) /*校验错，帧无效*/
				valid = 0;
		}

		/*3个信息字必须相同，否则帧无效*/
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

		if(valid == 0) {/* 帧无效，丢弃该帧 */
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

	/* 打包同步码 */
	g_u8_bs_send_buf[0] = 0xEB;
	g_u8_bs_send_buf[1] = 0x90;
	g_u8_bs_send_buf[2] = 0xEB;
	g_u8_bs_send_buf[3] = 0x90;
	g_u8_bs_send_buf[4] = 0xEB;
	g_u8_bs_send_buf[5] = 0x90;

	/*控制字打包*/
	g_u8_bs_send_buf[6] = CDT_CTRL;	           /*控制字节*/
	g_u8_bs_send_buf[7] = CDT_A_FRAME;	       /*帧类别*/
	g_u8_bs_send_buf[8] = 3;		               /*发送的信息字个数（以6为基数）*/
	g_u8_bs_send_buf[9] = g_u8_addr;	
	g_u8_bs_send_buf[10] = CDT_DEST;	           /*目的地址*/
	g_u8_bs_send_buf[11] = u8_cdt_crc(&g_u8_bs_send_buf[6], 5); /*CRC校验*/

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

#define TIMEOUT_TICK 5   //读串口超时值

static void v_cdt_recv(void)
{
	S32_T cnt = 0;
	U32_T time, time_cnt, time_out;
	CDT_CMD_T comm_select, comm_buf;

	time = u32_bs_calculate_time(CDT_YK_BYTE_SIZE, g_e_baud)+850;     //防止发包过快,接收延时等待850ms的时间

	while (1) {
		time_out = time / (TIMEOUT_TICK * 10);
		time_cnt = 0;

		while(time_cnt < time_out)
		{
			os_evt_set(BACKSTAGE_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
			
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
						//执行动作
						if(comm_select.operation == 0x33)
							s32_bs_yk_handle(comm_select.ordinal-1, 1);   //序号从1开始
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
			g_u8_bs_send_buf[7] = CDT_A_FRAME;          /*帧类别*/
			function_code = BS_CDT_MAJOR_YC_FUNC_START_CODE;
			pu16_buf = g_u16_bs_cdt_yc;
			
			if (g_u16_bs_cdt_yc_size > BS_CDT_MAJOR_YC_MAX_SIZE)
			{
				e_cdt_status = CDT_YC_MINOR;           //遥测总数量大于重要遥测的最大数量，切换到次要遥测状态
				u16_size = BS_CDT_MAJOR_YC_MAX_SIZE;
			}	
			else
			{
				e_cdt_status = CDT_YX;                //遥测总数量不大于重要遥测的最大数量，切换到遥信状态
				u16_size = g_u16_bs_cdt_yc_size;
			}
			
			break;
			
		case CDT_YC_MINOR:
			g_u8_bs_send_buf[7] = CDT_B_FRAME;          /*帧类别*/
			function_code = BS_CDT_MINOR_YC_FUNC_START_CODE;
			pu16_buf = g_u16_bs_cdt_yc + BS_CDT_MAJOR_YC_MAX_SIZE;
			
			if (g_u16_bs_cdt_yc_size > (BS_CDT_MAJOR_YC_MAX_SIZE + BS_CDT_MINOR_YC_MAX_SIZE))
			{
				e_cdt_status = CDT_YC_GENERAL;         //遥测总数量大于重要遥测、次要遥测的最大数量之和，切换到一般遥测状态
				u16_size = BS_CDT_MINOR_YC_MAX_SIZE;
			}	
			else
			{
				e_cdt_status = CDT_YX;                 //遥测总数量大于重要遥测、次要遥测的最大数量之和，切换到遥信状态 
				u16_size = g_u16_bs_cdt_yc_size - BS_CDT_MAJOR_YC_MAX_SIZE;
			}
			
			break;
			
		case CDT_YC_GENERAL:
			g_u8_bs_send_buf[7] = CDT_C_FRAME;         /*帧类别*/
			function_code = BS_CDT_GENERAL_YC_FUNC_START_CODE;
			pu16_buf = g_u16_bs_cdt_yc + BS_CDT_MAJOR_YC_MAX_SIZE + BS_CDT_MINOR_YC_MAX_SIZE;
			e_cdt_status = CDT_YX;
			
			if (g_u16_bs_cdt_yc_size > (BS_CDT_MAJOR_YC_MAX_SIZE + BS_CDT_MINOR_YC_MAX_SIZE + BS_CDT_GENERAL_YC_MAX_SIZE))
				u16_size = BS_CDT_GENERAL_YC_MAX_SIZE;	
			else
				u16_size = g_u16_bs_cdt_yc_size - BS_CDT_MAJOR_YC_MAX_SIZE - BS_CDT_MINOR_YC_MAX_SIZE;
			
			break;
			
		case CDT_YX:
			g_u8_bs_send_buf[7] = CDT_YX_FRAME;        /*帧类别*/
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
	
	/* 打包同步码 */
	g_u8_bs_send_buf[0] = 0xEB;
	g_u8_bs_send_buf[1] = 0x90;
	g_u8_bs_send_buf[2] = 0xEB;
	g_u8_bs_send_buf[3] = 0x90;
	g_u8_bs_send_buf[4] = 0xEB;
	g_u8_bs_send_buf[5] = 0x90;
	
	n = (u16_size + 1) / 2;
	if (n > ((BS_SEND_BUF_SIZE-12)/6))
		n = (BS_SEND_BUF_SIZE-12)/6;

	/*控制字打包*/
	g_u8_bs_send_buf[6] = CDT_CTRL;	           /*控制字节*/
	g_u8_bs_send_buf[8] = n;		               /*发送的信息字个数（以6为基数）*/
	g_u8_bs_send_buf[9] = g_u8_addr;	
	g_u8_bs_send_buf[10] = CDT_DEST;	           /*目的地址*/
	g_u8_bs_send_buf[11] = u8_cdt_crc(&g_u8_bs_send_buf[6], 5); /*CRC校验*/

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
    os_evt_set(BACKSTAGE_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
	v_cdt_recv();
}


