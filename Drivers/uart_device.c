/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：uart.c
版    本：1.00
创建日期：2012-03-19
作    者：李楚楠
功能描述：串口驱动

函数列表：

修改记录：
	作者      日期        版本     修改内容
	李楚楠    2012-03-19  1.00     创建
**************************************************************/
 
#include <lpc17xx.h>
#include "uart_device.h"

/*************************************************************/
/**						模块内部宏定义						**/
/*************************************************************/
#define UART_IER_RBR		0x01
#define UART_IER_THRE		0x02
#define UART_IER_RLS		0x04

#define UART_IIR_PEND		0x01
#define UART_IIR_RLS		0x03
#define UART_IIR_RDA		0x02
#define UART_IIR_CTI		0x06
#define UART_IIR_THRE		0x01

#define UART_LSR_RDR		0x01
#define UART_LSR_OE			0x02
#define UART_LSR_PE			0x04
#define UART_LSR_FE			0x08
#define UART_LSR_BI			0x10
#define UART_LSR_THRE		0x20
#define UART_LSR_TEMT		0x40
#define UART_LSR_RXFE		0x80

#define UART_SET_DLAB		0x80

#define SET_UART0_RX()	(LPC_GPIO1 -> FIOSET = 0x80000000)
#define SET_UART0_TX()	(LPC_GPIO1 -> FIOCLR = 0x80000000)


#define SET_UART1_RX()	(LPC_GPIO2 -> FIOSET = 0x00000020)
#define SET_UART1_TX()	(LPC_GPIO2 -> FIOCLR = 0x00000020) 

#define SET_UART2_RX()	(LPC_GPIO4 -> FIOSET = 0x10000000)
#define SET_UART2_TX()	(LPC_GPIO4 -> FIOCLR = 0x10000000) 

#define SET_UART0_TXRXDIR()	(LPC_GPIO1 -> FIODIR |= 0x80000000)
#define SET_UART1_TXRXDIR()	(LPC_GPIO2 -> FIODIR |= 0x00000020)
#define SET_UART2_TXRXDIR()	(LPC_GPIO4 -> FIODIR |= 0x10000000)

/*************************************************************/
/**						模块内部变量定义					**/
/*************************************************************/
volatile uint32_t u32_urat_status[UART_PORTNUM];	  					//串口线状态寄存器
volatile uint8_t u8_uart_rxque_empty[UART_PORTNUM]; 					//接受队列空标志
volatile uint8_t u8_uart_rxque_full[UART_PORTNUM]; 					//接受队列满标志
volatile uint16_t u16_uart_rxque_num[UART_PORTNUM]; 					//接受队列数据个数
volatile uint8_t u8_uart_rx_queue[UART_PORTNUM][UART_RX_Q_SIZE];  				//接受队列数据寄存器
volatile uint8_t * u8_uart_rx_toppointer[UART_PORTNUM];		   				//接受队列入队指针
volatile uint8_t * u8_uart_rx_bottompointer[UART_PORTNUM];	   				//接受队列出队指针

volatile uint8_t u8_uart_txque_empty[UART_PORTNUM]; 					//发送队列空标志
volatile uint8_t u8_uart_txque_full[UART_PORTNUM];  					//发送队列满标志
volatile uint16_t u16_uart_txque_avail[UART_PORTNUM];  				//发送队列空余数据个数
volatile uint8_t u8_uart_tx_queue[UART_PORTNUM][UART_TX_Q_SIZE];  				//发送队列数据寄存器

volatile uint8_t * u8_uart_tx_toppointer[UART_PORTNUM];		   				//发送队列入队指针
volatile uint8_t * u8_uart_tx_bottompointer[UART_PORTNUM];	  				//发送队列出队指针


/*************************************************************/
/**						模块内部函数声明					**/
/*************************************************************/
void Uart_increment_tx_bottom( uint32_t portnum);
void Uart_increment_tx_top( uint32_t portnum);
void Uart_increment_rx_top( uint32_t portnum);
void Uart_increment_rx_bottom( uint32_t portnum);

void Uart_varinit(uint8_t portnum);

/**************************************************************************************************
** 函数名称:		UART0Handler
** 功能描述:		串口0中断服务程序
** 输入参数:		无
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void UART0_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	
	IIRValue = LPC_UART0->IIR;
    
	IIRValue >>= 1;			
	IIRValue &= 0x07;			/* 取中断标识 */
	if ( IIRValue == UART_IIR_RLS )		/* 判断中断类型 */
	{
		LSRValue = LPC_UART0->LSR;
		/* 接收线状态 */
		if ( LSRValue & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_RXFE|UART_LSR_BI) )
		{
			
			u32_urat_status[UART_PORT0] = LSRValue;
			Dummy = LPC_UART0->RBR;		/*读出接收到的数据，清楚中断*/

			return;
		}
		if ( LSRValue & UART_LSR_RDR )	/* RBR包含未读取数据读数据 */			
		{

			/* 接收缓冲区满，数据丢掉 */
	 		if(u8_uart_rxque_full[UART_PORT0] == TRUE)
			{
				Dummy = LPC_UART0->RBR;		/*读出接收到的数据，清楚中断*/			  
			}
			else
			{
				(* u8_uart_rx_toppointer[UART_PORT0]) = LPC_UART0->RBR;
				Uart_increment_rx_top(UART_PORT0);	
			}

		}
	}
	else if ( IIRValue == UART_IIR_RDA )	 //接收数据有用
	{
			/* 接收缓冲区满，数据丢掉 */
		if(u8_uart_rxque_full[UART_PORT0] == TRUE)
		{
			Dummy = LPC_UART0->RBR;		/* 读取RBR数据，清楚中断 */				  
		}
		else
		{
			(* u8_uart_rx_toppointer[UART_PORT0]) = LPC_UART0->RBR;
			Uart_increment_rx_top(UART_PORT0);	
		}

	}
	else if ( IIRValue == UART_IIR_CTI )	/* 字符超时 */
	{
	
		u32_urat_status[UART_PORT0] |= 0x100;		
	}
	else if ( IIRValue == UART_IIR_THRE )	/* 发送保持器空 */
	{
	
		LSRValue = LPC_UART0->LSR;		
			/* 发送缓冲区还有未发送数据，则继续发送 */
		if(u8_uart_txque_empty[UART_PORT0] == FALSE)
		{
			SET_UART0_TX();//485T/R引脚置为发送

			LPC_UART0->THR = (* u8_uart_tx_bottompointer[UART_PORT0]);	
			Uart_increment_tx_bottom(UART_PORT0);
	
		}
		else
		{
			SET_UART0_RX();//485T/R引脚置为接收						
		}

	}
    
}

/**************************************************************************************************
** 函数名称:		UART1Handler
** 功能描述:		串口1中断服务程序
** 输入参数:		无
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void UART1_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	
	IIRValue = LPC_UART1->IIR;
    
	IIRValue >>= 1;			
	IIRValue &= 0x07;			/* 取中断标识 */
	if ( IIRValue == UART_IIR_RLS )		/* 判断中断类型 */
	{
		LSRValue = LPC_UART1->LSR;
		/* 接收线状态 */
		if ( LSRValue & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_RXFE|UART_LSR_BI) )
		{
		
			u32_urat_status[UART_PORT1] = LSRValue;
			Dummy = LPC_UART1->RBR;		/*读出接收到的数据，清楚中断*/

			return;
		}
		if ( LSRValue & UART_LSR_RDR )	/* RBR包含未读取数据读数据 */				
		{
			//* 接收缓冲区满，数据丢掉 */

	 		if(u8_uart_rxque_full[UART_PORT1] == TRUE)
			{
				Dummy = LPC_UART1->RBR;		/*读出接收到的数据，清楚中断*/				  
			}
			else
			{
				(* u8_uart_rx_toppointer[UART_PORT1]) = LPC_UART1->RBR;
				Uart_increment_rx_top(UART_PORT1);	
			}
	
		}
  	}
	else if ( IIRValue == UART_IIR_RDA )	 //接收数据有用
	{
		/* 接收缓冲区满，数据丢掉 */
		if(u8_uart_rxque_full[UART_PORT1] == TRUE)
		{
			Dummy = LPC_UART1->RBR;	/* 读取RBR数据，清楚中断 */						  
		}
		else
		{
			
			(* u8_uart_rx_toppointer[UART_PORT1]) = LPC_UART1->RBR;
			Uart_increment_rx_top(UART_PORT1);	
		}
	}
	else if ( IIRValue == UART_IIR_CTI )	/* 字符超时 */
	{
	
		u32_urat_status[UART_PORT1] |= 0x100;		
	}
	else if ( IIRValue == UART_IIR_THRE )	/* 发送保持器空 */
	{

		LSRValue = LPC_UART1->LSR;		

			/* 发送缓冲区还有未发送数据，则继续发送 */		
		if(u8_uart_txque_empty[UART_PORT1] == FALSE)
		{
			SET_UART1_TX();//485T/R引脚置为发送
			LPC_UART1->THR = (* u8_uart_tx_bottompointer[UART_PORT1]);	
			Uart_increment_tx_bottom(UART_PORT1);	
	
		}
		else
		{
			SET_UART1_RX();//485T/R引脚置为接收						
		}


	}

}

/**************************************************************************************************
** 函数名称:		UART2Handler
** 功能描述:		串口2中断服务程序
** 输入参数:		无
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void UART2_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	
	IIRValue = LPC_UART2->IIR;
    
	IIRValue >>= 1;			
	IIRValue &= 0x07;			/* 取中断标识 */
	if ( IIRValue == UART_IIR_RLS )	/* 判断中断类型 */
	{
		LSRValue = LPC_UART2->LSR;
		/* 接收线状态 */
		if ( LSRValue & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_RXFE|UART_LSR_BI) )
		{
			
			u32_urat_status[UART_PORT2] = LSRValue;
			Dummy = LPC_UART2->RBR;		/*读出接收到的数据，清楚中断*/
								
			return;
		}
		if ( LSRValue & UART_LSR_RDR )/* RBR包含未读取数据读数据 */			
		{
		
			/* 接收缓冲区满，数据丢掉 */
	 		if(u8_uart_rxque_full[UART_PORT2] == TRUE)
			{
				Dummy = LPC_UART2->RBR;		/*读出接收到的数据，清楚中断*/				  
			}
			else
			{
				(* u8_uart_rx_toppointer[UART_PORT2]) = LPC_UART2->RBR;
				Uart_increment_rx_top(UART_PORT2);	
			}
	
		}
	}
	else if ( IIRValue == UART_IIR_RDA )	 //接收数据有用
	{
		/* 接收缓冲区满，数据丢掉 */
		if(u8_uart_rxque_full[UART_PORT2] == TRUE)
		{
			Dummy = LPC_UART2->RBR;		/* 读取RBR数据，清楚中断 */					  
		}
		else
		{
			
			(* u8_uart_rx_toppointer[UART_PORT2]) = LPC_UART2->RBR;
			Uart_increment_rx_top(UART_PORT2);	
		}

	}
	else if ( IIRValue == UART_IIR_CTI )/* 字符超时 */
	{

		u32_urat_status[UART_PORT2] |= 0x100;	
	}
	else if ( IIRValue == UART_IIR_THRE )	/* 发送保持器空 */
	{

		LSRValue = LPC_UART2->LSR;	

		/* 发送缓冲区还有未发送数据，则继续发送 */	
		if(u8_uart_txque_empty[UART_PORT2] == FALSE)
		{
			SET_UART2_TX();//485T/R引脚置为发送
			LPC_UART2->THR = (* u8_uart_tx_bottompointer[UART_PORT2]);	
			Uart_increment_tx_bottom(UART_PORT2);
	
		}
		else
		{
			SET_UART2_RX();//485T/R引脚置为接收						
		}

	}

}


/**************************************************************************************************
** 函数名称:		Uart_varinit
** 功能描述:		串口变量初始化程序
** 输入参数:		portnum -- 设备端口号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Uart_varinit(uint8_t portnum)
{
	if(portnum >= UART_PORTNUM)
	{
		return;
	}
	else
	{
		u8_uart_rxque_empty[portnum] = TRUE ;
		u8_uart_rxque_full[portnum] = FALSE ;
		u8_uart_txque_empty[portnum] = TRUE ;
		u8_uart_txque_full[portnum] = FALSE ;	
		u16_uart_rxque_num[portnum] = 0;
		u16_uart_txque_avail[portnum] = UART_TX_Q_SIZE;
		u8_uart_rx_bottompointer[portnum] = &u8_uart_rx_queue[portnum][0];
		u8_uart_rx_toppointer[portnum] = &u8_uart_rx_queue[portnum][0];	
		u8_uart_tx_bottompointer[portnum] = &u8_uart_tx_queue[portnum][0];
		u8_uart_tx_toppointer[portnum] = &u8_uart_tx_queue[portnum][0];			
	}

}

/**************************************************************************************************
** 函数名称:		Uart_init
** 功能描述:		串口初始化程序
** 输入参数:		portnum -- 设备端口号(COM[0]、COM[1]、COM[2])
**              	baudrate -- 波特率
**            		frame -- 帧格式(数据位[DATA_BITS_8、DATA_BITS_7、DATA_BITS_5]、
**                              	奇偶位[EON、EOE、EOO、EO0、EO1]、
**                             	 	停止位[STOP_BIT_1、STOP_BIT_2]、三者为或运算)
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Uart_init( uint8_t portnum, uint32_t baudrate , uint8_t frame)
{
	Uart_varinit(portnum);
  	switch(portnum)
	{
		case 0:
			SET_UART0_TXRXDIR();
		
		 	Set_baudrate(portnum,baudrate);
			Set_frame(portnum,frame);
			LPC_UART0->FCR = 0x07;		/* 允许和复位 TX ， RX FIFO. */		
		   	NVIC_EnableIRQ(UART0_IRQn);
		
		    LPC_UART0->IER = UART_IER_RBR | UART_IER_THRE | UART_IER_RLS;	/* 允许UART0中断 */
	
			SET_UART0_RX();//485T/R引脚置为接收	
		break;



		case 1:
			SET_UART1_TXRXDIR();
			
		 	Set_baudrate(portnum,baudrate);
			Set_frame(portnum,frame);
	
			LPC_UART1->FCR = 0x07;		/* 允许和复位 TX ， RX FIFO. */	
		   	NVIC_EnableIRQ(UART1_IRQn);
		
		    LPC_UART1->IER = UART_IER_RBR | UART_IER_THRE | UART_IER_RLS;	/* 允许UART1中断 */
	
			SET_UART1_RX();//485T/R引脚置为接收	
		break;

		case 2:
 			SET_UART2_TXRXDIR();

		 	Set_baudrate(portnum,baudrate);
			Set_frame(portnum,frame);
	
			LPC_UART2->FCR = 0x07;			/* 允许和复位 TX ， RX FIFO. */		
		   	NVIC_EnableIRQ(UART2_IRQn);
		
		    LPC_UART2->IER = UART_IER_RBR | UART_IER_THRE | UART_IER_RLS;	/* 允许UART2中断 */
	
			SET_UART2_RX();//485T/R引脚置为接收	
		break;
 
		default:
   		break;
	}
}


/**************************************************************************************************
** 函数名称:		Close_uartirq
** 功能描述:		关闭串口中断
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Close_uartirq( uint8_t portnum)
{
	switch(portnum)
	{
	 	case  UART_PORT0:
			NVIC_DisableIRQ(UART0_IRQn);
			break;

	 	case  UART_PORT1:
			NVIC_DisableIRQ(UART1_IRQn);	
			break;

 	 	case  UART_PORT2:
			NVIC_DisableIRQ(UART2_IRQn);
			break;

		default : 
			break;
	}
}

/**************************************************************************************************
** 函数名称:		Open_uartirq
** 功能描述:		开启串口中断
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Open_uartirq( uint8_t portnum)
{

	switch(portnum)
	{
	 	case  UART_PORT0:
			NVIC_EnableIRQ(UART0_IRQn);
			break;

	 	case  UART_PORT1:
			NVIC_EnableIRQ(UART1_IRQn);	
			break;

 	 	case  UART_PORT2:
			NVIC_EnableIRQ(UART2_IRQn);
			break;

		default : 
			break;
	}	
}

/**************************************************************************************************
** 函数名称:		Set_frame
** 功能描述:		设置线路控制寄存器值
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]), frame -- 需写进的值
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Set_frame( uint8_t portnum, uint8_t frame)
{

	switch(portnum)
	{
	 	case  UART_PORT0:
			LPC_UART0->LCR = frame;	
			break;

	 	case  UART_PORT1:
			LPC_UART1->LCR = frame;	
			break;

 	 	case  UART_PORT2:
			LPC_UART2->LCR = frame;			
			break;

		default : 
			break;

	}

}

/**************************************************************************************************
** 函数名称:		Get_frame
** 功能描述:		获取线路控制寄存器值
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			LCR寄存器值
**************************************************************************************************/

uint8_t Get_frame( uint8_t portnum)
{
	uint8_t frame;

	switch(portnum)
	{
	 	case  UART_PORT0:
			frame = LPC_UART0->LCR;	
			break;

	 	case  UART_PORT1:
			frame = LPC_UART1->LCR;	
			break;

 	 	case  UART_PORT2:
	    	frame = LPC_UART2->LCR;			
			break;

		default : 
	   		 frame = 0;
			break;

	}

	return frame;

}

/**************************************************************************************************
** 函数名称:		Set_Baudrate
** 功能描述:		设置波特率
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
** 					baudrate -- 波特率大小
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Set_baudrate( uint8_t portnum, uint32_t baudrate)
{
	uint32_t fdiv;
	uint8_t frame;

	switch(portnum)
	{
	 	case  UART_PORT0:
		    frame = LPC_UART0->LCR;
			LPC_UART0->LCR = (frame | UART_SET_DLAB);
			if(baudrate == 115200)	 //当波特率大于57600时，计算出来的波特率误差太大，需要使用波特率小数寄存器，所以在波特率大于57600时，只支持115200
			{
			    LPC_UART0->DLM = 0;							
			    LPC_UART0->DLL = 0x08;
			    LPC_UART0->FDR = 0x92;											
			}
			else
			{
				fdiv = ( SystemCoreClock/4/16 ) / baudrate ;/*baud rate, Fpclk: 18MHz */
			    LPC_UART0->DLM = fdiv / 256;							
			    LPC_UART0->DLL = fdiv % 256;			
			    LPC_UART0->FDR = 0x10;
			}


			LPC_UART0->LCR = (frame & (~UART_SET_DLAB));

			break;

	 	case  UART_PORT1:
			frame = LPC_UART1->LCR;
			LPC_UART1->LCR = (frame | UART_SET_DLAB);	

			if(baudrate == 115200)	
			{
			    LPC_UART1->DLM = 0;							
			    LPC_UART1->DLL = 0x08;
			    LPC_UART1->FDR = 0x92;											
			}
			else
			{
				fdiv = ( SystemCoreClock/4/16 ) / baudrate ;/*baud rate, Fpclk: 18MHz */
			    LPC_UART1->DLM = fdiv / 256;							
			    LPC_UART1->DLL = fdiv % 256;			
			    LPC_UART1->FDR = 0x10;
			}

			LPC_UART1->LCR = (frame & (~UART_SET_DLAB));	
			break;

 	 	case  UART_PORT2:
			frame = LPC_UART2->LCR;
			LPC_UART2->LCR = (frame | UART_SET_DLAB);
			if(baudrate == 115200)	
			{
			    LPC_UART2->DLM = 0;							
			    LPC_UART2->DLL = 0x08;
			    LPC_UART2->FDR = 0x92;											
			}
			else
			{
				fdiv = ( SystemCoreClock/4/16 ) / baudrate ;/*baud rate, Fpclk: 18MHz */
			    LPC_UART2->DLM = fdiv / 256;							
			    LPC_UART2->DLL = fdiv % 256;			
			    LPC_UART2->FDR = 0x10;
			}

			LPC_UART2->LCR = (frame & (~UART_SET_DLAB));	
			break;

		default : 

			break;

	}

}

/**************************************************************************************************
** 函数名称:		Get_baudrate
** 功能描述:		获取波特率
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			baudrate -- 波特率
**************************************************************************************************/

uint32_t Get_baudrate( uint8_t portnum)
{
	uint32_t baudrate,fdiv;
	uint8_t	lcrx;
	   
	switch(portnum)
	{
	 	case  UART_PORT0:
		    lcrx = LPC_UART0->LCR;
			lcrx |= 0x80;
	
			fdiv = (LPC_UART0->DLM & 0xff)*256 + (LPC_UART0->DLL&0xff); 
			baudrate = 	( SystemCoreClock/4/16 )/fdiv;	   //	baudrate = 	( PFCLK/16 )/Fdiv;	 //PFCLK是外设时钟
			lcrx &= (~0x80);		
		    lcrx = LPC_UART0->LCR;
			break;

	 	case  UART_PORT1:
			lcrx = LPC_UART1->LCR;
			lcrx |= 0x80;
	
			fdiv = (LPC_UART1->DLM & 0xff)*256 + (LPC_UART1->DLL&0xff); 
			baudrate = 	( SystemCoreClock/4/16 )/fdiv;					
			lcrx &= (~0x80);		
		    lcrx = LPC_UART1->LCR;	
			break;

 	 	case  UART_PORT2:
		   	lcrx = LPC_UART2->LCR;
			lcrx |= 0x80;
			
			fdiv = (LPC_UART2->DLM & 0xff)*256 + (LPC_UART2->DLL&0xff); 
			baudrate = 	( SystemCoreClock/4/16 )/fdiv;				
			lcrx &= (~0x80);		
		    lcrx = LPC_UART2->LCR;	
			break;

		default : 
			baudrate = 0;
			break;

	}

	return 	baudrate;
}

/**************************************************************************************************
** 函数名称:		Uart_recv
** 功能描述:		UART接收数据
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
**              	*buf -- 保存接收数据的指针, 
**              	maxlen -- 接收数据的最大长度
** 输出参数:		无
** 返回值:			i -- 接收到数据的数目, -1 -- 接收失败
**************************************************************************************************/

int16_t Uart_recv( uint8_t portnum,uint8_t *buf,  uint16_t  maxlen)
{
	uint16_t length=0;
	int16_t	count=0;
	if(portnum >= UART_PORTNUM)
	{
		return 	-1;			
	}
	
	Close_uartirq(portnum);    //操作前关中断,保护中断中也要防问的数据
	
	length =  u16_uart_rxque_num[portnum];	 //接收缓冲区剩下的字节数
	if(length > maxlen)					 //如果接收缓冲区字节数超过读取最大字节数，则读取读取最大字节数，否则把缓冲区的剩余数据都读取。
	{
		length = maxlen;	
	}

	while(length > 0)
	{
		*buf = * u8_uart_rx_bottompointer[portnum];
		Uart_increment_rx_bottom(portnum);
		length --;
		buf ++;
		count ++;	
	}
	
	Open_uartirq(portnum);    //操作后开中断
	
	return 	count;	
}


/**************************************************************************************************
** 函数名称:		Uart_get_rxbuff_num
** 功能描述:		取接收缓冲区已接收的数据长度
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			接收缓冲区已接收的数据长度
**************************************************************************************************/

uint16_t Uart_get_rxbuff_num( uint8_t portnum)
{
	uint16_t buffnum;
	if(portnum >= UART_PORTNUM)
	{
		return 	0;			
	}
	buffnum = u16_uart_rxque_num[portnum];	
	 	
	return 	buffnum;	
}


/**************************************************************************************************
** 函数名称:		Uart_get_txbuff_num
** 功能描述:		取发送缓冲区空闲的数据长度
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2])
** 输出参数:		无
** 返回值:			发送缓冲区空闲的数据长度
**************************************************************************************************/

uint16_t Uart_get_txbuff_num(uint8_t portnum)
{
	uint16_t buffnum;
	if(portnum >= UART_PORTNUM)
	{
		return 	0;			
	}
	buffnum = u16_uart_txque_avail[portnum];		 	
	return 	buffnum;	
}


/**************************************************************************************************
** 函数名称:		Uart_send
** 功能描述:		UART发送数据
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
**              	*buf -- 发送数据的指针, 
**              	length -- 发送数据的长度
** 输出参数:		无
** 返回值:			0 -- 成功, 1 -- 失败
**************************************************************************************************/

uint16_t Uart_send( uint8_t portnum, uint8_t *buf, uint16_t length )
{
	if(portnum >= UART_PORTNUM)
	{
		return( FALSE ); 		
	}
	
	Close_uartirq(portnum);    //操作前关中断,保护中断中也要防问的数据
	
	if(length > u16_uart_txque_avail[portnum] || length == 0)
	{
		Open_uartirq(portnum);    //操作后开中断
		return( FALSE ); 		
	}
	while ( length != 0 )
	{	
		(* u8_uart_tx_toppointer[portnum]) = *buf;		//数据进 发送缓冲数据区
		Uart_increment_tx_top(portnum);						//入队列指针处理
		buf++;
		length--;
	}

	switch(portnum)
	{
	 	case  UART_PORT0:
			if((u8_uart_txque_empty[portnum] == FALSE) && (LPC_UART0->LSR & UART_LSR_THRE))   //保持寄存器为空才能写
			{
				SET_UART0_TX();//485T/R引脚置为发送
				LPC_UART0->THR = (* u8_uart_tx_bottompointer[portnum]);	
				Uart_increment_tx_bottom(portnum);				
			}
			break;

	 	case  UART_PORT1:
			if((u8_uart_txque_empty[portnum] == FALSE) && (LPC_UART1->LSR & UART_LSR_THRE))   //保持寄存器为空才能写 
			{
				SET_UART1_TX();//485T/R引脚置为发送
				LPC_UART1->THR = (* u8_uart_tx_bottompointer[portnum]);	
				Uart_increment_tx_bottom(portnum);				
			}	
			break;

 	 	case  UART_PORT2:
			if((u8_uart_txque_empty[portnum] == FALSE) && (LPC_UART2->LSR & UART_LSR_THRE))   //保持寄存器为空才能写 
			{
				SET_UART2_TX();//485T/R引脚置为发送
				LPC_UART2->THR = (* u8_uart_tx_bottompointer[portnum]);	
				Uart_increment_tx_bottom(portnum);				
			}	
			break;

		default : 
			break;

	}
	
	Open_uartirq(portnum);    //操作后开中断
	
	return TRUE;
}

/**************************************************************************************************
** 函数名称:		Uart_increment_tx_bottom
** 功能描述:		UART发送数据缓冲区的出队指针计算，队列空满判断。
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Uart_increment_tx_bottom( uint32_t portnum)
{
	/*	if at the top of the queue, need to wrap around to the bottom	*
	*	else just increment pointer	*/
	if (u8_uart_tx_bottompointer[portnum] == &u8_uart_tx_queue[portnum][UART_TX_Q_SIZE-1])
	{
		u8_uart_tx_bottompointer[portnum] = &u8_uart_tx_queue[portnum][0];
	}
	else
	{
		u8_uart_tx_bottompointer[portnum]++;
	}

	u8_uart_txque_full[portnum] = FALSE;	/*	just removed an entry so queue not full	*/
	u16_uart_txque_avail[portnum]++;			/*	indicate this one now available	*/

	if (u8_uart_tx_bottompointer[portnum] == u8_uart_tx_toppointer[portnum])
	{
		/*	If after incrementing bottom, the pointers are equal, the queue is empty	*/
		u8_uart_txque_empty[portnum] = TRUE;
	}
}/*	end Increment_SCI_Tx_Bottom	*/


/**************************************************************************************************
** 函数名称:		Uart_increment_tx_top
** 功能描述:		UART发送数据缓冲区的入队指针计算，队列空满判断。
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Uart_increment_tx_top( uint32_t portnum)
{
	/*	if at the top of the queue, need to wrap around to the bottom	*
	*	else just increment pointer	*/
	if (u8_uart_tx_toppointer[portnum] == &u8_uart_tx_queue[portnum][UART_TX_Q_SIZE-1])
	{
		u8_uart_tx_toppointer[portnum] = &u8_uart_tx_queue[portnum][0];
	}
	else
	{
		u8_uart_tx_toppointer[portnum]++;
	}

	u8_uart_txque_empty[portnum] = FALSE;	/*	just added something so queue is not empty	*/
	u16_uart_txque_avail[portnum]--;			/*	indicate this one now unavailable	*/

	if (u8_uart_tx_toppointer[portnum] == u8_uart_tx_bottompointer[portnum])
	{
		/*	If after incrementing the top, the pointers are equal, the queue is full	*/
		u8_uart_txque_full[portnum] = TRUE;
	}
}/*	end Increment_SCI_Tx_Top	*/

/**************************************************************************************************
** 函数名称:		Uart_increment_rx_top
** 功能描述:		UART接收数据缓冲区的入队指针计算，队列空满判断。
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Uart_increment_rx_top( uint32_t portnum)
{
	/*	if at the top of the queue, need to wrap around to the bottom	*
	*	else just increment pointer	*/

	if (u8_uart_rx_toppointer[portnum] == &(u8_uart_rx_queue[portnum][UART_RX_Q_SIZE-1]))
	{
		u8_uart_rx_toppointer[portnum] = &(u8_uart_rx_queue[portnum][0]);
	}
	else
	{
		u8_uart_rx_toppointer[portnum]++;
	}
	u8_uart_rxque_empty[portnum] = FALSE;	/*	just added something so buffer is not empty	*/

	if (u8_uart_rx_toppointer[portnum] == u8_uart_rx_bottompointer[portnum])
	{
		/*	If after incrementing the top, the pointers are equal, the queue is full	*/
		u8_uart_rxque_full[portnum] = TRUE;
	}

	u16_uart_rxque_num[portnum] ++;
}/*	end Increment_SCI_Rx_Top	*/


/**************************************************************************************************
** 函数名称:		Uart_increment_rx_top
** 功能描述:		UART接收数据缓冲区的出队指针计算，队列空满判断。
** 输入参数:		portnum -- 端口设备号(COM[0]、COM[1]、COM[2]),
** 输出参数:		无
** 返回值:			无
**************************************************************************************************/

void Uart_increment_rx_bottom( uint32_t portnum)
{
	/*	if at the top of the queue, need to wrap around to the bottom	*
	*	else just increment pointer	*/
	if (u8_uart_rx_bottompointer[portnum] == &(u8_uart_rx_queue[portnum][UART_RX_Q_SIZE-1]))
	{
		u8_uart_rx_bottompointer[portnum] = &(u8_uart_rx_queue[portnum][0]);
	}
	else
	{
		u8_uart_rx_bottompointer[portnum]++;
	}

	u8_uart_rxque_full[portnum] = FALSE;	/*	just removed an entry so queue not full	*/
	
	if (u8_uart_rx_bottompointer[portnum] == u8_uart_rx_toppointer[portnum])
	{
		/*	if after incrementing bottom, the pointers are equal, the queue is empty	*/
		u8_uart_rxque_empty[portnum] = TRUE;
	}

	u16_uart_rxque_num[portnum] --;
	
}


/******************************************************************************
**                            End Of File
******************************************************************************/
