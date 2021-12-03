/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����uart.c
��    ����1.00
�������ڣ�2012-03-19
��    �ߣ�����
������������������

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	����    2012-03-19  1.00     ����
**************************************************************/
 
#include <lpc17xx.h>
#include "uart_device.h"

/*************************************************************/
/**						ģ���ڲ��궨��						**/
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
/**						ģ���ڲ���������					**/
/*************************************************************/
volatile uint32_t u32_urat_status[UART_PORTNUM];	  					//������״̬�Ĵ���
volatile uint8_t u8_uart_rxque_empty[UART_PORTNUM]; 					//���ܶ��пձ�־
volatile uint8_t u8_uart_rxque_full[UART_PORTNUM]; 					//���ܶ�������־
volatile uint16_t u16_uart_rxque_num[UART_PORTNUM]; 					//���ܶ������ݸ���
volatile uint8_t u8_uart_rx_queue[UART_PORTNUM][UART_RX_Q_SIZE];  				//���ܶ������ݼĴ���
volatile uint8_t * u8_uart_rx_toppointer[UART_PORTNUM];		   				//���ܶ������ָ��
volatile uint8_t * u8_uart_rx_bottompointer[UART_PORTNUM];	   				//���ܶ��г���ָ��

volatile uint8_t u8_uart_txque_empty[UART_PORTNUM]; 					//���Ͷ��пձ�־
volatile uint8_t u8_uart_txque_full[UART_PORTNUM];  					//���Ͷ�������־
volatile uint16_t u16_uart_txque_avail[UART_PORTNUM];  				//���Ͷ��п������ݸ���
volatile uint8_t u8_uart_tx_queue[UART_PORTNUM][UART_TX_Q_SIZE];  				//���Ͷ������ݼĴ���

volatile uint8_t * u8_uart_tx_toppointer[UART_PORTNUM];		   				//���Ͷ������ָ��
volatile uint8_t * u8_uart_tx_bottompointer[UART_PORTNUM];	  				//���Ͷ��г���ָ��


/*************************************************************/
/**						ģ���ڲ���������					**/
/*************************************************************/
void Uart_increment_tx_bottom( uint32_t portnum);
void Uart_increment_tx_top( uint32_t portnum);
void Uart_increment_rx_top( uint32_t portnum);
void Uart_increment_rx_bottom( uint32_t portnum);

void Uart_varinit(uint8_t portnum);

/**************************************************************************************************
** ��������:		UART0Handler
** ��������:		����0�жϷ������
** �������:		��
** �������:		��
** ����ֵ:			��
**************************************************************************************************/

void UART0_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	
	IIRValue = LPC_UART0->IIR;
    
	IIRValue >>= 1;			
	IIRValue &= 0x07;			/* ȡ�жϱ�ʶ */
	if ( IIRValue == UART_IIR_RLS )		/* �ж��ж����� */
	{
		LSRValue = LPC_UART0->LSR;
		/* ������״̬ */
		if ( LSRValue & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_RXFE|UART_LSR_BI) )
		{
			
			u32_urat_status[UART_PORT0] = LSRValue;
			Dummy = LPC_UART0->RBR;		/*�������յ������ݣ�����ж�*/

			return;
		}
		if ( LSRValue & UART_LSR_RDR )	/* RBR����δ��ȡ���ݶ����� */			
		{

			/* ���ջ������������ݶ��� */
	 		if(u8_uart_rxque_full[UART_PORT0] == TRUE)
			{
				Dummy = LPC_UART0->RBR;		/*�������յ������ݣ�����ж�*/			  
			}
			else
			{
				(* u8_uart_rx_toppointer[UART_PORT0]) = LPC_UART0->RBR;
				Uart_increment_rx_top(UART_PORT0);	
			}

		}
	}
	else if ( IIRValue == UART_IIR_RDA )	 //������������
	{
			/* ���ջ������������ݶ��� */
		if(u8_uart_rxque_full[UART_PORT0] == TRUE)
		{
			Dummy = LPC_UART0->RBR;		/* ��ȡRBR���ݣ�����ж� */				  
		}
		else
		{
			(* u8_uart_rx_toppointer[UART_PORT0]) = LPC_UART0->RBR;
			Uart_increment_rx_top(UART_PORT0);	
		}

	}
	else if ( IIRValue == UART_IIR_CTI )	/* �ַ���ʱ */
	{
	
		u32_urat_status[UART_PORT0] |= 0x100;		
	}
	else if ( IIRValue == UART_IIR_THRE )	/* ���ͱ������� */
	{
	
		LSRValue = LPC_UART0->LSR;		
			/* ���ͻ���������δ�������ݣ���������� */
		if(u8_uart_txque_empty[UART_PORT0] == FALSE)
		{
			SET_UART0_TX();//485T/R������Ϊ����

			LPC_UART0->THR = (* u8_uart_tx_bottompointer[UART_PORT0]);	
			Uart_increment_tx_bottom(UART_PORT0);
	
		}
		else
		{
			SET_UART0_RX();//485T/R������Ϊ����						
		}

	}
    
}

/**************************************************************************************************
** ��������:		UART1Handler
** ��������:		����1�жϷ������
** �������:		��
** �������:		��
** ����ֵ:			��
**************************************************************************************************/

void UART1_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	
	IIRValue = LPC_UART1->IIR;
    
	IIRValue >>= 1;			
	IIRValue &= 0x07;			/* ȡ�жϱ�ʶ */
	if ( IIRValue == UART_IIR_RLS )		/* �ж��ж����� */
	{
		LSRValue = LPC_UART1->LSR;
		/* ������״̬ */
		if ( LSRValue & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_RXFE|UART_LSR_BI) )
		{
		
			u32_urat_status[UART_PORT1] = LSRValue;
			Dummy = LPC_UART1->RBR;		/*�������յ������ݣ�����ж�*/

			return;
		}
		if ( LSRValue & UART_LSR_RDR )	/* RBR����δ��ȡ���ݶ����� */				
		{
			//* ���ջ������������ݶ��� */

	 		if(u8_uart_rxque_full[UART_PORT1] == TRUE)
			{
				Dummy = LPC_UART1->RBR;		/*�������յ������ݣ�����ж�*/				  
			}
			else
			{
				(* u8_uart_rx_toppointer[UART_PORT1]) = LPC_UART1->RBR;
				Uart_increment_rx_top(UART_PORT1);	
			}
	
		}
  	}
	else if ( IIRValue == UART_IIR_RDA )	 //������������
	{
		/* ���ջ������������ݶ��� */
		if(u8_uart_rxque_full[UART_PORT1] == TRUE)
		{
			Dummy = LPC_UART1->RBR;	/* ��ȡRBR���ݣ�����ж� */						  
		}
		else
		{
			
			(* u8_uart_rx_toppointer[UART_PORT1]) = LPC_UART1->RBR;
			Uart_increment_rx_top(UART_PORT1);	
		}
	}
	else if ( IIRValue == UART_IIR_CTI )	/* �ַ���ʱ */
	{
	
		u32_urat_status[UART_PORT1] |= 0x100;		
	}
	else if ( IIRValue == UART_IIR_THRE )	/* ���ͱ������� */
	{

		LSRValue = LPC_UART1->LSR;		

			/* ���ͻ���������δ�������ݣ���������� */		
		if(u8_uart_txque_empty[UART_PORT1] == FALSE)
		{
			SET_UART1_TX();//485T/R������Ϊ����
			LPC_UART1->THR = (* u8_uart_tx_bottompointer[UART_PORT1]);	
			Uart_increment_tx_bottom(UART_PORT1);	
	
		}
		else
		{
			SET_UART1_RX();//485T/R������Ϊ����						
		}


	}

}

/**************************************************************************************************
** ��������:		UART2Handler
** ��������:		����2�жϷ������
** �������:		��
** �������:		��
** ����ֵ:			��
**************************************************************************************************/

void UART2_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;
	
	IIRValue = LPC_UART2->IIR;
    
	IIRValue >>= 1;			
	IIRValue &= 0x07;			/* ȡ�жϱ�ʶ */
	if ( IIRValue == UART_IIR_RLS )	/* �ж��ж����� */
	{
		LSRValue = LPC_UART2->LSR;
		/* ������״̬ */
		if ( LSRValue & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_RXFE|UART_LSR_BI) )
		{
			
			u32_urat_status[UART_PORT2] = LSRValue;
			Dummy = LPC_UART2->RBR;		/*�������յ������ݣ�����ж�*/
								
			return;
		}
		if ( LSRValue & UART_LSR_RDR )/* RBR����δ��ȡ���ݶ����� */			
		{
		
			/* ���ջ������������ݶ��� */
	 		if(u8_uart_rxque_full[UART_PORT2] == TRUE)
			{
				Dummy = LPC_UART2->RBR;		/*�������յ������ݣ�����ж�*/				  
			}
			else
			{
				(* u8_uart_rx_toppointer[UART_PORT2]) = LPC_UART2->RBR;
				Uart_increment_rx_top(UART_PORT2);	
			}
	
		}
	}
	else if ( IIRValue == UART_IIR_RDA )	 //������������
	{
		/* ���ջ������������ݶ��� */
		if(u8_uart_rxque_full[UART_PORT2] == TRUE)
		{
			Dummy = LPC_UART2->RBR;		/* ��ȡRBR���ݣ�����ж� */					  
		}
		else
		{
			
			(* u8_uart_rx_toppointer[UART_PORT2]) = LPC_UART2->RBR;
			Uart_increment_rx_top(UART_PORT2);	
		}

	}
	else if ( IIRValue == UART_IIR_CTI )/* �ַ���ʱ */
	{

		u32_urat_status[UART_PORT2] |= 0x100;	
	}
	else if ( IIRValue == UART_IIR_THRE )	/* ���ͱ������� */
	{

		LSRValue = LPC_UART2->LSR;	

		/* ���ͻ���������δ�������ݣ���������� */	
		if(u8_uart_txque_empty[UART_PORT2] == FALSE)
		{
			SET_UART2_TX();//485T/R������Ϊ����
			LPC_UART2->THR = (* u8_uart_tx_bottompointer[UART_PORT2]);	
			Uart_increment_tx_bottom(UART_PORT2);
	
		}
		else
		{
			SET_UART2_RX();//485T/R������Ϊ����						
		}

	}

}


/**************************************************************************************************
** ��������:		Uart_varinit
** ��������:		���ڱ�����ʼ������
** �������:		portnum -- �豸�˿ں�(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			��
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
** ��������:		Uart_init
** ��������:		���ڳ�ʼ������
** �������:		portnum -- �豸�˿ں�(COM[0]��COM[1]��COM[2])
**              	baudrate -- ������
**            		frame -- ֡��ʽ(����λ[DATA_BITS_8��DATA_BITS_7��DATA_BITS_5]��
**                              	��żλ[EON��EOE��EOO��EO0��EO1]��
**                             	 	ֹͣλ[STOP_BIT_1��STOP_BIT_2]������Ϊ������)
** �������:		��
** ����ֵ:			��
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
			LPC_UART0->FCR = 0x07;		/* ����͸�λ TX �� RX FIFO. */		
		   	NVIC_EnableIRQ(UART0_IRQn);
		
		    LPC_UART0->IER = UART_IER_RBR | UART_IER_THRE | UART_IER_RLS;	/* ����UART0�ж� */
	
			SET_UART0_RX();//485T/R������Ϊ����	
		break;



		case 1:
			SET_UART1_TXRXDIR();
			
		 	Set_baudrate(portnum,baudrate);
			Set_frame(portnum,frame);
	
			LPC_UART1->FCR = 0x07;		/* ����͸�λ TX �� RX FIFO. */	
		   	NVIC_EnableIRQ(UART1_IRQn);
		
		    LPC_UART1->IER = UART_IER_RBR | UART_IER_THRE | UART_IER_RLS;	/* ����UART1�ж� */
	
			SET_UART1_RX();//485T/R������Ϊ����	
		break;

		case 2:
 			SET_UART2_TXRXDIR();

		 	Set_baudrate(portnum,baudrate);
			Set_frame(portnum,frame);
	
			LPC_UART2->FCR = 0x07;			/* ����͸�λ TX �� RX FIFO. */		
		   	NVIC_EnableIRQ(UART2_IRQn);
		
		    LPC_UART2->IER = UART_IER_RBR | UART_IER_THRE | UART_IER_RLS;	/* ����UART2�ж� */
	
			SET_UART2_RX();//485T/R������Ϊ����	
		break;
 
		default:
   		break;
	}
}


/**************************************************************************************************
** ��������:		Close_uartirq
** ��������:		�رմ����ж�
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			��
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
** ��������:		Open_uartirq
** ��������:		���������ж�
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			��
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
** ��������:		Set_frame
** ��������:		������·���ƼĴ���ֵ
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]), frame -- ��д����ֵ
** �������:		��
** ����ֵ:			��
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
** ��������:		Get_frame
** ��������:		��ȡ��·���ƼĴ���ֵ
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			LCR�Ĵ���ֵ
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
** ��������:		Set_Baudrate
** ��������:		���ò�����
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
** 					baudrate -- �����ʴ�С
** �������:		��
** ����ֵ:			��
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
			if(baudrate == 115200)	 //�������ʴ���57600ʱ����������Ĳ��������̫����Ҫʹ�ò�����С���Ĵ����������ڲ����ʴ���57600ʱ��ֻ֧��115200
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
** ��������:		Get_baudrate
** ��������:		��ȡ������
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			baudrate -- ������
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
			baudrate = 	( SystemCoreClock/4/16 )/fdiv;	   //	baudrate = 	( PFCLK/16 )/Fdiv;	 //PFCLK������ʱ��
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
** ��������:		Uart_recv
** ��������:		UART��������
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
**              	*buf -- ����������ݵ�ָ��, 
**              	maxlen -- �������ݵ���󳤶�
** �������:		��
** ����ֵ:			i -- ���յ����ݵ���Ŀ, -1 -- ����ʧ��
**************************************************************************************************/

int16_t Uart_recv( uint8_t portnum,uint8_t *buf,  uint16_t  maxlen)
{
	uint16_t length=0;
	int16_t	count=0;
	if(portnum >= UART_PORTNUM)
	{
		return 	-1;			
	}
	
	Close_uartirq(portnum);    //����ǰ���ж�,�����ж���ҲҪ���ʵ�����
	
	length =  u16_uart_rxque_num[portnum];	 //���ջ�����ʣ�µ��ֽ���
	if(length > maxlen)					 //������ջ������ֽ���������ȡ����ֽ��������ȡ��ȡ����ֽ���������ѻ�������ʣ�����ݶ���ȡ��
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
	
	Open_uartirq(portnum);    //�������ж�
	
	return 	count;	
}


/**************************************************************************************************
** ��������:		Uart_get_rxbuff_num
** ��������:		ȡ���ջ������ѽ��յ����ݳ���
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			���ջ������ѽ��յ����ݳ���
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
** ��������:		Uart_get_txbuff_num
** ��������:		ȡ���ͻ��������е����ݳ���
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2])
** �������:		��
** ����ֵ:			���ͻ��������е����ݳ���
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
** ��������:		Uart_send
** ��������:		UART��������
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
**              	*buf -- �������ݵ�ָ��, 
**              	length -- �������ݵĳ���
** �������:		��
** ����ֵ:			0 -- �ɹ�, 1 -- ʧ��
**************************************************************************************************/

uint16_t Uart_send( uint8_t portnum, uint8_t *buf, uint16_t length )
{
	if(portnum >= UART_PORTNUM)
	{
		return( FALSE ); 		
	}
	
	Close_uartirq(portnum);    //����ǰ���ж�,�����ж���ҲҪ���ʵ�����
	
	if(length > u16_uart_txque_avail[portnum] || length == 0)
	{
		Open_uartirq(portnum);    //�������ж�
		return( FALSE ); 		
	}
	while ( length != 0 )
	{	
		(* u8_uart_tx_toppointer[portnum]) = *buf;		//���ݽ� ���ͻ���������
		Uart_increment_tx_top(portnum);						//�����ָ�봦��
		buf++;
		length--;
	}

	switch(portnum)
	{
	 	case  UART_PORT0:
			if((u8_uart_txque_empty[portnum] == FALSE) && (LPC_UART0->LSR & UART_LSR_THRE))   //���ּĴ���Ϊ�ղ���д
			{
				SET_UART0_TX();//485T/R������Ϊ����
				LPC_UART0->THR = (* u8_uart_tx_bottompointer[portnum]);	
				Uart_increment_tx_bottom(portnum);				
			}
			break;

	 	case  UART_PORT1:
			if((u8_uart_txque_empty[portnum] == FALSE) && (LPC_UART1->LSR & UART_LSR_THRE))   //���ּĴ���Ϊ�ղ���д 
			{
				SET_UART1_TX();//485T/R������Ϊ����
				LPC_UART1->THR = (* u8_uart_tx_bottompointer[portnum]);	
				Uart_increment_tx_bottom(portnum);				
			}	
			break;

 	 	case  UART_PORT2:
			if((u8_uart_txque_empty[portnum] == FALSE) && (LPC_UART2->LSR & UART_LSR_THRE))   //���ּĴ���Ϊ�ղ���д 
			{
				SET_UART2_TX();//485T/R������Ϊ����
				LPC_UART2->THR = (* u8_uart_tx_bottompointer[portnum]);	
				Uart_increment_tx_bottom(portnum);				
			}	
			break;

		default : 
			break;

	}
	
	Open_uartirq(portnum);    //�������ж�
	
	return TRUE;
}

/**************************************************************************************************
** ��������:		Uart_increment_tx_bottom
** ��������:		UART�������ݻ������ĳ���ָ����㣬���п����жϡ�
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
** �������:		��
** ����ֵ:			��
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
** ��������:		Uart_increment_tx_top
** ��������:		UART�������ݻ����������ָ����㣬���п����жϡ�
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
** �������:		��
** ����ֵ:			��
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
** ��������:		Uart_increment_rx_top
** ��������:		UART�������ݻ����������ָ����㣬���п����жϡ�
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
** �������:		��
** ����ֵ:			��
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
** ��������:		Uart_increment_rx_top
** ��������:		UART�������ݻ������ĳ���ָ����㣬���п����жϡ�
** �������:		portnum -- �˿��豸��(COM[0]��COM[1]��COM[2]),
** �������:		��
** ����ֵ:			��
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
