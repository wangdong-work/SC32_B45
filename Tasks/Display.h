/************************************************************
Copyright (C), 2012-2020, ����Ӣ����Ƽ��������޹�˾
�� �� ����Display.h
��    ����1.00
�������ڣ�2012-03-22
��    �ߣ�������
������������ʾ��صĶ���ͷ�ļ�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-22  1.00     ����
**************************************************************/

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "Type.h"
#include "../Drivers/Rtc.h"

/*
��д     ȫ��                   ����
mmi      Man-Machine Interface  �˻��ӿ�
win      window                 ����/ҳ��
cap      capicity               ����
his      history                ��ʷ
batt     battery                �����
         cell                   ������
insu     insulation             ��Ե
res      resistance             ����
rec      rectifier              ����ģ��
curr     current                ����/��ǰ
disp     display                ��ʾ
val      vaule                  ��ֵ
sel      select                 ѡ��
*/


#define MMI_NAME_MAX_LEN     25       //�����24��ASCII�ַ���12�������ַ�����һ��'\0'�ַ�
#define MMI_WIN_ICON_MAX_CNT 2        //ҳ������ͼ������Ϊ2
#define MMI_WIN_ITEM_MAX_CNT 12       //ҳ��������Ŀ����Ϊ11

#define MMI_WIN_TYPE_MASK    0xFF00   //ҳ��ID�ĸ�8λ��ʾ����
#define MMI_WIN_TYPE_OFFSET  8
#define MMI_WIN_INDEX_MASK   0x00FF   //ҳ��ID�ĵ�8λ��ʾҳ��ṹ���������
#define MMI_WIN_ID_NULL      0xFFFF   //��Чҳ��ID
        

/* �������ͣ���Ҫ���ڰ������� */
enum
{
	MMI_WIN_TYPE_MAIN_WIN = 0,              // ������
	MMI_WIN_TYPE_MENU,                      // �˵�����
	MMI_WIN_TYPE_SET,                       // ���ý���
	MMI_WIN_TYPE_ABOUT,                     // ���ڲ˵�
	MMI_WIN_TYPE_PASSWORD,                  // �����������
	MMI_WIN_TYPE_PARAM_RESTORE,             // �����ָ�����
	MMI_WIN_TYPE_BATT_CAP_RESTORE,          // ��������ָ�����
	MMI_WIN_TYPE_HIS_FAULT_CLEAR,           // ��ʷ�澯��¼�������
	MMI_WIN_TYPE_EXCEPTION_CLEAR,           // ����ĩ����澯��¼�������
	MMI_WIN_TYPE_RECORD_CLEAR,              // ��س�ŵ��¼�������
	MMI_WIN_TYPE_CONFIRM,                   // ȷ�ϲ�����ʾ����
	MMI_WIN_TYPE_DOING,                     // ���ڲ�����ʾ����
	MMI_WIN_TYPE_RUN_INFO,                  // ��ҳʵʱ���ݽ���

	/* ������ʵʱ���ݶ�ҳ�������� */
	MMI_WIN_TYPE_CELL1_VOLT,                // һ�鵥���ص�ѹ���棬��ҳ����
	MMI_WIN_TYPE_CELL2_VOLT,                // ���鵥���ص�ѹ���棬��ҳ����
	MMI_WIN_TYPE_RECT,                      // ����ģ����棬��ҳ����
	MMI_WIN_TYPE_DCDC_MODULE,               // ͨ��ģ����棬��ҳ����
	MMI_WIN_TYPE_DCAC_MODULE,               // ���ģ����棬��ҳ����
	MMI_WIN_TYPE_DC_FEEDER,                 // ֱ������֧·��Ϣ���棬��ҳ����
	MMI_WIN_TYPE_DCDC_FEEDER,               // ͨ������֧·��Ϣ���棬��ҳ����
	MMI_WIN_TYPE_DCAC_FEEDER,               // �������֧·��Ϣ���棬��ҳ����
	MMI_WIN_TYPE_AC_FEEDER,                 // ��������֧·��Ϣ���棬��ҳ����
	MMI_WIN_TYPE_SWITCH,                    // ����״̬���棬��ҳ����
	MMI_WIN_TYPE_EC_SWITCH,                 // ��ٿ���״̬���棬��ҳ����

	MMI_WIN_TYPE_CURR_FAULT,                // ��ǰ�澯���棬��ҳ����
	MMI_WIN_TYPE_HIS_FAULT,                 // ��ʷ�澯���棬��ҳ����
	MMI_WIN_TYPE_EXCEPTION,                 // ����ĩ������Ͻ��棬��ҳ����
	MMI_WIN_TYPE_BATT_RECORD,               // ��س�ŵ��¼����ҳ����
	
};

/* ����ID���� */
#define MMI_WIN_ID_MAIN_WINDOW              (0 |  (MMI_WIN_TYPE_MAIN_WIN<<8))            // ������-1��ĸ������
#define MMI_WIN_ID_MAIN_MENU                (1 |  (MMI_WIN_TYPE_MENU<<8))                // ���˵�           
#define MMI_WIN_ID_DC_RUN_INFO_MENU         (2 |  (MMI_WIN_TYPE_MENU<<8))                // ֱ��ϵͳ��Ϣ��ѯ�˵�
#define MMI_WIN_ID_FEEDER_RUN_INFO_MENU     (3 |  (MMI_WIN_TYPE_MENU<<8))                // ���߹���Ϣ��ѯ�˵�
#define MMI_WIN_ID_DCDC_RUN_INFO_MENU       (4 |  (MMI_WIN_TYPE_MENU<<8))                // ͨ��ϵͳ��Ϣ��ѯ�˵�
#define MMI_WIN_ID_DCAC_RUN_INFO_MENU       (5 |  (MMI_WIN_TYPE_MENU<<8))                // ���ϵͳ��Ϣ��ѯ�˵�
#define MMI_WIN_ID_SET_MENU                 (6 |  (MMI_WIN_TYPE_MENU<<8))                // �������ò˵�
#define MMI_WIN_ID_DC_SET_MENU              (7 |  (MMI_WIN_TYPE_MENU<<8))                // ֱ���������ò˵�
#define MMI_WIN_ID_FEEDER_SET_MENU          (8 |  (MMI_WIN_TYPE_MENU<<8))                // ���߹�������ò˵�


#define MMI_WIN_ID_NO_CONFIG                (9 |  (MMI_WIN_TYPE_RUN_INFO<<8))            // ĩ���ô�����Ϣ��ʾ����

#define MMI_WIN_ID_AC_TRIPHASE_INFO         (10 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ���ཻ����Ϣ��ʾҳ��
#define MMI_WIN_ID_AC_UNIPHASE_INFO         (11 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ���ཻ����Ϣ��ʾҳ��
#define MMI_WIN_ID_BATT_TOTAL_INFO          (12 | (MMI_WIN_TYPE_RUN_INFO<<8))           // һ������������
#define MMI_WIN_ID_BATT_GROUP_INFO          (13 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ��������������
#define MMI_WIN_ID_BMS_GROUP1_INFO          (14 | (MMI_WIN_TYPE_RUN_INFO<<8))           // һ����Ѳ������
#define MMI_WIN_ID_BMS1_CELL_INFO           (15 | (MMI_WIN_TYPE_CELL1_VOLT<<8))         // һ�鵥���ѹ����
#define MMI_WIN_ID_BMS_GROUP2_INFO          (16 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ������Ѳ������
#define MMI_WIN_ID_BMS2_CELL_INFO           (17 | (MMI_WIN_TYPE_CELL2_VOLT<<8))         // ���鵥���ѹ����
#define MMI_WIN_ID_PB_CB_INSU_INFO          (18 | (MMI_WIN_TYPE_RUN_INFO<<8))           // һ�κϿ�ĸ��Ե״̬��ѯ
#define MMI_WIN_ID_BUS_INSU_INFO            (19 | (MMI_WIN_TYPE_RUN_INFO<<8))           // һ��ĸ�߾�Ե��ѯ
#define MMI_WIN_ID_RECT_INFO                (20 | (MMI_WIN_TYPE_RECT<<8))               // AC/DCģ����Ϣ��ѯ
#define MMI_WIN_ID_SWITCH_INFO              (21 | (MMI_WIN_TYPE_SWITCH<<8))             // ����״̬��ѯ
#define MMI_WIN_ID_DC_FEEDER_INFO           (22 | (MMI_WIN_TYPE_DC_FEEDER<<8))          // һ�����������֧·��Ϣ
#define MMI_WIN_ID_DCDC_INFO                (23 | (MMI_WIN_TYPE_DCDC_MODULE<<8))        // ͨ��ģ����Ϣ��ѯ
#define MMI_WIN_ID_DCDC_FEEDER_INFO         (24 | (MMI_WIN_TYPE_DCDC_FEEDER<<8))        // ͨ��������֧·��Ϣ��ѯ
#define MMI_WIN_ID_DCAC_INFO                (25 | (MMI_WIN_TYPE_DCAC_MODULE<<8))        // ���ģ����Ϣ��ѯ
#define MMI_WIN_ID_DCAC_FEEDER_INFO         (26 | (MMI_WIN_TYPE_DCAC_FEEDER<<8))        // ���������֧·��Ϣ��ѯ

#define MMI_WIN_ID_CURR_FAULT_INFO          (27 | (MMI_WIN_TYPE_CURR_FAULT<<8))          // ��ǰ���Ͻ���
#define MMI_WIN_ID_NO_CURR_FAULT            (28 | (MMI_WIN_TYPE_RUN_INFO<<8))            // �޵�ǰ������ʾ����
#define MMI_WIN_ID_HIS_FAULT_INFO           (29 | (MMI_WIN_TYPE_HIS_FAULT<<8))           // ��ʷ���Ͻ���
#define MMI_WIN_ID_NO_HIS_FAULT             (30 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ����ʷ������ʾ����
#define MMI_WIN_ID_EXCEPTION_INFO           (31 | (MMI_WIN_TYPE_EXCEPTION<<8))           // ����ĩ������Ͻ���
#define MMI_WIN_ID_NO_EXCEPTION             (32 | (MMI_WIN_TYPE_RUN_INFO<<8))            // �޵���ĩ���������ʾ����
#define MMI_WIN_ID_RECORD                   (33 | (MMI_WIN_TYPE_BATT_RECORD<<8))         // ��س�ŵ��¼����
#define MMI_WIN_ID_NO_RECORD                (34 | (MMI_WIN_TYPE_RUN_INFO<<8))            // �޵�س�ŵ��¼����

#define MMI_WIN_ID_INPUT_PASSWORD           (35 | (MMI_WIN_TYPE_PASSWORD<<8))            // �����������
#define MMI_WIN_ID_PASSWORD_ERROR           (36 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ���������ʾ����

#define MMI_WIN_ID_PARAM_OUT_RANGE          (37 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��ʾ������Χ��������

#define MMI_WIN_ID_DC_SYSTEM_SET            (38 | (MMI_WIN_TYPE_SET<<8))                 // ֱ��ϵͳ���ý���
#define MMI_WIN_ID_AC_PARAM_SET             (39 | (MMI_WIN_TYPE_SET<<8))                 // ������������
#define MMI_WIN_ID_BATT_PARAM_SET           (40 | (MMI_WIN_TYPE_SET<<8))                 // �������
#define MMI_WIN_ID_DC_PARAM_SET             (41 | (MMI_WIN_TYPE_SET<<8))                 // ֱ����������
#define MMI_WIN_ID_RECT_PARAM_SET           (42 | (MMI_WIN_TYPE_SET<<8))                 // ����ģ���������
#define MMI_WIN_ID_BATT_METER1_SET          (43 | (MMI_WIN_TYPE_SET<<8))                 // һ����Ѳ������
#define MMI_WIN_ID_BATT_METER2_SET          (44 | (MMI_WIN_TYPE_SET<<8))                 // ������Ѳ������

#define MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET (45 | (MMI_WIN_TYPE_SET<<8))              // ���������ģ����������
#define MMI_WIN_ID_FEEDER_PANEL_MODULE_SET	(46 | (MMI_WIN_TYPE_SET<<8))                 // ���������ģ������

#define MMI_WIN_ID_AC_THR_SET               (47 | (MMI_WIN_TYPE_SET<<8))                 // ������������
#define MMI_WIN_ID_DC_THR_SET               (48 | (MMI_WIN_TYPE_SET<<8))                 // ֱ����������
#define MMI_WIN_ID_BATT_THR_SET             (49 | (MMI_WIN_TYPE_SET<<8))                 // �������ֵ����
#define MMI_WIN_ID_INSU_THR_SET             (50 | (MMI_WIN_TYPE_SET<<8))                 // ��Ե���ޱ�������

#define MMI_WIN_ID_BATT_CHARGE_SET          (51 | (MMI_WIN_TYPE_SET<<8))                 // ��س���������
#define MMI_WIN_ID_BATT_TO_FLO_SET          (52 | (MMI_WIN_TYPE_SET<<8))                 // ���ת�����о�
#define MMI_WIN_ID_BATT_TO_EQU_SET          (53 | (MMI_WIN_TYPE_SET<<8))                 // ���ת�����о�
#define MMI_WIN_ID_BATT_DISCHARGE_END_SET   (54 | (MMI_WIN_TYPE_SET<<8))                 // ��غ�����ֹ����
#define MMI_WIN_ID_BATT_DISCHARGE_SET       (55 | (MMI_WIN_TYPE_SET<<8))                 // ��طŵ���������

#define MMI_WIN_ID_SYSTEM_CTL_SET           (56 | (MMI_WIN_TYPE_SET<<8))                 // ϵͳ���Ʒ�ʽ
#define MMI_WIN_ID_BATT_CTL_SET             (57 | (MMI_WIN_TYPE_SET<<8))                 // ��س�緽ʽ
#define MMI_WIN_ID_RECT_ON_OFF              (58 | (MMI_WIN_TYPE_SET<<8))                 // ����ģ�鿪�ػ�

#define MMI_WIN_ID_DCDC_MODULE_SET           (59 | (MMI_WIN_TYPE_SET<<8))                 // ͨ��ģ���������
#define MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET (60 | (MMI_WIN_TYPE_SET<<8))                // ͨ��������ģ����������
#define MMI_WIN_ID_DCDC_FEEDER_MODULE_SET	 (61 | (MMI_WIN_TYPE_SET<<8))                 // ͨ��������ģ������

#define MMI_WIN_ID_DCAC_MODULE_SET           (62 | (MMI_WIN_TYPE_SET<<8))                 // ���ģ���������
#define MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET (63 | (MMI_WIN_TYPE_SET<<8))                // ���������ģ����������
#define MMI_WIN_ID_DCAC_FEEDER_MODULE_SET	 (64 | (MMI_WIN_TYPE_SET<<8))                 // ���������ģ������

#define MMI_WIN_ID_RELAY_OUT_SET            (65 | (MMI_WIN_TYPE_SET<<8))                 // �ɽӵ�����趨
#define MMI_WIN_ID_ALARM_SET                (66 | (MMI_WIN_TYPE_SET<<8))                 // �����趨
#define MMI_WIN_ID_BACKSTAGE_SET            (67 | (MMI_WIN_TYPE_SET<<8))                 // Զ��ͨѶ����
#define MMI_WIN_ID_RTC_PASSWORD_SET         (68 | (MMI_WIN_TYPE_SET<<8))                 // RTCʱ��������趨

#define MMI_WIN_ID_HIS_FAULT_CLEAR          (69 | (MMI_WIN_TYPE_HIS_FAULT_CLEAR<<8))     // ��ʷ�澯��¼���
#define MMI_WIN_ID_EXCEPTION_CLEAR          (70 | (MMI_WIN_TYPE_EXCEPTION_CLEAR<<8))     // ����ĩ����澯��¼���
#define MMI_WIN_ID_RECORD_CLEAR             (71 | (MMI_WIN_TYPE_RECORD_CLEAR<<8))        // �¼���¼���
#define MMI_WIN_ID_BATT_RESTORE             (72 | (MMI_WIN_TYPE_BATT_CAP_RESTORE<<8))    // ��������ָ�
#define MMI_WIN_ID_PARAM_RESTORE            (73 | (MMI_WIN_TYPE_PARAM_RESTORE<<8))       // ���ò����ָ�

#define MMI_WIN_ID_CONFIRM_OPERATION        (74 | (MMI_WIN_TYPE_CONFIRM<<8))             // ȷ�ϲ�����ʾ
#define MMI_WIN_ID_OPERATION_SCRESS         (75 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��ʾ�����ѳɹ�

#define MMI_WIN_ID_AC_ADJUST                (76 | (MMI_WIN_TYPE_SET<<8))                 // ����У׼����
#define MMI_WIN_ID_DC_VOLT_ADJUST           (77 | (MMI_WIN_TYPE_SET<<8))                 // ֱ����ѹУ׼����
#define MMI_WIN_ID_LOAD_CURR_ADJUST         (78 | (MMI_WIN_TYPE_SET<<8))                 // ���ص���У׼����
#define MMI_WIN_ID_BATT1_CURR_ADJUST        (79 | (MMI_WIN_TYPE_SET<<8))                 // һ���ص���У׼����
#define MMI_WIN_ID_BATT2_CURR_ADJUST        (80 | (MMI_WIN_TYPE_SET<<8))                 // �����ص���У׼����
#define MMI_WIN_ID_ADJUST_DOING             (81 | (MMI_WIN_TYPE_DOING<<8))               // У׼���ڽ�����ʾ����
#define MMI_WIN_ID_ADJUST_SCUESS            (82 | (MMI_WIN_TYPE_RUN_INFO<<8))            // У׼�ɹ���ʾ����
#define MMI_WIN_ID_ADJUST_FAIL              (83 | (MMI_WIN_TYPE_RUN_INFO<<8))            // У׼ʧ����ʾ����
#define MMI_WIN_ID_ADJUST_NEXT_CURR         (84 | (MMI_WIN_TYPE_RUN_INFO<<8))            // У׼��һ����������ʾ���� 

#define MMI_WIN_ID_DISPLAY                  (85 | (MMI_WIN_TYPE_SET<<8))                 // ��ʾ���ý���

#define MMI_WIN_ID_ABOUT                    (86 | (MMI_WIN_TYPE_ABOUT<<8))               // ���ڲ˵�

#define MMI_WIN_ID_NO_FUNCTION              (87 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ϵͳ�޴˹�����ʾ����

#define MMI_WIN_ID_GUIDELINE                (88 | (MMI_WIN_TYPE_MENU<<8))                // ά��ָ�ϲ˵�
#define MMI_WIN_ID_START                    (89 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��ͨҳ��
#define MMI_WIN_ID_COMM_FAULT               (90 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ͨѶ�쳣����ҳ��
#define MMI_WIN_ID_VOLT_FAULT               (91 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��Ƿѹ����ҳ��
#define MMI_WIN_ID_INSU_FAULT               (92 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��Ե����ҳ��
#define MMI_WIN_ID_BACKSTAGE_FAULT          (93 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��̨ͨѶ�쳣ҳ��
#define MMI_WIN_ID_CONTACT_VENDER           (94 | (MMI_WIN_TYPE_RUN_INFO<<8))            // ��ϵ����ҳ��
#define MMI_WIN_ID_REPAIR                   (95 | (MMI_WIN_TYPE_RUN_INFO<<8))            // �����豸����ҳ��

#define MMI_WIN_ID_MANUAL_LIMIT_CURR_SET    (96 | (MMI_WIN_TYPE_SET<<8))                 // �ֶ����������趨
#define MMI_WIN_ID_COMM_OFFLINE_SET         (97 | (MMI_WIN_TYPE_SET<<8))                 // ͨѶ���������趨

#define MMI_WIN_ID_INSU_MEAS_SET            (98 | (MMI_WIN_TYPE_SET<<8))                 // ��Ե������������
#define MMI_WIN_ID_ZERO_CURR_SET            (99 | (MMI_WIN_TYPE_SET<<8))                 // ���������������

#define MMI_WIN_ID_MAIN_WINDOW2             (100 | (MMI_WIN_TYPE_MAIN_WIN<<8))           // ������-2��ĸ������
#define MMI_WIN_ID_DC10_PARAM_SET           (101 | (MMI_WIN_TYPE_SET<<8))                // ֱ��DC10��������

#define MMI_WIN_ID_AC_MODULE_SET            (102 | (MMI_WIN_TYPE_SET<<8))                // ����ϵͳDC10��������
#define MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET (103 | (MMI_WIN_TYPE_SET<<8))               // ����ϵͳ����ģ����������
#define MMI_WIN_ID_ACS_FEEDER_MODULE_SET	(104 | (MMI_WIN_TYPE_SET<<8))                // ����ϵͳ���������ģ������

#define MMI_WIN_ID_ACS_AC_RUN_INFO_MENU     (105 | (MMI_WIN_TYPE_MENU<<8))               // ����ϵͳ���ཻ����Ϣ��ʾҳ��
#define MMI_WIN_ID_ACS_AC_RUN_INFO          (106 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ����ϵͳ��Ϣ��ѯ
#define MMI_WIN_ID_ACS_FEEDER_INFO          (107 | (MMI_WIN_TYPE_AC_FEEDER<<8))          // ����ϵͳ����֧·��Ϣ��ѯ

#define MMI_WIN_ID_SWT_ON_OFF               (108 | (MMI_WIN_TYPE_SET<<8))                // ��ٿ��ؿ���

#define MMI_WIN_ID_ECSWT_INFO               (109 | (MMI_WIN_TYPE_EC_SWITCH<<8))          // ��ٿ���״̬��ѯ

#define MMI_WIN_ID_FEEDER_SET_MENU2         (110 |  (MMI_WIN_TYPE_MENU<<8))              // �������߹�������ò˵�
#define MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2 (111 | (MMI_WIN_TYPE_SET<<8))            // �������������ģ����������
#define MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2	(112 | (MMI_WIN_TYPE_SET<<8))                // �������������ģ������

#define MMI_WIN_ID_PB_CB_INSU_INFO2         (113 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ���κϿ�ĸ��Ե״̬��ѯ
#define MMI_WIN_ID_BUS_INSU_INFO2           (114 | (MMI_WIN_TYPE_RUN_INFO<<8))           // ����ĸ�߾�Ե��ѯ
#define MMI_WIN_ID_FEEDER_RUN_INFO_MENU2    (115 | (MMI_WIN_TYPE_MENU<<8))               // �������߹���Ϣ��ѯ�˵�
#define MMI_WIN_ID_DC_FEEDER_INFO2          (116 | (MMI_WIN_TYPE_DC_FEEDER<<8))          // �������������֧·��Ϣ

#define MMI_WIN_MAX_CNT                     117


/*  ��ֵ����  */
#define MMI_VAL_TYPE_MASK      0xF0
#define MMI_VAL_TYPE_U8_TYPE   0x00
#define MMI_VAL_TYPE_U16_TYPE  0x10
#define MMI_VAL_TYPE_U32_TYPE  0x20
#define MMI_VAL_TYPE_F32_TYPE  0x30

#define MMI_VAL_TYPE_NONE      0x00       // ��Ŀû����ֵ
#define MMI_VAL_TYPE_ENUM      0x01       // ��ֵΪ�ַ�����
#define MMI_VAL_TYPE_U8        0x02       // 8λ����ֵ��ʵ����ʾ
#define MMI_VAL_TYPE_U8_2BIT   0x03       // 8λ����ֵ��ʾΪ2λ
#define MMI_VAL_TYPE_U8_3BIT   0x04       // 8λ����ֵ��ʾΪ3λ

#define MMI_VAL_TYPE_U16       0x10       // 16λ����ֵ��ʵ����ʾ
#define MMI_VAL_TYPE_U16_2BIT  0x11       // 16λ����ֵ��ʾΪ2λ
#define MMI_VAL_TYPE_U16_3BIT  0x12       // 16λ����ֵ��ʾΪ3λ
#define MMI_VAL_TYPE_U16_4BIT  0x13       // 16λ����ֵ��ʾΪ4λ

#define MMI_VAL_TYPE_U32       0x20       // 16λ����ֵ��ʵ����ʾ
#define MMI_VAL_TYPE_U32_2BIT  0x21       // 16λ����ֵ��ʾΪ2λ
#define MMI_VAL_TYPE_U32_3BIT  0x22       // 16λ����ֵ��ʾΪ3λ
#define MMI_VAL_TYPE_U32_4BIT  0x23       // 16λ����ֵ��ʾΪ4λ
#define MMI_VAL_TYPE_U32_5BIT  0x24       // 16λ����ֵ��ʾΪ5λ

#define MMI_VAL_TYPE_F32_1P    0x30       // 1λС���������ͣ��������ְ�ʵ�ʳ�����ʾ
#define MMI_VAL_TYPE_F32_3W1P  0x31       // ���Ϊ3��1λС���������ͣ�����������ʾ1λ
#define MMI_VAL_TYPE_F32_4W1P  0x32       // ���Ϊ4��1λС���������ͣ�����������ʾ2λ
#define MMI_VAL_TYPE_F32_5W1P  0x33       // ���Ϊ5��1λС���������ͣ�����������ʾ3λ
#define MMI_VAL_TYPE_F32_6W1P  0x34       // ���Ϊ5��1λС���������ͣ�����������ʾ3λ
#define MMI_VAL_TYPE_F32_2P    0x35       // 2λС���������ͣ��������ְ�ʵ�ʳ�����ʾ
#define MMI_VAL_TYPE_F32_4W2P  0x36       // ���Ϊ4��2λС���������ͣ�����������ʾ1λ
#define MMI_VAL_TYPE_F32_5W2P  0x37       // ���Ϊ5��2λС���������ͣ�����������ʾ2λ
#define MMI_VAL_TYPE_F32_6W2P  0x38       // ���Ϊ6��2λС���������ͣ�����������ʾ3λ
#define MMI_VAL_TYPE_F32_3P    0x39       // 3λС���������ͣ��������ְ�ʵ�ʳ�����ʾ
#define MMI_VAL_TYPE_F32_5W3P  0x3A       // ���Ϊ5��3λС���������ͣ�����������ʾ1λ
#define MMI_VAL_TYPE_F32_6W3P  0x3B       // ���Ϊ6��3λС���������ͣ�����������ʾ2λ
#define MMI_VAL_TYPE_F32_7W3P  0x3C       // ���Ϊ7��3λС���������ͣ�����������ʾ3λ
	

/* ����״̬�������� */
enum
{
	MMI_WIN_NORMAL = 0,             // �鿴״̬
	MMI_WIN_SET                     // ����״̬
};

/* ������ʾ�������� */
enum
{
	MMI_NORMAL_DISP = 0,           // ������ʾ
	MMI_INVERSE_DISP,              // ������ʾ
};


/* ��Ŀ�ṹ�嶨�� */
typedef struct
{
	U16_T    u16_name_index;       // �����ַ�������ֵ
	U8_T     u8_name_x;            // �����ַ�������x
	U8_T     u8_name_y;            // �����ַ�������y

	U16_T    u16_val_index;        // �����ֵ����Ϊ�ַ��������ֵ���ַ�����ʼ������ʵ�ʵ��ַ�������Ϊu16_val_index+*((U8_T *)pv_val)
	                               // �����ֵ���Ͳ����ַ����ͣ������ֵ�����壬��ʹ��

	U8_T     u8_val_x;             // ��ֵ������x
	U8_T     u8_val_y;             // ��ֵ������y

	U8_T     u8_val_type;          // ��ֵ����
	U32_T    u32_val_min;          // ��Сֵ�������ֵ����Ϊ�������������1000ת��Ϊ����
	U32_T    u32_val_max;          // ���ֵ�������ֵ����Ϊ�������������1000ת��Ϊ����
	void   	 *pv_val;              // ָ����ֵ��ָ��
}MMI_ITEM_T;

/* ͼ��ṹ�嶨�� */
typedef struct
{
	U16_T u16_index;               // icon��������ֵ
	U8_T  u8_x;                    // x����
	U8_T  u8_y;                    // y����
}MMI_ICON_T;


typedef struct
{
	U16_T u16_id;                  // ����ID��
	U16_T u16_id_father;           // �����ڵ�ID�ţ���"ESC"�����صĴ���
	U16_T u16_id_prev;             // ǰһҳ����ID��
	U16_T u16_id_next;             // ��һҳ����ID��

	U8_T u8_sel_father;            // ��ǰҳ���ڸ������е�����ֵ
	U8_T u8_icon_cnt;              // icon����
	U8_T u8_item_cnt;              // ��Ŀ����

	MMI_ICON_T t_icon[MMI_WIN_ICON_MAX_CNT];
	MMI_ITEM_T t_item[MMI_WIN_ITEM_MAX_CNT];
}MMI_WIN_T;

typedef struct
{
	U16_T u16_curr_id;                          //��ǰҳ��ID
	MMI_WIN_T t_curr_win;

	union                                       //��������壬���ڶ�ҳ���淭ҳ����ʱ��¼���
	{
		U16_T u16_cell;                         //������
		U16_T u16_feeder_module;                //����ģ���                
		U16_T u16_feeder_branch;                //����֧·���
		U16_T u16_curr_fault;                   //��ǰ�������           
		U16_T u16_his_fault;                    //��ʷ�������
		U16_T u16_exception;                    //����ĩ����������
		U16_T u16_rect_info;                    //����ģ����Ϣ���
		U16_T u16_dcdc_module;                  //ͨ��ģ����Ϣ���
		U16_T u16_dcac_module;                  //���ģ����Ϣ���
		U16_T u16_record;                       //����¼���
		U16_T u16_rect_on_off;                  //����ģ�鿪�ػ����
		U16_T u16_swt;                          //�Դ��������
		U16_T u16_swt_on_off;                   //��ٿ������
	}u_ordinal;

	U8_T  u8_special_name[MMI_WIN_ITEM_MAX_CNT][MMI_NAME_MAX_LEN];   //��ҳ�������Ŀ����һ����Ҫ��ʱ���ɣ�������������������ɵ�����
	U8_T  u8_item_inverse[MMI_WIN_ITEM_MAX_CNT];                     //ҳ�����Ŀ�Ƿ��ԣ�MMI_INVERSE_DISP���ԣ�MMI_NORMAL_DISP������
	U8_T  u8_buffer[43];                        //���ڱ����dataflash�ж�ȡ���������ơ�֧·����

	U8_T  u8_set_blink;                         //������״̬�£���ǰ��˸��״̬��MMI_INVERSE_DISP���ԣ�NORMAL_DISPLAY������
	U8_T  u8_win_status;	                    //��ǰ���ڵ�״̬��MMI_WIN_NORMAL�鿴״̬��MMI_WIN_SET����״̬ 
	U8_T  u8_sel_index;                         //�˵������ô����£���ǰѡ�����Ŀ
	U8_T  u8_bit_index;                         //������״̬�£���ʾ��ǰ���õ���һλ
	void *pv_back_item_val;                     //��������������Ŀ��pv_val��ֵ
	U32_T u32_set_value;                        //�����������õ�ֵ������ʱ��Ŀ��pv_valָ���ֵ

	RTC_TIME_T t_time;                          //rtcʱ��������������ú���ʾRTCʱ��
}MMI_WIN_RECORD_T;



/*************************************************************
��������: v_disp_share_data_restore		           				
��������: �ָ�Ĭ�����ݣ������ò����ָ���Ĭ��ֵ						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_disp_cfg_data_restore(void);


/*************************************************************
��������: v_disp_cfg_data_init		           				
��������: �������ݳ�ʼ��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_disp_cfg_data_init(void);


/*************************************************************
��������: v_disp_display_task		           				
��������: ��ʾ�Ͱ����������񣬸������ͨ��RTX����os_tsk_create������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
extern __task void v_disp_display_task(void);

#endif
