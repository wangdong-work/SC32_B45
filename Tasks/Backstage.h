/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����backstage.h
��    ����1.00
�������ڣ�2012-06-14
��    �ߣ�������
������������̨ͨ��ͷ�ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-06-14  1.00     ����
***********************************************************/

#ifndef __BACKSTAGE_H_
#define __BACKSTAGE_H_

#include "Type.h"
#include "PublicData.h"
#include "BackstageTable.h"

#define BS_COM_PORT_NUM             0      //���ڶ˿ں�
#define BS_SEND_BUF_SIZE            512    //���ͻ�������С
#define BS_RECV_BUF_SIZE            256    //���ջ�������С

#define BS_YC_FEEDER_SIZE         	2048      //֧·ң�����ݸ���
#define BS_YX_FEEDER_SIZE         	(2048*3)  //֧·ң�����ݸ���
#define BS_YX_FEEDER_ALARM_INDEX  	0         //֧·��բ����
#define BS_YX_FEEDER_STATE_INDEX  	(2048/16) //֧·״̬����
#define BS_YX_FEEDER_INSU_INDEX   	(4096/16) //֧·��Ե����


extern U8_T                        g_u8_addr;                   //��̨ͨ�ű�����ַ����
extern COM_BAUD_E                  g_e_baud;                    //��̨ͨ�Ų����ʱ���

extern U8_T g_u8_bs_send_buf[BS_SEND_BUF_SIZE];                 //���ͻ�����
extern U8_T g_u8_bs_recv_buf[BS_RECV_BUF_SIZE];                 //���ջ�����
extern U8_T g_u8_bs_recv_len;                                   //���ջ����������ݵĳ���

extern S16_T g_s16_bs_modbus_yc[BS_YC_SIZE];                    //MODBUSң��������
extern U16_T g_u16_bs_mdobus_yc_size;                           //MODBUSң��������ʵ�ʴ�С
extern U16_T g_u16_bs_cdt_yc[BS_YC_SIZE];                       //CDTң��������
extern U16_T g_u16_bs_cdt_yc_size;                              //CDTң��������ʵ�ʴ�С
extern S16_T g_s16_bs_modbus_yc_feeder[BS_YC_FEEDER_SIZE];      //MODBUS֧·ң��������

extern U16_T g_u16_bs_yx[BS_YX_SIZE/16+1];                      //ң��������
extern U16_T g_u16_bs_yx_size;                                  //ң��������ʵ�ʴ�С
extern U32_T g_u32_bs_yx_max_addr;                              //ң������ַ
extern U16_T g_u16_bs_yx_feeder[BS_YX_FEEDER_SIZE/16+1];        //֧·ң��������

extern U16_T g_u16_bs_yk[BS_YK_SIZE];                           //ң��������
extern U16_T g_u16_bs_yk_size;                                  //ң��������ʵ�ʴ�С

extern U16_T g_u16_bs_yt[BS_YT_SIZE];                           //ң��������
extern U16_T g_u16_bs_yt_size;                                  //ң��������ʵ�ʴ�С



/*************************************************************
��������: u32_bs_calculate_time		           				
��������: ���㴮����byte_cnt���ֽ�����ʱ��						
�������: byte_cnt -- �ֽ���
          baud     -- ������        		   				
�������: ��
����ֵ  ����������ʱ�䣬��λΪms														   				
**************************************************************/
U32_T u32_bs_calculate_time(U32_T byte_cnt, COM_BAUD_E baud);


/*************************************************************
��������: v_bs_send_data		           				
��������: ͨ�����ڷ�������						
�������: pu8_buf -- ָ��Ҫ���͵�������
          len     -- Ҫ���͵��ֽ���        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_bs_send_data(U8_T *pu8_buf, U32_T len);


/*************************************************************
��������: s32_bs_yk_handle		           				
��������: ��̨ң�ش�����						
�������: addr -- ң�ص�ַ
          val  -- Ҫ���õ���ֵ       		   				
�������: ��
����ֵ  ��0:���óɹ���-1������ʧ��														   				
**************************************************************/
S32_T s32_bs_yk_handle(U16_T addr, U16_T val);


/*************************************************************
��������: s32_bs_yt_handle		           				
��������: ��̨ң��������						
�������: addr -- ң����ַ
          val  -- Ҫ���õ���ֵ       		   				
�������: ��
����ֵ  ��0:���óɹ���-1������ʧ��														   				
**************************************************************/
S32_T s32_bs_yt_handle(U16_T addr, U16_T val);


/*************************************************************
��������: v_backstage_task		           				
��������: ��̨ͨ������������ȫ�ֹ�������������ȡ��̨�������������̨��ص�ͨ��						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
__task void v_bs_backstage_task(void);

#endif
