/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Lcd.c
版    本：1.00
创建日期：2012-03-13
作    者：郭数理
功能描述：LCD驱动实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-13  1.00     创建
**************************************************************/


#include <LPC17xx.h>
#include <string.h>
#include <rtl.h>

#include "Dataflash.h"
#include "Delay.h"
#include "Lcd.h"

#include "Asc6x12.h"



#define LCD_X_SIZE 160
#define LCD_Y_SIZE 160
#define LCD_BUF_SIZE (LCD_X_SIZE*LCD_Y_SIZE/8)

#define LCD_CS_LOW() (LPC_GPIO1->FIOCLR = (1<<25))
#define LCD_CS_HIGH() (LPC_GPIO1->FIOSET = (1<<25))

#define LCD_RD_LOW() (LPC_GPIO1->FIOCLR = (1<<22))
#define LCD_RD_HIGH() (LPC_GPIO1->FIOSET = (1<<22))

#define LCD_WR_LOW() (LPC_GPIO1->FIOCLR = (1<<19))
#define LCD_WR_HIGH() (LPC_GPIO1->FIOSET = (1<<19))

#define LCD_CD_LOW() (LPC_GPIO1->FIOCLR = (1<<18))
#define LCD_CD_HIGH() (LPC_GPIO1->FIOSET = (1<<18))


//由于端口读-修改-写不是原子操作，会引起冲突，所以修改写数据的方式，改用位操作方式
#define LCD_WRITE_DATA(u8_x) {  U32_T  i; \
								for (i=0; i<8; i++) \
								{ \
									if (((u8_x>>i) & 0x01) != 0) \
										LPC_GPIO0->FIOSET = (1<<(21+i)); \
									else \
										LPC_GPIO0->FIOCLR = (1<<(21+i)); \
								} \
							}

typedef enum
{
	COMMAND,  //命令
	DATA,     //数据
}CD_TYPE_E;

const U8_T m_u8_icon_tab[] =                // icon size 12x12
{
	/* ICON CLOCK */
	0x0E, 0x00, 0x31, 0x80, 0x44, 0x40, 0x44, 0x40, 0x84, 0x20, 0xBC, 0x20,
	0x80, 0x20, 0x40, 0x40, 0x40, 0x40, 0x31, 0x80, 0x0E, 0x00, 0x00, 0x00,
	/* ICON SIGNAL */
	0x0E, 0x00, 0x31, 0x80, 0x40, 0x40, 0x5F, 0x40, 0x95, 0x20, 0x8E, 0x20,
	0x84, 0x20, 0x44, 0x40, 0x44, 0x40, 0x31, 0x80, 0x0E, 0x00, 0x00, 0x00,
	/* ICON UP */
	0x00, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x1F, 0x00, 0x1F, 0x00,
	0x3F, 0x80, 0x3F, 0x80, 0x7F, 0xC0, 0x7F, 0xC0, 0xFF, 0xE0, 0x00, 0x00,
	/* ICON DOWN */
	0x00, 0x00, 0xFF, 0xE0, 0x7F, 0xC0, 0x7F, 0xC0, 0x3F, 0x80, 0x3F, 0x80,
	0x1F, 0x00, 0x1F, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00,
};

static U8_T m_u8_lcd_buf[LCD_Y_SIZE][LCD_X_SIZE/8];               //显示缓冲区



/*************************************************************
函数名称: v_lcd_lcd_write		           				
函数功能: 向LCD控制器写数据或者命令						
输入参数: e_type  -- COMMAND/DATA，表示要写入的是命令/数据
          u8_data -- 要写入的命令或数据     		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_lcd_lcd_write(CD_TYPE_E e_type, U8_T u8_data)
{
	if (e_type == COMMAND)
		LCD_CD_LOW();
	else
		LCD_CD_HIGH();

	LCD_RD_HIGH();

	LCD_CS_LOW();

	LCD_WRITE_DATA(u8_data);

	LCD_WR_LOW();
	__NOP();           //总共200ns，一个__NOP()函数耗时20ns
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	
	LCD_WR_HIGH();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	
	LCD_CS_HIGH();
}

/*************************************************************
函数名称: v_lcd_lcd_init		           				
函数功能: 初始化LCD控制器						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_lcd_lcd_init(void)
{
	U32_T reg;

	reg = LPC_GPIO0->FIODIR;
	reg |= (0xFF << 21);
	LPC_GPIO0->FIODIR = reg;

	reg = LPC_GPIO1->FIODIR;
	reg |= ((3 << 18) | (1 << 22) | (1 << 25));
	LPC_GPIO1->FIODIR = reg;


	// software reset
	v_lcd_lcd_write(COMMAND,0xe2);   //23:reset by command
	v_delay_mdelay(200);

	// power control       
	v_lcd_lcd_write(COMMAND,0x2b);   //6:power control set as internal power 
	v_lcd_lcd_write(COMMAND,0x24);   //5:set temperate compensation as 0% 

	// lcd control
	v_lcd_lcd_write(COMMAND,0xa1);   //14:line rate a3--15.2klps, a2--12.6klps, a1--10.4klps, a0--8.5klps
	v_lcd_lcd_write(COMMAND,0xe9);   //26:Bias Ratio:1/10 bias
	v_lcd_lcd_write(COMMAND,0xc4);   //18:partial display and MX disable,MY enable
	v_lcd_lcd_write(COMMAND,0xd1);   //20:rgb-rgb 
	v_lcd_lcd_write(COMMAND,0xd5);   //21:4k color mode

	// n-line inversion 
	v_lcd_lcd_write(COMMAND, 0xc8);   //19,set n-line inversion 
	v_lcd_lcd_write(COMMAND, 0x10);   //enable NIV

	// scroll line 
	v_lcd_lcd_write(COMMAND,0x40);   //8:low bit of scroll line 
	v_lcd_lcd_write(COMMAND,0x50);   //8:high bit of scroll line
	v_lcd_lcd_write(COMMAND,0x90);   //13:FLT,FLB set 
	v_lcd_lcd_write(COMMAND,0x00);

	// partial display 
	v_lcd_lcd_write(COMMAND, 0x84);   //11:partial display control disable
	v_lcd_lcd_write(COMMAND, 0xf1);   //27:com end 
	v_lcd_lcd_write(COMMAND, 0x9f);   //160
	v_lcd_lcd_write(COMMAND, 0xf2);   //28:display start 
	v_lcd_lcd_write(COMMAND, 0x0);    //0
	v_lcd_lcd_write(COMMAND, 0xf3);   //29:display end 
	v_lcd_lcd_write(COMMAND, 0x9f);   //160

	// display control
	v_lcd_lcd_write(COMMAND,0xa4);   //15:all pixel off
	v_lcd_lcd_write(COMMAND,0xa6);   //16:inverse display off

	// electronic potentionmeter
	v_lcd_lcd_write(COMMAND,0x81);   //10:electronic potentionmeter 
	v_lcd_lcd_write(COMMAND,190);    //185);    

	// com scan fuction 
	v_lcd_lcd_write(COMMAND,0xda);   //22:enable FRC,PWM,LRM sequence 

	// window     
	v_lcd_lcd_write(COMMAND,0xf4);   //30:wpc0:column 
	v_lcd_lcd_write(COMMAND,0x25);   //start from 1 
	v_lcd_lcd_write(COMMAND,0xf6);   //32:wpc1  
	v_lcd_lcd_write(COMMAND,0x5A);   //end of:160 
 
	v_lcd_lcd_write(COMMAND,0xf5);   //31:wpp0:row 
	v_lcd_lcd_write(COMMAND,0x00);   //start from 1 
	v_lcd_lcd_write(COMMAND,0xf7);   //33:wpp1 
	v_lcd_lcd_write(COMMAND,0x9f);   //end 160 
 
	v_lcd_lcd_write(COMMAND,0xf8);   //34:inside mode  
	v_lcd_lcd_write(COMMAND,0x89);   //12:RAM control

	v_lcd_lcd_write(COMMAND,0xad);   //17:display on,select on/off mode.Green Enhance mode disable
	v_delay_mdelay(20);

	memset(m_u8_lcd_buf, 0, LCD_BUF_SIZE);
}

/*************************************************************
函数名称: v_lcd_lcd_draw_hline		           				
函数功能: 画水平直线，注意此函数操作的显示缓冲区，并没有刷屏，如需刷屏请调用lcd_flush						
输入参数: u32_x, u32_y   -- 直线的起始坐标值
          u32_len        -- 直线的长度
          u32_line_width -- 直线的线宽       		   				
输出参数: 无
返回值  ：无          		   															   				
**************************************************************/
void v_lcd_lcd_draw_hline(U32_T u32_x, U32_T u32_y, U32_T u32_len, U32_T u32_line_width)
{
	U32_T i;
	U32_T index, offset;
	U32_T remaining, slen;

	if (u32_x>LCD_X_SIZE-1 || u32_y>LCD_Y_SIZE-1)
		return;

	if (u32_x+u32_len >	LCD_X_SIZE)
		u32_len = LCD_X_SIZE - u32_x;
	if (u32_y+u32_line_width > LCD_Y_SIZE)
		u32_line_width = LCD_Y_SIZE - u32_y;

	for (i=0; i<u32_line_width; i++, u32_y++)
	{
		remaining = u32_len;
		index = (u32_x / 8);
		offset = (u32_x % 8);
		if (offset + u32_len > 8)
			slen = 8 - offset;
		else
			slen = u32_len;

		while (remaining != 0)
		{
			m_u8_lcd_buf[u32_y][index] |= (((1<<slen)-1)<<(8-slen-offset));
			
			index++;
			offset = 0;
			remaining -= slen;
			
			if (remaining > 8)
				slen = 8;
			else
				slen = remaining;	
		}
	}
}

/*************************************************************
函数名称: v_lcd_lcd_draw_vline		           				
函数功能: 画竖直直线，注意此函数操作的显示缓冲区，并没有刷屏，如需刷屏请调用lcd_flush						
输入参数: u32_x, u32_y   -- 直线的起始坐标值
          u32_len        -- 直线的长度
          u32_line_width -- 直线的线宽       		   				
输出参数: 无
返回值  ：无         		   															   				
**************************************************************/
void v_lcd_lcd_draw_vline(U32_T u32_x, U32_T u32_y, U32_T u32_len, U32_T u32_line_width)
{
	U32_T i;
	U32_T index, offset;
	U32_T remaining, swidth;

	if (u32_x>LCD_X_SIZE-1 || u32_y>LCD_Y_SIZE-1)
		return;

	if (u32_x+u32_line_width >	LCD_X_SIZE)
		u32_line_width = LCD_X_SIZE - u32_x;
	if (u32_y+u32_len > LCD_Y_SIZE)
		u32_len = LCD_Y_SIZE - u32_y;

	for (i=0; i<u32_len; i++, u32_y++)
	{
		remaining = u32_line_width;
		index = (u32_x / 8);
		offset = (u32_x % 8);
		if (offset + u32_line_width > 8)
			swidth = 8 - offset;
		else
			swidth = u32_line_width;

		while (remaining != 0)
		{
			m_u8_lcd_buf[u32_y][index] |= (((1<<swidth)-1)<<(8-swidth-offset));
			index++;
			offset = 0;
			remaining -= swidth;

			if (remaining > 8)
				swidth = 8;
			else
				swidth = remaining;
		}
	}
}								 

/*************************************************************
函数名称: v_lcd_lcd_draw_rectangle		           				
函数功能: 画矩形，注意此函数操作的显示缓冲区，并没有刷屏，如需刷屏请调用lcd_flush						
输入参数: u32_x, u32_y   -- 矩形左上角的坐标值
          u32_width      -- 矩形的宽度
          u32_height     -- 矩形的高度
          u32_line_width -- 线宽       		   				
输出参数: 无
返回值  ：无          		   															   				
**************************************************************/
void v_lcd_lcd_draw_rectangle(U32_T u32_x, U32_T u32_y, U32_T u32_width, U32_T u32_height, U32_T u32_line_width)
{
	v_lcd_lcd_draw_hline(u32_x, u32_y, u32_width, u32_line_width);
   	v_lcd_lcd_draw_hline(u32_x, u32_y+u32_height-u32_line_width, u32_width, u32_line_width);

	v_lcd_lcd_draw_vline(u32_x, u32_y, u32_height, u32_line_width);
	v_lcd_lcd_draw_vline(u32_x+u32_width-u32_line_width, u32_y, u32_height, u32_line_width);
}

/*************************************************************
函数名称: v_lcd_lcd_draw_icon		           				
函数功能: 画图标，注意此函数操作的显示缓冲区，并没有刷屏，如需刷屏请调用lcd_flush						
输入参数: u32_x, u32_y  -- 图标左上角的坐标值
          u32_icon_type -- 图标类型，见文件开始的宏定义       		   				
输出参数: 无
返回值  ：无          		   															   				
**************************************************************/
void v_lcd_lcd_draw_icon(U32_T u32_x, U32_T u32_y, U32_T u32_icon_type)
{
	U8_T code[32];
	U32_T i, byte_index, byte_offset, slen;

	if (((u32_y+12)>=LCD_Y_SIZE) || ((u32_x+12)>=LCD_X_SIZE))           // 超过边界退出
		return;

	byte_index = (u32_x / 8);
	byte_offset = (u32_x % 8);
	slen = 8 - byte_offset;

	memcpy(code, &m_u8_icon_tab[u32_icon_type*24], 24);

	if (byte_offset == 0)
	{
		for (i=0; i<12; i++, u32_y++)
		{
			m_u8_lcd_buf[u32_y][byte_index] = code[2*i];
			m_u8_lcd_buf[u32_y][byte_index+1] &= 0xF0;
			m_u8_lcd_buf[u32_y][byte_index+1] |= (code[2*i+1] & 0xF0);
		}
	}
	else if (byte_offset <= 4)
	{
		for (i=0; i<12; i++, u32_y++)
		{
			m_u8_lcd_buf[u32_y][byte_index] &= ~((1<<slen)-1);
			m_u8_lcd_buf[u32_y][byte_index] |= (code[2*i]>>byte_offset);
			m_u8_lcd_buf[u32_y][byte_index+1] &= ~(((1<<byte_offset)-1)<<slen);
			m_u8_lcd_buf[u32_y][byte_index+1] |= (code[2*i]<<slen);

			m_u8_lcd_buf[u32_y][byte_index+1] &= ~(0x0F<<(slen-4));
			m_u8_lcd_buf[u32_y][byte_index+1] |= ((code[2*i+1]&0xF0)>>byte_offset);
		}
	}
	else
	{
		for (i=0; i<12; i++, u32_y++)
		{
			m_u8_lcd_buf[u32_y][byte_index] &= ~((1<<slen)-1);
			m_u8_lcd_buf[u32_y][byte_index] |= (code[2*i]>>byte_offset);
			m_u8_lcd_buf[u32_y][byte_index+1] &= ~(((1<<byte_offset)-1)<<slen);
			m_u8_lcd_buf[u32_y][byte_index+1] |= (code[2*i]<<slen);

			m_u8_lcd_buf[u32_y][byte_index+1] &= ~((1<<slen)-1);
			m_u8_lcd_buf[u32_y][byte_index+1] |= (code[2*i+1]>>byte_offset);


			m_u8_lcd_buf[u32_y][byte_index+2] &= ~(((1<<(4-slen))-1)<<(4+slen));
			m_u8_lcd_buf[u32_y][byte_index+2] |= ((code[2*i+1]&0xF0)<<slen);
		}
	}
}

/*************************************************************
函数名称: v_lcd_get_ascii_code		           				
函数功能: 获取ASCII字符的字库点阵数据						
输入参数: c       -- ASCII字符
输出参数: pu8_buf -- 保存查找到点阵数据，调用者需为其分配足够的空间，6X12点阵ASCII字体需分配12字节
返回值  ：无														   				
**************************************************************/
static void v_lcd_get_ascii_code(char c, U8_T *pu8_buf)
{
	memcpy(pu8_buf, &ASC12x6[(c-0x20)*12], 12);
}

/*************************************************************
函数名称: v_lcd_get_chinese_code		           				
函数功能: 由汉字区位码获取字库点阵数据						
输入参数: cq      -- 汉字区码
          wq      -- 汉字位码
输出参数: pu8_buf -- 保存查找到点阵数据，调用者需为其分配足够的空间，12X12点阵汉字需分配24字节       		   				
返回值  ：无														   				
**************************************************************/
static void v_lcd_get_chinese_code(char cq, char cw, U8_T *pu8_buf)
{
	U32_T addr;

	addr = ((cq-0xA1)*94 + (cw-0xA1)) * 24 + DATAFLASH_FONT_ADDR;
	s32_flash_dataflash_read(addr, pu8_buf, 24);
}

/*************************************************************
函数名称: v_lcd_lcd_draw_string		           				
函数功能: 画字符串，注意此函数操作的显示缓冲区，并没有刷屏，如需刷屏请调用lcd_flush				
输入参数: u32_x, u32_y     -- 字符串的起始坐标值
          string           -- 指向要画的字符串
          u32_invert_index -- 反白显示索引(从0开始），表示从它指向的字符开始反白，后面invert_len个字符都将反白
          u32_invert_len   -- 见invert_index的解释，invert_len为0表示不反白，一个汉字的长度为2   		   				
输出参数: 无
返回值  ：无          		   															   				
**************************************************************/
void v_lcd_lcd_draw_string(U32_T u32_x, U32_T u32_y, const char *string, U32_T u32_invert_index, U32_T u32_invert_len)
{
	U32_T yy, xx = u32_x;
	U32_T char_index, index, u32_len;
	U8_T code[24];
	U32_T invert = 0;

	U32_T i, byte_index, byte_offset, slen;

	if ((u32_y+12) >= LCD_Y_SIZE)             // 超过底部边界退出
		return;

	u32_len = strlen(string);

	for (char_index=0, index=0; index<u32_len; char_index++)
	{
		yy = u32_y;

		byte_index = (xx / 8);
		byte_offset = (xx % 8);
		slen = 8 - byte_offset;

		if (string[index] > 0xA0)
		{
			if ((xx+12) >= LCD_X_SIZE)	                            // 到达右边界退出
				return;

			if (u32_invert_len == 0)
				invert = 0;
			else if (index>=u32_invert_index && index+2<u32_invert_index+u32_invert_len)
				invert = 2;
			else if (index>=u32_invert_index && index<u32_invert_index+u32_invert_len)
				invert = 1;
			else
				invert = 0;

			v_lcd_get_chinese_code(string[index], string[index+1],code);
			index += 2;

			if (invert != 0)
			{
				for (i=0; i<24; i++)
					code[i] = ~code[i];
			}

			if (byte_offset == 0)
			{
				for (i=0; i<12; i++, yy++)
				{
					m_u8_lcd_buf[yy][byte_index] = code[2*i];
					m_u8_lcd_buf[yy][byte_index+1] &= 0xF0;
					m_u8_lcd_buf[yy][byte_index+1] |= (code[2*i+1] & 0xF0);
				}
			}
			else if (byte_offset <= 4)
			{
				for (i=0; i<12; i++, yy++)
				{
					m_u8_lcd_buf[yy][byte_index] &= ~((1<<slen)-1);
					m_u8_lcd_buf[yy][byte_index] |= (code[2*i]>>byte_offset);
					m_u8_lcd_buf[yy][byte_index+1] &= ~(((1<<byte_offset)-1)<<slen);
					m_u8_lcd_buf[yy][byte_index+1] |= (code[2*i]<<slen);

					m_u8_lcd_buf[yy][byte_index+1] &= ~(0x0F<<(slen-4));
					m_u8_lcd_buf[yy][byte_index+1] |= ((code[2*i+1]&0xF0)>>byte_offset);
				}
			}
			else
			{
				for (i=0; i<12; i++, yy++)
				{
					m_u8_lcd_buf[yy][byte_index] &= ~((1<<slen)-1);
					m_u8_lcd_buf[yy][byte_index] |= (code[2*i]>>byte_offset);
					m_u8_lcd_buf[yy][byte_index+1] &= ~(((1<<byte_offset)-1)<<slen);
					m_u8_lcd_buf[yy][byte_index+1] |= (code[2*i]<<slen);

					m_u8_lcd_buf[yy][byte_index+1] &= ~((1<<slen)-1);
					m_u8_lcd_buf[yy][byte_index+1] |= (code[2*i+1]>>byte_offset);


					m_u8_lcd_buf[yy][byte_index+2] &= ~(((1<<(4-slen))-1)<<(4+slen));
					m_u8_lcd_buf[yy][byte_index+2] |= ((code[2*i+1]&0xF0)<<slen);
				}
			}

			if (invert == 2)
			{
				yy = u32_y;

				byte_index = ((xx+12) / 8);
				byte_offset = ((xx+12) % 8);
				slen = 8 - byte_offset;

				for (i=0; i<12; i++, yy++)
					m_u8_lcd_buf[yy][byte_index] |= (1<<(slen-1));
			}

			xx += 13;
		}
		else
		{
			if ((xx+6) >= LCD_X_SIZE)	                            // 到达右边界退出
				return;

			if (u32_invert_len == 0)
				invert = 0;
			else if (index>=u32_invert_index && index+1<u32_invert_index+u32_invert_len)
				invert = 2;
			else if (index>=u32_invert_index && index<u32_invert_index+u32_invert_len)
				invert = 1;
			else
				invert = 0;

			v_lcd_get_ascii_code(string[index], code);
			index++;

			if (invert != 0)
			{
				for (i=0; i<12; i++)
					code[i] = ~code[i];
			}

			if (byte_offset <= 2)
			{
				for (i=0; i<12; i++, yy++)
				{
					m_u8_lcd_buf[yy][byte_index] &= ~(0xFC>>byte_offset);
					m_u8_lcd_buf[yy][byte_index] |= ((code[i]&0xFC)>>byte_offset);
				}
			}
			else
			{
				for (i=0; i<12; i++, yy++)
				{
					m_u8_lcd_buf[yy][byte_index] &= ~((1<<slen)-1);
					m_u8_lcd_buf[yy][byte_index] |= (code[i]>>byte_offset);
					m_u8_lcd_buf[yy][byte_index+1] &= ~(((1<<(6-slen))-1)<<(2+slen));
					m_u8_lcd_buf[yy][byte_index+1] |= ((code[i]&0xFC)<<slen);

				}
			}

			if (invert == 2)
			{
				yy = u32_y;

				byte_index = ((xx+6) / 8);
				byte_offset = ((xx+6) % 8);
				slen = 8 - byte_offset;

				for (i=0; i<12; i++, yy++)
					m_u8_lcd_buf[yy][byte_index] |= (1<<(slen-1));
			}

			xx += 7;
		}
	}
}

/*************************************************************
函数名称: s_lcd_lcd_cut_string		           				
函数功能: LCD分割字符串函数						
输入参数: string        -- 指向要分割的字符串
          u32_max_width -- 分割的宽度      		   				
输出参数: 无
返回值  ：第二个字符串的指针，如果字符串长度不于u32_max_width，则返回空														   				
**************************************************************/
char *s_lcd_lcd_cut_string(char *string, U32_T u32_max_width)
{
	U32_T i, u32_width = 0;
	U32_T u32_len = strlen(string);
	
	for (i=0; i<u32_len; i++)
	{
		if (string[i] > 0xA0)
		{
			u32_width += 13;
			if (u32_width > u32_max_width)
			{
				memmove(&(string[i+1]), &(string[i]), u32_len-i);
				string[i] = '\0';
				return &(string[i+1]);
			}
			i += 1;
		}
		else
		{
			u32_width += 7;
			if (u32_width > u32_max_width)
			{
				if ((string[i-1] > 0xA0) || (string[i-1] == 0x20))
				{
					memmove(&(string[i+1]), &(string[i]), u32_len-i);
					string[i] = '\0';
					return &(string[i+1]);
				}
				else if ((string[i-2] > 0xA0) || (string[i-2] == 0x20))
				{
					memmove(&(string[i]), &(string[i-1]), u32_len-i+1);
					string[i-1] = '\0';
					return &(string[i]);
				}
				else
				{
					memmove(&(string[i+1]), &(string[i-1]), u32_len-i+1);
					string[i-1] = '-';
					string[i] = '\0';
					return &(string[i+1]);
				}
				
			}
		}
	}
	
	return NULL;
}

/*************************************************************
函数名称: v_lcd_lcd_flush		           				
函数功能: LCD刷屏函数，把显示缓冲区的数据发送给LCD控制器，水平显示本函数耗时36.8ms，竖直显示本函数耗时44.2ms						
输入参数: u8_rotate -- 旋转参数，LCD_ROTATE_0不旋转，LCD_ROTATE_90旋转90度       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_lcd_lcd_flush(U8_T u8_rotate)
{
	S32_T i, j, k;
	U8_T table[4] = {0x00, 0x0F, 0xF0, 0xFF};
	U8_T n;
	
	v_lcd_lcd_write(COMMAND, 0x60);    //row address LSB
	v_lcd_lcd_write(COMMAND, 0x70);    //row address MSB
	v_lcd_lcd_write(COMMAND, 0x05);    //culomn address LSB
	v_lcd_lcd_write(COMMAND, 0x12);    //culomn address MSB

	if (u8_rotate == LCD_ROTATE_0)
	{
		for (i=0; i<LCD_Y_SIZE; i++)
		{
			for (j=0; j<LCD_X_SIZE/8; j++)
			{
				n = m_u8_lcd_buf[i][j];
				v_lcd_lcd_write(DATA, table[(n>>6)&0x03]);
				v_lcd_lcd_write(DATA, table[(n>>4)&0x03]);
				v_lcd_lcd_write(DATA, table[(n>>2)&0x03]);
				v_lcd_lcd_write(DATA, table[n&0x03]);
			}
			v_lcd_lcd_write(DATA, 0);	       // 160点不能被3整除，所以多打两个点
		}
	}
	else
	{
		for (i=19; i>=0; i--)
		{
			for (j=0; j<8; j++)
			{
				for (k=0; k<160; k+=2)
				{
					if (((m_u8_lcd_buf[k][i] & (1<<j)) == 0) && ((m_u8_lcd_buf[k+1][i] & (1<<j)) == 0))
					{
						v_lcd_lcd_write(DATA, table[0]);
					}
					else if (((m_u8_lcd_buf[k][i] & (1<<j)) == 0) && ((m_u8_lcd_buf[k+1][i] & (1<<j)) != 0))
					{
						v_lcd_lcd_write(DATA, table[1]);
					}
					else if (((m_u8_lcd_buf[k][i] & (1<<j)) != 0) && ((m_u8_lcd_buf[k+1][i] & (1<<j)) == 0))
					{
						v_lcd_lcd_write(DATA, table[2]);
					}
					else if (((m_u8_lcd_buf[k][i] & (1<<j)) != 0) && ((m_u8_lcd_buf[k+1][i] & (1<<j)) != 0))
					{
						v_lcd_lcd_write(DATA, table[3]);
					}
				}
				v_lcd_lcd_write(DATA, 0);	       // 160点不能被3整除，所以多打两个点
			}
		}
	}
}

/*************************************************************
函数名称: v_lcd_lcd_clear		           				
函数功能: LCD清屏函数，清零显示缓冲区，注意此函数操作的显示缓冲区，并没有刷屏，如需刷屏请调用lcd_flush					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_lcd_lcd_clear(void)
{
	memset(m_u8_lcd_buf, 0, LCD_BUF_SIZE);
}
