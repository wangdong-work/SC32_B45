/************************************************************
Copyright (C), 深圳英可瑞科技开发有限公司, 保留一切权利
文 件 名：BSModbus.c
版    本：1.00
创建日期：2012-06-14
作    者：郭数理
功能描述：MODBUS规约实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-06-14  1.00     创建
***********************************************************/


#include <string.h>
#include "BSModbus.h"
#include "Backstage.h"
#include "BSInternal.h"
#include "Crc.h"

#include "../Drivers/uart_device.h"


//#define DEBUG

#ifdef DEBUG
	#define DPRINT(fmt, args...) u32_usb_debug_print(fmt, ##__VA_ARGS__)
#else
	#define DPRINT(fmt, args...)
#endif


#define MODBUS_BYTE_SIZE           8
#define MODBUS_MAXREG_NUM          125    /* 一次最多可读取的寄存器个数 */

#define MODBUS_YX_FUNC_CODE        0x02
#define MODBUS_YC_FUNC_CODE        0x04
#define MODBUS_YK_READ_FUNC_CODE   0x03
#define MODBUS_YK_WRITE_FUNC_CODE  0x06

#define MODBUS_YC_START_ADDR         0x0000
#define MODBUS_YC_END_ADDR           0x1FFF
#define MODBUS_YC_FEEDER_START_ADDR  0x6000
#define MODBUS_YC_FEEDER_END_ADDR    0x6FFF

#define MODBUS_YX_START_ADDR         0x0000
#define MODBUS_YX_END_ADDR           0x1FFF
#define MODBUS_YX_FEEDER_START_ADDR  0x8000
#define MODBUS_YX_FEEDER_END_ADDR    0x9FFF

#define MODBUS_YK_REG_ADDR           0x4000
#define MODBUS_YT_REG_ADDR           0x5000
#define MODBUS_REG_ADDR_MASK         0xF000

typedef struct
{
	U8_T  u8_func_code;   //功能码
	U16_T u16_reg_addr;   //寄存器地址
	U16_T u16_reg_cnt;    //寄存器数量
}MODBUS_CMD_T;


/*************************************************************
函数名称: v_modbus_yc_send		           				
函数功能: modubs协议遥测发送数据函数						
输入参数: addr -- 遥测寄存器地址
          cnt  -- 遥测寄存器个数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_modbus_yc_send(U16_T addr, U16_T cnt)
{
	S16_T *ps16_buf;
	U16_T u16_size;
	U16_T index, data;
	U32_T i;
	U8_T  u8_ret_cnt;
	
	if (cnt > MODBUS_MAXREG_NUM)
		cnt = MODBUS_MAXREG_NUM;
	
	if (addr <= MODBUS_YC_END_ADDR)
	{
		ps16_buf = g_s16_bs_modbus_yc;
		u16_size = g_u16_bs_mdobus_yc_size;
		index = addr & (~MODBUS_REG_ADDR_MASK);
	}
	else if ((addr >= MODBUS_YC_FEEDER_START_ADDR) && (addr <= MODBUS_YC_FEEDER_END_ADDR))
	{
		ps16_buf = g_s16_bs_modbus_yc_feeder;
		u16_size = BS_YC_FEEDER_SIZE;
		index = addr - MODBUS_YC_FEEDER_START_ADDR;
	}
	else
	{
		return;
	}
	
	if (index >= u16_size)
		return;

	memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	
	if (u16_size - index < cnt)
		cnt = u16_size - index;
	u8_ret_cnt = (U8_T)(cnt * 2);
	
	g_u8_bs_send_buf[0] = g_u8_addr;
	g_u8_bs_send_buf[1] = MODBUS_YC_FUNC_CODE;
	g_u8_bs_send_buf[2] = u8_ret_cnt;
	
	DPRINT("modbus yc data: ");
	for (i=0; i<u8_ret_cnt; i+=2)
	{
		data = ps16_buf[index+i/2];
		DPRINT("%x ", data);		

		g_u8_bs_send_buf[3+i] = (U8_T)(data >> 8);
		g_u8_bs_send_buf[3+i+1] = (U8_T)data;
	}
	DPRINT("\n");		

	data = u16_crc_calculate_crc(g_u8_bs_send_buf, u8_ret_cnt+3);
	g_u8_bs_send_buf[u8_ret_cnt+3] = (U8_T)data;
	g_u8_bs_send_buf[u8_ret_cnt+4] = (U8_T)(data >> 8);
	
	v_bs_send_data(g_u8_bs_send_buf, u8_ret_cnt+5);
}

/*************************************************************
函数名称: v_modbus_yx_send		           				
函数功能: modubs协议遥信发送数据函数						
输入参数: addr -- 遥信寄存器地址
          cnt  -- 遥信寄存器个数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_modbus_yx_send(U16_T addr, U16_T cnt)
{
	U16_T *pu16_buf;
	U16_T u16_size;
	U32_T word_index;
	U16_T data;
	U32_T i;
	U8_T  bit_index, byte_cnt, rem_bit_cnt;
	U16_T index;
	
	if (addr <= MODBUS_YX_END_ADDR)
	{
		pu16_buf = g_u16_bs_yx;
		u16_size = g_u32_bs_yx_max_addr;
		index = addr & (~MODBUS_REG_ADDR_MASK);
	}
	else if ((addr >= MODBUS_YX_FEEDER_START_ADDR) && (addr <= MODBUS_YX_FEEDER_END_ADDR))
	{
		pu16_buf = g_u16_bs_yx_feeder;
		u16_size = BS_YX_FEEDER_SIZE;
		index = addr - MODBUS_YX_FEEDER_START_ADDR;
	}
	else
	{
		return;
	}

	if (index >= u16_size)
		return;
	
	memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
		
	if (index + cnt > u16_size)
		cnt = u16_size - index;
 	
	word_index = index / 16;
    bit_index  = index % 16;
    byte_cnt    = cnt / 8;
	rem_bit_cnt = cnt % 8;
	
    if (rem_bit_cnt > 0)
		byte_cnt++;
	
	if (byte_cnt > MODBUS_MAXREG_NUM*2)
	{
		byte_cnt = MODBUS_MAXREG_NUM*2;
		cnt = byte_cnt * 8;
		rem_bit_cnt = 0;
	}
	
	g_u8_bs_send_buf[0] = g_u8_addr;
	g_u8_bs_send_buf[1] = MODBUS_YX_FUNC_CODE;
	g_u8_bs_send_buf[2] = byte_cnt;	

	for (i=0; i<byte_cnt; i++)
	{
        data = pu16_buf[word_index];
        g_u8_bs_send_buf[3+i] = (U8_T)(data>>bit_index);

        if (bit_index > 8)  //不够8位数据部分提取
        {
            data = pu16_buf[++word_index];
            g_u8_bs_send_buf[3+i] |= (U8_T)(data<<(16-bit_index));
        }
        else if (bit_index==8)
            ++word_index;

        bit_index = ((bit_index+8)%16);
	}
	
	if (rem_bit_cnt > 0)
		g_u8_bs_send_buf[2+byte_cnt] &= ((1<<rem_bit_cnt) - 1);
		
	data = u16_crc_calculate_crc(g_u8_bs_send_buf, byte_cnt+3);
	g_u8_bs_send_buf[byte_cnt+3] = (U8_T)data;
	g_u8_bs_send_buf[byte_cnt+4] = (U8_T)(data >> 8);
	
	v_bs_send_data(g_u8_bs_send_buf, byte_cnt+5);
}

/*************************************************************
函数名称: v_modbus_yk_yt_read		           				
函数功能: modubs协议遥控、遥调发送读数据函数						
输入参数: addr -- 遥测寄存器地址
          cnt  -- 遥测寄存器个数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_modbus_yk_yt_read(U16_T addr, U16_T cnt)
{
	U16_T index, data;
	U16_T *pu16_buf;
	U16_T u16_size;
	U32_T i;
	U8_T  u8_ret_cnt;
	
	if (cnt > MODBUS_MAXREG_NUM)
		cnt = MODBUS_MAXREG_NUM;
	
	if ((addr & MODBUS_REG_ADDR_MASK) == MODBUS_YK_REG_ADDR)
	{
		pu16_buf = g_u16_bs_yk;
		u16_size = g_u16_bs_yk_size;
		index = addr & (~MODBUS_REG_ADDR_MASK);
	}
	else if ((addr & MODBUS_REG_ADDR_MASK) == MODBUS_YT_REG_ADDR)
	{
		pu16_buf = g_u16_bs_yt;
		u16_size = g_u16_bs_yt_size;
		index = addr & (~MODBUS_REG_ADDR_MASK);
	}
	else
	{
		return;
	}
	
	if (index >= u16_size)
		return;
		
	memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	
	if (u16_size - index < cnt)
		cnt = u16_size - index;
	u8_ret_cnt = (U8_T)(cnt * 2);
	
	g_u8_bs_send_buf[0] = g_u8_addr;
	g_u8_bs_send_buf[1] = MODBUS_YK_READ_FUNC_CODE;
	g_u8_bs_send_buf[2] = u8_ret_cnt;
	
	DPRINT("modbus yc data: ");
	for (i=0; i<u8_ret_cnt; i+=2)
	{
		data = pu16_buf[index+i/2];
		DPRINT("%x ", data);		

		g_u8_bs_send_buf[3+i] = (U8_T)(data >> 8);
		g_u8_bs_send_buf[3+i+1] = (U8_T)data;
	}
	DPRINT("\n");		

	data = u16_crc_calculate_crc(g_u8_bs_send_buf, u8_ret_cnt+3);
	g_u8_bs_send_buf[u8_ret_cnt+3] = (U8_T)data;
	g_u8_bs_send_buf[u8_ret_cnt+4] = (U8_T)(data >> 8);
	
	v_bs_send_data(g_u8_bs_send_buf, u8_ret_cnt+5);
}

/*************************************************************
函数名称: v_modbus_yk_yt_write		           				
函数功能: modubs协议遥控、遥调写返回函数						
输入参数: addr -- 遥控寄存器地址
          cnt  -- 遥控寄存器数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_modbus_yk_yt_write(U16_T addr, U16_T value)
{
	U16_T data;
	U16_T index;

	if ((addr & MODBUS_REG_ADDR_MASK) == MODBUS_YK_REG_ADDR)
	{
		index = addr & (~MODBUS_REG_ADDR_MASK);
		if (index >= g_u16_bs_yk_size)
			return;
		
		if (value == 0x0000)
		{	
			s32_bs_yk_handle(index, 0);
		}
		else if (value == 0xFF00)
		{
			s32_bs_yk_handle(index, 1);
		}
		else
		{
			return;
		}
		
	}
	else if ((addr & MODBUS_REG_ADDR_MASK) == MODBUS_YT_REG_ADDR)
	{
		index = addr & (~MODBUS_REG_ADDR_MASK);
		if (index >= g_u16_bs_yt_size)
			return;
			
		if (s32_bs_yt_handle(index, value) != 0)
			return;
	}
	else
	{
		return;
	}

	memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	
	g_u8_bs_send_buf[0] = g_u8_addr;
	g_u8_bs_send_buf[1] = MODBUS_YK_WRITE_FUNC_CODE;
	g_u8_bs_send_buf[2] = (U8_T)(addr>>8);
	g_u8_bs_send_buf[3] = (U8_T)addr;
	g_u8_bs_send_buf[4] = (U8_T)(value>>8);
	g_u8_bs_send_buf[5] = (U8_T)value;
	
	data = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
    g_u8_bs_send_buf[6] = (U8_T)data;
	g_u8_bs_send_buf[7] = (U8_T)(data >> 8);
	
	v_bs_send_data(g_u8_bs_send_buf, 8);
}

/*************************************************************
函数名称: s32_modbus_unpacket		           				
函数功能: modubs协议解包函数						
输入参数: addr -- 本机地址     		   				
输出参数: cmd  -- 返回解包的结果
返回值  ：返回0表示解包成功，返回-1表示解包失败														   				
**************************************************************/
static S32_T s32_modbus_unpacket(U8_T addr, MODBUS_CMD_T *cmd)
{
	U32_T i, n, h, plen;
	S32_T ret = -1;

	n = g_u8_bs_recv_len;
	h = 0;
	
	/*查找头尾标志以便取得一完整的数据包*/
	while (n >= MODBUS_BYTE_SIZE)
	{
		/*地址不相符*/
		if ((g_u8_bs_recv_buf[h] != addr) &&
			(g_u8_bs_recv_buf[h] != (DC10_START_ADDR + g_u8_addr - 1)))		/*地址不相符，头指针前移一个位置*/
		{
			h++;
			n--;
			continue;
		}

		/*功能码不相符，头指针前移一个位置*/
		if ((g_u8_bs_recv_buf[h+1] != MODBUS_YX_FUNC_CODE) &&
		    (g_u8_bs_recv_buf[h+1] != MODBUS_YC_FUNC_CODE) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_YK_READ_FUNC_CODE) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_YK_WRITE_FUNC_CODE) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_16))
		{
			h++;
			n--;
			continue;
		}		
	
		//比较校验码
		if (g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_16)
		{
			/*比较校验码*/
			if (u16_crc_calculate_crc(&g_u8_bs_recv_buf[h], 8) != 0)
			{
				h++;
				n--;
				continue;
			}
			plen = 8;
		}
		else
		{
			//只有DC10有16功能码，16功能码需要加一些限制条件，地址高位为0x10、字节数等于寄存器个数乘以2、字节数小于86
			if (((g_u8_bs_recv_buf[h+2] != 0x10) && (g_u8_bs_recv_buf[h+2] != 0x11)) ||
				(((g_u8_bs_recv_buf[h+4]<<8)+g_u8_bs_recv_buf[h+5])*2 != g_u8_bs_recv_buf[h+6]) ||
				(g_u8_bs_recv_buf[h+6] > 86))
			{
				h++;
				n--;
				continue;
			}
				
			if ((g_u8_bs_recv_buf[h+6]+9) > n)
			{ //数据长度不够
				if (h > 0)
				{
					for (i=0; i<n; i++)
					{
						g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
					}
				}					
				return ret;
			}
			else 
			{
				if (u16_crc_calculate_crc(&g_u8_bs_recv_buf[h], g_u8_bs_recv_buf[h+6]+9) != 0)
				{
					h++;
					n--;
					continue;
				}
				plen = g_u8_bs_recv_buf[h+6] + 9;
			}
		}

		//到此收到合法数据
		if(g_u8_bs_recv_buf[h] == addr)
		{	//SC32后台通信		
			cmd->u8_func_code = g_u8_bs_recv_buf[h+1];
			cmd->u16_reg_addr = (g_u8_bs_recv_buf[h+2]<<8) + g_u8_bs_recv_buf[h+3];
			cmd->u16_reg_cnt = (g_u8_bs_recv_buf[h+4]<<8) + g_u8_bs_recv_buf[h+5];
			DPRINT("modbus, u8_func_code=%x, u16_reg_addr=%x, cnt=%x\n", cmd->u8_func_code, cmd->u16_reg_addr, cmd->u16_reg_cnt);
			
			switch(cmd->u8_func_code)
			{
				case MODBUS_YX_FUNC_CODE:
					DPRINT("modbus yx\n");
					v_modbus_yx_send(cmd->u16_reg_addr, cmd->u16_reg_cnt);
					break;
					
				case MODBUS_YC_FUNC_CODE:
					DPRINT("modbus yc\n");
					v_modbus_yc_send(cmd->u16_reg_addr, cmd->u16_reg_cnt);
					break;
						
				case MODBUS_YK_READ_FUNC_CODE:
					DPRINT("modbus yk yt read\n");
					v_modbus_yk_yt_read(cmd->u16_reg_addr, cmd->u16_reg_cnt);
					break;
						
				case MODBUS_YK_WRITE_FUNC_CODE:
					DPRINT("modbus yk yt write\n");
					v_modbus_yk_yt_write(cmd->u16_reg_addr, cmd->u16_reg_cnt);
					break;
						
				default:
					break;
			}
		}
		else
		{	//DC10通信协议处理
			cmd->u8_func_code = g_u8_bs_recv_buf[h+1];
			cmd->u16_reg_addr = (g_u8_bs_recv_buf[h+2]<<8) + g_u8_bs_recv_buf[h+3];
			cmd->u16_reg_cnt = (g_u8_bs_recv_buf[h+4]<<8) + g_u8_bs_recv_buf[h+5];			
			DPRINT("modbus, u8_func_code=%x, u16_reg_addr=%x, cnt=%x\n", cmd->u8_func_code, cmd->u16_reg_addr, cmd->u16_reg_cnt);
			
			if (h > 0)
			{
				for (i=0; i< n; i++)
				{
					g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
				}
				h = 0;
			}	
			v_internal_dc10_handle(cmd->u8_func_code, cmd->u16_reg_addr, cmd->u16_reg_cnt);
		}
		
		h += plen;
		n -= plen;
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

/*************************************************************
函数名称: v_modbus_run		           				
函数功能: modubs协议运行函数						
输入参数: 无     		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_modbus_run(void)
{
	S32_T cnt = 0;
	MODBUS_CMD_T command;
	
	cnt = Uart_recv(BS_COM_PORT_NUM, g_u8_bs_recv_buf+g_u8_bs_recv_len, BS_RECV_BUF_SIZE-g_u8_bs_recv_len);
	if (cnt == -1)
		return;
			
	g_u8_bs_recv_len += cnt;
			
	if (s32_modbus_unpacket(g_u8_addr, &command) == 0)
	{		
		memset(g_u8_bs_recv_buf, 0, sizeof(g_u8_bs_recv_buf));      //成功处理命令后,清除接收缓冲区
		g_u8_bs_recv_len = 0;
	}
}

