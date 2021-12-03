/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����CAN.c
��    ����1.00
�������ڣ�2012-09-03
��    �ߣ�������
����������CAN����������ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-09-03  1.00     ����
**************************************************************/


#include <LPC17xx.h>                  /* LPC17xx definitions                 */
#include "CAN.h"



/* Values of bit time register for different baudrates
   NT = Nominal bit time = TSEG1 + TSEG2 + 3
   SP = Sample point     = ((1+TSEG1+1)/(1+TSEG1+1+TSEG2+1)) * 100%
                                            SAM,  SJW, TSEG1, TSEG2, NT,  SP */
const U32_T CAN_BIT_TIME[] = {          0, /*             not used             */
                                      0, /*             not used             */
                                      0, /*             not used             */
                                      0, /*             not used             */
                             0x0001C000, /* 0+1,  3+1,   1+1,   0+1,  4, 75% */
                                      0, /*             not used             */
                             0x0012C000, /* 0+1,  3+1,   2+1,   1+1,  6, 67% */
                                      0, /*             not used             */
                             0x0023C000, /* 0+1,  3+1,   3+1,   2+1,  8, 63% */
                                      0, /*             not used             */
                             0x0025C000, /* 0+1,  3+1,   5+1,   2+1, 10, 70% */
                                      0, /*             not used             */
                             0x0036C000, /* 0+1,  3+1,   6+1,   3+1, 12, 67% */
                                      0, /*             not used             */
                                      0, /*             not used             */
                             0x0048C000, /* 0+1,  3+1,   8+1,   4+1, 15, 67% */
                             0x0049C000, /* 0+1,  3+1,   9+1,   4+1, 16, 69% */
                           };


/************************* CAN Hardware Configuration ************************/

// *** <<< Use Configuration Wizard in Context Menu >>> ***

// <o> PCLK value (in Hz) <1-1000000000>
//     <i> Peripheral clock, depends on VPBDIV
//     <i> Default: 25000000
#define PCLK             18000000

// *** <<< End of Configuration section             >>> ***


/* CAN FIFO���� */
typedef struct {
	CAN_MSG_T t_msg[CAN_TX_MSG_LIST_MAX];    //��Ϣ����
	U32_T u32_read_index;                    //������
	U32_T u32_write_index;                   //д����
}CAN_TX_MSG_LIST_T;

typedef struct {
	CAN_MSG_T t_msg[CAN_RX_MSG_LIST_MAX];    //��Ϣ����
	U32_T u32_read_index;                    //������
	U32_T u32_write_index;                   //д����
}CAN_RX_MSG_LIST_T;

CAN_RX_MSG_LIST_T t_can1_rx_msg;             //CAN1����FIFO
CAN_TX_MSG_LIST_T t_can1_tx_msg;             //CAN1����FIFO
CAN_RX_MSG_LIST_T t_can2_rx_msg;             //CAN2����FIFO
CAN_TX_MSG_LIST_T t_can2_tx_msg;             //CAN2����FIFO

U32_T u32_can1_tx_interval = 0;              //CAN1���ͼ�������ʱ��Ϊu32_can1_tx_interval*CAN_TX_INTERVAL���� ��0��ʾû�з��ͼ��
U32_T u32_can2_tx_interval = 0;              //CAN2���ͼ�������ʱ��Ϊu32_can2_tx_interval*CAN_TX_INTERVAL���� ��0��ʾû�з��ͼ��


/*************************************************************
��������: v_can_set_baudrate
��������: ����CAN�������Ĳ�����
�������: ctrl     -- CAN������������
          baudrate -- ������
�������: ��
����ֵ  ����
**************************************************************/
static void v_can_set_baudrate(U32_T ctrl, U32_T baudrate)
{
	LPC_CAN_TypeDef *pCAN = (ctrl == 1) ? LPC_CAN1 : LPC_CAN2;  /* CAN ctrl    */
	U32_T result = 0;
	U32_T nominal_time;
	
	/* Determine which nominal time to use for PCLK */
	if (((PCLK / 1000000) % 6) == 0)
	{
		nominal_time = 12;                   /* PCLK based on  72MHz CCLK */
	} else
	{
		nominal_time = 10;                   /* PCLK based on 100MHz CCLK */
	}
	
	/* Prepare value appropriate for bit time register                         */
	result  = (PCLK / nominal_time) / baudrate - 1;
	result &= 0x000003FF;
	result |= CAN_BIT_TIME[nominal_time];
	
	pCAN->BTR  = result;                           /* Set bit timing           */
}

/*************************************************************
��������: v_can_init
��������: ��ʼ��CAN������
�������: ctrl     -- CAN������������
          baudrate -- ������
�������: ��
����ֵ  ����
**************************************************************/
void v_can_init(U32_T ctrl, U32_T baudrate)
{
	LPC_CAN_TypeDef *pCAN = (ctrl == 1) ? LPC_CAN1 : LPC_CAN2;  /* CAN ctrl    */
	
	if (ctrl == 1)
		t_can1_rx_msg.u32_read_index = t_can1_rx_msg.u32_write_index = 0;
	else
		t_can2_rx_msg.u32_read_index = t_can2_rx_msg.u32_write_index = 0;
	
	LPC_CANAF->AFMR = 2;                /* By default filter is not used       */
	pCAN->MOD       = 1;                /* Enter reset mode                    */
	pCAN->IER       = 0;                /* Disable all interrupts              */
	pCAN->GSR       = 0;                /* Clear status register               */
	v_can_set_baudrate(ctrl, baudrate); /* Set bit timing                      */
	pCAN->IER       = 0x0083;           /* Enable Tx and Rx and Bus error interrupt */
}

/*************************************************************
��������: v_can_start
��������: ʹ��CAN�������жϣ���ʼ���ݴ���
�������: ctrl     -- CAN������������
�������: ��
����ֵ  ����
**************************************************************/
void v_can_start(U32_T ctrl)
{
	LPC_CAN_TypeDef *pCAN = (ctrl == 1) ? LPC_CAN1 : LPC_CAN2;  /* CAN ctrl    */

	NVIC_EnableIRQ(CAN_IRQn);           /* Enable CAN interrupt        */
	pCAN->MOD = 0;                      /* Enter normal operating mode         */
}

/*************************************************************
��������: v_can_hw_wr
��������: д��Ϣ��CAN��������Ӳ���Ĵ���
�������: ctrl -- CAN������������
          msg  -- ָ��Ҫ��д�����Ϣ
�������: ��
����ֵ  ����
**************************************************************/
static void v_can_hw_wr(U32_T ctrl, CAN_MSG_T *msg)
{
	LPC_CAN_TypeDef *pCAN = (ctrl == 1) ? LPC_CAN1 : LPC_CAN2;  /* CAN ctrl    */
	U32_T CANData;
	
	CANData       = (((U32_T) msg->len) << 16) & 0x000F0000 | 
	                (msg->format == 1) * 0x80000000 |
	                (msg->type   == 1) * 0x40000000;
	
	if (pCAN->SR & 0x00000004)          /* Transmit buffer 1 free              */
	{
		pCAN->TFI1  = CANData;            /* Write frame informations            */
		pCAN->TID1 = msg->id;             /* Write CAN message identifier        */
		pCAN->TDA1 = *(U32_T *) &msg->data[0];      /* Write first 4 data bytes    */
		pCAN->TDB1 = *(U32_T *) &msg->data[4];      /* Write second 4 data bytes   */
		//pCAN->CMR  = 0x31;              /* Select Tx1 for Self Tx/Rx           */
		pCAN->CMR  = 0x21;                /* Start transmission without loop-back*/
	}
}


/*************************************************************
��������: v_can_hw_wr
��������: ��CAN��������Ӳ���Ĵ�����ȡ��Ϣ
�������: ctrl -- CAN������������
�������: msg  -- ָ������������Ϣ�Ļ�����
����ֵ  ����
**************************************************************/
static void v_can_hw_rd(U32_T ctrl, CAN_MSG_T *msg)
{
	LPC_CAN_TypeDef *pCAN = (ctrl == 1) ? LPC_CAN1 : LPC_CAN2;  /* CAN ctrl    */
	U32_T CANData;
	U32_T *CANAddr;
	
	/* Read frame informations                                                 */
	CANData = pCAN->RFS;
	msg->format   = (CANData & 0x80000000) == 0x80000000;
	msg->type     = (CANData & 0x40000000) == 0x40000000;
	msg->len      = ((U8_T)(CANData >> 16)) & 0x0F;
	
	/* Read CAN message identifier                                             */
	msg->id = pCAN->RID;
	
	/* Read the data if received message was DATA FRAME                        */
	if (msg->type == 0)
	{
		/* Read first 4 data bytes                                               */
		CANAddr = (U32_T *) &msg->data[0];
		*CANAddr++ = pCAN->RDA;
		
		/* Read second 4 data bytes                                              */
		*CANAddr   = pCAN->RDB;
	}
}

/*************************************************************
��������: CAN_IRQHandler
��������: CAN���������жϴ�����
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void CAN_IRQHandler (void)  {
	CAN_MSG_T t_msg;
	U32_T run1, run2, pconp;
	U32_T icr1 = 0, icr2 = 0;
	
	/* Check which channels are running                                        */
	pconp = LPC_SC->PCONP;
	run1  = (pconp & (1 << 13)) != 0;
	run2  = (pconp & (1 << 14)) != 0;
	
	/* If message is received and if mailbox isn't full read message from 
		hardware and send it to message queue                                   */
	if (run1) {
		icr1 = LPC_CAN1->ICR;
		if (icr1 & (1 << 0)) {
			if (((t_can1_rx_msg.u32_write_index+1)%CAN_RX_MSG_LIST_MAX) != t_can1_rx_msg.u32_read_index)
			{
				v_can_hw_rd(1, &t_msg);
				t_can1_rx_msg.t_msg[t_can1_rx_msg.u32_write_index] = t_msg;
				t_can1_rx_msg.u32_write_index = ((t_can1_rx_msg.u32_write_index+1)%CAN_RX_MSG_LIST_MAX);
			}
			else
			{
				icr1 = icr1;
			}
			
			LPC_CAN1->CMR = 0x04;           /* Release receive buffer            */
		}
	}

	if (run2) {
		icr2 = LPC_CAN2->ICR;
		if (icr2 & (1 << 0)) {
    		if (((t_can2_rx_msg.u32_write_index+1)%CAN_RX_MSG_LIST_MAX) != t_can2_rx_msg.u32_read_index)
			{
				v_can_hw_rd(2, &t_msg);
				t_can2_rx_msg.t_msg[t_can2_rx_msg.u32_write_index] = t_msg;
				t_can2_rx_msg.u32_write_index = ((t_can2_rx_msg.u32_write_index+1)%CAN_RX_MSG_LIST_MAX);
			}
			
			LPC_CAN2->CMR = 0x04;           /* Release receive buffer            */
		}
	}


  /* If there is message in mailbox ready for send, and if transmission
     hardware is ready, read the message from mailbox and send it            */
	if (run1)
	{
		if ((icr1 & (1 << 1)) && (u32_can1_tx_interval==0)) {
			if (t_can1_tx_msg.u32_write_index != t_can1_tx_msg.u32_read_index)
			{
				t_msg = t_can1_tx_msg.t_msg[t_can1_tx_msg.u32_read_index];
				t_can1_tx_msg.u32_read_index = ((t_can1_tx_msg.u32_read_index+1)%CAN_TX_MSG_LIST_MAX);
				v_can_hw_wr(1, &t_msg);
			}
		}
	}

	if (run2)
	{
		if ((icr2 & (1 << 1)) && (u32_can2_tx_interval==0)) {
			if (t_can2_tx_msg.u32_write_index != t_can2_tx_msg.u32_read_index)
			{
				t_msg = t_can2_tx_msg.t_msg[t_can2_tx_msg.u32_read_index];
				t_can2_tx_msg.u32_read_index = ((t_can2_tx_msg.u32_read_index+1)%CAN_TX_MSG_LIST_MAX);
				v_can_hw_wr(2, &t_msg);
			}
		}
	}

	/* If bus-off occur, clear RM bit of CANxMOD */
	if (run1)
	{
		if (icr1 & (1 << 7)) {
			if (LPC_CAN1->GSR & 0x80)
				LPC_CAN1->MOD = 0;                      /* Enter normal operating mode */
		}
	}

	if (run2)
	{
		if (icr2 & (1 << 7)) {
			if (LPC_CAN2->GSR & 0x80)
				LPC_CAN2->MOD = 0;                      /* Enter normal operating mode */
		}
	}
}

/*************************************************************
��������: u32_can_receive
��������: CAN������Ϣ���������ڴ������ж�ȡ���յ�����Ϣ
�������: u32_ctrl -- CAN������������
�������: pt_msg   -- ָ�򷵻ص���Ϣ
����ֵ  ��0���ɹ���pt_msgָ�򷵻ص���Ϣ��1��ʧ�ܣ�����Ϣ�ɶ�
**************************************************************/
U32_T u32_can_receive(U32_T u32_ctrl, CAN_MSG_T *pt_msg)
{
	U32_T u32_ret;
	CAN_RX_MSG_LIST_T *pt_rx_msg_list = (u32_ctrl == 1) ? &t_can1_rx_msg : &t_can2_rx_msg;
	LPC_CAN_TypeDef *pCAN = (u32_ctrl == 1) ? LPC_CAN1 : LPC_CAN2;
	
	//NVIC_DisableIRQ(CAN_IRQn);              /* Disable CAN interrupt     */
	pCAN->IER       = 0;                      /* Disable interrupt */
	if (pt_rx_msg_list->u32_write_index == pt_rx_msg_list->u32_read_index)
	{
		u32_ret = 1;
	}
	else
	{
		u32_ret = 0;
		*pt_msg = pt_rx_msg_list->t_msg[pt_rx_msg_list->u32_read_index];
		pt_rx_msg_list->u32_read_index = ((pt_rx_msg_list->u32_read_index+1)%CAN_RX_MSG_LIST_MAX);
	}
	//NVIC_EnableIRQ(CAN_IRQn);             /* Enable CAN interrupt        */
	pCAN->IER       = 0x0083;               /* Enable Tx and Rx and Bus error interrupt */

	if (pCAN->MOD != 0)
		pCAN->MOD = 0;                      /* Enter normal operating mode */
	
	return u32_ret;
}

/*************************************************************
��������: u32_can_send
��������: CAN������Ϣ����
�������: u32_ctrl -- CAN������������
          pt_msg   -- ָ��Ҫ���͵���Ϣ
�������: ��
����ֵ  ��0���ɹ���1��ʧ�ܣ�����������û�пռ��ˣ�Ӧ�ó�������Ժ�����
**************************************************************/
U32_T u32_can_send(U32_T u32_ctrl, CAN_MSG_T *pt_msg)
{
	U32_T u32_ret, u32_interval;
	CAN_TX_MSG_LIST_T *pt_tx_msg_list = (u32_ctrl == 1) ? &t_can1_tx_msg : &t_can2_tx_msg;
	LPC_CAN_TypeDef *pCAN = (u32_ctrl == 1) ? LPC_CAN1 : LPC_CAN2;
	
	NVIC_DisableIRQ(TIMER1_IRQn);
	//NVIC_DisableIRQ(CAN_IRQn);              /* Disable CAN interrupt     */
	pCAN->IER       = 0;                      /* Disable interrupt */
	
	u32_interval = (u32_ctrl == 1) ? u32_can1_tx_interval : u32_can2_tx_interval;

	if ((u32_interval==0) && (pCAN->SR&0x00000004))
	{
		u32_ret = 0;
		v_can_hw_wr (u32_ctrl, pt_msg);
	}
	else
	{
		if (((pt_tx_msg_list->u32_write_index+1)%CAN_TX_MSG_LIST_MAX) == pt_tx_msg_list->u32_read_index)
		{
			u32_ret = 1;
		}
		else
		{
			u32_ret = 0;
			pt_tx_msg_list->t_msg[pt_tx_msg_list->u32_write_index] = *pt_msg;
			pt_tx_msg_list->u32_write_index = ((pt_tx_msg_list->u32_write_index+1)%CAN_TX_MSG_LIST_MAX);
		}
	}

	//NVIC_EnableIRQ(CAN_IRQn);             /* Enable CAN interrupt        */
	pCAN->IER       = 0x0083;               /* Enable Tx and Rx and Bus error interrupt */
	NVIC_EnableIRQ(TIMER1_IRQn);

	if (pCAN->MOD != 0)
		pCAN->MOD = 0;                      /* Enter normal operating mode */
	
	return u32_ret;
}

/*************************************************************
��������: v_can_timer_init
��������: ��ʼ��TIMER1��10MS�ж�һ�Σ�����һ��CAN���TIMER1����CAN��������������������ʹ��TIMER1
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_can_timer_init(void)
{
	LPC_TIM1->TCR = 0;	          // disable timer
	LPC_TIM1->TC = 0;
	LPC_TIM1->PC = 0;
	LPC_TIM1->PR = 17;            // pclk=18MHZ, prescale=17, ��Ƶ��Ϊ1MHZ
	LPC_TIM1->MR0 = CAN_TX_INTERVAL*1000;
	LPC_TIM1->MCR = 0x00000003;
	LPC_TIM1->CCR = 0;
	LPC_TIM1->CTCR = 0;           // timer mode
	LPC_TIM1->IR = 0x0000003F;    // clear interrrupt flag
	LPC_TIM1->TCR = 1;            // enable timer
	NVIC_EnableIRQ(TIMER1_IRQn);
}

/*************************************************************
��������: v_can_set_tx_interval
��������: ����CAN�ڷ��ͼ��
�������: u32_ctrl     -- CAN������������ 
          u32_interval -- ���ͼ���������ʱ��Ϊu32_interval*CAN_TX_INTERVAL���� ��0��ʾû�з��ͼ��
�������: ��
����ֵ  ����
**************************************************************/
void v_can_set_tx_interval(U32_T u32_ctrl, U32_T u32_interval)
{
	NVIC_DisableIRQ(TIMER1_IRQn);

	if (u32_ctrl == 1)
		u32_can1_tx_interval = u32_interval;
	else
		u32_can2_tx_interval = u32_interval; 

	NVIC_EnableIRQ(TIMER1_IRQn);
}

/*************************************************************
��������: TIMER1_IRQHandler
��������: TIMER1���жϴ�����
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void TIMER1_IRQHandler(void)
{
	static U32_T u32_can1_cnt = 0;
	static U32_T u32_can2_cnt = 0;
	CAN_MSG_T t_msg;

	if (u32_can1_tx_interval != 0)
	{
		if (++u32_can1_cnt >= u32_can1_tx_interval)
		{
			if ((LPC_CAN1->SR & 0x00000004)
				&& (t_can1_tx_msg.u32_write_index != t_can1_tx_msg.u32_read_index))
			{
				t_msg = t_can1_tx_msg.t_msg[t_can1_tx_msg.u32_read_index];
				t_can1_tx_msg.u32_read_index = ((t_can1_tx_msg.u32_read_index+1)%CAN_TX_MSG_LIST_MAX);
				v_can_hw_wr(1, &t_msg);
			}

			u32_can1_cnt = 0;
		}
	}
	
	if (u32_can2_tx_interval != 0)
	{
		if (++u32_can2_cnt >= u32_can2_tx_interval)
		{
			if ((LPC_CAN2->SR & 0x00000004)
				&& (t_can2_tx_msg.u32_write_index != t_can2_tx_msg.u32_read_index))
			{
				t_msg = t_can2_tx_msg.t_msg[t_can2_tx_msg.u32_read_index];
				t_can2_tx_msg.u32_read_index = ((t_can2_tx_msg.u32_read_index+1)%CAN_TX_MSG_LIST_MAX);
				v_can_hw_wr(2, &t_msg);
			}

			u32_can2_cnt = 0;
		}
	}
	
	LPC_TIM1->IR = 0x0000003F;    // clear interrrupt flag
}
