/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����FaultId.h
��    ����1.00
�������ڣ�2012-05-15
��    �ߣ�������
��������������ID�궨��ͷ�ļ�


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-05-15  1.00     ����
**************************************************************/

#ifndef __FAULT_ID_H_
#define __FAULT_ID_H_


/* ����� */
#define FAULT_GROUP_OFFSET     10        //�����ƫ��
#define FAULT_AC_GROUP         0         //����
#define	FAULT_DC_BUS_GROUP     1         //ֱ��ĸ��
#define FAULT_BATT1_GROUP      2         //1�����쳣
#define FAULT_BATT2_GROUP      3         //2�����쳣
#define	FAULT_BMS_GROUP        4         //���Ѳ�����
#define	FAULT_RECT_GROUP       5         //����ģ�����
#define FAULT_DCAC_GROUP       6         //���ģ�����
#define	FAULT_DCDC_GROUP       7         //ͨ��ģ�����
#define FAULT_FC_GROUP         8         //����ģ�����
#define FAULT_RC_GROUP         9         //RC10ģ�����
#define FAULT_SWT_GROUP        10        //�Դ�����������
#define	FAULT_DC_PANEL1_GROUP  11        //�ֵ���01����
#define FAULT_DC_PANEL2_GROUP  12        //�ֵ���02����
#define FAULT_DC_PANEL3_GROUP  13        //�ֵ���03����
#define	FAULT_DC_PANEL4_GROUP  14        //�ֵ���04����
#define FAULT_DC_PANEL5_GROUP  15        //�ֵ���05����
#define FAULT_DC_PANEL6_GROUP  16        //�ֵ���06����
#define	FAULT_DC_PANEL7_GROUP  17        //�ֵ���07����
#define FAULT_DC_PANEL8_GROUP  18        //�ֵ���08����
#define FAULT_DCDC_PANEL_GROUP 19        //ͨ��������
#define FAULT_DCAC_PANEL_GROUP 20        //UPS������
#define FAULT_AC_PANEL_GROUP   21        //����������


/* ����������� */
#define FAULT_AC_PATH1_OFF         0     //ֱ��ϵͳ����һ·ͣ��
#define FAULT_AC_PATH1_UNDER_VOLT  1     //ֱ��ϵͳ����һ·Ƿѹ
#define FAULT_AC_PATH1_OVER_VOLT   2     //ֱ��ϵͳ����һ·��ѹ
#define FAULT_AC_PATH1_LACK_PHASE  3     //ֱ��ϵͳ����һ·ȱ��
#define FAULT_AC_PATH2_OFF         4     //ֱ��ϵͳ������·ͣ��
#define FAULT_AC_PATH2_UNDER_VOLT  5     //ֱ��ϵͳ������·Ƿѹ
#define FAULT_AC_PATH2_OVER_VOLT   6     //ֱ��ϵͳ������·��ѹ
#define FAULT_AC_PATH2_LACK_PHASE  7     //ֱ��ϵͳ������·ȱ��
#define FAULT_AC2_PATH1_OFF         8    //����ϵͳ����һ·ͣ��
#define FAULT_AC2_PATH1_UNDER_VOLT  9    //����ϵͳ����һ·Ƿѹ
#define FAULT_AC2_PATH1_OVER_VOLT   10   //����ϵͳ����һ·��ѹ
#define FAULT_AC2_PATH1_LACK_PHASE  11   //����ϵͳ����һ·ȱ��
#define FAULT_AC2_PATH2_OFF         12   //����ϵͳ������·ͣ��
#define FAULT_AC2_PATH2_UNDER_VOLT  13   //����ϵͳ������·Ƿѹ
#define FAULT_AC2_PATH2_OVER_VOLT   14   //����ϵͳ������·��ѹ
#define FAULT_AC2_PATH2_LACK_PHASE  15   //����ϵͳ������·ȱ��
#define FAULT_AC_FC1_COMM_FAIL      16   //������1#����ģ��ͨ���ж�
#define FAULT_AC_FC2_COMM_FAIL      17   //������2#����ģ��ͨ���ж�
#define FAULT_AC_FC3_COMM_FAIL      18   //������3#����ģ��ͨ���ж�
#define FAULT_AC_NUM               19    //���������������


/* ֱ��ĸ�߷������ */
#define FAULT_DC_PB_OVER_VOLT        0     //һ�κ�ĸ��ѹ     
#define FAULT_DC_PB_UNDER_VOLT       1     //һ�κ�ĸǷѹ
#define FAULT_DC_CB_OVER_VOLT        2     //һ�ο�ĸ��ѹ
#define FAULT_DC_CB_UNDER_VOLT       3     //һ�ο�ĸǷѹ
#define FAULT_DC_BUS_OVER_VOLT       4     //һ��ĸ�߹�ѹ
#define FAULT_DC_BUS_UNDER_VOLT      5     //һ��ĸ��Ƿѹ
#define FAULT_BATT_OVER_VOLT         6     //һ�ε�ع�ѹ
#define FAULT_BATT_UNDER_VOLT        7     //һ�ε��Ƿѹ
#define FAULT_DC10_COMM_FAIL         8     //1#DC10ģ��ͨ���ж�//�����������쳣
#define FAULT_BATT1_OVER_CURR        9     //һ���ع���
#define FAULT_BATT2_OVER_CURR        10    //�����ع���
#define FAULT_DC_BUS_CURR_IMBALANCE  11    //һ��ĸ�ߵ�����ƽ��
#define FAULT_DC_BUS_VOLT_IMBALANCE  12    //һ��ĸ�ߵ�ѹ��ƽ��
#define FAULT_DC_BUS_INSU_FAULT      13    //һ��ĸ�߾�Ե�½�
#define FAULT_INSU_RELAY_FAULT       14    //һ�ξ�Ե�̵�������
#define FAULT_DC_BUS_INPUT_AC        15    //һ��ֱ��ĸ�ߴ��뽻���澯

#define FAULT_DC_PB2_OVER_VOLT       16    //���κ�ĸ��ѹ     
#define FAULT_DC_PB2_UNDER_VOLT      17    //���κ�ĸǷѹ
#define FAULT_DC_CB2_OVER_VOLT       18    //���ο�ĸ��ѹ
#define FAULT_DC_CB2_UNDER_VOLT      19    //���ο�ĸǷѹ
#define FAULT_DC_BUS2_OVER_VOLT      20    //����ĸ�߹�ѹ
#define FAULT_DC_BUS2_UNDER_VOLT     21    //����ĸ��Ƿѹ
#define FAULT_BATT2_OVER_VOLT        22    //���ε�ع�ѹ
#define FAULT_BATT2_UNDER_VOLT       23    //���ε��Ƿѹ
#define FAULT_DC_BUS2_CURR_IMBALANCE 24    //����ĸ�ߵ�����ƽ��
#define FAULT_DC_BUS2_VOLT_IMBALANCE 25    //����ĸ�ߵ�ѹ��ƽ��
#define FAULT_DC_BUS2_INSU_FAULT     26    //����ĸ�߾�Ե�½�
#define FAULT_INSU2_RELAY_FAULT      27    //���ξ�Ե�̵�������
#define FAULT_DC_BUS2_INPUT_AC       28    //����ֱ��ĸ�ߴ��뽻���澯
#define FAULT_AC10_COMM_FAIL         29    //2#DC10ģ��ͨ���ж�
                                           //30~31Ԥ��
#define FAULT_DC_NUM                 30    //ֱ��ĸ�߷����������


/* һ�����쳣��� */
/*
	0~119��120�ڵ����ع�ѹ
	120~239��120�ڵ�����Ƿѹ
*/
#define FAULT_BATT1_NUM                240    //һ���ع�������


/* �������쳣��� */
/*
	0~119��120�ڵ����ع�ѹ
	120~239��120�ڵ�����Ƿѹ
*/
#define FAULT_CELL_OVER_VOLT_BASE_NUM  0      //�����ع�ѹ��ʼ���
#define FAULT_CELL_UNDER_VOLT_BASE_NUM 120    //������Ƿѹ��ʼ���
#define FAULT_BATT2_NUM                240    //һ���ع�������


/* ���Ѳ�������� */
/*
	0~4��һ���ص�5�����Ѳ��ͨ���ж�
	5~9�������ص�5�����Ѳ��ͨ���ж�
*/
#define FAULT_BMS_NUM            10    //���Ѳ���������


/* ����ģ�������� */
/*
	0~4��1������ģ��ͨѶ�жϡ����¡������쳣����ѹ������ģ�����
	5~10��2������ģ��ͨѶ�жϡ����¡������쳣����ѹ������ģ�����
	...
*/
#define FAULT_RECT_COMM_FAIL          0     //����ģ��ͨ���ж�
#define FAULT_RECT_VOER_TEMPERATURE   1     //����ģ�����
#define FAULT_RECT_AC_EXCEPTION       2     //����ģ�齻���쳣
#define FAULT_RECT_OVER_VOLT_PROTECT  3     //����ģ���ѹ����
#define FAULT_RECT_FAULT              4     //����ģ�����
#define FAULT_RECT_CNT                5     //����ģ���������
#define FAULT_RECT_NUM                120   //����ģ������������


/* ���ģ�������� */
/*
	0~5��1�����ģ��ͨѶ�жϡ�ģ����ϡ����ء����¡����Ƿѹ����·�쳣
	6~11��2�����ģ��ͨѶ�жϡ�ģ����ϡ����ء����¡����Ƿѹ����·�쳣
	...
*/
#define FAULT_DCAC_COMM_FAIL          0     //���ģ��ͨ���ж�
#define FAULT_DCAC_FAULT              1     //���ģ�����
#define FAULT_DCAC_VOERLOAD           2     //���ģ�����
#define FAULT_DCAC_VOER_TEMPERATURE   3     //���ģ�����
#define FAULT_DCAC_BATT_UNDERVOLT     4     //���ģ����Ƿѹ
#define FAULT_DCAC_BYPASS_EXCEPTION   5     //���ģ����·�쳣
#define FAULT_DCAC_CNT                6     //���ģ���������
#define FAULT_DCAC_NUM                48    //���ģ������������




/* ͨ��ģ�������� */
/*
	0~2��1��ͨ��ģ��ͨѶ�жϡ�������ģ�����
	3~5��2��ͨ��ģ��ͨѶ�жϡ�������ģ�����
	...
*/
#define FAULT_DCDC_COMM_FAIL          0     //ͨ��ģ��ͨ���ж�
#define FAULT_DCDC_PROTECT            1     //ͨ��ģ�鱣��
#define FAULT_DCDC_FAULT              2     //ͨ��ģ�����
#define FAULT_DCDC_CNT                3     //ͨ��ģ���������
#define FAULT_DCDC_NUM                24    //ͨ��ģ������������


/* ����ģ��ͨ���ж� */
/*
	0~3��1#�����1#~4#����ģ��ͨ���ж�
	...
	28~31��8#�����1#~4#����ģ��ͨ���ж�
	32: ͨ��������ģ��ͨ���ж�
	33��UPS������ģ��ͨ���ж�
*/
#define FAULT_DCDC_FC_COMM_FAIL 32
#define FAULT_DCAC_FC_COMM_FAIL 33
#define FAULT_FC_COMM_FIAL_NUM  34    //����ģ��ͨ���жϹ�������

/* RC10ģ��ͨ���ж� */
/*
	0~15��1#~16#RC10ģ��ͨ���ж�
*/
#define FAULT_RC10_COMM_FAIL    0     //1#RC10ģ��ͨ���ж�
#define FAULT_RC_COMM_FIAL_NUM  16    //RCģ��ͨ���жϹ�������


/* �Դ����������� */
/*
	 0~19��SC32�Բ�1~20������բ
	20~35��1#DC10�Բ�1~16������բ
	36~51��2#DC10�Բ�1~16������բ
*/
#define FAULT_SC32_SWT_BEGIN	0     //SC32�Դ�������բ��ʼ����
#define FAULT_DC10_SWT_BEGIN	20    //1#DC10������բ��ʼ����
#define FAULT_AC10_SWT_BEGIN	36    //2#DC10������բ��ʼ����
#define FAULT_SWT_NUM           (20+16+16)    //�Դ���������������

/* �ֵ���01������� */
/* �ֵ���02������� */
/* �ֵ���03������� */
/* �ֵ���04������� */
/* �ֵ���05������� */
/* �ֵ���06������� */
/* �ֵ���07������� */
/* �ֵ���08������� */
/*
	0~2��1#֧·������բ����Ե�½����������쳣
	3~5��2#֧·������բ����Ե�½����������쳣
	...
	189~191��64#֧·������բ����Ե�½����������쳣
*/
#define FAULT_FEEDER_TOATL_SWT_FAULT  0     //�ܿ��ع���
#define FAULT_FEEDER_BRANCH_BASE      1     //֧·������ʼ���
#define FAULT_FEEDER_BRANCH_SWT       0     //������բ
#define FAULT_FEEDER_BRANCH_INSU      1     //��Ե�½�
#define FAULT_FEEDER_BRANCH_SENSOR    2     //�������쳣
#define FAULT_FEEDER_BRANCH_FAULT_CNT 3     //ÿ������֧·�Ĺ�������
#define FAULT_PANEL_BRANCH_NUM        64    //ÿ���ֵ��������֧·��
#define FAULT_DC_PANEL_NUM            193   //�������������


/* ͨ����07������� */
/* UPS��08������� */
/*
	0~1��1#֧·������բ���������쳣
	2~3��2#֧·������բ���������쳣
	...
	126~127��64#֧·������բ���������쳣
*/
#define FAULT_DCDC_TOATL_SWT_FAULT    0     //�ܿ��ع���
#define FAULT_DCDC_BRANCH_BASE        1     //֧·������ʼ���
#define FAULT_DCDC_BRANCH_SWT         0     //������բ
#define FAULT_DCDC_BRANCH_SENSOR      1     //�������쳣
#define FAULT_DCDC_BRANCH_FAULT_CNT   2     //ÿ������֧·�Ĺ�������
#define FAULT_DCDC_PANEL_NUM          129   //��������

#define FAULT_DCAC_TOATL_SWT_FAULT    0     //�ܿ��ع���
#define FAULT_DCAC_BRANCH_BASE        1     //֧·������ʼ���
#define FAULT_DCAC_BRANCH_SWT         0     //������բ
#define FAULT_DCAC_BRANCH_SENSOR      1     //�������쳣
#define FAULT_DCAC_BRANCH_FAULT_CNT   2     //ÿ������֧·�Ĺ�������
#define FAULT_DCAC_PANEL_NUM          129   //��������

#define FAULT_AC_TOATL_SWT_FAULT    0     //�ܿ��ع���
#define FAULT_AC_BRANCH_BASE        1     //֧·������ʼ���
#define FAULT_AC_BRANCH_SWT         0     //������բ
#define FAULT_AC_BRANCH_SENSOR      1     //�������쳣
#define FAULT_AC_BRANCH_FAULT_CNT   2     //ÿ������֧·�Ĺ�������
#define FAULT_AC_PANEL_NUM          129   //��������



/************************ ������ͨ�����䷢�͹���ID����غ��� ****************************/

/*************************************************************
��������: v_fauid_fault_mbx_init		           				
��������: ��ʼ������ID�����䣬����RTX������������������ǰ����						
�������: ��         		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_fauid_fault_mbx_init(void);


/*************************************************************
��������: v_fauid_send_fault_id_occur		           				
��������: ���Ϸ���ʱ�����ô˺���ͨ�����䷢�͹���ID						
�������: u16_fault_id -- ����ID         		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_fauid_send_fault_id_occur(U16_T u16_fault_id);


/*************************************************************
��������: v_fauid_send_fault_id_resume		           				
��������: ���ϻָ�ʱ�����ô˺���ͨ�����䷢�͹���ID						
�������: u16_fault_id -- ����ID         		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_fauid_send_fault_id_resume(U16_T u16_fault_id);


/*************************************************************
��������: u32_fauid_recv_fault_id		           				
��������: ���չ���ID���˺����ɹ��ϴ���������ã���ȡ���������͹���������ID						
�������: timeout -- ��ʱֵ         		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
U32_T u32_fauid_recv_fault_id(U16_T timeout);


/*********** ���³���ֻ�ڹ��ϴ��������FaultId.c�е��ã���������Ӧ�õ��� **************/

#define FAULT_INVALID           0xFFFFFFFF      //��Ч����
#define FAULT_OCCUR             0x00000000      //���Ϸ���
#define FAULT_RESUME            0x00010000      //���ϻָ�
#define FAULT_OCCUR_MASK        0xFFFF0000      //�����ָ�������
#define FAULT_ID_MASK           0x0000FFFF      //����ID������
#define FAULT_ID_GROUP_MASK     0x7C00          //����ID��������
#define FAULT_ID_INDEX_MASK     0x3FF           //����ID���������


#endif
