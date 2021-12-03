/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Dataflash.h
��    ����1.00
�������ڣ�2012-03-12
��    �ߣ�������
����������AT45DB08����ͷ�ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-12  1.00     ����
**************************************************************/

#ifndef __DATAFLASH_H_
#define __DATAFLASH_H_

#include "Type.h"


#define DATAFLASH_SIZE (264*4096)


                                         //��ʼ��ַ            //�ռ��С(�ֽڣ�    //�������
#define DATAFLASH_MODEL_ADDR             0                      
#define DATAFLASH_FONT_ADDR              1                     //200*264*4          //�ֿ�
#define	DATAFLASH_CFG_DATA1_ADDR         (200*264*4)           //4*264*4            //��1����������
#define DATAFLASH_CFG_DATA2_ADDR         (204*264*4)           //4*264*4            //��2����������
#define DATAFLASH_CFG_DATA3_ADDR         (208*264*4)           //4*264*4            //��3����������
#define	DATAFLASH_ADJUST1_ADDR           (212*264*4)           //264                //��1��У׼����
#define	DATAFLASH_ADJUST2_ADDR           (212*264*4 + 264)     //264                //��2��У׼����
#define	DATAFLASH_ADJUST3_ADDR           (212*264*4 + 264*2)   //264                //��2��У׼����
#define	DATAFLASH_RESERVE1_ADDR          (212*264*4 + 264*3)   //264                //��1�ݱ������ݣ�Ϊ�˼����ϰ汾
#define	DATAFLASH_RESERVE2_ADDR          (212*264*4 + 264*4)   //264                //��2�ݱ������ݣ�Ϊ�˼����ϰ汾
#define DATAFLASH_RESERVE3_ADDR          (212*264*4 + 264*5)   //264                //��3�ݱ������ݣ�Ϊ�˼����ϰ汾
#define DATAFLASH_RECORD_ADDR            (212*264*4 + 264*6)   //4*264*4            //�¼���¼
#define DATAFLASH_HIS_FAULT_ADDR         (216*264*4 + 264*6)   //16*264*4           //��ʷ�澯��¼
#define DATAFLASH_NO_RESUME_FAULT_ADDR   (232*264*4 + 264*6)   //8*264*4            //��ʷĩ����澯��¼
#define DATAFLASH_CURR_FAULT_ADDR        (240*264*4 + 264*6)   //4*264*4            //��ǰ�澯��¼
#define DATAFLASH_BRANCH_NAME_ADDR       (244*264*4 + 264*6)   //30*264*4           //�Զ���֧·����
#define DATAFLASH_FAULT_NAME_ADDR        (274*264*4 + 264*6)   //240*264*4          //�Զ���澯����
#define DATAFLASH_YC_DATA_ADDR           (514*264*4 + 264*6)   //8*264*4            //�Զ���ң����
#define DATAFLASH_YX_DATA_ADDR           (522*264*4 + 264*6)   //20*264*4           //�Զ���ң�ŵ��
#define DATAFLASH_YK_DATA_ADDR           (542*264*4 + 264*6)   //264*4              //�Զ���ң�ص��
#define DATAFLASH_YT_DATA_ADDR           (542*264*4 + 264*10)  //264*4              //�Զ���ң�����
#define DATAFLASH_ABOUT_ADDR             (542*264*4 + 264*14)  //264*4              //�Զ��������Ϣ
#define DATAFLASH_SWT_ADDR               (542*264*4 + 264*18)  //2*264*4            //�Դ�������������Ϣ
#define DATAFLASH_ENCRY_ADDR             (DATAFLASH_SIZE-264)            //�����ַ���

#define DATAFLASH_ENCRY_SIZE             18             //�����ַ���ʵ�ʴ�С
#define DATAFLASH_ABOUT_ITEM_SIZE        44             //������Ŀʵ�ʴ�С
#define DATAFLASH_BRANCH_NAME_ITME_SIZE  40             //֧·����ʵ�ʴ�С
#define DATAFLASH_FAULT_NAME_ITEM_SIZE   80             //�澯����ʵ�ʴ�С
#define DATAFLASH_SWT_ITEM_SIZE          68             //������������Ŀ��С

#define DATAFLASH_BRANCH_NAME_SIZE       (64*11*DATAFLASH_BRANCH_NAME_ITME_SIZE)            //֧·����ʵ�ʴ�С
#define DATAFLASH_FAULT_NAME_SIZE        (2638*DATAFLASH_FAULT_NAME_ITEM_SIZE)              //�澯����ʵ�ʴ�С
#define DATAFLASH_SWT_SIZE               (20*DATAFLASH_SWT_ITEM_SIZE)                       //����������ʵ�ʴ�С

#define DATAFLASH_AC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 0)            //����������ʼ��ַ
#define DATAFLASH_DC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 20*80)        //ֱ��������ʼ��ַ
#define DATAFLASH_BATT1_GROUP_START_ADDR       (DATAFLASH_FAULT_NAME_ADDR + 52*80)        //1�����쳣��ʼ��ַ
#define DATAFLASH_BATT2_GROUP_START_ADDR       (DATAFLASH_FAULT_NAME_ADDR + 292*80)       //2�����쳣��ʼ��ַ
#define	DATAFLASH_BMS_GROUP_START_ADDR         (DATAFLASH_FAULT_NAME_ADDR + 532*80)       //���Ѳ�������ʼ��ַ
#define	DATAFLASH_RECT_GROUP_START_ADDR        (DATAFLASH_FAULT_NAME_ADDR + 542*80)       //����ģ�������ʼ��ַ
#define DATAFLASH_DCAC_GROUP_START_ADDR        (DATAFLASH_FAULT_NAME_ADDR + 662*80)       //���ģ�������ʼ��ַ
#define	DATAFLASH_DCDC_GROUP_START_ADDR        (DATAFLASH_FAULT_NAME_ADDR + 710*80)       //ͨ��ģ�������ʼ��ַ
#define DATAFLASH_FC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 734*80)       //����ģ�������ʼ��ַ
#define DATAFLASH_RC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 768*80)       //RC10ģ�������ʼ��ַ
#define DATAFLASH_SWT_GROUP_START_ADDR         (DATAFLASH_FAULT_NAME_ADDR + 784*80)       //�Դ�������������ʼ��ַ
#define	DATAFLASH_DC_PANEL1_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 836*80)       //�ֵ���01������ʼ��ַ
#define DATAFLASH_DC_PANEL2_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1029*80)      //�ֵ���02������ʼ��ַ
#define DATAFLASH_DC_PANEL3_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1222*80)      //�ֵ���03������ʼ��ַ
#define	DATAFLASH_DC_PANEL4_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1415*80)      //�ֵ���04������ʼ��ַ
#define DATAFLASH_DC_PANEL5_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1608*80)      //�ֵ���05������ʼ��ַ
#define DATAFLASH_DC_PANEL6_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1801*80)      //�ֵ���06������ʼ��ַ
#define	DATAFLASH_DC_PANEL7_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1994*80)      //�ֵ���07������ʼ��ַ
#define DATAFLASH_DC_PANEL8_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 2187*80)      //�ֵ���08������ʼ��ַ
#define DATAFLASH_DCDC_PANEL_GROUP_START_ADDR  (DATAFLASH_FAULT_NAME_ADDR + 2380*80)      //ͨ����������ʼ��ַ
#define DATAFLASH_DCAC_PANEL_GROUP_START_ADDR  (DATAFLASH_FAULT_NAME_ADDR + 2509*80)      //UPS��������ʼ��ַ
#define DATAFLASH_AC_PANEL_GROUP_START_ADDR    (DATAFLASH_FAULT_NAME_ADDR + 2638*80)      //������������ʼ��ַ

/*************************************************************
��������: v_flash_dataflash_init		           				
��������: ��ʼ��dataflash, ������dataflash��ص�spi��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_flash_dataflash_init(void);


/*************************************************************
��������: u8_flash_read_font_task_pin
��������: ��ȡ�ֿ���ͺ���д�������ŵ�״̬
�������: ��
�������: ��
����ֵ  ��1��������д��0����������д
**************************************************************/
U8_T u8_flash_read_font_task_pin(void);


/*************************************************************
��������: v_flash_dataflash_init_mutex		           				
��������: ��ʼ��dataflash��صĻ���������RTX����֮����ã��ڵ�һ������Ŀ�ͷ���ֵ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_flash_dataflash_init_mutex(void);

/*************************************************************
��������: s32_flash_dataflash_read		           				
��������: ��ȡdataflash����						
�������: u32_from -- Ҫ��ȡ�ĵ�ַ
		  u32_len  -- Ҫ��ȡ�ĳ���        		   				
�������: pu8_buf  -- ���ض�ȡ�������ݣ���������Ϊ������㹻�Ŀռ䣨���Ȳ�С��len)
����ֵ  ������0��ʾ�ɹ���������-1���ʾ����          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_read(U32_T u32_from, U8_T *pu8_buf, U32_T u32_len);

/*************************************************************
��������: s32_flash_dataflash_write		           				
��������: д���ݵ�dataflash						
�������: u32_to -- Ҫд��ĵ�ַ
		  u32_len  -- Ҫд�����ݵĳ���        		   				
          pu8_buf  -- ָ��Ҫд�������
�������: ��
����ֵ  ������0��ʾ�ɹ���������-1���ʾ����          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_write(U32_T u32_to, U8_T *pu8_buf, U32_T u32_len);


#endif
