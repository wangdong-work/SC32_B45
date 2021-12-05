/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����ShareDataStruct.h
��    ����1.00
�������ڣ�2012-04-12
��    �ߣ�������
�����������������ݽṹ����ͷ�ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-04-12  1.00     ����
**************************************************************/


#ifndef __SHARE_DATA_STRUCT_H_
#define __SHARE_DATA_STRUCT_H_

#include <rtl.h>


/*
��д     ȫ��         ����
volt     voltage      ��ѹ
curr     current      ����/����
res      resistance   ����
batt     battery      ���
thr      threshold    ��ֵ
in       input        ����
out      output       ���
insu     insulation   ��Ե
num      number       ����
cfg      configure    ����
def      default      Ĭ��
comm     commucation  ͨ��
mgmt     management   ����
lang     language     ����
ctl      control      ����
bms      battery monitor set ���Ѳ��
dur      duration     ����
comp     compensation ����
dis      discharge    �ŵ�
pos      positive     ��
neg      negative     ��
rt       real time    ʵʱ
freq     frequency    Ƶ��
pf       power factor ��������
*/

#define CFG_DATA_VERSION_L  4     //�����������汾�ĵ��ֽ�
#define CFG_DATA_VERSION_H  1     //�����������汾�ĸ��ֽ�

#define ACS_FEEDER_MODULE_MAX 3   // ����������ģ���������
#define DC10_SWT_BRANCH_MAX  16    // ���������֧·��

#define BATT_METER_MAX  5     // ���Ѳ���������
#define BATT_GROUP_MAX  2     // ����������
#define BATT_CELL_MAX   120   // ��������������
#define RECT_CNT_MAX    24    // ����ģ���������
#define SWT_BRANCH_MAX  20    // ���������֧·��
#define	SWT_NAME_CHAR_LEN 32  //���俪�������ַ�����

#define FEEDER_PANEL_MODULE_MAX 4  // һ����������ģ���������
#define FEEDER_PANEL_MAX  8        // �������������
#define FEEDER_BRANCH_MAX 64       // ����֧·�������

#define DCDC_MODULE_MAX        8   // ͨ��ģ���������
#define DCDC_FEEDER_MODULE_MAX 1   // ͨ��������ģ���������

#define DCAC_MODULE_MAX        8   // ���ģ���������
#define DCAC_FEEDER_MODULE_MAX 1   // ���������ģ���������

#define RELAY_MAX       6     // �̵����������

#define RC10_MODULE_MAX        16   // ϵͳ�����RC10ģ���������
#define RC10_NODE_MAX          16   // RC10ģ��ɽ���������
#define SWT_CTRL_MAX           (RC10_NODE_MAX / 2)   // RC10ģ�鿪�ؿ����������
#define TTL_SWT_CTRL_MAX       (RC10_MODULE_MAX * SWT_CTRL_MAX)     // �����Ͽ�ͨ��RC10���Ƶ���󿪹�������

#define FACT_SWT_CTRL_MAX      (3 * SWT_CTRL_MAX)	//ʵ�ʶ���ɿؿ���·��

#define	BATT_CURR_FROM_SENSOR	0		//1��ʾ��ص�����DC10�Ĵ�����������	0��ʾ��DC10�ķ���������

/* ϵͳ��ѹ�ȼ� */
typedef enum
{
	VOLT_LEVEL_220=0,      //220v
	VOLT_LEVEL_110,        //110v
}SYS_VOLT_LEVEL_E;

/* ��ع���ʽ */
typedef enum
{
	AUTO_MODE=0,	       //�Զ�
	MANUAL_MODE,           //�ֶ�
}BATT_MGMT_MODE_E;

/* ϵͳ��ʾ���� */
typedef enum
{
	CHINESE=0,             //����
	ENGLISH,               //Ӣ��
}LANG_TYPE_E;

/* LCD���� */
typedef enum
{
	LCD_HORIZONTAL=0,      //ˮƽ
	LCD_VERTICAL,          //����
}LCD_DIRECTION_E;

/* �������澯���� */
typedef enum
{
	BEEP=0,	               //����
	QUIET,                 //����
}BUZZER_ALM_SET_E;

/* �Ƿ񱣴� */
typedef enum
{
	NO_SAVE=0,             //������
	SAVE,                  //����
}SAVE_E;

/* ����/�ر�ö�� */
typedef enum
{
	CLOSE=0,               //�ر�
	OPEN,                  //����
}OPEN_E;

/* ĸ�߶���ö�� */
typedef enum
{
	ONE=0,                //һ��
	TWO,                  //����
}BUS_NUM_E;

/* ��/��ö�� */
typedef enum
{
	NO=0,                  //��
	HAVE,                  //��
}HAVE_E;

/* ��������ö�� */
typedef enum
{
	NO_DIODE_CHAIN=0,      //�޹�������
	STEP_5_4V,             //5��4V
	STEP_5_7V,             //5��7V
	STEP_7_3V,             //7��3V
	STEP_7_5V,             //7��5V
}DIODE_CHAIN_E;

/* ������������ */
typedef enum
{
	NORMALLY_OPEN=0,       //����
	NORMALLY_CLOSE,        //����
}SWT_INPUT_TYPE_E;	

/* ��̨ͨ��Э�� */
typedef enum
{
	BS_MODBUS=0,           //MODBUS
	BS_CDT,                //CDT
	BS_ADJUST,             //У׼
	BS_IEC103,             //IEC103
}BACKSTAGE_PROTROL_E;

/* ����ͨ������ */
typedef enum
{
	COM_BAUD_1200=0,
	COM_BAUD_2400,
	COM_BAUD_4800,
	COM_BAUD_9600,
	COM_BAUD_19200
}COM_BAUD_E;

/* CAN��ͨ������ */
typedef enum
{
	CAN_BAUD_125K=0,
	CAN_BAUD_50K,
	CAN_BAUD_20K,
	CAN_BAUD_10K
}CAN_BAUD_E;

/* ����У������ */
typedef enum
{
	ODD_PARITY=0,          //��У��
	EVEN_PARITY,           //żУ��
	NONE_PARITY,           //��У��
}COM_PARITY_E;

/* �������� */
typedef enum
{
	AC_3_PHASE=0,          //����
	AC_1_PHASE,            //����
}AC_INPUT_TYPE_E;

/* ϵͳ�ڲ�ͨ��Э���� */
typedef enum
{
	MODBUS=0,
}INTERNAL_PROTOCOL_E;

/* ���ģ������ */
typedef enum
{
	CURR_5A=0,
	CURR_7A,
	CURR_10A,
	CURR_20A,
	CURR_30A,
	CURR_35A,
	CURR_40A,
	CURR_50A,
}RATED_CURR_E;

/* ������������ */
typedef enum
{
	NONE_PATH=0,           //��
	ONE_PATH,              //1·
	TWO_PATH,              //2·
}AC_INPUT_NUM_E;

/* ���������ѹ */
typedef enum
{
	VOLT_75MV=0,           //75mv
	VOLT_50MV,             //50mv
}SHUNT_RATED_VOLT_E;

/* ���Ѳ������	*/
typedef enum
{
	B21=0,
	B3,
	B4,
}BMS_TYPE_E;

/* ���״̬ */
typedef enum
{
	FLO=0,                 // ����
	EQU,                   // ����
	DIS,                   // ����
}BATT_STATE_E;


/* ģ��״̬ */
typedef enum
{
	START_UP=0,
	SHUT_DOWN,
	EXCEPTION,
}MODULE_STATE_E;

/* ���ģ��״̬ */
typedef enum
{
	INVERT,
	BYPASS,
	SHUT,
}DCAC_STATE_E;

/* ������ʽ */
typedef enum
{
	AUTO_LIMIT_CURR=0,	   //�Զ�
	MANUAL_LIMIT_CURR,     //�ֶ�
}LIMIT_CURR_E;

/* ͨ��ģ��Э�� */
typedef enum
{
	DCDC_MODBUS=0,	       //MODBUSЭ��
	DCDC_CAN,              //CANЭ��
}DCDC_PROTOCOL_E;

/* ͨ��ģ������ */
typedef enum
{
	DCDC_CURR_5A=0,
	DCDC_CURR_10A,
	DCDC_CURR_20A,
	DCDC_CURR_30A,
	DCDC_CURR_40A,
	DCDC_CURR_50A,
	DCDC_CURR_60A,
	DCDC_CURR_80A,
	DCDC_CURR_100A,
}DCDC_RATED_CURR_E;

/* ��Ե������ʽ */
typedef enum
{
    INSU_PROJECT = 0,  //����ģʽ
    INSU_DEBUG         //����ģʽ
}INSU_MEAS_WAY_E;

/* ����ֵ���� */
typedef enum
{
    POSITIVE = 0,  //��
    NEGATIVE       //��
}SIGN_E;

/* ң�ؿ��ض���ṹ */
typedef struct                                           /* ң�ؿ������� */
{
	U8_T              u8_ctrl_old;                       // ԭ���Ŀ��ؿ���ֵ����Χ0~1(0��ʾ��բ��1��ʾ��բ)��Ĭ��0
	U8_T              u8_state;                          // ��ǰ����״̬����Χ0~1(0��ʾ��բ��1��ʾ��բ)��Ĭ��0
} SWT_OBJ_T;
	
/******************************** ϵͳ���ò������� *********************************/
//g_t_share_data.t_sys_cfg.t_batt_mgmt
typedef struct                                           /* ��ع����ֲ��� */
{
	BATT_MGMT_MODE_E  e_mode[2];		                 // ��ع���ʽ �ֶ����Զ�
	U16_T             u16_rate_c10;                      // ��ر����������Χ30~3000Ah��Ĭ��100Ah
	F32_T             f32_equ_volt;                      // ��ؾ����ѹ����Χ�����Ƿѹ��~��ѹ�㣬Ĭ��245V
	F32_T             f32_flo_volt;                      // ��ظ����ѹ����Χ�����Ƿѹ��~��ѹ�㣬Ĭ��235V
	F32_T             f32_limit_curr;                    // ��س�������㣬��Χ0.05~�������㣬Ĭ��0.1C10
	
	//ת�����о�1
	F32_T             f32_to_equ_curr;		             // ���ת�����������Χ0.04~��������㣬Ĭ��0.08C10
	U16_T             u16_to_equ_dur_time;               // ��������ת���������ĳ���ʱ�䣬��Χ1~999�룬Ĭ��60��
	//ת�����о�2
	U16_T             u16_equ_cycle;	    	         // ��ض�ʱ�������ڣ���Χ1~9999Сʱ��Ĭ��4320Сʱ
	//ת�����о�3
	U16_T             u16_ac_fail_time;                  // ����ͣ��ʱ������Χ1~999���ӣ�Ĭ��20����
	//ת�����о�4
	F32_T             f32_low_cap;                       // ����������ڣ���Χ0.8~0.95C10��Ĭ��0.8C10
	
	//ת�����о�1
	F32_T             f32_to_flo_curr;			         // ���ת�����������Χ0.01~0.03C10��Ĭ��0.02C10
	U16_T             u16_curr_go_time;                  // ���ת���䵹��ʱʱ�䣬��Χ1~360���ӣ�Ĭ��180����
	//ת�����о�2
	U16_T             u16_max_equ_time;	                 // ��ؾ��䱣��ʱ�䣬��Χ1~999���ӣ�Ĭ��720����
	
	//��طŵ���ֹ�о�1
	U16_T             u16_total_end_volt;                // ��طŵ���ֹ��ѹ����Χ190~220V��Ĭ��200V
	//��طŵ���ֹ�о�2
	F32_T             f32_cell_end_volt;                 // ��طŵ���ֹ�����ѹ����Χ1.8~12V��Ĭ��1.8V
	//��طŵ���ֹ�о�3
	U16_T             u16_max_dis_time;                  // ��طŵ���ֹʱ�䣬��Χ1~999���ӣ�Ĭ��600����
	
	//��ؾ��䲹����ѹ
	F32_T             f32_batt_temp_comp_volt;           // �¶Ȳ�����ѹ����Χ0~500mV��Ĭ��0mV
	
	//������ѹ�澯����
	F32_T             f32_high_volt_limit;               // ������ѹ�㣬��Χ220~320V��Ĭ��264V
	F32_T             f32_low_volt_limit;                // �����Ƿѹ�㣬��Χ186~220��Ĭ��187V
	F32_T             f32_high_curr_limit;               // ��س������㣬��Χ0.1~1.0C10��Ĭ��0.2
	
	/* �ŵ����� */
	F32_T             f32_batt_10c_dis_rate;             // ���1.0C�ŵ���   [0~10.0], Ĭ��0.3
	F32_T             f32_batt_09c_dis_rate;             // ���0.9C�ŵ���   [0~10.0], Ĭ��0.4
	F32_T             f32_batt_08c_dis_rate;             // ���0.8C�ŵ���   [0~10.0], Ĭ��0.5
	F32_T             f32_batt_07c_dis_rate;             // ���0.7C�ŵ���   [0~10.0], Ĭ��0.7
	F32_T             f32_batt_06c_dis_rate;             // ���0.6C�ŵ���   [0~10.0], Ĭ��0.8
	F32_T             f32_batt_05c_dis_rate;             // ���0.5C�ŵ���   [0~10.0], Ĭ��0.9
	F32_T             f32_batt_04c_dis_rate;             // ���0.4C�ŵ���   [0~10.0], Ĭ��2.2
	F32_T             f32_batt_03c_dis_rate;             // ���0.3C�ŵ���   [0~10.0], Ĭ��3.3
	F32_T             f32_batt_02c_dis_rate;             // ���0.2C�ŵ���   [0~10.0], Ĭ��4
	F32_T             f32_batt_01c_dis_rate;             // ���0.1C�ŵ���   [0~10.0], Ĭ��10
}BATT_MGMT_CFG_T;

//g_t_share_data.t_sys_cfg.t_sys_param
typedef struct                                           /* ϵͳ�������� */
{
	/* ֱ��ϵͳ���� */
	SYS_VOLT_LEVEL_E        e_volt_level;                // ϵͳ���ͣ�220/110V
	
	/* �������ò��ֲ��� */
	BUS_NUM_E               e_bus_seg_num;			     // ֱ��ĸ�߶�������ΧONE/TWO��Ĭ��ONE
	U8_T                    u8_dc_feeder_panel_num;      // һ��ֱ����������������Χ0~4��Ĭ��1
	U8_T                    u8_seg1_fdl_master_add;      // һ�ξ�Ե������ַ����Χ14~111��Ĭ��14
	U8_T                    u8_dc2_feeder_panel_num;     // ����ֱ����������������Χ0~4��Ĭ��0
	U8_T                    u8_seg2_fdl_master_add;      // ���ξ�Ե������ַ����Χ14~111��Ĭ��30
	U8_T                    u8_batt_group_num;           // �����������Χ0~2��Ĭ��1
	U8_T                    u8_rc10_module_num;          // RC10ģ��������Χ0~16��Ĭ��0
	
	/* ���� */
	LANG_TYPE_E             e_lang;                      // ���ԣ�����/Ӣ�ģ�Ĭ������
	LCD_DIRECTION_E         e_lcd_driection;             // ��ʾ������ˮƽ/���ԣ�Ĭ��ˮƽ
	
	/* �ֶ��������� */
	LIMIT_CURR_E            e_limit_curr;                // ������ʽ���Զ�/�ֶ���Ĭ���ֶ�
	F32_T                   f32_manual_limit_curr;       // �ֶ������㣬5%~110%��Ĭ��100%
	
	/* ͨѶ�������� */
	U8_T                    u8_fc10_offline_time;        // ����ģ��ͨѶ�ж�ʱ�䣬��Χ5~60S��Ĭ��10
	U8_T                    u8_bms_offline_cnt;          // ���Ѳ��ͨѶ�ж������������Χ4~99����Ĭ��10
	U8_T                    u8_rect_offline_cnt;         // ����ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	U8_T                    u8_dcdc_offline_cnt;         // ͨ��ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	U8_T                    u8_dcac_offline_cnt;         // ���ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	U8_T                    u8_ats_offline_cnt;          // ATSģ��ͨѶ�ж������������Χ4~99����Ĭ��10       xj-2020-6-23  ���¶���Ϊ��������ȷ��ʱ��  
	U8_T                    u8_acm_offline_cnt;          // �๦�ܵ��ͨѶ�ж������������Χ4~99����Ĭ��10
	U8_T                    u8_dc10_offline_cnt;         // DC10ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	U8_T                    u8_rc10_offline_cnt;         // RC10ģ��ͨѶ�ж������������Χ4~99����Ĭ��10

	/* ��Ե������������ */
	CAN_BAUD_E              e_can_baud;		             // CANͨ�����ʣ�125K��50K��20K��10K��Ĭ��125K
	INSU_MEAS_WAY_E         e_insu_meas_way;             // ĸ�ߵ��������ʽ������ģʽ�͵���ģʽ��Ĭ�Ϲ���ģʽ
    U8_T 					u8_res_switch_delay;         // ƽ�⼰��ƽ�����Ͷ����ʱ��1~120���裬����Ϊ��λ��Ĭ��2��
    U16_T                   u16_insu_sensor_range;       // ֧·���������̣�1~500���裬��mAΪ��λ��Ĭ��10mA
    U8_T                    u8_insu_bus_err_confirm;     // ĸ�߾�Եѹ���ȷ��ʱ�䣬1~180���裬����Ϊ��λ��Ĭ��3��
    U8_T                    u8_insu_meas_period;         // ��Ե���ڲ�������ʱ�䣬0~180���裬��СʱΪ��λ��Ĭ��24Сʱ
    U8_T                    u8_insu_meas_hour;           // ǿ��������Ե������ʱ��-ʱ��0~23Сʱ��Ĭ��8��
    U8_T                    u8_insu_meas_min;            // ǿ��������Ե������ʱ��-�֣�0~59���ӣ�Ĭ��0��

	/* ���������������� */
	SIGN_E                  e_load_sign;		         // ���ص����������ֵ���ţ�������, Ĭ����
	F32_T                   f32_load_zero;               // ���ص��������������ֵ����Χ[0~2.0], Ĭ��0.0
    SIGN_E                  e_bat1_sign;		         // ���1�����������ֵ���ţ�������, Ĭ����
	F32_T                   f32_bat1_zero;               // ���1���������������ֵ����Χ[0~2.0], Ĭ��0.0
    SIGN_E                  e_bat2_sign;		         // ���2�����������ֵ���ţ�������, Ĭ����
	F32_T                   f32_bat2_zero;               // ���2���������������ֵ����Χ[0~2.0], Ĭ��0.0
	
	/* ���� */
	BUZZER_ALM_SET_E        e_buzzer_state;              // �������澯���ã�����/������Ĭ������
	SAVE_E                  e_minor_fault_save;          // ��Ҫ�澯�������ã�0-�����棬1-����
	SAVE_E                  e_general_fault_save;        // һ��澯�������ã�0-�����棬1-����
	OPEN_E                  e_curr_imbalance_alm;        // ������ƽ�ⱨ�����ر�/������Ĭ�Ϲر�
	U32_T                   u32_password; 				 // �û������������ã�5λ���룬Ĭ��Ϊ11111����������Ϊ02051��ά��������02012
	U8_T                    u8_local_addr;               // ������ַ����Χ1~255��Ĭ��1
	BACKSTAGE_PROTROL_E     e_backstage_protrol;         // ��̨ͨ��Э�飬MODBUS/CDT/IEC101/IEC102��Ĭ��MODBUS
	COM_BAUD_E              e_backstage_baud;            // �����ʣ�Ĭ��9600
	COM_PARITY_E            e_backstage_parity;          // У�飬Ĭ�������
	HAVE_E                  e_compare_time;              // B���ʱ����/�У�Ĭ����
	
	/* ������� */
	U8_T                    u8_ac_fault_output;          //�������߹��ϣ���Χ0~6��Ĭ��1
	U8_T                    u8_dc_bus_fault_output;      //ֱ��ĸ�߹��ϣ���Χ0~6��Ĭ��2
	U8_T                    u8_batt_fault_output;        //����쳣���ϣ���Χ0~6��Ĭ��3
	U8_T                    u8_insu_fault_output;        //��Ե�½����ϣ���Χ0~6��Ĭ��4
	U8_T                    u8_rect_fault_output;        //���ģ����ϣ���Χ0~6��Ĭ��0
	U8_T                    u8_fuse_fault_output;        //����۶������ϣ���Χ0~6��Ĭ��0
	U8_T                    u8_dcac_fault_output;        //ͨ�ż�����Դ���ϣ���Χ0~6��Ĭ��0
	U8_T                    u8_feeder_fault_output;      //����֧·���ϣ���Χ0~6��Ĭ��0
	U8_T                    u8_batt_equ_output;          //����ָʾ����Χ0~6��Ĭ��5
	U8_T                    u8_batt_dis_output;          //����ָʾ����Χ0~6��Ĭ��6
}MISC_PARAM_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_ac
typedef struct                                           /* ������������ */
{
	AC_INPUT_NUM_E   e_path;                             // ��������·����1·/2·/�ޣ�Ĭ��2·
	AC_INPUT_TYPE_E  e_phase;                            // ������������������/���࣬Ĭ������
	                                                     
	U16_T            u16_high_volt;                      // ������ѹ�㣬��Χ220~530V��Ĭ��456V
	U16_T            u16_low_volt;                       // ����Ƿѹ�㣬��Χ187~380V��Ĭ��327V
	U16_T            u16_lack_phase;                     // ����ȱ��㣬��Χ0~380V��Ĭ��200V���ڵ�������£�������Ч
}AC_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect
typedef struct                                           /* ����ģ������ */
{
	U8_T                 u8_d21_num;                     // D21ģ����������Χ0~1��Ĭ��0
	U8_T                 u8_rect_num;                    // ģ����������Χ1~24��Ĭ��4
	INTERNAL_PROTOCOL_E  e_protocol;                     // ͨ��Э�飬Э��̶�Ϊmodbus��������
	RATED_CURR_E         e_rated_curr;                   // �������5A/7A/10A/20A/30A/35A/40A/50A��ѡ��Ĭ��10A
	F32_T                f32_out_volt[2];                // �����ѹ�����ֵ���治���趨���ɵ�ع��������޸�
	F32_T                f32_curr_percent[2];            // �����ٷֱȣ����ֵ���治���趨���ɵ�ع��������޸�
	F32_T                f32_max_out_volt;               // �����ѹ���ޣ���Χ220~286V��Ĭ��286V�����ֵ����û����ص��趨�����Ĭ��ֵ
	F32_T                f32_min_out_volt;               // �����ѹ���ޣ���Χ176~220V��Ĭ��176V�����ֵ����û����ص��趨�����Ĭ��ֵ
	F32_T                f32_offline_out_volt;           // Ĭ�������ѹ����Χ176~286V��Ĭ��220V
	F32_T                f32_offline_curr_percent;       // Ĭ����������ȣ���Χ5~110%��Ĭ��110%
}RECT_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[]
typedef struct
{
	U8_T u8_join_way;	                //���ؽ��뷽ʽ��0��������1������
	U8_T u8_meas_type;	                //���ز������ͣ�0���澯��1��״̬
	U8_T u8_fault_type;	                //���ع������ͣ�ֻ������Ϊ�澯��������������ݴ�ֵ�������
	                                    //0������������
	                                    //1��ֱ��ĸ�߹�����
	                                    //2����ع�����
	                                    //3����Ե������
	                                    //4�����ģ�������
	                                    //5��ͨ��ģ�������
	                                    //6��UPSģ�������
	                                    //7������֧·������

    U8_T u8_reserve;		            //Ԥ��
	char s_ch_name[SWT_NAME_CHAR_LEN];	//���ز����������ƣ����16������
	char s_en_name[SWT_NAME_CHAR_LEN];	//���ز���Ӣ�����ƣ����32����ĸ
} SWT_CFG_ITEM_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_swt
typedef struct                                           /* ������������ */
{
	SWT_CFG_ITEM_T t_swt_item[SWT_BRANCH_MAX];           //������������
	U8_T           u8_state_cnt;                         //����״̬���ĸ���
}SWT_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc
typedef struct                                           /* ֱ������������� */
{
	HAVE_E              e_have_cb;                       // ��ĸ���ã���/�У�Ĭ����
	DIODE_CHAIN_E       e_diode_chain_ctl;               // �������ƣ���/5��4��/5��7��/7��3��/7��5����Ĭ����

	SHUNT_RATED_VOLT_E  e_shunt_rated_volt;              // ���������ѹ��75mv/50mv��Ĭ��75mv
	U16_T               u16_load_shunt_range;            // ���ط��������̣���Χ25~2000A��Ĭ��100A
	U16_T               u16_batt1_shunt_range;           // ���1���������̣���Χ25~2000A��Ĭ��100A
	U16_T               u16_batt2_shunt_range;           // ���2���������̣���Χ25~2000A��Ĭ��100A
	U16_T               u16_feeder_shunt_range;          // ֧·�������������̣���Χ10~1000A��Ĭ��100A
	                                                     
	U16_T               u16_pb_high_volt;                // ��ĸ��ѹ���ޣ���Χ220~320V��Ĭ��286V
	U16_T               u16_pb_low_volt;                 // ��ĸǷѹ���ޣ���Χ186~220V��Ĭ��187V
	U16_T               u16_cb_high_volt;                // ��ĸ��ѹ���ޣ���Χ220~242V��Ĭ��235V
	U16_T               u16_cb_low_volt;                 // ��ĸǷѹ���ޣ���Χ198~220V��Ĭ��205V
	U16_T               u16_bus_high_volt;               // ĸ�߹�ѹ���ޣ���Χ220~320V��Ĭ��286V
	U16_T               u16_bus_low_volt;                // ĸ��Ƿѹ���ޣ���Χ186~220V��Ĭ��187V
	U16_T               u16_cb_output_volt;              // ��ĸ�����ѹ�����������ѹ������Χ210~230V��Ĭ��220V

	U16_T               u16_insu_volt_imbalance;         // ��Ե��ѹ��ƽ�⣬��Χ20~100V��Ĭ��50V
	U16_T               u16_insu_res_thr;                // ��Ե�������ޣ���Χ5~99K��Ĭ��25K
	U16_T               u16_dc_bus_input_ac_thr;         // ������ֱ����������ֵ����Χ1~50V��Ĭ��10V
}DC_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10
typedef struct                                           /* DC10ģ������ */
{
	U8_T                 u8_dc10_module_num;			 //DC10ģ���������Χ0~1��Ĭ��0

	U16_T                u16_shunt_a1_rated_volt;        // ���������ѹ����Χ1~500mv��Ĭ��75mv
	U16_T                u16_a1_s1_shunt_range;          // A1��S1��������(������������������)����Χ25~2000A��Ĭ��100A
	U16_T                u16_shunt_a2_rated_volt;        // ���������ѹ����Χ1~500mv��Ĭ��75mv
	U16_T                u16_a2_s2_shunt_range;          // A2��S2��������(������������������)����Χ25~2000A��Ĭ��100A
	U16_T                u16_shunt_a3_rated_volt;        // ���������ѹ����Χ1~500mv��Ĭ��75mv	
	U16_T                u16_a3_s3_shunt_range;          // A3��S3��������(������������������)����Χ25~2000A��Ĭ��100A

	U16_T                u16_cb_output_volt;             //����ֱ����ĸ���

	//DC10ģ����Ҫ���������ڶ���ֱ��ĸ�ߵ�ѹ���������Լ��������ƣ��������¼�������δ�õ�
	U16_T                u16_ac_meas_modle;              //DC10ģ�齻������ģʽ��Ĭ��0x0100һ·����
	U16_T                u16_ac_meas_num;                //DC10ģ�齻������·����Ĭ��0x0002�̶�һ·	

#define	RELAY_OUT_NUM    9
	U8_T                 u8_relay_out[RELAY_OUT_NUM];    // �������·��
}DC10_CFG_T;

//g_t_share_data.t_sys_cfg.t_feeder_panel[].t_feeder_module[]
//g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[]
//g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[]
//g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[]
typedef struct                                           /* ����ģ������ */
{
	U8_T               u8_feeder_num;                    // ������ģ���ʵ��֧·��
	
	U8_T               u8_alarm_feeder_num;              // �澯��֧·������Χ0~64��Ĭ��21
	SWT_INPUT_TYPE_E   e_alarm_type;                     // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���

	U8_T               u8_state_feeder_num;              // ״̬��֧·������Χ0~64��Ĭ��21
	SWT_INPUT_TYPE_E   e_state_type;                     // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���

	U8_T               u8_insu_feeder_num;               // ��Ե��֧·������Χ0~64��Ĭ��21
	U8_T               u8_curr_feeder_num;               // ������֧·������Χ0~64��Ĭ��0	
}FEEDER_MODULE_CFG_T;


//g_t_share_data.t_sys_cfg.t_feeder_panel[]
typedef struct                                           /* ���������� */
{
	U8_T                 u8_feeder_module_num;           //���߼��ģ���������Χ0~4��Ĭ��1
	U8_T                 u8_feeder_branch_num;           //����֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_state_feeder_num;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_insu_feeder_num;             //��Ե��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_alarm_feeder_num;            //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_curr_feeder_num;             //������֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_feeder_start_num[FEEDER_PANEL_MODULE_MAX];//����ģ��֧·��������֧·�е���ʼ֧·��
	FEEDER_MODULE_CFG_T  t_feeder_module[FEEDER_PANEL_MODULE_MAX]; //����ģ����������                
}DC_FEEDER_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel
typedef struct                                           /* ֱ����������� */
{
	RECT_CFG_T  t_rect;                                  // ����ģ������
	AC_CFG_T    t_ac;                                    // ֱ��ϵͳ������������
	DC_CFG_T    t_dc;                                    // ֱ�������������
	SWT_CFG_T   t_swt;                                   // ������������ 
	DC10_CFG_T  t_dc10;                                  // 1#DC10�������ݣ����ڶ���ֱ��ĸ����ز���                                              
}DC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_batt
typedef struct                                           /* ��������� */
{
	BMS_TYPE_E   e_bms_type;                             // ��������࣬B21/B3/B4��Ĭ��B21
	                                                     
	F32_T        f32_cell_high_volt;                     // �����ѹ����Χ2~15V��Ĭ��12.05V
	F32_T        f32_cell_low_volt;                      // ����Ƿѹ����Χ1.8~12V��Ĭ��10.08V
	F32_T        f32_tail_high_volt;                     // ĩ�˵����ѹ����Χ2~15V��Ĭ��12.05V
	F32_T        f32_tail_low_volt;                      // ĩ�˵���Ƿѹ����Χ1.8~12V��Ĭ��10.08V
	                                                     
	struct                                               
	{                                                    
		U8_T     u8_bms_num;                             // ����Ǹ�������Χ0~5��Ĭ��0
		U8_T     u8_cell_total_num;                      // ʵ�ʵ����ؽ���
		U8_T     u8_cell_num[BATT_METER_MAX];            // ����ǵĵ�ص�����
		                                                 // B21���ɲ�24�ڵ��
													     // B3���ɲ�54�ڵ�أ�ֻ��u8_cell_num[0]��u8_cell_num[1]��Ч
		                                                 // B4���ɲ�120�ڵ�أ�ֻ��u8_cell_num[0]��Ч
		                                                 // u8_cell_num[0]�ķ�Χ0~120��Ĭ��24
														 // u8_cell_num[1]�ķ�Χ0~54��Ĭ��24
														 // u8_cell_num[2]�ķ�Χ0~24��Ĭ��24
														 // u8_cell_num[3]�ķ�Χ0~24��Ĭ��24
														 // u8_cell_num[4]�ķ�Χ0~24��Ĭ��24
	}t_batt_group[BATT_GROUP_MAX];
}BATT_CFG_T;

//g_t_share_data.t_sys_cfg.t_dcdc_panel
typedef struct                                           /* ͨ�ŵ�Դ������ */                                           
{
	U8_T                 u8_dcdc_module_num;             //ͨ��ģ���������Χ0~8��Ĭ��0
	U8_T                 u8_feeder_module_num;           //���߼��ģ���������Χ0~1��Ĭ��0
	U8_T                 u8_feeder_branch_num;           //����֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_state_feeder_num;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_alarm_feeder_num;            //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_curr_feeder_num;             //������֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U16_T                u16_feeder_shunt_range;         // ֧·�������������̣���Χ10~1000A��Ĭ��100A

	DCDC_PROTOCOL_E      e_protocol;                     // ͨ��Э�飬Э��Ϊmodbus/can��Ĭ��modbus
	DCDC_RATED_CURR_E    e_rated_curr;                   // �������5A/10A/20A/30A/40A/50A/60A/80A/100A��ѡ��Ĭ��100A
	F32_T                f32_out_volt;                   // �����ѹ����Χ40~60��Ĭ��48
	F32_T                f32_curr_percent;               // �����ٷֱȣ���Χ10%~100%��Ĭ��100

	FEEDER_MODULE_CFG_T  t_feeder_module[DCDC_FEEDER_MODULE_MAX];                // ����ģ������
}DCDC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_dcac_panel
typedef struct                                           /* ����Դ������ */
{
	U8_T                 u8_dcac_module_num;             //���ģ���������Χ0~8��Ĭ��0
	U8_T                 u8_feeder_module_num;           //���߼��ģ���������Χ0~1��Ĭ��0
	U8_T                 u8_feeder_branch_num;           //����֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_state_feeder_num;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_alarm_feeder_num;            //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_curr_feeder_num;             //������֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U16_T                u16_feeder_shunt_range;         // ֧·�������������̣���Χ10~1000A��Ĭ��100A

	INTERNAL_PROTOCOL_E  e_protocol;                     // ͨ��Э�飬Э��̶�Ϊmodbus��������

	FEEDER_MODULE_CFG_T  t_feeder_module[DCAC_FEEDER_MODULE_MAX];                // ����ģ������


}DCAC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_ac_panel.t_feed
typedef struct                                           /* ���������� */
{
	//����ϵͳ����ģ�����ò���
	U8_T                 u8_feeder_module_num;           //���߼��ģ���������Χ0~3��Ĭ��0
	U8_T                 u8_feeder_branch_num;           //����֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_state_feeder_num;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_alarm_feeder_num;            //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_curr_feeder_num;             //������֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	U8_T                 u8_feeder_start_num[ACS_FEEDER_MODULE_MAX];//����ģ��֧·��������֧·�е���ʼ֧·��
	U16_T                u16_feeder_shunt_range;         // ֧·�������������̣���Χ10~1000A��Ĭ��100A

	FEEDER_MODULE_CFG_T  t_feeder_module[ACS_FEEDER_MODULE_MAX];                // ����ģ������               
}AC_FEEDER_CFG_T;

//g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.
typedef struct                                           /* DC10ģ������ */
{
	//����ϵͳDC10ģ�����ò���
	U16_T            	 u16_high_volt;                  // ������ѹ�㣬��Χ220~530V��Ĭ��456V
	U16_T            	 u16_low_volt;                   // ����Ƿѹ�㣬��Χ187~380V��Ĭ��327V
	U16_T            	 u16_lack_phase;                 // ����ȱ��㣬��Χ0~380V��Ĭ��200V���ڵ�������£�������Ч

	U8_T                 u8_ac10_module_num;			 //DC10ģ���������Χ0~1��Ĭ��0

	U16_T                u16_ac_meas_modle;              //DC10ģ�齻������ģʽ��Ĭ��0x0200��·����
	U16_T                u16_ac_meas_num;                //DC10ģ�齻������·����Ĭ��0x0000һ·����

//	U16_T                u16_shunt_a1_rated_volt;        // ���������ѹ����Χ1~500mv��Ĭ��75mv
	U16_T                u16_a1_s1_shunt_range;          // A1��S1��������(������������������)����Χ25~2000A��Ĭ��100A
//	U16_T                u16_shunt_a2_rated_volt;        // ���������ѹ����Χ1~500mv��Ĭ��75mv
	U16_T                u16_a2_s2_shunt_range;          // A2��S2��������(������������������)����Χ25~2000A��Ĭ��100A
//	U16_T                u16_shunt_a3_rated_volt;        // ���������ѹ����Χ1~500mv��Ĭ��75mv	
	U16_T                u16_a3_s3_shunt_range;          // A3��S3��������(������������������)����Χ25~2000A��Ĭ��100A	

//#define	RELAY_OUT_NUM    9
//	U8_T                 u8_relay_out[RELAY_OUT_NUM];    // �������·��
}AC10_CFG_T;

//g_t_share_data.t_sys_cfg.t_ac_panel
typedef struct                                           /* ֱ����������� */
{
	AC10_CFG_T  	t_ac10;                                  // 2#DC10�������ݣ����ڶ���ֱ��ĸ����ز���
	AC_FEEDER_CFG_T t_feed;                                  // �������߲������� 	                                              
}AC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_swt_ctrl[]
typedef struct                                           /* ϵͳ���ؿ������� */
{
	U8_T                 u8_swt_ctrl[SWT_CTRL_MAX];      //RC10ģ�鿪�ؿ���ֵ����Χ0~1(0��բ��1��բ)��Ĭ��0
}SWT_CTRL_T;

//g_t_share_data.t_sys_cfg.t_ctl
typedef struct                                           /* ϵͳ�������� */
{
                                                         // ����ֶ�����
	U16_T u16_batt[2];                                   // bit0~1:�ֶ������ƣ�0:���䣬1:���䣬2:����
	                                                     // bit2~15:����   
	              
	U16_T u16_ac;                                        // bit0~1:�����������ԣ�0��һ·���ȣ�1���̶�һ·��2����·���ȣ�3���̶���·
														 // bit2~15:����
	
	U16_T u16_rect_ctrl[RECT_CNT_MAX];                   // ���ģ��״̬���ƣ�1���ػ���0������

	U8_T  u8_sys_relay;                                  // ϵͳ�ܹ��ϼ̵�����0����������1������
	U8_T  u8_relay[RELAY_MAX];                           // 6���̵�����0����������1������

	U8_T  u8_swt_ctrl[SWT_CTRL_MAX];                     // �ɽ������õĿ��ؿ���ֵ��0����բ��1����բ
}SYS_CTL_T;

//g_t_share_data.t_sys_cfg
typedef struct                                           /*��������*/
{
	BATT_MGMT_CFG_T        t_batt_mgmt;                       // ��ع�������
	MISC_PARAM_CFG_T       t_sys_param;                       // ϵͳ��������

	AC_PANEL_CFG_T         t_ac_panel;                        // ���������� 
	DC_PANEL_CFG_T         t_dc_panel;                        // ֱ��������
	DC_FEEDER_PANEL_CFG_T  t_feeder_panel[FEEDER_PANEL_MAX];  // ����������
	DCDC_PANEL_CFG_T       t_dcdc_panel;                      // ͨ�ŵ�Դ������
	DCAC_PANEL_CFG_T       t_dcac_panel;                      // ����Դ������

	BATT_CFG_T             t_batt;                            // ���������
	SYS_CTL_T              t_ctl;                             // ϵͳ����

	SWT_CTRL_T             t_swt_ctrl[RC10_MODULE_MAX];       // ��ٿ��ؿ���
}SYS_CFG_T;


/***************************** ʵʱ���ݶ��� *********************************************/
//g_t_share_data.t_rt_data.t_batt.t_batt_group[]
typedef struct                                           /* �����ʵʱ���� */
{
	F32_T   f32_max_cell_volt;                           // �����ѹ���ֵ
	F32_T   f32_min_cell_volt;                           // �����ѹ���ֵ
	F32_T   f32_temperature1;                            // �����¶�1
	F32_T   f32_temperature2;                            // �����¶�2
	F32_T   f32_cell_volt[BATT_CELL_MAX];                // �����ѹ
	F32_T   f32_cell_max_volt_id;                        // ��ߵ�ѹ����ţ���̨��
	F32_T   f32_cell_min_volt_id;                        // ��͵�ѹ����ţ���̨��
	U8_T    u8_cell_max_volt_id;                         // ��ߵ�ѹ����ţ���ʾ��
	U8_T    u8_cell_min_volt_id;                         // ��͵�ѹ����ţ���ʾ��
	U8_T    u8_cell_state[BATT_CELL_MAX];                // �����Ƿѹ��0��������1����ѹ��2��Ƿѹ
	U8_T    u8_comm_state[BATT_METER_MAX];               // [��������Ǹ���]��ÿ��Ԫ�ش���һ�������ͨ��״̬��0��������1���ж�
	F32_T   f32_capacity;                                // ����鵱ǰ������0~C10Ah
}BATT_GROUP_RT_DATA_T;

//g_t_share_data.t_rt_data.t_batt
typedef struct                                           /* ���ʵʱ����ͨ��*/
{
	BATT_STATE_E   e_state[2];                              // ����鵱ǰ״̬
	U16_T          u16_time_from_change_state[2];           // ���ϴε��״̬ת�俪ʼ������������ȥʱ�䣬����״̬��λΪСʱ������ͺ���״̬��λ�Ƿ���
	F32_T          f32_total_capacity;                   // ���������������������֮��
	BATT_GROUP_RT_DATA_T t_batt_group[BATT_GROUP_MAX];   // �����ʵʱ����
}BATT_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel.t_rect[]
typedef struct                                           /* ����ģ��ʵʱ���� */
{
	U32_T   u32_hw_ver;                                  // Ӳ���汾��
	U32_T   u32_sw_ver;                                  // ����汾��
	U32_T	u32_barcode;                                 // �豸������
	F32_T   f32_out_volt;                                // �����ѹ
	F32_T   f32_out_curr;                                // �������
	F32_T   f32_curr_percent;                            // ģ�������ٷ���
	F32_T   f32_max_out_volt;                            // �����ѹ����
	F32_T   f32_min_out_volt;                            // �����ѹ����
	//F32_T   f32_offline_out_volt;                        // ģ��Ĭ�������ѹ����Χ176~286V��Ĭ��220V
	//F32_T   f32_offline_out_curr;                        // ģ��Ĭ�������������Χ5~110%��Ĭ��110%
	
	U16_T   u16_state;                                   // ģ��״̬��Ϣ
	                                                     // bit0:ģ�鿪�ػ�״̬      1���ػ���0������
										                 // bit1:ģ���Զ�/�ֶ�״̬   1���ֶ���0���Զ�
	                                                     // bit2:����״̬            1��������0������
	                                                     // bit3:ģ�����״̬        1�����ϣ�0������
	                                                     // bit4:�����ѹ״̬        1���������ѹ��0������
	                                                     // bit5:����			
	                                                     // bit6:����			
	                                                     // bit7:����			
	                                                     // bit8:ģ�鿪�ػ�״̬      1���ػ���0������
	                                                     // bit9:ģ���Զ�/�ֶ�״̬   1���ֶ���0���Զ�
	                                                     // bit10:ģ�����Ƿѹ״̬   1��Ƿѹ��0������
	                                                     // bit11:ģ���¶�״̬       1�����£�0������
	                                                     // bit12:ģ������״̬       1����ѹ��Ƿѹ��0������
	                                                     // bit13:ģ������״̬       1��ȱ�࣬0������
	                                                     // bit14:ģ�鿪�ػ�״̬     1���ػ���0������
	                                                     // bit15:ģ�������ѹ״̬   1����ѹ��0������

	BOOL_T  b_ctl_mode;                                  // ģ����Ʒ�ʽ             0���Զ���1���ֶ�
	MODULE_STATE_E  e_module_state;                      // ģ�鿪�ػ����쳣״̬     0��������1���ػ���2���쳣
	U8_T    u8_comm_state;                               // ͨѶ״̬��0��������1���ж�
}RECT_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel.t_ac
typedef struct                                           /* �����ɼ���Ԫʵʱ���� */
{
	// ״̬��ʵʱ����
	U16_T   u16_state;                                   // ����״̬��Ϣ��16λ��ÿλ����һ��״̬
	                                                     // bit0:����һ·ͣ��		
	                                                     // bit1:����һ·Ƿѹ		
	                                                     // bit2:����һ·��ѹ		
	                                                     // bit3:����һ·ȱ��		
	                                                     // bit4:������·ͣ��		
	                                                     // bit5:������·Ƿѹ		
	                                                     // bit6:������·��ѹ		
	                                                     // bit7:������·ȱ��
														 // bit8:ԭ���������ϸ�Ϊd21ģ��ͨ���ж�
	
	// ģ����ʵʱ����
	F32_T   f32_first_path_volt_uv;                      // 1·UV�ߵ�ѹ
	F32_T   f32_first_path_volt_vw;                      // 1·VW�ߵ�ѹ
	F32_T   f32_first_path_volt_wu;                      // 1·WU�ߵ�ѹ
	        
	F32_T   f32_second_path_volt_uv;                     // 2·UV�ߵ�ѹ
	F32_T   f32_second_path_volt_vw;                     // 2·VW�ߵ�ѹ
	F32_T   f32_second_path_volt_wu;                     // 2·WU�ߵ�ѹ
}AC_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel.t_dc
typedef struct                                           /* ֱ���ɼ���Ԫʵʱ���� */
{
	// ״̬��Ϣ                                          
	U16_T   u16_state;		                             // ֱ��״̬��Ϣ
	                                                     // bit0:һ�κ�ĸ��ѹ		
	                                                     // bit1:һ�κ�ĸǷѹ		
	                                                     // bit2:һ�ο�ĸ��ѹ		
	                                                     // bit3:һ�ο�ĸǷѹ
														 // bit4:һ��ĸ�߹�ѹ		
	                                                     // bit5:һ��ĸ��Ƿѹ		
	                                                     // bit6:һ���ع�ѹ		
	                                                     // bit7:һ����Ƿѹ				
	                                                     // bit8:DC10ģ��ͨ���ж�//�����������쳣	
	                                                     // bit9: һ���ع���		
	                                                     // bit10:�����ع���		
	                                                     // bit11:һ��ĸ�ߵ�����ƽ��		
	                                                     // bit12:һ��ĸ�ߵ�ѹ��ƽ��		
	                                                     // bit13:һ��ĸ�߾�Ե�½�		
	                                                     // bit14:ĸ�߾�Ե�̵����澯��ֱ������ʹ�ã�		
	                                                     // bit15:ֱ�����뽻���澯	
														 	
	U16_T   u16_state2;		                             // ֱ��״̬��Ϣ
	                                                     // bit0:���κ�ĸ��ѹ		
	                                                     // bit1:���κ�ĸǷѹ		
	                                                     // bit2:���ο�ĸ��ѹ		
	                                                     // bit3:���ο�ĸǷѹ
														 // bit4:����ĸ�߹�ѹ		
	                                                     // bit5:����ĸ��Ƿѹ		
	                                                     // bit6:�����ع�ѹ		
	                                                     // bit7:������Ƿѹ				
	                                                     // bit8:Ԥ��
	                                                     // bit9: Ԥ��		
	                                                     // bit10:Ԥ��		
	                                                     // bit11:����ĸ�ߵ�����ƽ��		
	                                                     // bit12:����ĸ�ߵ�ѹ��ƽ��		
	                                                     // bit13:����ĸ�߾�Ե�½�		
	                                                     // bit14:��ĸ�߾�Ե�̵����澯��ֱ������ʹ�ã�		
	                                                     // bit15:����ֱ�����뽻���澯	                                                     
	                                                     
	// ֱ���ɼ���Ԫʵʱ����
	//һ��ĸ��ʵʱ����-��SC32����ɼ�                              
	F32_T   f32_pb_volt;                                 // һ�κ�ĸ��ѹ
	F32_T   f32_cb_volt;                                 // һ�ο�ĸ��ѹ
	F32_T   f32_load_curr;                               // һ�ο�ĸ������Ҳ�Ƹ��ص���	                                                     
	F32_T   f32_batt_volt;                               // һ���ص�ѹ

	//����ĸ��ʵʱ����-��DC10ģ��ɼ�
	F32_T   f32_pb2_volt;                                // ���κ�ĸ��ѹ
	F32_T   f32_cb2_volt;                                // ���ο�ĸ��ѹ
	F32_T   f32_load2_curr;                              // ���ο�ĸ������Ҳ�Ƹ��ص���	                                                     
	F32_T   f32_batt2_volt;                              // �����ص�ѹ

	F32_T   f32_batt_curr[BATT_GROUP_MAX];               // һ�������ص���
	F32_T   f32_batt_total_curr;                         // ����ܵ����������ص���֮��
	                                                     
	F32_T   f32_temperature;                             // �����¶�

	// ����ģ�������ĸ�߾�Ե��Ϣ����ʾ����ʹ������ģ��������Ϣ��
	//һ��ĸ�߾�Եʵʱ����
	F32_T   f32_cb_pos_to_gnd_volt;                      // һ�ο�ĸ���Եص�ѹ
	F32_T   f32_pb_pos_to_gnd_volt;                      // һ�κ�ĸ���Եص�ѹ	                                                     
	F32_T   f32_bus_neg_to_gnd_volt;                     // һ��ĸ�߸��Եص�ѹ

	F32_T   f32_bus_pos_to_gnd_res;                      // һ��ĸ�����Եص���
	F32_T   f32_bus_neg_to_gnd_res;                      // һ��ĸ�߸��Եص���

	F32_T   f32_feeder_min_to_gnd_res;                   // һ��֧·�Եؾ�Ե������Сֵ
	F32_T   f32_bus_to_gnd_ac_volt;                      // һ��ĸ�߶Եؽ�����ѹ��Чֵ

	//����ĸ�߾�Եʵʱ����
	F32_T   f32_cb2_pos_to_gnd_volt;                     // ���ο�ĸ���Եص�ѹ
	F32_T   f32_pb2_pos_to_gnd_volt;                     // ���κ�ĸ���Եص�ѹ	                                                     
	F32_T   f32_bus2_neg_to_gnd_volt;                    // ����ĸ�߸��Եص�ѹ

	F32_T   f32_bus2_pos_to_gnd_res;                     // ����ĸ�����Եص���
	F32_T   f32_bus2_neg_to_gnd_res;                     // ����ĸ�߸��Եص���

	F32_T   f32_feeder2_min_to_gnd_res;                  // ����֧·�Եؾ�Ե������Сֵ
	F32_T   f32_bus2_to_gnd_ac_volt;                     // ����ĸ�߶Եؽ�����ѹ��Чֵ
	
	//�Դ�ֱ��������ĸ�߾�Ե��Ϣ
	F32_T   f32_dc_cb_pos_to_gnd_volt;                   // һ�ο�ĸ���Եص�ѹ
	F32_T   f32_dc_pb_pos_to_gnd_volt;                   // һ�κ�ĸ���Եص�ѹ	                                                     
	F32_T   f32_dc_bus_neg_to_gnd_volt;                  // һ��ĸ�߸��Եص�ѹ                                                 
}DC_RT_DATA_T;  

//g_t_share_data.t_rt_data.t_dc_panel.t_dc10.
typedef struct                                           /* DC10ģ��ʵʱ���� */
{
	F32_T   f32_temperature;                             // �����¶�
		
	F32_T   f32_v1_volt;                                 // ��ĸ��ѹ
	F32_T   f32_a1_curr;                                 // ��ĸ������Ҳ�Ƹ��ص���	                                                     
	F32_T   f32_v2_volt;                                 // ��ص�ѹ
	F32_T   f32_a2_curr;                                 // ��ص���
	F32_T   f32_v3_volt;                                 // ��ĸ��ѹ
	F32_T   f32_a3_curr;                                 // ���2����

	F32_T   f32_s1_curr;                                 // ��ĸ������Ҳ�Ƹ��ص���
	F32_T   f32_s2_curr;                                 // ��ص���
	F32_T   f32_s3_curr;                                 // ���2����

	U16_T   u16_swt_state;                               //DC10��������Ϣ
	                                                     //bit0~15:����01~16״̬��0��������1�����ϣ�
	U8_T    u8_swt_state[DC10_SWT_BRANCH_MAX];	         //ֱ��ϵͳDC10ģ�鿪��״̬����1���պϣ�0���Ͽ���16·��������״̬�����ں�̨ͨ��

	U8_T    u8_comm_state;                               //ͨѶ״̬��0��������1���ж�
}DC10_RT_DATA_T;

//g_t_share_data.t_rt_data.t_feeder_panel[].t_feeder[]
//g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[]
//g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[]
//g_t_share_data.t_rt_data.t_ac_panel.t_feeder[]
typedef struct                                           /* ֧·ʵʱ���� */
{
	U8_T u8_alarm;                                       // ���ظ澯��00��·�޿��ظ澯��⣬01��ʾ������02��ʾ�澯
	U8_T u8_state;                                       // ����״̬��00��·�޿���״̬��⣬01��ʾ�Ͽ���02��ʾ�պ�
	U8_T  u8_insu_state;                                 // ֧·��Ե״̬��00��·�޾�Ե��⣬01��ʾ������02��ʾ��Ե�쳣��03��ʾ��Ե�������쳣
	U8_T  u8_curr_state;                                 // ֧·����������״̬��00��·��֧·������⣬01��ʾ������03��ʾ�����������쳣
	
	F32_T f32_res;                                       // ֧·��Ե����
	F32_T f32_curr;                                      // ֧·����
}FEEDER_RT_DATA_T;

//g_t_share_data.t_rt_data.t_feeder_panel[].t_feeder_module[]
//g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[]
//g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[]
//g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[]
typedef struct                                           /* ����ģ��ʵʱ���� */
{
	U32_T   u32_hw_ver;                                  // Ӳ���汾��
	U32_T   u32_sw_ver;                                  // ����汾��
	U32_T	u32_barcode;                                 // �豸������
	U8_T    u8_comm_state;                               // ͨѶ״̬��0��������1���ж�
}FEEDER_MODULE_RT_DATA_T;

//g_t_share_data.t_rt_data.t_ac_panel.t_ac10.
typedef struct                                           /* ���ڽ����������2#DC10ģ��ʵʱ���� */
{	//һ·Ϊ�������	
	F32_T   f32_ac1_uv_volt;                             // һ·UV�ߵ�ѹ
	F32_T   f32_ac1_vw_volt;                             // һ·VW�ߵ�ѹ	                                                     
	F32_T   f32_ac1_wu_volt;                             // һ·WU�ߵ�ѹ
	F32_T   f32_s1_curr;                                 // һ·��������

	//��·Ϊ�������
	F32_T   f32_ac2_uv_volt;                             // ��·UV�ߵ�ѹ
	F32_T   f32_ac2_vw_volt;                             // ��·VW�ߵ�ѹ	                                                     
	F32_T   f32_ac2_wu_volt;                             // ��·WU�ߵ�ѹ		
	F32_T   f32_s3_curr;                                 // ��·��������

	U16_T   u16_ac_state;                                // ��������״̬

	U16_T   u16_swt_state;                               //AC10��������Ϣ
	                                                     //bit0~15:����01~16״̬��0��������1�����ϣ�
	U8_T    u8_swt_state[DC10_SWT_BRANCH_MAX];	         //����ϵͳDC10ģ�鿪��״̬����1���պϣ�0���Ͽ���16·��������״̬�����ں�̨ͨ��

	U8_T    u8_comm_state;                               //ͨѶ״̬��0��������1���ж�
} AC10_RT_DATA_T;

//g_t_share_data.t_rt_data.t_ac_panel.
typedef struct                                           /* ���ڽ����������2#DC10ģ��ʵʱ���� */
{	
	AC10_RT_DATA_T			t_ac10;

	//����ϵͳ����ʵʱ����
	FEEDER_MODULE_RT_DATA_T t_feeder_module[ACS_FEEDER_MODULE_MAX];   //����ģ������
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //����֧·����
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //֧·������״̬��00��·�޴�������01��ʾ������������02��ʾ�������쳣
	                                                                  //��u8_sensor_state�ᵽFEEDER_RT_DATA_T�ṹ����ɽ�ʡ�ڴ�4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX

	U8_T                    u8_total_swt_falut;                       //�ܿ��ر���״̬��00��ʾ������01��ʾ�澯
}AC_PANEL_RT_DATA_T;//                                         

//g_t_share_data.t_rt_data.t_dc_panel.t_swt
typedef struct
{
	U8_T u8_raw_swt_state[SWT_BRANCH_MAX];               //����״̬����1���պϣ�0���Ͽ����Դ�20·��������״̬������ģ��DC10ͨ��
	U8_T u8_swt_state[SWT_BRANCH_MAX];                   //����״̬����1���պϣ�0���Ͽ����Դ�20·��������״̬�����ں�̨ͨ��
	U8_T u8_state[SWT_BRANCH_MAX];                       //����״̬����1���պϣ�0���Ͽ�
	U8_T u8_alm[SWT_BRANCH_MAX];                         //���ظ澯����1���澯��0�����澯

#define	ECSWT_MAX_NUM_FROM_FDL		(11 * SWT_CTRL_MAX)
	U8_T u8_fdl_swt_state[ECSWT_MAX_NUM_FROM_FDL];       //����״̬����1���պϣ�0���Ͽ�����FC10ģ��������Ŀ���״ֵ̬
}SWT_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel
typedef struct                                           /* ֱ����ʵʱ���� */
{
	// ״̬��ʵʱ����
	U16_T   u16_total_state;                             // ֱ����״̬��Ϣ��16λ��ÿλ����һ��״̬
	                                                     // bit0:��������		
	                                                     // bit1:ֱ��ĸ�߹���		
	                                                     // bit2:��ع���		
	                                                     // bit3:��Ե����		
	                                                     // bit4:���ģ�����		
	                                                     // bit5:ͨ�ŵ�Դ����		
	                                                     // bit6:����Դ����		
	                                                     // bit7:�������ع���
	                                                     // bit8:ֱ��ϵͳֱ��ĸ�߾�Ե�쳣
	                                                     // bit9:ֱ��ϵͳ����֧·��Ե�쳣
	                                                     // bit10:ֱ��ϵͳ�������ѹ�쳣
	                                                     
	AC_RT_DATA_T    t_ac;                                // ֱ��ϵͳ�н�������ʵʱ����
	DC_RT_DATA_T    t_dc;                                // ֱ��ʵʱ����
	DC10_RT_DATA_T  t_dc10;                              // 1#DC10ģ������Ķ���ֱ��ĸ�����ʵʱ����	 
	RECT_RT_DATA_T  t_rect[RECT_CNT_MAX];                // ģ��ʵʱ����
	SWT_RT_DATA_T   t_swt;                               // ������ʵʱ����              
}DC_PANEL_RT_DATA_T;


//g_t_share_data.t_rt_data.t_feeder_panel[]
typedef struct                                                        /* ������ʵʱ���� */
{
	FEEDER_MODULE_RT_DATA_T t_feeder_module[FEEDER_PANEL_MODULE_MAX]; //����ģ������
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //����֧·����
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //֧·������״̬��00��·�޴�������01��ʾ������������02��ʾ�������쳣
	                                                                  //��u8_sensor_state�ᵽFEEDER_RT_DATA_T�ṹ����ɽ�ʡ�ڴ�4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX
	
	U8_T                    u8_total_swt_falut;                       //�ܿ��ر���״̬��00��ʾ������01��ʾ�澯
}FEEDER_PANEL_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[]
typedef struct                                           /* ͨ��ģ��ʵʱ���� */
{
	U32_T   u32_hw_ver;                                  // Ӳ���汾��
	U32_T   u32_sw_ver;                                  // ����汾��
	U32_T	u32_barcode;                                 // �豸������
	F32_T   f32_out_volt;                                // �����ѹ
	F32_T   f32_out_curr;                                // �������
	F32_T   f32_curr_percent;                            // ģ�������ٷ���
	F32_T   f32_max_out_volt;                            // �����ѹ����
	F32_T   f32_min_out_volt;                            // �����ѹ����
	//F32_T   f32_offline_out_volt;                        // ģ��Ĭ�������ѹ����Χ176~286V��Ĭ��220V
	//F32_T   f32_offline_out_curr;                        // ģ��Ĭ�������������Χ5~110%��Ĭ��110%
	
	U16_T   u16_state;                                   // ģ��״̬��Ϣ
	                                                     // bit0:ģ�鿪�ػ�״̬      1���ػ���0������
										                 // bit1:ģ���Զ�/�ֶ�״̬   1���ֶ���0���Զ�
	                                                     // bit2:����״̬            1��������0������
	                                                     // bit3:ģ�����״̬        1�����ϣ�0������
	                                                     // bit4~15:����

	BOOL_T  b_ctl_mode;                                  // ģ����Ʒ�ʽ             0���Զ���1���ֶ�
	MODULE_STATE_E  e_module_state;                      // ģ�鿪�ػ����쳣״̬     0��������1���ػ���2���쳣
	U8_T    u8_comm_state;                               // ͨѶ״̬��0��������1���ж�
}DCDC_MODULE_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dcdc_panel
typedef struct                                                        /* ͨ�ŵ�Դ��ʵʱ���� */
{
	F32_T                   f32_dcdc_bus_volt;                        //ͨ��ϵͳĸ�ߵ�ѹ������ͨ��ģ���е���������ѹΪĸ�ߵ�ѹ
	F32_T                   f32_dcdc_bus_curr;                        //ͨ��ϵͳĸ�ߵ���������ͨ��ģ���������֮��Ϊĸ�ߵ���
	DCDC_MODULE_RT_DATA_T   t_dcdc_module[DCDC_MODULE_MAX];           //ͨ��ģ������
	FEEDER_MODULE_RT_DATA_T t_feeder_module[DCDC_FEEDER_MODULE_MAX];  //����ģ������
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //����֧·����
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //֧·������״̬��00��·�޴�������01��ʾ������������02��ʾ�������쳣
	                                                                  //��u8_sensor_state�ᵽFEEDER_RT_DATA_T�ṹ����ɽ�ʡ�ڴ�4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX

	U8_T                    u8_total_swt_falut;                       //�ܿ��ر���״̬��00��ʾ������01��ʾ�澯
}DCDC_PANEL_RT_DATA_T;


//g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[]
typedef struct                                           /* ���ģ��ʵʱ���� */
{
	U32_T   u32_hw_ver;                                  // Ӳ���汾��
	U32_T   u32_sw_ver;                                  // ����汾��
	U32_T	u32_barcode;                                 // �豸������
	F32_T   f32_out_volt;                                // ���ģ�������ѹ
	F32_T   f32_out_curr;                                // ���ģ���������
	F32_T   f32_out_freq;                                // ���ģ�����Ƶ��
	F32_T   f32_out_power_factor;                        // ���ģ�������������
	F32_T   f32_inverter_volt;                           // ���ģ������ѹ
	F32_T   f32_bypass_input_volt;                       // ��·�����ѹ
	F32_T   f32_bypass_input_freq;                       // ��·����Ƶ��
	F32_T   f32_batt_input_volt;                         // ��������ѹ   
	F32_T   f32_active_power;                            // ���ģ������й�����
	F32_T   f32_apparen_power;                           // ���ģ��������ڹ���
	F32_T   f32_load_ratio;                              // ���ģ�����������
	F32_T   f32_temperature;                             // ģ���¶�
	F32_T   f32_outage_capacity_ratio;                   // ���ģ�����������
	F32_T   f32_bypass_high_volt;                        // ��·��ѹ����
	F32_T   f32_bypass_low_volt;                         // ��·��ѹ����
	
	U16_T   u16_state;                                   // ģ��״̬��Ϣ
	                                                     // bit0:ģ�鿪�ػ�״̬      1���ػ���0������
										                 // bit1:ģ�鹤����ʽ        1�����ߣ�0����
	                                                     // bit2:ģ�����״̬        1�����ϣ�0������
														 // bit3:ģ�����״̬        1�����أ�0������
														 // bit4:ģ���¶�״̬        1�����£�0������
														 // bit5:ģ����Ƿѹ        1��Ƿѹ��0������
														 // bit6:ģ����·����״̬    1����ѹ��Ƿѹ��0������
														 // bit7:�����ʽ            1��������    0����·���
	                                                     // bit8~15:����

	BOOL_T  b_alarm_state;                               // ģ�鱨��״̬             0��������1������
	DCAC_STATE_E  e_module_state;                      // ģ��״̬     0������/��䣬1������/��·��2���ػ�
	U8_T    u8_comm_state;                               // ͨѶ״̬��0��������1���ж�
}DCAC_MODULE_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dcac_panel
typedef struct                                                        /* ͨ�ŵ�Դ��ʵʱ���� */
{
	DCAC_MODULE_RT_DATA_T   t_dcac_module[DCAC_MODULE_MAX];           //ͨ��ģ������
	FEEDER_MODULE_RT_DATA_T t_feeder_module[DCAC_FEEDER_MODULE_MAX];  //����ģ������
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //����֧·����
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //֧·������״̬��00��·�޴�������01��ʾ������������02��ʾ�������쳣
	                                                                  //��u8_sensor_state�ᵽFEEDER_RT_DATA_T�ṹ����ɽ�ʡ�ڴ�4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX

	U8_T                    u8_total_swt_falut;                       //�ܿ��ر���״̬��00��ʾ������01��ʾ�澯
}DCAC_PANEL_RT_DATA_T;

//g_t_share_data.t_rt_data.t_rc10[]
typedef struct                                                        /* RC10ģ��ʵʱ���� */
{
	U8_T                    u8_comm_state;                       //RC10ģ��ͨ��״̬��00��ʾ������01��ʾ�澯
}RC10_RT_DATA_T;

//g_t_share_data.t_rt_data
typedef struct                                                /*ʵʱ����*/
{
	BATT_RT_DATA_T 	        t_batt;                           // �����
	AC_PANEL_RT_DATA_T      t_ac_panel;                       // ������2#DC10ģ������Ľ������ѹ���������ʵʱ����
	DC_PANEL_RT_DATA_T      t_dc_panel;                       // ֱ����
	FEEDER_PANEL_RT_DATA_T  t_feeder_panel[FEEDER_PANEL_MAX]; // ������
	DCDC_PANEL_RT_DATA_T    t_dcdc_panel;                     // ͨ����
	DCAC_PANEL_RT_DATA_T    t_dcac_panel;                     // UPS��
	RC10_RT_DATA_T          t_rc10[RC10_MODULE_MAX];          // RC10״̬
	U8_T                    t_u8_dc10_fault_out[7];           // ����DC10�ɽӵ����
}RT_DATA_T;


/************************ ������ֱ��У׼���ݶ��� ************************************/
//g_t_share_data.t_dc_adjust_data
typedef struct
{
	F32_T   f32_batt_volt;                 //��ص�ѹ                    
	F32_T   f32_pb_volt;                   //��ĸ��ѹ
	F32_T   f32_cb_volt;                   //��ĸ��ѹ
	F32_T   f32_bus_neg_to_gnd_volt;       //ĸ�߸��Եص�ѹ

	F32_T   f32_load_curr_1;                //���ص���1
	F32_T   f32_load_curr_2;                //���ص���2
	F32_T   f32_batt1_curr_1;               //һ���ص���1
	F32_T   f32_batt1_curr_2;               //һ���ص���2
	F32_T   f32_batt2_curr_1;               //�����ص���1
	F32_T   f32_batt2_curr_2;               //�����ص���2
}DC_ADJUST_DATA_T;


/************************ ����У׼���ݶ��� ************************************/
//g_t_share_data.t_ac_adjust_data
typedef struct
{
	F32_T   f32_first_path_volt_uv;        //1·UV�ߵ�ѹ
	F32_T   f32_first_path_volt_vw;        //1·VW�ߵ�ѹ
	F32_T   f32_first_path_volt_wu;        //1·WU�ߵ�ѹ
	        
	F32_T   f32_second_path_volt_uv;       //2·UV�ߵ�ѹ
	F32_T   f32_second_path_volt_vw;       //2·VW�ߵ�ѹ
	F32_T   f32_second_path_volt_wu;       //2·WU�ߵ�ѹ
}AC_ADJUST_DATA_T;

/***************************** У׼ϵ������ *********************************************/
//g_t_share_data.t_coeff_data.
typedef struct
{
	F32_T			f32_ac1_uv_slope;      // ����1·UVб��
	F32_T			f32_ac1_vw_slope;      // ����1·VWб��
	F32_T			f32_ac1_wu_slope;      // ����1·WUб��
	F32_T			f32_ac2_uv_slope;      // ����2·UVб��
	F32_T			f32_ac2_vw_slope;      // ����2·VWб��
	F32_T			f32_ac2_wu_slope;      // ����2·WUб��

	F32_T			f32_v1_vol_slope;      // V1��ѹ����б��
	S16_T			s16_v1_vol_zero;       // V1��ѹ�������
	F32_T			f32_v2_vol_slope;      // V2��ѹ����б��
	S16_T			s16_v2_vol_zero;       // V2��ѹ�������
	F32_T			f32_v3_vol_slope;      // V3��ѹ����б��
	S16_T			s16_v3_vol_zero;       // V3��ѹ�������

	S16_T			s16_a1_fixed_zero;     // A1��·�̶����
	S16_T			s16_a2_fixed_zero;     // A2��·�̶����
	S16_T			s16_a3_fixed_zero;     // A3��·�̶����

	F32_T			f32_a1_curr_slope;     // A1��������б��
	F32_T			f32_a1_curr_zero;      // A1�����������
	F32_T			f32_a2_curr_slope;     // A2��������б��
	F32_T			f32_a2_curr_zero;      // A2�����������
	F32_T			f32_a3_curr_slope;     // A3��������б��
	F32_T			f32_a3_curr_zero;      // A3�����������

	F32_T			f32_ref_volt;          // AD������׼��ѹ
	F32_T			f32_neg_vol_slope;     // ���Եص�ѹ����б��
	S16_T			s16_neg_vol_zero;      // ���Եص�ѹ�������
	
}COEFF_DATA_T;

/***************************** У׼��ADֵ *********************************************/
//g_t_share_data.t_ad_data.
typedef struct
{
	U16_T	u16_ac1_uv_ad;                 // ����1·UV����ADֵ
	U16_T	u16_ac1_vw_ad;                 // ����1·VW����ADֵ
	U16_T	u16_ac1_wu_ad;                 // ����1·WU����ADֵ
	U16_T	u16_ac2_uv_ad;                 // ����2·UV����ADֵ
	U16_T	u16_ac2_vw_ad;                 // ����2·VW����ADֵ
	U16_T	u16_ac2_wu_ad;                 // ����2·WU����ADֵ
	U16_T	u16_temp_ad;                   // �����¶Ȳ���ADֵ
	U16_T	u16_v1_ad;                     // V1����ADֵ
	U16_T	u16_v2_ad;                     // V2����ADֵ
	U16_T	u16_v3_ad;                     // V3����ADֵ
	U16_T	u16_a1_ad;                     // A1����ADֵ
	U16_T	u16_a2_ad;                     // A2����ADֵ
	U16_T	u16_a3_ad;                     // A3����ADֵ
	U16_T	u16_neg_v_ad;                     //���Եص�ѹ����AD
}AD_DATA_T;

/************************* �������ݶ��� ***************************************/
//g_t_share_data
typedef struct
{
	SYS_CFG_T         t_sys_cfg;           // ϵͳ����
	RT_DATA_T         t_rt_data;           // ʵʱ����
	AC_ADJUST_DATA_T  t_ac_adjust_data;    // ����У׼����ʵ����ֵ
	DC_ADJUST_DATA_T  t_dc_adjust_data;    // ֱ��У׼����ʵ����ֵ
	COEFF_DATA_T      t_coeff_data;        // У׼ϵ��
	AD_DATA_T         t_ad_data;           // У׼ADֵ

	SWT_OBJ_T         t_swt_obj[SWT_CTRL_MAX]; //
}SHARE_DATA_T;


/* ��ٿ����Ƿ���Ч���� */
typedef enum
{	
	INVALID = 0,     //��Ч
	VALID,           //��Ч
}SWT_VALID_E;

/* ��ٿ��ض���ṹ���� */
typedef struct
{
	U8_T u8_swt_valid;          //�õ�ٿ����Ƿ���Ч��ʶ��0��Ч��1��Ч
	U8_T *p_u8_swt_ctrl;        //ÿ����ٿ��ؿ���ֵָ��
	U8_T *p_u8_swt_state;       //ÿ����ٿ��ص�ǰ�Ϸ�״ֵָ̬��
}RC10_SWT_ITEM_T;

/* ������ģ������ĵ�ٿ��ض�Ӧת���ṹ���� */
typedef struct
{
	U8_T *p_u8_swt_dest;        //ÿ����ٿ��ص�ǰ״ֵ̬
	U8_T *p_u8_swt_sorce;       //ÿ����ٿ�����FC10ģ�����ֵ
	U8_T data_sorce;			//0:����FC10ģ�飬1:����SC32ģ��
#define FC10_SWT		0
#define SC32_SWT		1
}FDL_SWT_PAIR_T;

#endif
