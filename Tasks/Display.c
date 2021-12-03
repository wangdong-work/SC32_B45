/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Display.c
��    ����1.00
�������ڣ�2012-04-18
��    �ߣ�������
������������ʾ�Ͱ���������ص�ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-04-18  1.00     ����
**************************************************************/

#include <rtl.h>
#include <string.h>
#include <stdio.h>

#include "../Drivers/Lcd.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"
#include "../Drivers/Dataflash.h"

#include "CompareTime.h"
#include "PublicData.h"
#include "ShareDataStruct.h"
#include "Display.h"
#include "DisplayString.h"
#include "Type.h"
#include "FetchFlash.h"
#include "Fault.h"
#include "FaultId.h"
#include "Log.h"

#define MMI_REFRESH_INTERVAL     10         //ˢ�����10��tick��100ms
#define MMI_RETURN_MAIN_WIN_TIME 180000000  //(3*60*1000*1000), 3����û�а��������ر��ⷵ��������

#define MMI_ICON_UP_X          143     //"UP"ͼ���X����
#define MMI_ICON_UP_Y          2       //"UP"ͼ���Y����
#define MMI_ICON_DOWN_X        143     //"DOWN"ͼ���X����
#define MMI_ICON_DOWN_Y        145     //"DOWN"ͼ���X����

#define MMI_CELL_CNT           10      //��ص����ѹҳ����ʾ�ĵ�ؽ���
#define MMI_INSU_CNT           5       //����֧·��Եҳ����ʾ��֧·��
#define MMI_SWT_CNT            5       //����֧·����״̬ҳ����ʾ��֧·��
#define MMI_HIS_FAULT_CNT      2       //��ʷ����ҳ����ʾ�Ĺ�������
#define MMI_CURR_FAULT_CNT     2       //��ǰ����ҳ����ʾ�Ĺ�������
#define MMI_EXCEPTION_CNT      2       //����ĩ�������ҳ����ʾ�Ĺ�������
#define MMI_RECORD_CNT         3       //�¼���¼ҳ����ʾ�ļ�¼����
#define MMI_RECT_ON_OFF_CNT    10      //����ģ�鿪�ػ�ҳ����ʾ��ģ������
#define MMI_SWT_ON_OFF_CNT     10      //��ٿ���ҳ����ʾ�Ŀ�������
#define MMI_FEEDER_BRANCH_CNT  2       //����֧·��Ϣҳ����ʾ������֧·����

#define MMI_FAULT_NAME_WIDTH   120     //�������ƵĿ��

#define MMI_AC_FEEDER_MODULE_ADDR    1       //����������ģ����ʼ��ַ
#define MMI_FEEDER_PANEL_MODULE_ADDR 14      //����������ģ����ʼ��ַ
#define MMI_DCDC_FEEDER_MODULE_ADDR  112     //ͨ��������ģ����ʼ��ַ
#define MMI_DCAC_FEEDER_MODULE_ADDR  114     //UPS������ģ����ʼ��ַ

#define MMI_USER_PASSWORD      11111   //�û�Ĭ������
#define MMI_SUPER_PASSWORD     2051    //�û�����������
#define MMI_DEFEND_PASSWORD    2012    //ά��������

static U16_T  m_u16_swt_index=0;                    //��ٿ���������

/* ��220V��110Vϵͳ��صĵ�ѹֵ */
typedef struct
{
	F32_T  f32_batt_equ_volt;                 // ��ؾ����ѹ
	F32_T  f32_batt_flo_volt;                 // ��ظ����ѹ
	U16_T  u16_batt_total_end_volt;           // ��طŵ���ֹ��ѹ
	F32_T  f32_batt_high_volt_limit;          // ������ѹ��
	F32_T  f32_batt_low_volt_limit;           // �����Ƿѹ��
	       
	F32_T  f32_rect_out_volt;                 // ���ģ�������ѹ
	F32_T  f32_rect_max_out_volt;             // ���ģ�������ѹ����
	F32_T  f32_rect_min_out_volt;             // ���ģ�������ѹ����
	F32_T  f32_rect_offline_out_volt;         // ���ģ��Ĭ�������ѹ
	       
	U16_T  u16_pb_high_volt;                  // ��ĸ��ѹ����
	U16_T  u16_pb_low_volt;                   // ��ĸǷѹ����
	U16_T  u16_cb_high_volt;                  // ��ĸ��ѹ����
	U16_T  u16_cb_low_volt;                   // ��ĸǷѹ����
	U16_T  u16_bus_high_volt;                 // ĸ�߹�ѹ����
	U16_T  u16_bus_low_volt;                  // ĸ��Ƿѹ����
	U16_T  u16_cb_output_volt;                // ��ĸ�����ѹ
}DC_VOLT_VALUE_T;

/* 220Vϵͳ��ѹĬ��ֵ��110Vϵͳ����2 */
static const DC_VOLT_VALUE_T m_t_dc_volt_def =
{
	245,      // ��ؾ����ѹ
	235,      // ��ظ����ѹ
	200,      // ��طŵ���ֹ��ѹ
	264,      // ������ѹ��
	187,      // �����Ƿѹ��
	
	220,      // ���ģ�������ѹ
	286,      // ���ģ�������ѹ����
	176,      // ���ģ�������ѹ����
	220,      // ���ģ��Ĭ�������ѹ
	
	286,      // ��ĸ��ѹ����
	186,      // ��ĸǷѹ����
	235,      // ��ĸ��ѹ����
	205,      // ��ĸǷѹ����
	286,      // ĸ�߹�ѹ����
	186,      // ĸ��Ƿѹ����
	220,      // ��ĸ�����ѹ
};

/* 220Vϵͳ��ѹ��Сֵ��110Vϵͳ����2 */
static const DC_VOLT_VALUE_T m_t_dc_volt_min =
{
	200000,   // ��ؾ����ѹ������������1000
	200000,   // ��ظ����ѹ������������1000
	190,      // ��طŵ���ֹ��ѹ
	220000,   // ������ѹ�㣬����������1000
	186000,   // �����Ƿѹ�㣬����������1000
	
	176000,   // ���ģ�������ѹ������������1000
	220000,   // ���ģ�������ѹ���ޣ�����������1000
	176000,   // ���ģ�������ѹ���ޣ�����������1000
	176000,   // ���ģ��Ĭ�������ѹ������������1000
	
	220,      // ��ĸ��ѹ����
	186,      // ��ĸǷѹ����
	220,      // ��ĸ��ѹ����
	198,      // ��ĸǷѹ����
	220,      // ĸ�߹�ѹ����
	186,      // ĸ��Ƿѹ����
	210,      // ��ĸ�����ѹ
};

/* 220Vϵͳ��ѹ���ֵ��110Vϵͳ����2 */
static const DC_VOLT_VALUE_T m_t_dc_volt_max =
{
	300000,   // ��ؾ����ѹ������������1000
	300000,   // ��ظ����ѹ������������1000
	220,      // ��طŵ���ֹ��ѹ
	320000,   // ������ѹ�㣬����������1000
	220000,   // �����Ƿѹ�㣬����������1000
	
	286000,   // ���ģ�������ѹ������������1000
	286000,   // ���ģ�������ѹ���ޣ�����������1000
	220000,   // ���ģ�������ѹ���ޣ�����������1000
	286000,   // ���ģ��Ĭ�������ѹ������������1000
	
	320,      // ��ĸ��ѹ����
	220,      // ��ĸǷѹ����
	242,      // ��ĸ��ѹ����
	220,      // ��ĸǷѹ����
	320,      // ĸ�߹�ѹ����
	220,      // ĸ��Ƿѹ����
	230,      // ��ĸ�����ѹ
};	

/* ��¼��ǰ��ʾ�Ĵ���ID��������Ŀ��������Ϣ */
static MMI_WIN_RECORD_T m_t_win_record;


/* ��¼���ò˵���Ŀ������ */
static U8_T m_u8_set_menu_item_cnt;

/* ����ϵͳ��ѹ�ȼ� */
SYS_VOLT_LEVEL_E m_e_volt_level;

/* �����������ź�����ģ��� */
U8_T m_u8_feeder_panel_ordianl;               //��������
U16_T m_u16_feeder_module_ordianl;            //����ģ���

/* ������������ */
U16_T g_u16_key_cancel;
U16_T g_u16_key_enter;
U16_T g_u16_key_up;
U16_T g_u16_key_down;
U16_T g_u16_key_add;
U16_T g_u16_key_sub;

/* LCD���򱸷� */
LCD_DIRECTION_E m_e_lcd_direction;

/* ���Ա��� */
LANG_TYPE_E     m_e_lang;


/* ����ṹ������ */
static const MMI_WIN_T m_t_win_array[MMI_WIN_MAX_CNT] =
{
	/******************* һ��ĸ��������ṹ���ʼ�� ******************************/
	{
        //u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MAIN_WINDOW,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_MAIN_WINDOW2,  0,             2,           10,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index   u8_x   u8_y
			{ ICON_CLOCK,  3,     3 }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X+4, MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x   u8_val_y  u8_val_type          u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME,  16,        3,         0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 0,                        3,         19,        0,             115,       19,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature) },
			{ 1,                        3,         32,        0,             115,       32,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt) },
			{ 2,                        3,         45,        0,             115,       45,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]) },
			{ 3,                        3,         58,        0,             115,       58,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity) },
			{ 4,                        3,         71,        0,             115,       71,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr) },
			{ 8,                        3,         132,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 14,                       3,         145,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL }, 
			{ 5,                        3,         84,        0,             115,       84,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt) },
			{ 7,                        3,         97,        0,             115,       97,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt) },
		}
	},	

	/******************* ���˵�����ṹ���ʼ�� ****************************/
	{
		//u16_id               u16_id_father            u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_MAIN_WINDOW,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           12,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x   u8_val_y   u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 23,             3,         3,         0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 603,            3,         16,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 24,             3,         29,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 25,             3,         42,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 26,             3,         55,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 27,             3,         68,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 28,             3,         81,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 29,             3,         94,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 30,             3,         107,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 31,             3,         120,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 447,            3,         133,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 32,             3,         146,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ֱ��ϵͳ��Ϣ��ѯ�˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           10,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 33,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 34,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 35,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 36,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 37,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 38,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 39,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 613,           3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 45,            3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 46,            3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ���߹���Ϣ��ѯ�˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                          u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_RUN_INFO_MENU,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 40,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 41,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 42,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 43,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 44,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ͨ��ϵͳ��Ϣ��ѯ�˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                        u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 49,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 50,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 51,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ���ϵͳ��Ϣ��ѯ�˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                        u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  4,             0,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 52,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 53,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 54,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �������ò˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id              u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  9,             0,           12,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 55,             3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 591,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 56,             3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 57,             3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 58,             3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 59,             3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 60,             3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 61,             3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 63,             3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 64,             3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 65,             3,         133,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 517,            3,         146,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },	
		}
	},
	
	/******************* ֱ���������ò˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 66,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 67,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 68,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 69,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 70,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 71,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 611,           3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 80,            3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ���߹�������ò˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_SET_MENU,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 72,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 73,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 74,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 75,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 76,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ĩ���ô�����ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_CONFIG,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 81,             6,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 82,             6,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ���ཻ����Ϣ��ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_TRIPHASE_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 83,             3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 84,             3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv) },
			{ 87,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw) },
			{ 88,             9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu) },
			{ 85,             3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         94,        0,             110,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv) },
			{ 87,             9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw) },
			{ 88,             9,         120,       0,             110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu) },
		}
	},

	/******************* ���ཻ����Ϣ��ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_UNIPHASE_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 83,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 89,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 91,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv) },
			{ 90,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 91,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv) },
		}
	},

	/******************* �������������ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_TOTAL_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS2_CELL_INFO,  MMI_WIN_ID_BATT_GROUP_INFO,  2,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 103,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 93,             9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity) },
			{ 94,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt) },
			{ 95,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]) },
			{ 96,             9,         55,        100,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           &(g_t_share_data.t_rt_data.t_batt.e_state[0]) },
			{ 97,             9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16,     0,           0,           &(g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[0]) },
		}
	},
	
	/******************* �������ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_GROUP_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BATT_TOTAL_INFO,  MMI_WIN_ID_BMS_GROUP1_INFO,  2,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index   u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 104,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 93,             9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity) },
			{ 94,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt2_volt) },
			{ 95,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]) },
			{ 96,             9,         55,        100,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           &(g_t_share_data.t_rt_data.t_batt.e_state[1]) },
			{ 97,             9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16,     0,           0,           &(g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[1]) },
		}
	},

	/******************* һ����Ѳ������ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                  u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS_GROUP1_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BATT_GROUP_INFO,  MMI_WIN_ID_BMS1_CELL_INFO,  2,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 107,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 0,              9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_temperature1) },
			{ 109,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_cell_max_volt_id) },
			{ 112,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_max_cell_volt) },
			{ 110,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_cell_min_volt_id) },
			{ 112,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_min_cell_volt) },
		}
	},

	/******************* һ�鵥���ѹ����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father                 u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS1_CELL_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS_GROUP1_INFO,  MMI_WIN_ID_BMS_GROUP2_INFO,  2,             2,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 113,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             80,       16,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         29,        0,             80,       29,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        0,             80,       42,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         55,        0,             80,       55,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        0,             80,       68,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             80,       81,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        0,             80,       94,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         107,       0,             80,       107,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       0,             80,       120,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         133,       0,             80,       133,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL }
		}
	},
	
	/******************* ������Ѳ������ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS_GROUP2_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS1_CELL_INFO,  MMI_WIN_ID_BMS2_CELL_INFO,  2,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 108,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 0,              9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_temperature1) },
			{ 109,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_cell_max_volt_id) },
			{ 112,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_max_cell_volt) },
			{ 110,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_cell_min_volt_id) },
			{ 112,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_min_cell_volt) },
		}
	},

	/******************* ���鵥���ѹ����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father                 u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS2_CELL_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS_GROUP2_INFO,  MMI_WIN_ID_BATT_TOTAL_INFO,  2,             2,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 114,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             80,       16,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         29,        0,             80,       29,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        0,             80,       42,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         55,        0,             80,       55,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        0,             80,       68,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             80,       81,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        0,             80,       94,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         107,       0,             80,       107,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       0,             80,       120,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         133,       0,             80,       133,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL }
		}
	},

	/******************* �Ͽ�ĸ�߾�Ե״̬��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PB_CB_INSU_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 115,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 116,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 117,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 120,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         107,       0,             115,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
		}
	},
	
	/******************* һ�ξ�Ե״̬��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BUS_INSU_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 115,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 118,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 120,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         81,        0,             115,      81,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL }
		}
	},

	/******************* AC/DCģ����Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father                 u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECT_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_RECT_INFO,  MMI_WIN_ID_RECT_INFO,  4,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 123,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 125,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 126,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 127,                            9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 128,                            9,         55,        130,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 129,                            9,         68,        132,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* ����״̬��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father                 u16_id_prev              u16_id_next              u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SWITCH_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_SWITCH_INFO,  MMI_WIN_ID_SWITCH_INFO,  5,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         pv_val  u32_val_min  u32_val_max
			{ 135,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        141,           100,      29,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        141,           100,      55,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        141,           100,      81,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        141,           100,      107,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       141,           100,      131,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 }
		}
	},
	
	/******************* ���������֧·��Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father                     u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_FEEDER_INFO,  MMI_WIN_ID_FEEDER_RUN_INFO_MENU,  MMI_WIN_ID_DC_FEEDER_INFO,  MMI_WIN_ID_DC_FEEDER_INFO,  1,             2,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 143,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         42,        154,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         55,        154,           110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         68,        154,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         107,       151,           110,      107,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         120,       154,           110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         133,       154,           110,      133,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         146,       154,           110,      146,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* ͨ��ģ����Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father                   u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_INFO,  MMI_WIN_ID_DCDC_RUN_INFO_MENU,  MMI_WIN_ID_DCDC_INFO,  MMI_WIN_ID_DCDC_INFO,  1,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 124,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 125,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 126,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 127,                            9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 128,                            9,         55,        130,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 129,                            9,         68,        132,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* ͨ��������֧·��Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                   u16_id_prev                   u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_FEEDER_INFO,  MMI_WIN_ID_DCDC_RUN_INFO_MENU,  MMI_WIN_ID_DCDC_FEEDER_INFO,  MMI_WIN_ID_DCDC_FEEDER_INFO,  2,             2,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 144,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         42,        151,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         55,        154,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         94,        151,           110,      94,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         107,       151,           110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         120,       154,           110,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* ���ģ����Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father                   u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_INFO,  MMI_WIN_ID_DCAC_RUN_INFO_MENU,  MMI_WIN_ID_DCAC_INFO,  MMI_WIN_ID_DCAC_INFO,  1,             2,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 157,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 125,                            9,         16,        0,             100,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 126,                            9,         29,        0,             100,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 158,                            9,         42,        0,             100,      42,       MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 159,                            9,         55,        0,             100,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 160,                            9,         68,        0,             100,      68,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 161,                            9,         81,        0,             100,      81,       MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 162,                            9,         94,        0,             100,      94,       MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 163,                            9,         107,       0,             100,      107,      MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 164,                            9,         120,       165,           100,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 168,                            9,         133,       169,           100,      133,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* ���������֧·��Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                   u16_id_prev                   u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_FEEDER_INFO,  MMI_WIN_ID_DCAC_RUN_INFO_MENU,  MMI_WIN_ID_DCAC_FEEDER_INFO,  MMI_WIN_ID_DCAC_FEEDER_INFO,  2,             2,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index           u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 145,                     3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME, 9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                     9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                     9,         42,        151,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                     9,         55,        154,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME, 9,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                     9,         94,        151,           110,      94,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                     9,         107,       151,           110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                     9,         120,       154,           110,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},

	/******************* ��ǰ���Ͻ���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father          u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_CURR_FAULT_INFO,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_CURR_FAULT_INFO,  MMI_WIN_ID_CURR_FAULT_INFO,  5,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 171,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �޵�ǰ������ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_CURR_FAULT,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  5,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 177,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 178,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ��ʷ���Ͻ���ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father          u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_HIS_FAULT_INFO,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_HIS_FAULT_INFO,  MMI_WIN_ID_HIS_FAULT_INFO,  6,             2,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 172,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 176,  3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 176,  3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ����ʷ������ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_HIS_FAULT,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           1,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x   u8_y
			{ 0,          0,     0 }, 
			{ 0,          0,     0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type        u32_val_min  u32_val_max  pv_val  
			{ 179,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE, 0,           0,           NULL },
		}
	},

	/******************* ����ĩ������Ͻ���ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father          u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_EXCEPTION_INFO,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_EXCEPTION_INFO,  MMI_WIN_ID_EXCEPTION_INFO,  7,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 173,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �޵���ĩ���������ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_EXCEPTION,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 180,            10,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 181,            10,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �¼���¼����ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father     u16_id_prev         u16_id_next         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECORD,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_RECORD,  MMI_WIN_ID_RECORD,  8,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                 u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 174, 3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,       3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175, 3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,       3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175, 3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,       3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175, 3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ���¼���¼����ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_RECORD,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  8,             0,           1,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type          u32_val_min  u32_val_max  pv_val  
			{ 182,            6,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �����������ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INPUT_PASSWORD, MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  9,             0,           1,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x,  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 184,            3,         16,        0,             60,       29,       MMI_VAL_TYPE_U32_5BIT,  0,           99999,       &(m_t_win_record.u32_set_value) },
		}
	},

	/******************* ���������ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father               u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PASSWORD_ERROR, MMI_WIN_ID_INPUT_PASSWORD,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           1,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x, u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 185,            6,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* �������ó�����ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PARAM_OUT_RANGE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }  
		},

		//��ʼ��item�ṹ����					
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 186,                            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 187,                            9,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 188,                            9,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 189,  9,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 190,  9,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ֱ��ϵͳ���ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father            u16_id_prev                  u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_SYSTEM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_METER2_SET,  MMI_WIN_ID_AC_PARAM_SET,  1,             2,           12,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����					
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 191,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 615,            9,         16,        616,           110,      16,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num) },
			{ 192,            9,         29,        197,           110,      29,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level) },
			{ 193,            9,         42,        199,           110,      42,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb) },
			{ 195,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  210,         230,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt) },
			{ 194,            9,         68,        201,           110,      68,       MMI_VAL_TYPE_ENUM,      0,           4,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl) },
			{ 196,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8,        0,           4,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num) },  
			{  48,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8,        0,           4,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num) },
			{ 610,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U8_2BIT,   0,           11,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num) },
			{ 618,            3,         120,       0,             0,        120,      MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 619,            9,         133,       0,             110,      133,      MMI_VAL_TYPE_U8_3BIT,   14,          111,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add) },
		    { 620,            9,         146,       0,             110,      146,      MMI_VAL_TYPE_U8_3BIT,   14,          111,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add) },
		}
	},

	/******************* �����������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC_SYSTEM_SET,  MMI_WIN_ID_BATT_PARAM_SET,  1,             2,           4,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 206,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 207,            9,         16,        210,           100,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase) },
			{ 208,            9,         29,        212,           100,      29,       MMI_VAL_TYPE_ENUM,  0,           2,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path) },
			{ 209,            9,         42,        215,           100,      42,       MMI_VAL_TYPE_ENUM,  0,           3,           &(g_t_share_data.t_sys_cfg.t_ctl.u16_ac) },
		}
	},

	/******************* ������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father            u16_id_prev               u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_AC_PARAM_SET,  MMI_WIN_ID_DC_PARAM_SET,  1,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 219,           3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 220,           9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,       0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num) },
			{ 221,           9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_4BIT, 10,          3000,        &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10) },
		}
	},

	/******************* ֱ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_PARAM_SET,  MMI_WIN_ID_DC10_PARAM_SET,  1,             2,           9-2,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x             u8_y
			{ ICON_UP,    MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 222,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 223,            9,         16,        224,           110,      16,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt) },
			{ 226,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range) },
			{ 227,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt1_shunt_range) },
			{ 228,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt2_shunt_range) },
			{ 229,            3,         68,        0,             110,      68,       MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 230,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_4BIT,  10,          1000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_feeder_shunt_range) },
			{ 564,            3,         94,        0,             110,      94,       MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 565,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U8     ,   0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num) },
		}
	},

	/******************* ����ģ��������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father            u16_id_prev               u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECT_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC10_PARAM_SET,  MMI_WIN_ID_BATT_METER1_SET,  1,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 231,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 232,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8_2BIT,   1,          24,          &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num) },
			{ 233,            9,         29,        530,           110,      29,       MMI_VAL_TYPE_ENUM,      0,          7,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr) },
			{ 234,            9,         42,        375,           110,      42,       MMI_VAL_TYPE_ENUM,      0,          0,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_protocol) },
			{ 235,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_5W1P,  176000,     286000,      &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt) },   //������*1000������
			{ 236,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_5W1P,  5000,       110000,      &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_curr_percent) },   //������*1000������
		}
	},

	/******************* һ����Ѳ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_METER1_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_RECT_PARAM_SET,  MMI_WIN_ID_BATT_METER2_SET,  1,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 244,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 246,            9,         16,        253,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           2,           &(g_t_share_data.t_sys_cfg.t_batt.e_bms_type) },
			{ 247,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8,       0,           5,           &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num) },
			{ 248,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[0]) },
			{ 249,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[1]) },
			{ 250,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[2]) },
			{ 251,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[3]) },
			{ 252,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[4]) },
		}
	},
	
	/******************* ������Ѳ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                  u16_id_next                u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_METER2_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_METER1_SET,  MMI_WIN_ID_DC_SYSTEM_SET,  1,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 245,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 247,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,       0,           5,           &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num) },
			{ 248,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[0]) },
			{ 249,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[1]) },
			{ 250,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[2]) },
			{ 251,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[3]) },
			{ 252,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[4]) },
		}
	},
	
	/******************* ���������ģ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                                 u16_id_father                u16_id_prev                          u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET,  MMI_WIN_ID_FEEDER_SET_MENU,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  1,             2,           2,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 262,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           4,           NULL },
		}
	},
	
	/******************* ���������ģ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                             u16_id_father                u16_id_prev                              u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  MMI_WIN_ID_FEEDER_SET_MENU,  MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  1,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 264,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 268,                            9,         42,        260,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 269,                            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 270,                            9,         68,        260,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 271,                            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 272,                            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
		}
	},

	/******************* �����������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                u16_id_father            u16_id_prev               u16_id_next             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_INSU_THR_SET,  MMI_WIN_ID_DC_THR_SET,  3,             2,           4,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 273,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 274,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U16_3BIT,  220,        530,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_high_volt) },
			{ 275,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  186,        380,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_low_volt) },
			{ 276,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  0,          380,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_lack_phase) },
		}
	},

	/******************* ֱ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                u16_id_father            u16_id_prev             u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_AC_THR_SET,  MMI_WIN_ID_BATT_THR_SET,  3,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 277,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 278,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U16_3BIT,  220,         320,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt) },
			{ 279,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  186,         220,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt) },
			{ 280,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  220,         242,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt) },
			{ 281,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  198,         220,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt) },
			{ 282,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16_3BIT,  220,         320,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt) },
			{ 283,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_3BIT,  186,         220,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt) },
		}
	},

	/******************* �������ֵ���ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev             u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC_THR_SET,  MMI_WIN_ID_INSU_THR_SET,  3,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 284,           3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 285,           9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_5W1P,  220000,      320000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit) },   //������*1000������
			{ 286,           9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_5W1P,  176000,      220000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit) },   //������*1000������
			{ 287,           9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_5W2P,  2000,        15000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_cell_high_volt) },   //������*1000������
			{ 288,           9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_5W2P,  1800,        12000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_cell_low_volt) },   //������*1000������
			{ 289,           9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_5W2P,  2000,        15000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_tail_high_volt) },   //������*1000������
			{ 290,           9,         81,        0,             110,      81,       MMI_VAL_TYPE_F32_5W2P,  1800,        12000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_tail_low_volt) },   //������*1000������
			{ 291,           9,         94,        0,             110,      94,       MMI_VAL_TYPE_F32_4W2P,  100,         1000,        &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit) },   //������*1000������
		}
	},

	/******************* ��Ե���ޱ������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev               u16_id_next             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INSU_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_THR_SET,  MMI_WIN_ID_AC_THR_SET,  3,             2,           4,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 292,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 293,            9,         16,        0,             125,      16,       MMI_VAL_TYPE_U16_3BIT,  20,         100,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance) },
			{ 294,            9,         29,        0,             125,      29,       MMI_VAL_TYPE_U16_2BIT,  5,          99,          &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr) },
			{ 515,            9,         42,        0,             125,      42,       MMI_VAL_TYPE_U16_2BIT,  1,          50,          &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_dc_bus_input_ac_thr) },
		}
	},

	/******************* ��س��������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                     u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_CHARGE_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_DISCHARGE_SET,  MMI_WIN_ID_BATT_TO_FLO_SET,  4,             2,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 295,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 296,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_5W1P, 200000,      300000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt) },   //������*1000������
			{ 297,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_5W1P, 200000,      300000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt) },   //������*1000������
			{ 298,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_5W1P, 0,           500000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt) },   //������*1000������
			{ 299,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_F32_4W2P, 50,          400,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr) },   //������*1000������
		}
	},

	/******************* ���ת�����оݽ���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_TO_FLO_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_CHARGE_SET,  MMI_WIN_ID_BATT_TO_EQU_SET,  4,             2,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 300,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 301,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_4W2P,  10,          30,          &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_flo_curr) },   //������*1000������
			{ 302,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_U16_3BIT,  1,           360,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_curr_go_time) },
			{ 303,            60,        42,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 304,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_equ_time) },
		}
	},

	/******************* ���ת�����оݽ���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_TO_EQU_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_TO_FLO_SET,  MMI_WIN_ID_BATT_DISCHARGE_END_SET,  4,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 305,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 306,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_4W2P,  40,          100,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr) },   //������*1000������
			{ 307,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_to_equ_dur_time) },
			{ 303,            60,        42,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 308,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_ac_fail_time) },
			{ 303,            60,        68,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 309,            9,         81,        0,             115,      81,       MMI_VAL_TYPE_U16_4BIT,  1,           9999,        &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_equ_cycle) },
		}
	},
	
	/******************* ��غ�����ֹ���ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                            u16_id_father            u16_id_prev                  u16_id_next                     u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_DISCHARGE_END_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_TO_EQU_SET,  MMI_WIN_ID_BATT_DISCHARGE_SET,  4,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 310,            3,        3,          0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 311,            9,        16,         0,             115,      16,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_dis_time) },
			{ 303,            60,       29,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 312,            9,        42,         0,             115,      42,       MMI_VAL_TYPE_U16_3BIT,  190,         220,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt) },
			{ 303,            60,       55,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 313,            9,        68,         0,             115,      68,       MMI_VAL_TYPE_F32_4W1P,  1800,        12000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_cell_end_volt) },   //������*1000������
		}
	},

	/******************* ��طŵ��������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                        u16_id_father            u16_id_prev                         u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_DISCHARGE_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_DISCHARGE_END_SET,  MMI_WIN_ID_BATT_CHARGE_SET,  4,             2,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 314,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 315,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_01c_dis_rate) },   //������*1000������
			{ 316,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_02c_dis_rate) },   //������*1000������
			{ 317,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_03c_dis_rate) },   //������*1000������
			{ 318,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_04c_dis_rate) },   //������*1000������
			{ 319,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_05c_dis_rate) },   //������*1000������
			{ 320,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_06c_dis_rate) },   //������*1000������
			{ 321,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_07c_dis_rate) },   //������*1000������
			{ 322,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_08c_dis_rate) },   //������*1000������
			{ 323,            9,         120,       0,             110,      120,      MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_09c_dis_rate) },   //������*1000������
			{ 324,            9,         131,       0,             110,      131,      MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_10c_dis_rate) },   //������*1000������
		}
	},
	
	/******************* ϵͳ���Ʒ�ʽ����ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father            u16_id_prev              u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SYSTEM_CTL_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_RECT_ON_OFF,  MMI_WIN_ID_BATT_CTL_SET,  5,             2,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 325,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 128,            3,         29,        130,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0]) },
		}
	},

	/******************* ��س�緽ʽ����ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                 u16_id_next              u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_CTL_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_SYSTEM_CTL_SET,  MMI_WIN_ID_RECT_ON_OFF,  5,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 326,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 327,            3,         29,        100,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           2,           &(g_t_share_data.t_sys_cfg.t_ctl.u16_batt[0]) },
			{ 590,            3,         55,        100,           110,      55,       MMI_VAL_TYPE_ENUM,  0,           2,           &(g_t_share_data.t_sys_cfg.t_ctl.u16_batt[1]) },
		}
	},

	/******************* ����ģ�鿪�ػ�����ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father            u16_id_prev               u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECT_ON_OFF,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_CTL_SET,  MMI_WIN_ID_SYSTEM_CTL_SET,  5,             2,           11,
		                                                          
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 328,                            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         16,        132,           110,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         29,        132,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         42,        132,           110,      42,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         55,        132,           110,      55,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         68,        132,           110,      68,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         81,        132,           110,      81,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         94,        132,           110,      94,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         107,       132,           110,      107,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         120,       132,           110,      120,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         131,       132,           110,      131,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
		}
	},

	/******************* ͨ��ģ��������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                         u16_id_next                             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCDC_FEEDER_MODULE_SET,  MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET,  3,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 330,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 232,            9,         16,        0,             100,      16,       MMI_VAL_TYPE_U8,        0,          8,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num) },
			{ 233,            9,         29,        540,           100,      29,       MMI_VAL_TYPE_ENUM,      0,          8,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr) },
			{ 234,            9,         42,        538,           100,      42,       MMI_VAL_TYPE_ENUM,      0,          1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol) },
			{ 125,            9,         55,        0,             100,      55,       MMI_VAL_TYPE_F32_4W1P,  20000,      60000,       &(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt) },   //������*1000������
			{ 332,            9,         68,        0,             100,      68,       MMI_VAL_TYPE_F32_5W1P,  5000,       110000,      &(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent) },   //������*1000������
		}
	},
	
	/******************* ͨ��������ģ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                                u16_id_father         u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCDC_MODULE_SET,  MMI_WIN_ID_DCDC_FEEDER_MODULE_SET,  3,             2,           2,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 333,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num) },
		}
	},
	
	/******************* ͨ��������ģ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                            u16_id_father         u16_id_prev                             u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_FEEDER_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_DCDC_MODULE_SET,  3,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 335,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             100,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_alarm_feeder_num) },
			{ 268,                            9,         42,        260,           100,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].e_alarm_type) },
			{ 269,                            9,         55,        0,             100,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_state_feeder_num) },
			{ 270,                            9,         68,        260,           100,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].e_state_type) },
			{ 272,                            9,         81,        0,             100,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_curr_feeder_num) },
			{ 339,                            9,         94,        0,             100,      94,       MMI_VAL_TYPE_U16_4BIT, 10,          1000,        &(g_t_share_data.t_sys_cfg.t_dcdc_panel.u16_feeder_shunt_range) },
		}
	},
	
	/******************* ���ģ��������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                         u16_id_next                             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCAC_FEEDER_MODULE_SET,  MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET,  4,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min u32_val_max  pv_val  
			{ 331,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,          0,           NULL },
			{ 232,            9,         16,        0,             100,      16,       MMI_VAL_TYPE_U8,    0,          8,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num) },
			{ 234,            9,         29,        375,           100,      29,       MMI_VAL_TYPE_ENUM,  0,          0,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.e_protocol) },
		}
	},
	
	/******************* ���������ģ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                                u16_id_father         u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCAC_MODULE_SET,  MMI_WIN_ID_DCAC_FEEDER_MODULE_SET,  4,             2,           2,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 334,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           1,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num) },
		}
	},
	
	/******************* ���������ģ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                            u16_id_father         u16_id_prev                             u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_FEEDER_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_DCAC_MODULE_SET,  4,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 337,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             100,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_alarm_feeder_num) },
			{ 268,                            9,         42,        260,           100,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].e_alarm_type) },
			{ 269,                            9,         55,        0,             100,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_state_feeder_num) },
			{ 270,                            9,         68,        260,           100,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].e_state_type) },
			{ 272,                            9,         81,        0,             100,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_curr_feeder_num) },
			{ 339,                            9,         94,        0,             100,      94,       MMI_VAL_TYPE_U16_4BIT, 10,          1000,        &(g_t_share_data.t_sys_cfg.t_dcac_panel.u16_feeder_shunt_range) },
		}
	},

	/******************* �ɽӵ�����趨����ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father         u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RELAY_OUT_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_ALARM_SET,  MMI_WIN_ID_ALARM_SET,  5,             2,           12,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 340,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 341,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 342,            9,         29,        0,             130,      29,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output) },
			{ 343,            9,         42,        0,             130,      42,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output) },
			{ 344,            9,         55,        0,             130,      55,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output) },
			{ 345,            9,         68,        0,             130,      68,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output) },
			{ 346,            9,         81,        0,             130,      81,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output) },
			{ 347,            9,         94,        0,             130,      94,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output) },
			{ 348,            9,         107,       0,             130,      107,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output) },
			{ 349,            9,         120,       0,             130,      120,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output) },
			{ 350,            9,         133,       0,             130,      133,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output) },
			{ 351,            9,         146,       0,             130,      146,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output) },
		}
	},

	/******************* �����趨����ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father         u16_id_prev                u16_id_next                u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ALARM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_RELAY_OUT_SET,  MMI_WIN_ID_RELAY_OUT_SET,  5,             2,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 352,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 353,            9,         16,        357,           115,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_buzzer_state) },
			{ 354,            9,         29,        359,           115,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_curr_imbalance_alm) },
			{ 355,            9,         42,        361,           115,      42,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_minor_fault_save) },
			{ 356,            9,         55,        361,           115,      55,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_general_fault_save) },
		}
	},

	/******************* Զ��ͨѶ���ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BACKSTAGE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0   }, 
			{ 0,         0,    0 }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 363,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 266,            9,         16,        0,             100,      16,       MMI_VAL_TYPE_U8_3BIT,  1,           255,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr) },
			{ 364,            9,         29,        367,           100,      29,       MMI_VAL_TYPE_ENUM,     0,           4,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud) },
			{ 365,            9,         42,        372,           100,      42,       MMI_VAL_TYPE_ENUM,     0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity) },
			{ 234,            9,         55,        375,           100,      55,       MMI_VAL_TYPE_ENUM,     0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol) },
			{ 366,            9,         81,        379,           100,      81,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time) },
		}
	},

	/******************* RTCʱ��������趨����ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RTC_PASSWORD_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index    u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 381,              3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 382,              9,         16,        0,             100,      16,       MMI_VAL_TYPE_U16_2BIT,  0,           99,          &(m_t_win_record.t_time.year) },
			{ 383,              9,         29,        0,             100,      29,       MMI_VAL_TYPE_U8_2BIT,   1,           12,          &(m_t_win_record.t_time.month) },
			{ 384,              9,         42,        0,             100,      42,       MMI_VAL_TYPE_U8_2BIT,   1,           31,          &(m_t_win_record.t_time.day) },
			{ 385,              9,         55,        0,             100,      55,       MMI_VAL_TYPE_U8_2BIT,   0,           23,          &(m_t_win_record.t_time.hour) },
			{ 386,              9,         68,        0,             100,      68,       MMI_VAL_TYPE_U8_2BIT,   0,           59,          &(m_t_win_record.t_time.min) },
			{ 387,              9,         81,        0,             100,      81,       MMI_VAL_TYPE_U8_2BIT,   0,           59,          &(m_t_win_record.t_time.sec) },
			{ 388,              9,         107,       0,             100,      107,      MMI_VAL_TYPE_U32_5BIT,  0,           99999,       &(g_t_share_data.t_sys_cfg.t_sys_param.u32_password) },
		}
	},
	
	/******************* ��ʷ�澯��¼�������ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_HIS_FAULT_CLEAR,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_EXCEPTION_CLEAR,  8,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }   
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 389,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 390,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 391,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ����ĩ����澯��¼�������ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                  u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_EXCEPTION_CLEAR,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_HIS_FAULT_CLEAR,  MMI_WIN_ID_RECORD_CLEAR,  8,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }   
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 392,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 393,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 394,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �¼���¼�������ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father         u16_id_prev                  u16_id_next                u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECORD_CLEAR,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_EXCEPTION_CLEAR,  MMI_WIN_ID_BATT_RESTORE,  8,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }   
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 395,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 396,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 397,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ��������ָ�����ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father         u16_id_prev               u16_id_next                    u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_RESTORE,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_RECORD_CLEAR,  MMI_WIN_ID_PARAM_RESTORE,  8,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 398,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 399,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 400,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ���ò����ָ�����ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father         u16_id_prev               u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_BATT_RESTORE,  MMI_WIN_ID_HIS_FAULT_CLEAR,  8,             2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 401,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 402,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 403,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ȷ�ϲ�����ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                       u16_id_father              u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_CONFIRM_OPERATION,  MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0  }, 
			{ 0,          0,    0  }   
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 404,            10,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 405,            10,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ��ʾ�����ѳɹ�����ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father              u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_OPERATION_SCRESS,  MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0  }, 
			{ 0,          0,    0  }   
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 406,            10,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL  },
			{ 407,            10,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL  },
		}
	},

	/******************* ����У׼����ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father         u16_id_prev                    u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_BATT2_CURR_ADJUST,  MMI_WIN_ID_DC_VOLT_ADJUST,  9,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index    u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 408,              3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 409,              9,         16,        0,             105,      16,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv) },   //������*1000������
			{ 410,              9,         29,        0,             105,      29,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw) },   //������*1000������
			{ 411,              9,         42,        0,             105,      42,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu) },   //������*1000������
			{ 412,              9,         55,        0,             105,      55,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv) },   //������*1000������
			{ 413,              9,         68,        0,             105,      68,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw) },   //������*1000������
			{ 414,              9,         81,        0,             105,      81,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu) },   //������*1000������
		}
	},

	/******************* ֱ����ѹУ׼����ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father         u16_id_prev            u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_VOLT_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_AC_ADJUST,  MMI_WIN_ID_LOAD_CURR_ADJUST,  9,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 415,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 1,              9,         29,        0,             120,      29,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_batt_volt) },   //������*1000������
			{ 5,              9,         42,        0,             120,      42,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_pb_volt) },   //������*1000������
			{ 7,              9,         55,        0,             120,      55,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_cb_volt) },   //������*1000������
			{ 516,            9,         68,        0,             120,      68,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt) },   //������*1000������
		}
	},

	/******************* ���ص���У׼����ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father         u16_id_prev                 u16_id_next                    u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_LOAD_CURR_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DC_VOLT_ADJUST,  MMI_WIN_ID_BATT1_CURR_ADJUST,  9,             2,           4,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 417,            3,         3,        0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,       0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 418,            9,         29,       0,              110,      29,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_load_curr_1) },   //������*1000������
			{ 419,            9,         42,       0,              110,      42,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_load_curr_2) },   //������*1000������
		}
	},

	/******************* һ���ص���У׼����ṹ���ʼ�� ******************************/
	{
		//u16_id                       u16_id_father         u16_id_prev                   u16_id_next                    u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT1_CURR_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_LOAD_CURR_ADJUST,  MMI_WIN_ID_BATT2_CURR_ADJUST,  9,             2,           4,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 420,            3,         3,        0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,       0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 422,            9,         29,       0,              110,      29,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1) },   //������*1000������
			{ 423,            9,         42,       0,              110,      42,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2) },   //������*1000������
		}
	},

	/******************* �����ص���У׼����ṹ���ʼ�� ******************************/
	{
		//u16_id                       u16_id_father         u16_id_prev                    u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT2_CURR_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_BATT1_CURR_ADJUST,  MMI_WIN_ID_AC_ADJUST,  9,             2,           4,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 421,            3,         3,        0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,       0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 422,            9,         29,       0,              110,      29,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1) },   //������*1000������
			{ 423,            9,         42,       0,              110,      42,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2) },   //������*1000������
		}
	},

	/******************* У׼���ڽ�����ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_DOING,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           1,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 424,            30,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* У׼�ɹ���ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_SCUESS,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 425,            10,         42,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 407,            10,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* У׼ʧ����ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_FAIL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 426,            10,         42,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 407,            10,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ��ʾ������һ������ֵҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_NEXT_CURR,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 427,            10,         42,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 428,            10,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 429,            10,         68,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ��ʾ����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id             u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DISPLAY,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  10,             0,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 430,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 431,            9,         16,        433,           110,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_lang) },
			{ 432,            9,         29,        435,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection) },
		}
	},

	/******************* ����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id           u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ABOUT,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  11,             0,          12,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME,        5,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 437,  5,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 438,  5,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 439,  5,         133,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 529,  5,         146,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ĩ���ô�����ʾ����ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_FUNCTION,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           1,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 446,            6,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ά��ָ��ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id               u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  10,             0,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 448,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 449,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 450,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 451,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 452,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 453,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 454,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 455,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ��ͨҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id           u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_START,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 456,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 457,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 458,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 459,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 460,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 461,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 462,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ͨѶ�쳣����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_COMM_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           10,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 463,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 464,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 465,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 466,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 467,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 468,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 469,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 470,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 471,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 472,        3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ��Ƿѹ����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_VOLT_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 473,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 474,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 475,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 476,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 477,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 478,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 479,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 480,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 481,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ��Ե����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INSU_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  4,             0,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 482,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 483,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 484,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 485,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 486,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 487,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 488,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 489,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 490,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 491,        3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 492,        3,         133,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ��̨ͨѶ�쳣ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BACKSTAGE_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  5,             0,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 493,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 494,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 495,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 496,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 497,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 498,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 499,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* ��ϵ����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_CONTACT_VENDER,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           9,

		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 500,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 501,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 502,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 503,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 504,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 505,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 506,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 507,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 508,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* �����豸����ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id            u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_REPAIR,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 509,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 510,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 511,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 512,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 513,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* �ֶ������趨����ṹ���ʼ�� ******************************/
	{
		//u16_id                           u16_id_father         u16_id_prev                   u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MANUAL_LIMIT_CURR_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_ZERO_CURR_SET,  MMI_WIN_ID_COMM_OFFLINE_SET,  11,            2,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 518,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 519,            9,         16,        130,           115,      16,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_limit_curr) },
			{ 520,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_5W1P,  5000,        110000,      &(g_t_share_data.t_sys_cfg.t_sys_param.f32_manual_limit_curr) },
		}
	},
	
	/******************* ͨѶ�����趨����ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father         u16_id_prev                        u16_id_next                        u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_COMM_OFFLINE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_MANUAL_LIMIT_CURR_SET,  MMI_WIN_ID_INSU_MEAS_SET,             11,            2,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index    u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 521,              3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 522,              9,         16,        0,             115,      16,       MMI_VAL_TYPE_U8_2BIT,   5,           60,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_fc10_offline_time) },
			{ 523,              9,         29,        0,             115,      29,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_bms_offline_cnt) },
			{ 524,              9,         42,        0,             115,      42,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_offline_cnt) },
			{ 525,              9,         55,        0,             115,      55,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt) },
			{ 526,              9,         68,        0,             115,      68,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_offline_cnt) },
			{ 608,              9,         81,        0,             115,      81,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt) },
			{ 609,              9,         94,        0,             115,      94,       MMI_VAL_TYPE_U8_2BIT,   1,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_offline_cnt) },
			{ 622,              9,         107,       0,             115,      107,      MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt) },
//			{ 528,              9,         120,       0,             115,      120,      MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_acm_offline_cnt) },
		}
	},

	/******************* ֱ��ĸ�߾�Ե�������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INSU_MEAS_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_COMM_OFFLINE_SET,  MMI_WIN_ID_ZERO_CURR_SET,  11,             2,           9,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 549,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 550,            9,         16,        558,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           3,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_can_baud) },
			{ 551,            9,         29,        562,           110,      29,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way) },
			{ 552,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U8_3BIT,  1,           120,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay) },
			{ 553,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range) },
			{ 554,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U8_3BIT,  1,           180,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm) },
			{ 555,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_3BIT,  1,           180,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period) },
			{ 556,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_2BIT,  0,           23,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_hour) },
			{ 557,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U8_2BIT,  0,           59,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_min) },
		}
	},

	/******************* ֱ����������������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ZERO_CURR_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_INSU_MEAS_SET,  MMI_WIN_ID_MANUAL_LIMIT_CURR_SET,  11,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 566,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
//			{ 567,            9,         16,        573,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign) },
//			{ 568,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,   0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero) },
//			{ 569,            9,         42,        573,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign) },
//			{ 570,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_1P,   0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero) },
//			{ 571,            9,         68,        573,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign) },
//			{ 572,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_F32_1P,   0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero) },
			{ 567,            9,         16,        573,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign) },
			{ 568,            9,         16,        0,             118,      16,       MMI_VAL_TYPE_F32_3W1P, 0,           2000,        &(g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero) },
			{ 569,            9,         29,        573,           110,      29,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign) },
			{ 570,            9,         29,        0,             118,      29,       MMI_VAL_TYPE_F32_3W1P, 0,           2000,        &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero) },
			{ 571,            9,         42,        573,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign) },
			{ 572,            9,         42,        0,             118,      42,       MMI_VAL_TYPE_F32_3W1P, 0,           2000,        &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero) },
		}
	},

	/******************* ����ĸ��������ṹ���ʼ�� ******************************/
	{
        //u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MAIN_WINDOW2,  MMI_WIN_ID_NULL,  MMI_WIN_ID_MAIN_WINDOW,  MMI_WIN_ID_NULL,  0,             2,           10,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index   u8_x   u8_y
			{ ICON_CLOCK,  3,     3 }, 
			{ ICON_UP,   MMI_ICON_UP_X+4,   MMI_ICON_UP_Y   }
		},
			
		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x   u8_val_y  u8_val_type          u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME,  16,        3,         0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 0,                        3,         19,        0,             115,       19,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature) },
			{ 575,                      3,         32,        0,             115,       32,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt2_volt) },
			{ 576,                      3,         45,        0,             115,       45,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]) },
			{ 577,                      3,         58,        0,             115,       58,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity) },
			{ 578,                      3,         71,        0,             115,       71,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr) },
			{ 8,                        3,         132,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 14,                       3,         145,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL }, 
			{ 579,                      3,         84,        0,             115,       84,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_volt) },
			{ 581,                      3,         97,        0,             115,       97,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_volt) },
		}
	},

	/******************* ֱ��DC10�������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC10_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC_PARAM_SET,  MMI_WIN_ID_RECT_PARAM_SET,  1,             2,           9,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x             u8_y
			{ ICON_UP,    MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 582,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 583,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8     ,   0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u8_dc10_module_num) },
			{ 584,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt) },
			{ 585,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt) },
			{ 586,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt) },
			{ 587,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range) },
			{ 588,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range) },
			{ 589,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range) },
			{ 195,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U16_3BIT,  (210/2),     230,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt) },			
		}
	},

	/******************* ���ģ��������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev           u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_MODULE_SET,  MMI_WIN_ID_SET_MENU,    MMI_WIN_ID_ACS_FEEDER_MODULE_SET,      MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET,         1,             0,           9,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min u32_val_max  pv_val  
			{ 592,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 593,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8     ,   0,           1,           &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num) },
			{ 594,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  220,         530,         &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_high_volt) },
			{ 595,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  186,         380,         &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_low_volt) },
			{ 596,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  0,           380,         &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_lack_phase) },
			{ 597,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range) },
			{ 598,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range) },
			{ 599,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range) },
			{ 339,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U16_4BIT,  10,          1000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u16_feeder_shunt_range) },
		}
	},

	/******************* ����ϵͳ����ģ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                                u16_id_father         u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_AC_MODULE_SET,  MMI_WIN_ID_ACS_FEEDER_MODULE_SET,      1,             2,           2,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 600,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           3,           &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num) },
		}
	},

	/******************* ���������ģ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                             u16_id_father                u16_id_prev                              u16_id_next     u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_FEEDER_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_AC_MODULE_SET,  1,             2,           7,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 601,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 268,                            9,         42,        260,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 269,                            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 270,                            9,         68,        260,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 272,                            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
		}
	},

	/******************* ����ϵͳ������Ϣ��ʾ�˵�ҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_AC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,   1,             0,           3,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 604,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 605,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 606,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* ���������ཻ����Ϣ��ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_AC_RUN_INFO,  MMI_WIN_ID_ACS_AC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 83,             3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 84,             3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac1_uv_volt) },
			{ 87,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac1_vw_volt) },
			{ 88,             9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac1_wu_volt) },
			{ 621,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_s1_curr) },
			{ 85,             3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac2_uv_volt) },
			{ 87,             9,         120,       0,             110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac2_vw_volt) },
			{ 88,             9,         133,       0,             110,      133,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac2_wu_volt) },
			{ 621,            9,         146,       0,             110,      146,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_s3_curr) },
		}
	},

	/******************* ��������֧·��Ϣ��ʾҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_FEEDER_INFO,  MMI_WIN_ID_ACS_AC_RUN_INFO_MENU,  MMI_WIN_ID_ACS_FEEDER_INFO,  MMI_WIN_ID_ACS_FEEDER_INFO,  2,             0,           9,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 607,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         42,        151,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         55,        154,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         94,        151,           110,      94,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         107,       151,           110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         120,       154,           110,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},

	/******************* ��ٿ��ؿ��ƽ���ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father            u16_id_prev               u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SWT_ON_OFF,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_SWT_ON_OFF,  MMI_WIN_ID_SWT_ON_OFF,         6,             2,           11,
		                                                          
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 612,                            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         16,        141,           110,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         29,        141,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         42,        141,           110,      42,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         55,        141,           110,      55,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         68,        141,           110,      68,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         81,        141,           110,      81,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         94,        141,           110,      94,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         107,       141,           110,      107,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         120,       141,           110,      120,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         131,       141,           110,      131,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
		}
	},

	/******************* ����״̬��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                 u16_id_father                 u16_id_prev              u16_id_next              u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ECSWT_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_ECSWT_INFO,  MMI_WIN_ID_ECSWT_INFO,     7,             2,           6,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         pv_val  u32_val_min  u32_val_max
			{ 614,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        141,           100,      29,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        141,           100,      55,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        141,           100,      81,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        141,           100,      107,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       141,           100,      131,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 }
		}
	},

	/******************* �������߹�������ò˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_SET_MENU2,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 77,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 73,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 74,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 75,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 76,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �������������ģ���������ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                                 u16_id_father                u16_id_prev                          u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2,  MMI_WIN_ID_FEEDER_SET_MENU2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  1,             2,           2,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 262,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           4,           NULL },
		}
	},
	
	/******************* �������������ģ�����ý���ṹ���ʼ�� ******************************/
	{
		//u16_id                             u16_id_father                u16_id_prev                              u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  MMI_WIN_ID_FEEDER_SET_MENU2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  1,             2,           8,
		
		//��ʼ��icon�ṹ����
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 264,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 268,                            9,         42,        260,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 269,                            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 270,                            9,         68,        260,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 271,                            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 272,                            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
		}
	},

	/******************* ���κϿ�ĸ�߾�Ե״̬��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PB_CB_INSU_INFO2,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  8,             0,           8,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{  78,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 116,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 117,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{  79,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         107,       0,             115,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
		}
	},

	/******************* ���ξ�Ե״̬��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                   u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BUS_INSU_INFO2,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  8,             0,           7,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{  78,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 118,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{  79,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         81,        0,             115,      81,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL }
		}
	},

	/******************* �������߹���Ϣ��ѯ�˵�����ṹ���ʼ�� ******************************/
	{
		//u16_id                          u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_RUN_INFO_MENU2,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  9,             0,           5,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 47,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 41,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 42,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 43,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 44,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* �������������֧·��Ϣ��ѯҳ��ṹ���ʼ�� ******************************/
	{
		//u16_id                    u16_id_father                     u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_FEEDER_INFO2,  MMI_WIN_ID_FEEDER_RUN_INFO_MENU2,  MMI_WIN_ID_DC_FEEDER_INFO2,  MMI_WIN_ID_DC_FEEDER_INFO2,  1,             2,           11,
		
		//��ʼ��icon�ṹ����
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//��ʼ��item�ṹ����
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 143,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         42,        154,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         55,        154,           110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         68,        154,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         107,       151,           110,      107,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         120,       154,           110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         133,       154,           110,      133,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         146,       154,           110,      146,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
};

static STR_T s_disp_get_string_pointer(U16_T index)
{
	if (m_e_lang == CHINESE)
		return (g_s_string[index][0]);
	else
		return (g_s_string[index][1]);
}

static STR_T s_disp_get_swt_string_pointer(U16_T index)
{
	if (m_e_lang == CHINESE)
		return (g_s_ctrl_swt_name[index][0]);
	else
		return (g_s_ctrl_swt_name[index][1]);
}


/*************************************************************
��������: v_disp_volt_init_by_level		           				
��������: ���ݵ�ѹ�ȼ�������صĵ�ѹ���������������ý�������ֵ����Сֵ						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_volt_init_by_level(void)
{
	U32_T div;
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
		div = 1;
	else
		div = 2;
	
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt               = m_t_dc_volt_def.f32_batt_equ_volt / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt               = m_t_dc_volt_def.f32_batt_flo_volt / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt         = m_t_dc_volt_def.u16_batt_total_end_volt / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit        = m_t_dc_volt_def.f32_batt_high_volt_limit / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit         = m_t_dc_volt_def.f32_batt_low_volt_limit / div;
	
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[0]      = m_t_dc_volt_def.f32_rect_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[1]      = m_t_dc_volt_def.f32_rect_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_max_out_volt     = m_t_dc_volt_def.f32_rect_max_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_min_out_volt     = m_t_dc_volt_def.f32_rect_min_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt = m_t_dc_volt_def.f32_rect_offline_out_volt / div;
	
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt       = m_t_dc_volt_def.u16_pb_high_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt        = m_t_dc_volt_def.u16_pb_low_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt       = m_t_dc_volt_def.u16_cb_high_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt        = m_t_dc_volt_def.u16_cb_low_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt      = m_t_dc_volt_def.u16_bus_high_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt       = m_t_dc_volt_def.u16_bus_low_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt     = m_t_dc_volt_def.u16_cb_output_volt / div;

	//����ֱ����ĸ�����ѹ
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt   = m_t_dc_volt_def.u16_cb_output_volt / div; 
	
	os_mut_release(g_mut_share_data);	
}


#define B21_CELL_MAX 24
#define B3_CELL_MAX 56
#define B4_CELL_MAX 112

/*************************************************************
��������: v_disp_batt1_data_init		           				
��������: ���ݵ��Ѳ���������ý�������ֵ����Сֵ�������ڵ�ǰҳ�漺������Ϊһ����Ѳ������ҳ�������£����ܵ���					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_batt1_data_init(void)
{
	U32_T i, j;
	U8_T u8_cell_max;
	U8_T u8_bms_max;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B21)
	{
		u8_cell_max = B21_CELL_MAX;
		u8_bms_max = 5;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B3)
	{
		u8_cell_max = B3_CELL_MAX;
		u8_bms_max = 2;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B4)
	{
		u8_cell_max = B4_CELL_MAX;
		u8_bms_max = 1;
	}
	
	for (i=0; i<BATT_GROUP_MAX; i++)
	{
		for (j=0; j<BATT_METER_MAX; j++)
		{
			if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_num[j] > u8_cell_max)
				g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_num[j] = u8_cell_max;
		}
		
		if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_bms_num > u8_bms_max)
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_bms_num = u8_bms_max;
		
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num = 0;	
		for (j=0; j<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_bms_num; j++)
		{
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num +=
						g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_num[j];
		}
					
		if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num > BATT_CELL_MAX)
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num = BATT_CELL_MAX;
	}
	
	win->t_item[2].u32_val_max = u8_bms_max;
	win->t_item[3].u32_val_max = u8_cell_max;
	win->t_item[4].u32_val_max = u8_cell_max;
	win->t_item[5].u32_val_max = u8_cell_max;
	win->t_item[6].u32_val_max = u8_cell_max;
	win->t_item[7].u32_val_max = u8_cell_max;
	
	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
		win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;

	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_batt2_data_init		           				
��������: ���ݵ��Ѳ���������ý�������ֵ����Сֵ�������ڵ�ǰҳ�漺������Ϊ������Ѳ������ҳ�������£����ܵ���					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_batt2_data_init(void)
{
	U32_T i;
	U8_T u8_cell_max;
	U8_T u8_bms_max;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B21)
	{
		u8_cell_max = B21_CELL_MAX;
		u8_bms_max = 5;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B3)
	{
		u8_cell_max = B3_CELL_MAX;
		u8_bms_max = 2;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B4)
	{
		u8_cell_max = B4_CELL_MAX;
		u8_bms_max = 1;
	}
	
	for (i=0; i<BATT_METER_MAX; i++)
	{
		if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i] > u8_cell_max)
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i] = u8_cell_max;
	}
		
	if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > u8_bms_max)
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num = u8_bms_max;

	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num = 0;	
	for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num; i++)
	{
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num +=
					g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i];
	}
				
	if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num > BATT_CELL_MAX)
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num = BATT_CELL_MAX;
		
	
	win->t_item[1].u32_val_max = u8_bms_max;
	win->t_item[2].u32_val_max = u8_cell_max;
	win->t_item[3].u32_val_max = u8_cell_max;
	win->t_item[4].u32_val_max = u8_cell_max;
	win->t_item[5].u32_val_max = u8_cell_max;
	win->t_item[6].u32_val_max = u8_cell_max;

	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: u8_disp_get_product_type		           				
��������: �õ���Ʒ���ͺű���					
�������: ��       		   				
�������: ��
����ֵ  ���ͺ���, 0--SC12, 1--SC22, 2--SC32														   				
**************************************************************/
static U8_T u8_disp_get_product_type(void)
{
	U8_T u8_type;
	
	s32_flash_dataflash_read(DATAFLASH_MODEL_ADDR, &u8_type, 1);
	
	if ((u8_type == PRODUCT_TYPE_SC12)
		|| (u8_type == PRODUCT_TYPE_SC22)
		|| (u8_type == PRODUCT_TYPE_SC32))
	{
		return u8_type;
	}
	else
	{
		return PRODUCT_TYPE_SC12;
	}
}

/*************************************************************
��������: v_disp_share_data_restore		           				
��������: �ָ�Ĭ�����ݣ������ò����ָ���Ĭ��ֵ						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_disp_cfg_data_restore(void)
{
	U32_T i, j;

	g_u8_product_type = u8_disp_get_product_type();
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	m_e_lang = g_t_share_data.t_sys_cfg.t_sys_param.e_lang;
	m_e_lcd_direction = g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection;
	
	memset(&(g_t_share_data.t_sys_cfg), 0, sizeof(g_t_share_data.t_sys_cfg));
	
	if (g_u8_product_type == PRODUCT_TYPE_SC12)
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = ONE_PATH;                     // ��������·������/1·��Ĭ��1·
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num = 0;                // ���������������������̶�Ϊ0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num = 0;               // ���������������������̶�Ϊ0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num = 1;                     // �����������Χ0~1��Ĭ��1		
		
		g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B���ʱ���̶�Ϊ��
		g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;                   // ͨ��ģ��������̶�Ϊ0
		g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;                   // UPSģ��������̶�Ϊ0
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 2;                     // ����ģ����������Χ1~3��Ĭ��2
		
		g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output = 4;                    //�������߹��ϣ���Χ0,4~6��Ĭ��4
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output = 5;                //ֱ��ĸ�߹��ϣ���Χ0,4~6��Ĭ��5
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output = 6;                  //����쳣���ϣ���Χ0,4~1��Ĭ��6
		g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output = 0;                  //��Ե�½����ϣ���Χ0,4~1��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output = 0;                  //���ģ����ϣ���Χ0,4~1��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output = 0;                  //����۶������ϣ���Χ0,4~1��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output = 0;                  //ͨ�ż�����Դ���ϣ���Χ0,4~1��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output = 0;                //����֧·���ϣ���Χ0,4~1��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output = 0;                    //����ָʾ����Χ0,4~1��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output = 0;                    //����ָʾ����Χ0,4~1��Ĭ��0
		
	}
	else if (g_u8_product_type == PRODUCT_TYPE_SC22)
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = TWO_PATH;                     // ��������·����1·/2·/�ޣ�Ĭ��2·
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num = 1;                // ��������������Χ0~1��Ĭ��1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num = 1;               // ��������������Χ0~1��Ĭ��1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num = 1;                     // �����������Χ0~1��Ĭ��1
		
		g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B���ʱ����/�У�Ĭ����
		g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;                   // ͨ��ģ���������Χ0~4��Ĭ��0
		g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;                   // UPSģ���������Χ0~4��Ĭ��0
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 4;                     // ģ����������Χ1~8��Ĭ��4
		
		g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output = 1;                    //�������߹��ϣ���Χ0~6��Ĭ��1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output = 2;                //ֱ��ĸ�߹��ϣ���Χ0~6��Ĭ��2
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output = 3;                  //����쳣���ϣ���Χ0~6��Ĭ��3
		g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output = 4;                  //��Ե�½����ϣ���Χ0~6��Ĭ��4
		g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output = 0;                  //���ģ����ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output = 0;                  //����۶������ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output = 0;                  //ͨ�ż�����Դ���ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output = 0;                //����֧·���ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output = 5;                    //����ָʾ����Χ0~6��Ĭ��5
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output = 6;                    //����ָʾ����Χ0~6��Ĭ��6
	}
	else
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = TWO_PATH;                     // ��������·����1·/2·/�ޣ�Ĭ��2·
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num = 1;                // ��������������Χ0~4��Ĭ��1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num = 1;               // ��������������Χ0~4��Ĭ��1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num = 2;                     // �����������Χ0~2��Ĭ��1
		
		g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B���ʱ����/�У�Ĭ����
		g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;                   // ͨ��ģ���������Χ0~8��Ĭ��0
		g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;                   // UPSģ���������Χ0~8��Ĭ��0
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 4;                     // ģ����������Χ1~24��Ĭ��4
		
		g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output = 1;                    //�������߹��ϣ���Χ0~6��Ĭ��1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output = 2;                //ֱ��ĸ�߹��ϣ���Χ0~6��Ĭ��2
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output = 3;                  //����쳣���ϣ���Χ0~6��Ĭ��3
		g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output = 4;                  //��Ե�½����ϣ���Χ0~6��Ĭ��4
		g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output = 0;                  //���ģ����ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output = 0;                  //����۶������ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output = 0;                  //ͨ�ż�����Դ���ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output = 0;                //����֧·���ϣ���Χ0~6��Ĭ��0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output = 5;                    //����ָʾ����Χ0~6��Ĭ��5
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output = 6;                    //����ָʾ����Χ0~6��Ĭ��6
	}
	
	g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num = 9;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num = 0;                      //Ĭ��û��D21ģ��
	g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] = AUTO_MODE;                        // ��ع���ʽ �ֶ����Զ�
	g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[1] = AUTO_MODE;                        // ��ع���ʽ �ֶ����Զ�
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 = 65;                        // ��ر����������Χ30~3000Ah��Ĭ��100Ah
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt = 245;                        // ��ؾ����ѹ����Χ�����Ƿѹ��~��ѹ�㣬Ĭ��245V
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt = 235;                        // ��ظ����ѹ����Χ�����Ƿѹ��~��ѹ�㣬Ĭ��235V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr = 0.1;                      // ��س�������㣬��Χ0.05~�������㣬Ĭ��0.1C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr = 0.08;		            // ���ת�����������Χ0.04~��������㣬Ĭ��0.08C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_to_equ_dur_time = 60;                  // ��������ת���������ĳ���ʱ�䣬��Χ30~999�룬Ĭ��60��
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_equ_cycle = 4320;	    	            // ��ض�ʱ�������ڣ���Χ1~9999Сʱ��Ĭ��4320Сʱ
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_ac_fail_time = 20;                     // ����ͣ��ʱ������Χ1~999���ӣ�Ĭ��20����
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_cap = 0.8;                         // ����������ڣ���Χ0.8~0.95C10��Ĭ��0.8C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_flo_curr = 0.02;			        // ���ת�����������Χ0.01~0.03C10��Ĭ��0.02C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_curr_go_time = 180;                    // ���ת�����ʱʱ�䣬��Χ1~360���ӣ�Ĭ��180����
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_equ_time = 720;	                // ��ؾ��䱣��ʱ�䣬��Χ1~999���ӣ�Ĭ��720����
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt = 200;                  // ��طŵ���ֹ��ѹ����Χ190~220V��Ĭ��200V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_cell_end_volt = 1.8;                   // ��طŵ���ֹ�����ѹ����Χ1.8~12V��Ĭ��1.8V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_dis_time = 600;                    // ��طŵ���ֹʱ�䣬��Χ1~999���ӣ�Ĭ��600����
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt = 0;               // �¶Ȳ�����ѹ����Χ0~500mV��Ĭ��0mV
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit = 264;                 // ������ѹ�㣬��Χ220~320V��Ĭ��264V
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit = 187;                  // �����Ƿѹ�㣬��Χ186~220��Ĭ��187V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit = 0.2;                 // ��س������㣬��Χ0.1~1.0C10��Ĭ��0.2
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_10c_dis_rate = 0.3;               // ���1.0C�ŵ���   [0~10.0], Ĭ��0.3
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_09c_dis_rate = 0.4;               // ���0.9C�ŵ���   [0~10.0], Ĭ��0.4
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_08c_dis_rate = 0.5;               // ���0.8C�ŵ���   [0~10.0], Ĭ��0.5
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_07c_dis_rate = 0.7;               // ���0.7C�ŵ���   [0~10.0], Ĭ��0.7
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_06c_dis_rate = 0.8;               // ���0.6C�ŵ���   [0~10.0], Ĭ��0.8
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_05c_dis_rate = 0.9;               // ���0.5C�ŵ���   [0~10.0], Ĭ��0.9
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_04c_dis_rate = 2.2;               // ���0.4C�ŵ���   [0~10.0], Ĭ��2.2
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_03c_dis_rate = 3.3;               // ���0.3C�ŵ���   [0~10.0], Ĭ��3.3
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_02c_dis_rate = 4;                 // ���0.2C�ŵ���   [0~10.0], Ĭ��4
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_01c_dis_rate = 10;                // ���0.1C�ŵ���   [0~10.0], Ĭ��10
	
	g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level = VOLT_LEVEL_220;             // ϵͳ���ͣ�220/110V
	m_e_volt_level = g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level;
	g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add = 14;               // ��������������Χ14~111��Ĭ��14
	g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add = 30;               // ��������������Χ14~111��Ĭ��30
	g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num = TWO;			            // ֱ��ĸ�߶�������ΧONE/TWO��Ĭ��ONE
	g_t_share_data.t_sys_cfg.t_sys_param.e_lang = m_e_lang;                         // ���ԣ�����/Ӣ�ģ�Ĭ������
	g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection = m_e_lcd_direction;       // ��ʾ������ˮƽ/���ԣ�Ĭ��ˮƽ
	g_t_share_data.t_sys_cfg.t_sys_param.e_limit_curr = AUTO_LIMIT_CURR;            // ������ʽ���Զ�/�ֶ���Ĭ���ֶ�
	g_t_share_data.t_sys_cfg.t_sys_param.f32_manual_limit_curr = 100;               // �ֶ������㣬5%~110%��Ĭ��100%
	g_t_share_data.t_sys_cfg.t_sys_param.u8_fc10_offline_time = 10;                 // ����ģ��ͨѶ�ж�ʱ�䣬��Χ5~60S��Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_bms_offline_cnt = 10;                   // ���Ѳ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_offline_cnt = 10;                  // ����ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt = 10;                  // ͨ��ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_offline_cnt = 10;                  // ���ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt = 10;                   // ATSģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_acm_offline_cnt = 10;                   // �๦�ܵ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt = 10;                  // DC10ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_offline_cnt = 3;                   // RC10ģ��ͨѶ�ж������������Χ4~99����Ĭ��10
	g_t_share_data.t_sys_cfg.t_sys_param.e_can_baud = CAN_BAUD_125K;		        // CANͨ�����ʣ�125K��50K��20K��10K��Ĭ��125K
	g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way = INSU_PROJECT;            // ĸ�ߵ��������ʽ������ģʽ�͵���ģʽ��Ĭ�Ϲ���ģʽ
    g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay = 2;                   // ƽ�⼰��ƽ�����Ͷ����ʱ��1~120���裬����Ϊ��λ��Ĭ��2��
    g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range = 10;                // ֧·���������̣�1~500���裬��mAΪ��λ��Ĭ��10mA
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm = 3;               // ĸ�߾�Եѹ���ȷ��ʱ�䣬1~180���裬����Ϊ��λ��Ĭ��3��
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period = 24;                  // ��Ե���ڲ�������ʱ�䣬0~180���裬��СʱΪ��λ��Ĭ��24Сʱ
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_hour = 8;                     // ǿ��������Ե������ʱ��-ʱ��0~23Сʱ��Ĭ��8��
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_min = 0;                      // ǿ��������Ե������ʱ��-�֣�0~59���ӣ�Ĭ��0��
	g_t_share_data.t_sys_cfg.t_sys_param.e_buzzer_state = BEEP;                     // �������澯���ã�����/������Ĭ������
	g_t_share_data.t_sys_cfg.t_sys_param.e_minor_fault_save = SAVE;                 // ��Ҫ�澯�������ã�0-�����棬1-����
	g_t_share_data.t_sys_cfg.t_sys_param.e_general_fault_save = NO_SAVE;            // һ��澯�������ã�0-�����棬1-����
	g_t_share_data.t_sys_cfg.t_sys_param.e_curr_imbalance_alm = CLOSE;              // ������ƽ�ⱨ�����ر�/������Ĭ�Ϲر�
	g_t_share_data.t_sys_cfg.t_sys_param.u32_password = MMI_USER_PASSWORD; 			// �û������������ã�5λ���룬Ĭ��Ϊ11111����������Ϊ02051
	g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr = 1;                         // ������ַ����Χ1~255��Ĭ��1
	g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol = BS_MODBUS;           // ��̨ͨ��Э�飬MODBUS/CDT/IEC101/IEC102��Ĭ��MODBUS
	g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud = COM_BAUD_9600;          // �����ʣ�Ĭ��9600
	g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity = ODD_PARITY;           // У�飬Ĭ�������
	//g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B���ʱ����/�У�Ĭ����

	g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign = POSITIVE;		            // ���ص����������ֵ���ţ�������, Ĭ����
	g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero = 0.0;                       // ���ص��������������ֵ����Χ[0~2.0], Ĭ��0.0
    g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign = POSITIVE;		            // ���1�����������ֵ���ţ�������, Ĭ����
	g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero = 0.0;                       // ���1���������������ֵ����Χ[0~2.0], Ĭ��0.0
    g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign = POSITIVE;		            // ���2�����������ֵ���ţ�������, Ĭ����
	g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero = 0.0;                       // ���2���������������ֵ����Χ[0~2.0], Ĭ��0.0
		
	
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 4;                     // ģ����������Χ1~24��Ĭ��4
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_protocol = MODBUS;                 // ͨ��Э�飬Э��̶�Ϊmodbus��������
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr = CURR_10A;             // �������5A/7A/10A/20A/30A/35A/40A/50A��ѡ��Ĭ��10A
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt = 220;                  // �����ѹ�����ֵ���治���趨���ɵ�ع��������޸�
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[0] = 110;             // �����ٷֱȣ����ֵ���治���趨���ɵ�ع��������޸�
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[1] = 110;             // �����ٷֱȣ����ֵ���治���趨���ɵ�ع��������޸�
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_max_out_volt = 286;              // �����ѹ���ޣ���Χ220~286V��Ĭ��286V�����ֵ����û����ص��趨�����Ĭ��ֵ
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_min_out_volt = 176;              // �����ѹ���ޣ���Χ176~220V��Ĭ��176V�����ֵ����û����ص��趨�����Ĭ��ֵ
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt = 220;          // Ĭ�������ѹ����Χ176~286V��Ĭ��220V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_curr_percent = 110;      // Ĭ����������ȣ���Χ5~110%��Ĭ��110%
  
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = ONE_PATH;                     // ��������·����1·/2·/�ޣ�Ĭ��1·
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase = AC_3_PHASE;                  // ������������������/���࣬Ĭ������
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_high_volt = 456;//253;                   // ������ѹ�㣬��Χ220~530V��Ĭ��456V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_low_volt = 327;//187;                    // ����Ƿѹ�㣬��Χ187~380V��Ĭ��327V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_lack_phase = 200;//100;                  // ����ȱ��㣬��Χ0~380V��Ĭ��200V���ڵ�������£�������Ч
	
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb = HAVE;                      // ��ĸ���ã���/�У�Ĭ����
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl = STEP_5_7V;    // �������ƣ���/5��4��/5��7��/7��3��/7��5����Ĭ����
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_75MV;        // ���������ѹ��75mv/50mv��Ĭ��75mv
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range = 50;            // ���ط��������̣���Χ25~2000A��Ĭ��100A
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt1_shunt_range = 30;           // ���1���������̣���Χ25~2000A��Ĭ��100A
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt2_shunt_range = 30;           // ���2���������̣���Χ25~2000A��Ĭ��100A
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_feeder_shunt_range = 100;          // ֧·�������������̣���Χ10~1000A��Ĭ��100A

	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u8_dc10_module_num = 1;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_ac_meas_modle = 0x0200;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_ac_meas_num = 0x0000;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt = 75;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt = 75;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt = 75;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range = 50;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range = 30;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range = 30;

	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num = 1;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_ac_meas_modle = 0x0200;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_ac_meas_num = 0x0000;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_high_volt = 456;//253;                   // ������ѹ�㣬��Χ220~530V��Ĭ��456V
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_low_volt  = 327;//187;                    // ����Ƿѹ�㣬��Χ187~380V��Ĭ��327V
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_lack_phase = 200;//100;                  // ����ȱ��㣬��Χ0~380V��Ĭ��200V���ڵ�������£�������Ч
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range = 100;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range = 100;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range = 100;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u16_feeder_shunt_range = 100;          // ֧·�������������̣���Χ10~1000A��Ĭ��100A 

	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num = 1;        //���߼��ģ���������Χ0~3��Ĭ��1
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num = 31;       //����֧·������Χ0~64
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_state_feeder_num = 31;        //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_alarm_feeder_num = 0;        //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_curr_feeder_num = 0;         //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_feeder_num = 27;           // ������ģ���ʵ��֧·��
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_alarm_feeder_num = 0;     // �澯��֧·������Χ0~64��Ĭ��21
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_state_feeder_num = 27;     // ״̬��֧·������Χ0~64��Ĭ��21
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_insu_feeder_num = 0;      // ��Ե��֧·������Χ0~64��Ĭ��21
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
		
	for (j=1; j<ACS_FEEDER_MODULE_MAX; j++)
	{
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
	}

	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt = 286;			    // ��ĸ��ѹ���ޣ���Χ220~320V��Ĭ��286V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt = 187;               // ��ĸǷѹ���ޣ���Χ187~220V��Ĭ��187V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt = 235;              // ��ĸ��ѹ���ޣ���Χ220~242V��Ĭ��235V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt = 205;               // ��ĸǷѹ���ޣ���Χ198~220V��Ĭ��205V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt = 286;             // ĸ�߹�ѹ���ޣ���Χ220~320V��Ĭ��286V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt = 187;              // ĸ��Ƿѹ���ޣ���Χ186~220V��Ĭ��187V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt = 220;            // ��ĸ�����ѹ�����������ѹ������Χ210~230V��Ĭ��220V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance = 50;          // ��Ե��ѹ��ƽ�⣬��Χ20~100V��Ĭ��50V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr = 25;                 // ��Ե�������ޣ���Χ5~99K��Ĭ��25K
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_dc_bus_input_ac_thr = 10;          // ������ֱ����������ֵ����Χ1~50V��Ĭ��10V

	g_t_share_data.t_sys_cfg.t_batt.e_bms_type = B21;                               // ��������࣬B21/B3/B4��Ĭ��B21
	g_t_share_data.t_sys_cfg.t_batt.f32_cell_high_volt = 12.05;                     // �����ѹ����Χ2~15V��Ĭ��12.05V
	g_t_share_data.t_sys_cfg.t_batt.f32_cell_low_volt = 10.08;                      // ����Ƿѹ����Χ1.8~12V��Ĭ��10.08V
	g_t_share_data.t_sys_cfg.t_batt.f32_tail_high_volt = 12.05;                     // ĩ�˵����ѹ����Χ2~15V��Ĭ��12.05V
	g_t_share_data.t_sys_cfg.t_batt.f32_tail_low_volt = 10.08;                      // ĩ�˵���Ƿѹ����Χ1.8~12V��Ĭ��10.08V
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num = 1;                 // ����Ǹ�������Χ0~5��Ĭ��0
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num = 0;          // ʵ�ʵ����ؽ���
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[0] = 18;            // u8_cell_num[0]�ķ�Χ0~120��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[1] = 24;            // u8_cell_num[1]�ķ�Χ0~54��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[2] = 24;            // u8_cell_num[2]�ķ�Χ0~24��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[3] = 24;            // u8_cell_num[3]�ķ�Χ0~24��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[4] = 24;            // u8_cell_num[4]�ķ�Χ0~24��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num = 1;                 // ����Ǹ�������Χ0~5��Ĭ��0
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num = 0;          // ʵ�ʵ����ؽ���
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[0] = 18;            // u8_cell_num[0]�ķ�Χ0~120��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[1] = 24;            // u8_cell_num[1]�ķ�Χ0~54��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[2] = 24;            // u8_cell_num[2]�ķ�Χ0~24��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[3] = 24;            // u8_cell_num[3]�ķ�Χ0~24��Ĭ��24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[4] = 24;            // u8_cell_num[4]�ķ�Χ0~24��Ĭ��24
	
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num = 1;        //���߼��ģ���������Χ0~4��Ĭ��1
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_branch_num = 17;       //����֧·������Χ0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_state_feeder_num = 17;        //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_insu_feeder_num = 17;         //��Ե��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_alarm_feeder_num = 0;        //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_curr_feeder_num = 0;         //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_feeder_num = 17;           // ������ģ���ʵ��֧·��
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_alarm_feeder_num = 0;     // �澯��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_state_feeder_num = 17;     // ״̬��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_insu_feeder_num = 17;      // ��Ե��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
			
		for (j=1; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
		}
	}
	
	for (i=g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i<FEEDER_PANEL_MAX/2; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num = 0;        //���߼��ģ���������Χ0~4��Ĭ��1
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_branch_num = 0;        //����֧·������Χ0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_state_feeder_num = 0;         //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_insu_feeder_num = 0;          //��Ե��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_alarm_feeder_num = 0;         //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_curr_feeder_num = 0;          //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
			
		for (j=0; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
		}
	}
	
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num = 1;        //���߼��ģ���������Χ0~4��Ĭ��1
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_branch_num = 17;       //����֧·������Χ0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_state_feeder_num = 17;        //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_insu_feeder_num = 17;         //��Ե��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_alarm_feeder_num = 0;        //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_curr_feeder_num = 0;         //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_feeder_num = 17;           // ������ģ���ʵ��֧·��
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_alarm_feeder_num = 0;     // �澯��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_state_feeder_num = 17;     // ״̬��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_insu_feeder_num = 17;      // ��Ե��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
			
		for (j=1; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
		}
	}
	
	for (i=g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i<FEEDER_PANEL_MAX/2; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num = 0;        //���߼��ģ���������Χ0~4��Ĭ��1
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_branch_num = 0;        //����֧·������Χ0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_state_feeder_num = 0;         //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_insu_feeder_num = 0;          //��Ե��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_alarm_feeder_num = 0;         //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_curr_feeder_num = 0;          //�澯��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
			
		for (j=0; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
		}
	}
	
	//g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;             //ͨ��ģ���������Χ0~8��Ĭ��0
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num = 0;           //���߼��ģ���������Χ0~1��Ĭ��0
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num = 0;           //����֧·������Χ0~64
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_state_feeder_num = 0;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_alarm_feeder_num = 0;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_curr_feeder_num = 0;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u16_feeder_shunt_range = 100;       // ֧·�������������̣���Χ10~1000A��Ĭ��100A
	g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol = DCDC_MODBUS;           // ͨ��Э�飬Э��Ϊmodbus/can��Ĭ��modbus
	g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr = DCDC_CURR_100A;       // �������5A/10A/20A/30A/40A/50A/60A/80A/100A��ѡ��Ĭ��100A
	g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt = 48;                  // �����ѹ����Χ40~60��Ĭ��48
	g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent = 100;             // �����ٷֱȣ���Χ10%~100%
	
	for (i=0; i<DCDC_FEEDER_MODULE_MAX; i++)
	{
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
	}
	
	//g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;             //UPSģ���������Χ0~8��Ĭ��0
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num = 0;           //���߼��ģ���������Χ0~1��Ĭ��0
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num = 0;           //����֧·������Χ0~64
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_state_feeder_num = 0;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_alarm_feeder_num = 0;            //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_curr_feeder_num = 0;             //״̬��֧·������Χ0~64����ֵû�ж�Ӧ��������Ǽ��������
	g_t_share_data.t_sys_cfg.t_dcac_panel.u16_feeder_shunt_range = 100;       // ֧·�������������̣���Χ10~1000A��Ĭ��100A
	g_t_share_data.t_sys_cfg.t_dcac_panel.e_protocol = MODBUS;                // ͨ��Э�飬Э��̶�Ϊmodbus��������
	
	for (i=0; i<DCAC_FEEDER_MODULE_MAX; i++)
	{
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_feeder_num = 0;            // ������ģ���ʵ��֧·��
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_alarm_feeder_num = 0;      // �澯��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].e_alarm_type = NORMALLY_OPEN; // �澯�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_state_feeder_num = 0;      // ״̬��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].e_state_type = NORMALLY_OPEN; // ״̬�����뷽ʽ�����������գ�Ĭ�ϳ���
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_insu_feeder_num = 0;       // ��Ե��֧·������Χ0~64��Ĭ��21
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_curr_feeder_num = 0;       // ������֧·������Χ0~64��Ĭ��0
	}
	
	v_disp_volt_init_by_level();    //���ݵ�ѹ�ȼ�������صĵ�ѹֵ

	g_t_share_data.t_sys_cfg.t_ctl.u16_batt[0] = 0;                                    //�ֶ������ƣ�0:���䣬1:���䣬2:����
	g_t_share_data.t_sys_cfg.t_ctl.u16_batt[1] = 0;                                    //�ֶ������ƣ�0:���䣬1:���䣬2:����
	g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;                                      // bit0~1:�����������ԣ�0��һ·���ȣ�1����·���ȣ�2���̶�һ·��3���̶���·
														                            // bit2~15:����
	for (i=0; i<RECT_CNT_MAX; i++)
		g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl[i] = 0;                        // ���ģ��״̬���ƣ�1���ػ���0������

	os_mut_release(g_mut_share_data);
	
	v_fetch_save_cfg_data();   //���������DATAFLASH
}

/*************************************************************
��������: v_disp_adjust_data_restore	           				
��������: У׼ϵ����ʼ��					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_adjust_data_restore(void)
{
	os_mut_wait(g_mut_share_data, 0xFFFF);

	memset(&g_t_share_data.t_coeff_data, 0, sizeof(g_t_share_data.t_coeff_data));

	//������ѹȱʡб��
	g_t_share_data.t_coeff_data.f32_ac1_uv_slope = 1.19;				
	g_t_share_data.t_coeff_data.f32_ac1_vw_slope = 1.19;
	g_t_share_data.t_coeff_data.f32_ac1_wu_slope = 1.19;
	
	g_t_share_data.t_coeff_data.f32_ac2_uv_slope = 1.19;				
	g_t_share_data.t_coeff_data.f32_ac2_vw_slope = 1.19;
	g_t_share_data.t_coeff_data.f32_ac2_wu_slope = 1.19;
	
	//ֱ����ѹȱʡ����ϵ��
	g_t_share_data.t_coeff_data.f32_v1_vol_slope = 0.1209;
	g_t_share_data.t_coeff_data.s16_v1_vol_zero  = 0;
	g_t_share_data.t_coeff_data.f32_v2_vol_slope = 0.1209;
	g_t_share_data.t_coeff_data.s16_v2_vol_zero	 = 0;
	g_t_share_data.t_coeff_data.f32_v3_vol_slope = 0.1209;
	g_t_share_data.t_coeff_data.s16_v3_vol_zero  = 0;

	g_t_share_data.t_coeff_data.s16_a1_fixed_zero = 2048;
	g_t_share_data.t_coeff_data.s16_a2_fixed_zero = 2048;
	g_t_share_data.t_coeff_data.s16_a3_fixed_zero = 2048;

	g_t_share_data.t_coeff_data.f32_a1_curr_slope = 0.0562;//0.0649;
	g_t_share_data.t_coeff_data.f32_a1_curr_zero  = -115;//133
	g_t_share_data.t_coeff_data.f32_a2_curr_slope = 0.0562;//0.0649;
	g_t_share_data.t_coeff_data.f32_a2_curr_zero  = -115;//133
	g_t_share_data.t_coeff_data.f32_a3_curr_slope = 0.0562;//0.0649;
	g_t_share_data.t_coeff_data.f32_a3_curr_zero  = -115;//133

	g_t_share_data.t_coeff_data.f32_ref_volt = 2.5;

	g_t_share_data.t_coeff_data.f32_neg_vol_slope = 0.1209;  // ���Եص�ѹ����б��
	g_t_share_data.t_coeff_data.s16_neg_vol_zero  = 0;       // ���Եص�ѹ�������

	os_mut_release(g_mut_share_data);

	v_fetch_save_adjust_coeff();
}

/*************************************************************
��������: v_disp_cfg_data_init		           				
��������: �������ݳ�ʼ��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_disp_cfg_data_init(void)
{
	U8_T i, j;
	os_mut_init(g_mut_share_data);           //��ʼ��������

	os_mut_wait(g_mut_share_data, 0xFFFF);
	memset(&g_t_share_data, 0, sizeof(g_t_share_data));
	os_mut_release(g_mut_share_data);

	g_u8_product_type = u8_disp_get_product_type();

	if (s32_fetch_read_cfg_data() == -1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		memset(&(g_t_share_data.t_sys_cfg), 0, sizeof(g_t_share_data.t_sys_cfg));
		os_mut_release(g_mut_share_data);

		v_disp_cfg_data_restore();
	}

	if (s32_fetch_read_adjust_coeff() == -1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		memset(&(g_t_share_data.t_coeff_data), 0, sizeof(g_t_share_data.t_coeff_data));
		os_mut_release(g_mut_share_data);

		v_disp_adjust_data_restore();
	}
	
	v_fetch_read_swt_cfg_data();    //��ȡ����������
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	for (i=0; i<FEEDER_PANEL_MAX; i++)
	{
		for (j=0; j<FEEDER_BRANCH_MAX; j++)
		{
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_alarm = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_state = 1;//00��·�޿���״̬��⣬01��ʾ�Ͽ���02��ʾ�պ�
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_insu_state = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_curr_state = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].u8_sensor_state[j] = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].f32_res = 999.9;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].f32_curr = 0.0;
		}
	}
		
	for (j=0; j<FEEDER_BRANCH_MAX; j++)
	{
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].u8_alarm = 1;
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].u8_state = 2;//00��·�޿���״̬��⣬01��ʾ�Ͽ���02��ʾ�պ�
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].u8_curr_state = 1;
		g_t_share_data.t_rt_data.t_dcdc_panel.u8_sensor_state[j] = 1;
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].f32_curr = 0.0;
	}
	
	for (j=0; j<FEEDER_BRANCH_MAX; j++)
	{
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].u8_alarm = 1;
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].u8_state = 2;//00��·�޿���״̬��⣬01��ʾ�Ͽ���02��ʾ�պ�
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].u8_curr_state = 1;
		g_t_share_data.t_rt_data.t_dcac_panel.u8_sensor_state[j] = 1;
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].f32_curr = 0.0;
	}

	g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = 0;
	for (i=0; i<RELAY_MAX; i++)
		g_t_share_data.t_sys_cfg.t_ctl.u8_relay[i] = 0;

	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_display_init		           				
��������: ��ʾ��ʼ������ʼ��ȫ������������ʼ��ҳ��ṹ������ָ��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_display_init(void)
{
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	m_t_win_record.u16_curr_id = MMI_WIN_ID_MAIN_WINDOW;
	m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
	{
		m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
		m_t_win_record.t_curr_win.u8_icon_cnt = 1;
	}
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	m_e_volt_level = g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level;
	m_e_lang = g_t_share_data.t_sys_cfg.t_sys_param.e_lang;
	m_e_lcd_direction = g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection;
	
	if (m_e_lcd_direction == LCD_HORIZONTAL)   //������ʾ�����ʼ������ֵ
	{
		g_u16_key_cancel = KEY_K7;
		g_u16_key_enter = KEY_K6;
		g_u16_key_up = KEY_K4;
		g_u16_key_down = KEY_K5;
		g_u16_key_add = KEY_K3;
		g_u16_key_sub = KEY_K2;
	}
	else
	{
		g_u16_key_cancel = KEY_K7;
		g_u16_key_enter = KEY_K6;
		g_u16_key_up = KEY_K3;
		g_u16_key_down = KEY_K2;
		g_u16_key_add = KEY_K5;
		g_u16_key_sub = KEY_K4;
	}
	 
	os_mut_release(g_mut_share_data);

	v_relay_relay_operation(ENABLE_BACKLIGHT);
}


/*************************************************************
��������: s_disp_get_record_name_by_id		           				
��������: ͨ���¼�ID�����¼�������						
�������: u16_id -- �¼�ID        		   				
�������: ��
����ֵ  �������ַ���ָ��														   				
**************************************************************/
static char *s_disp_get_record_name_by_id(U16_T u16_id)
{
	U16_T u16_group = ((u16_id & LOG_ID_GROUP_MASK) >> LOG_ID_GROUP_OFFSET);
	U16_T u16_index = (u16_id & LOG_ID_INDEX_MASK);
	char *s_name = NULL;

	if (u16_group == LOG_ID_BATT_GROUP)
	{
		if (m_e_lang == CHINESE)
			s_name = g_s_batt_record[u16_index][0];
		else
			s_name = g_s_batt_record[u16_index][1];
	}

	return s_name;
}


/*************************************************************
��������: s_disp_get_fault_name_by_id		           				
��������: ͨ������ID���ҹ��ϵ�����						
�������: u16_id -- ����ID        		   				
�������: ��
����ֵ  �������ַ���ָ��														   				
**************************************************************/
static char *s_disp_get_fault_name_by_id(U16_T u16_id)
{
	U16_T u16_group = ((u16_id & FAULT_ID_GROUP_MASK) >> FAULT_GROUP_OFFSET);
	U16_T u16_index = (u16_id & FAULT_ID_INDEX_MASK);
	U32_T u32_addr;
	
	switch (u16_group)
	{
		case FAULT_AC_GROUP:                  //����
			u32_addr = DATAFLASH_AC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_DC_BUS_GROUP:              //ֱ��ĸ��
			u32_addr = DATAFLASH_DC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_BATT1_GROUP:               //1�����쳣
			u32_addr = DATAFLASH_BATT1_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
		
		case FAULT_BATT2_GROUP:               //2�����쳣
			u32_addr = DATAFLASH_BATT2_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_BMS_GROUP:                 //���Ѳ�����
			u32_addr = DATAFLASH_BMS_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_RECT_GROUP:                //����ģ�����
			u32_addr = DATAFLASH_RECT_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCAC_GROUP:                //���ģ�����
			u32_addr = DATAFLASH_DCAC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCDC_GROUP:                //ͨ��ģ�����
			u32_addr = DATAFLASH_DCDC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_FC_GROUP:                  //����ģ�����
			u32_addr = DATAFLASH_FC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_RC_GROUP:                  //RC10ģ�����
			u32_addr = DATAFLASH_RC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_SWT_GROUP:                 //�Դ�����������
			u32_addr = DATAFLASH_SWT_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_DC_PANEL1_GROUP:           //�ֵ���01����
			u32_addr = DATAFLASH_DC_PANEL1_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_DC_PANEL2_GROUP:           //�ֵ���02����
			u32_addr = DATAFLASH_DC_PANEL2_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL3_GROUP:           //�ֵ���03����
			u32_addr = DATAFLASH_DC_PANEL3_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL4_GROUP:           //�ֵ���04����
			u32_addr = DATAFLASH_DC_PANEL4_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL5_GROUP:           //�ֵ���05����
			u32_addr = DATAFLASH_DC_PANEL5_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL6_GROUP:           //�ֵ���06����
			u32_addr = DATAFLASH_DC_PANEL6_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL7_GROUP:           //�ֵ���07����
			u32_addr = DATAFLASH_DC_PANEL7_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL8_GROUP:           //�ֵ���08����
			u32_addr = DATAFLASH_DC_PANEL8_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCDC_PANEL_GROUP:          //ͨ��������
			u32_addr = DATAFLASH_DCDC_PANEL_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCAC_PANEL_GROUP:          //UPS������
			u32_addr = DATAFLASH_DCAC_PANEL_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_AC_PANEL_GROUP:          //����������
			u32_addr = DATAFLASH_AC_PANEL_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		default:
			return NULL;
	}
	
	if (m_e_lang == ENGLISH)
		u32_addr += (DATAFLASH_FAULT_NAME_ITEM_SIZE / 2);
	
	memset(m_t_win_record.u8_buffer, 0, sizeof(m_t_win_record.u8_buffer));
	s32_flash_dataflash_read(u32_addr, m_t_win_record.u8_buffer, DATAFLASH_FAULT_NAME_ITEM_SIZE/2);

	return (char *)(m_t_win_record.u8_buffer);
}

/*************************************************************
��������: s_disp_get_branch_name_by_id		           				
��������: ͨ������ID���ҹ��ϵ�����						
�������: u8_panel  -- ��������
          u8_branch -- ����֧·��
�������: ��
����ֵ  �������ַ���ָ��														   				
**************************************************************/
static char *s_disp_get_branch_name_by_id(U8_T u8_panel, U8_T u8_branch)
{
	U32_T u32_addr = DATAFLASH_BRANCH_NAME_ADDR + (u8_panel*64 + u8_branch)*DATAFLASH_BRANCH_NAME_ITME_SIZE;
	
	if (m_e_lang == ENGLISH)
		u32_addr += (DATAFLASH_BRANCH_NAME_ITME_SIZE / 2);
	
	memset(m_t_win_record.u8_buffer, 0, sizeof(m_t_win_record.u8_buffer));
	s32_flash_dataflash_read(u32_addr, m_t_win_record.u8_buffer, DATAFLASH_BRANCH_NAME_ITME_SIZE/2);

	return (char *)(m_t_win_record.u8_buffer);
}

/*************************************************************
��������: s_disp_get_about_name_by_index		           				
��������: ͨ������ID���ҹ��ϵ�����						
�������: u8_index -- ��������Χ0~7����8����Ϣ       		   				
�������: ��
����ֵ  �������ַ���ָ��														   				
**************************************************************/
static char *s_disp_get_about_name_by_index(U8_T u8_index)
{
	U32_T u32_addr = DATAFLASH_ABOUT_ADDR + u8_index*DATAFLASH_ABOUT_ITEM_SIZE;
	
	if (m_e_lang == ENGLISH)
		u32_addr += (DATAFLASH_ABOUT_ITEM_SIZE / 2);
	
	memset(m_t_win_record.u8_buffer, 0, sizeof(m_t_win_record.u8_buffer));
	s32_flash_dataflash_read(u32_addr, m_t_win_record.u8_buffer, DATAFLASH_ABOUT_ITEM_SIZE/2);

	return (char *)(m_t_win_record.u8_buffer);
}

/*************************************************************
��������: s_disp_get_swt_name_by_index		           				
��������: ͨ�������������ҿ��ص�����						
�������: u8_index -- ����      		   				
�������: ��
����ֵ  �������ַ���ָ��														   				
**************************************************************/
static char *s_disp_get_swt_name_by_index(U8_T u8_index)
{
	if (m_e_lang == CHINESE)
		return (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[u8_index].s_ch_name);
	else
		return (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[u8_index].s_en_name);
}

/*************************************************************
��������: s_disp_get_product_type_name		           				
��������: �õ���Ʒ���ͺ�����					
�������: ��       		   				
�������: ��
����ֵ  �������ַ���ָ��														   				
**************************************************************/
static char *s_disp_get_product_type_name(void)
{
	if (g_u8_product_type == PRODUCT_TYPE_SC12)
	{
		if (m_e_lang == CHINESE)
			return (g_s_string[443][0]);
		else
			return (g_s_string[443][1]);	
	}
		
	else if (g_u8_product_type == PRODUCT_TYPE_SC22)
	{
		if (m_e_lang == CHINESE)
			return (g_s_string[444][0]);
		else
			return (g_s_string[444][1]);
	}
		
	else
	{
		if (m_e_lang == CHINESE)
			return (g_s_string[445][0]);
		else
			return (g_s_string[445][1]);
	}
}

/*************************************************************
��������: s32_disp_judge_record		           				
��������: ͨ���¼�ID�ж��¼�ID�Ƿ���ȷ						
�������: u16_id -- �¼�ID        		   				
�������: ��
����ֵ  ��0����ȷ��-1������ȷ														   				
**************************************************************/
static S32_T s32_disp_judge_record(LOG_DATA_T *pt_record)
{
	U16_T u16_group = ((pt_record->u16_log_id & LOG_ID_GROUP_MASK) >> LOG_ID_GROUP_OFFSET);
	U16_T u16_index = (pt_record->u16_log_id & LOG_ID_INDEX_MASK);

	if (u16_group == LOG_ID_BATT_GROUP)
	{
		if (u16_index < LOG_ID_BATT_NUM * 2)
			return 0;
	}

	return -1;
}

/*************************************************************
��������: s32_disp_judge_no_resume_fault		           				
��������: �жϵ���ĩ��������Ƿ���ȷ						
�������: t_fault -- ָ��Ҫ�жϵĵ���ĩ�������        		   				
�������: ��
����ֵ  ��0--����������-1--�����쳣														   				
**************************************************************/
static S32_T s32_disp_judge_no_resume_fault(const CURR_FAULT_DATA_T *pt_fault)
{
	U16_T u16_group = ((pt_fault->u16_id & FAULT_ID_GROUP_MASK) >> FAULT_GROUP_OFFSET);
	U16_T u16_index = (pt_fault->u16_id & FAULT_ID_INDEX_MASK);
	
	switch (u16_group)
	{
		case FAULT_AC_GROUP:                  //����
			if (u16_index >= FAULT_AC_NUM)
				return -1;
			break;

		case FAULT_DC_BUS_GROUP:              //ֱ��ĸ��
			if (u16_index >= FAULT_DC_NUM)
				return -1;
			break;

		case FAULT_BATT1_GROUP:               //1�����쳣
			if (u16_index >= FAULT_BATT1_NUM)
				return -1;
			break;
			
		case FAULT_BATT2_GROUP:               //2�����쳣
			if (u16_index >= FAULT_BATT2_NUM)
				return -1;
			break;

		case FAULT_BMS_GROUP:                 //���Ѳ�����
			if (u16_index >= FAULT_BMS_NUM)
				return -1;
			break;

		case FAULT_RECT_GROUP:                //����ģ�����
			if (u16_index >= FAULT_RECT_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_GROUP:                //ͨ��ģ�����
			if (u16_index >= FAULT_DCDC_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_GROUP:                //���ģ�����
			if (u16_index >= FAULT_DCAC_NUM)
				return -1;
			break;
			
		case FAULT_FC_GROUP:                  //����ģ�����
			if (u16_index >= FAULT_FC_COMM_FIAL_NUM)
				return -1;
			break;

		case FAULT_RC_GROUP:                  //RC10ģ�����
			if (u16_index >= FAULT_RC_COMM_FIAL_NUM)
				return -1;
			break;
			
		case FAULT_SWT_GROUP:                 //�Դ�����������
			if (u16_index >= FAULT_SWT_NUM)
				return -1;
			break;

		case FAULT_DC_PANEL1_GROUP:           //�ֵ���01����
		case FAULT_DC_PANEL2_GROUP:           //�ֵ���02����
		case FAULT_DC_PANEL3_GROUP:           //�ֵ���03����
		case FAULT_DC_PANEL4_GROUP:           //�ֵ���04����
		case FAULT_DC_PANEL5_GROUP:           //�ֵ���05����
		case FAULT_DC_PANEL6_GROUP:           //�ֵ���06����
		case FAULT_DC_PANEL7_GROUP:           //�ֵ���07����
		case FAULT_DC_PANEL8_GROUP:           //�ֵ���08����
			if (u16_index >= FAULT_DC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_PANEL_GROUP:          //ͨ��������
			if (u16_index >= FAULT_DCDC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_PANEL_GROUP:          //UPS������
			if (u16_index >= FAULT_DCAC_PANEL_NUM)
				return -1;
			break;

		case FAULT_AC_PANEL_GROUP:          //����������
			if (u16_index >= FAULT_AC_PANEL_NUM)
				return -1;
			break;
						
		default:
			return -1;
	}

	if ((pt_fault->u8_occur_year > 99) || (pt_fault->u8_occur_mon < 1) || (pt_fault->u8_occur_mon > 12)
		|| (pt_fault->u8_occur_day < 1) || (pt_fault->u8_occur_day > 31) || (pt_fault->u8_occur_hour > 23)
		|| (pt_fault->u8_occur_min > 59) && (pt_fault->u8_occur_sec > 59))
	{
		return -1;
	}

	return 0;
}


/*************************************************************
��������: s32_disp_judge_his_fault		           				
��������: �ж���ʷ�����Ƿ���ȷ						
�������: t_fault -- ָ��Ҫ�жϵ���ʷ����        		   				
�������: ��
����ֵ  ��0--����������-1--�����쳣														   				
**************************************************************/
static S32_T s32_disp_judge_his_fault(const HIS_FAULT_DATA_T *pt_fault)
{
	U16_T u16_group = ((pt_fault->u16_id & FAULT_ID_GROUP_MASK) >> FAULT_GROUP_OFFSET);
	U16_T u16_index = (pt_fault->u16_id & FAULT_ID_INDEX_MASK);
	
	switch (u16_group)
	{
		case FAULT_AC_GROUP:                  //����
			if (u16_index >= FAULT_AC_NUM)
				return -1;
			break;

		case FAULT_DC_BUS_GROUP:              //ֱ��ĸ��
			if (u16_index >= FAULT_DC_NUM)
				return -1;
			break;

		case FAULT_BATT1_GROUP:               //1�����쳣
			if (u16_index >= FAULT_BATT1_NUM)
				return -1;
			break;
			
		case FAULT_BATT2_GROUP:               //2�����쳣
			if (u16_index >= FAULT_BATT2_NUM)
				return -1;
			break;

		case FAULT_BMS_GROUP:                 //���Ѳ�����
			if (u16_index >= FAULT_BMS_NUM)
				return -1;
			break;

		case FAULT_RECT_GROUP:                //����ģ�����
			if (u16_index >= FAULT_RECT_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_GROUP:                //ͨ��ģ�����
			if (u16_index >= FAULT_DCDC_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_GROUP:                //���ģ�����
			if (u16_index >= FAULT_DCAC_NUM)
				return -1;
			break;
			
		case FAULT_FC_GROUP:                  //����ģ�����
			if (u16_index >= FAULT_FC_COMM_FIAL_NUM)
				return -1;
			break;

		case FAULT_RC_GROUP:                  //RC10ģ�����
			if (u16_index >= FAULT_RC_COMM_FIAL_NUM)
				return -1;
			break;
			
		case FAULT_SWT_GROUP:                 //�Դ�����������
			if (u16_index >= FAULT_SWT_NUM)
				return -1;
			break;

		case FAULT_DC_PANEL1_GROUP:           //�ֵ���01����
		case FAULT_DC_PANEL2_GROUP:           //�ֵ���02����
		case FAULT_DC_PANEL3_GROUP:           //�ֵ���03����
		case FAULT_DC_PANEL4_GROUP:           //�ֵ���04����
		case FAULT_DC_PANEL5_GROUP:           //�ֵ���05����
		case FAULT_DC_PANEL6_GROUP:           //�ֵ���06����
		case FAULT_DC_PANEL7_GROUP:           //�ֵ���07����
		case FAULT_DC_PANEL8_GROUP:           //�ֵ���08����
			if (u16_index >= FAULT_DC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_PANEL_GROUP:          //ͨ��������
			if (u16_index >= FAULT_DCDC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_PANEL_GROUP:          //UPS������
			if (u16_index >= FAULT_DCAC_PANEL_NUM)
				return -1;
			break;

		case FAULT_AC_PANEL_GROUP:          //����������
			if (u16_index >= FAULT_AC_PANEL_NUM)
				return -1;
			break;
			
		default:
			return -1;
	}

	if ((pt_fault->u8_occur_year > 99) || (pt_fault->u8_occur_mon < 1) || (pt_fault->u8_occur_mon > 12)
		|| (pt_fault->u8_occur_day < 1) || (pt_fault->u8_occur_day > 31) || (pt_fault->u8_occur_hour > 23)
		|| (pt_fault->u8_occur_min > 59) || (pt_fault->u8_occur_sec > 59))
	{
		return -1;
	}

	if ((pt_fault->u8_resume_year > 99) || (pt_fault->u8_resume_mon < 1) || (pt_fault->u8_resume_mon > 12)
		|| (pt_fault->u8_resume_day < 1) || (pt_fault->u8_resume_day > 31) || (pt_fault->u8_resume_hour > 23)
		|| (pt_fault->u8_resume_min > 59) || (pt_fault->u8_resume_sec > 59))
	{
		return -1;
	}

	return 0;
}

/*************************************************************
��������: v_disp_his_falut_page		           				
��������: ��ʾ��ʷ����ҳ������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_mut_fetch_his_fault						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_his_falut_page(void)
{
	U16_T i, num;
	HIS_FAULT_DATA_T t_fault;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	char *s1, *s2;
	
	num = u16_fault_read_his_fault_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d", 
					s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);

	num -=m_t_win_record.u_ordinal.u16_his_fault;
	
	for (i=0; i<MMI_HIS_FAULT_CNT && i<num; i++)
	{
		v_fault_read_his_fault(&t_fault, m_t_win_record.u_ordinal.u16_his_fault+i);
	
		if (s32_disp_judge_his_fault(&t_fault) == 0)				
		{
			s1 = s_disp_get_fault_name_by_id(t_fault.u16_id);
			s2 = s_lcd_lcd_cut_string(s1, MMI_FAULT_NAME_WIDTH);
			 
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s1);
			if (s2 != NULL)
				snprintf((char *)(m_t_win_record.u8_special_name[i*4+2]), sizeof(m_t_win_record.u8_special_name[i*4+2]),
						"    %s", s2);
			else
				memset(m_t_win_record.u8_special_name[i*4+2], 0, sizeof(m_t_win_record.u8_special_name[i*4+2]));

			snprintf((char *)(m_t_win_record.u8_special_name[i*4+3]), sizeof(m_t_win_record.u8_special_name[i*4+3]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*4+3].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_fault.u8_occur_year, t_fault.u8_occur_mon, t_fault.u8_occur_day, t_fault.u8_occur_hour,
					t_fault.u8_occur_min, t_fault.u8_occur_sec);
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+4]), sizeof(m_t_win_record.u8_special_name[i*4+4]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*4+4].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_fault.u8_resume_year, t_fault.u8_resume_mon, t_fault.u8_resume_day, t_fault.u8_resume_hour,
					t_fault.u8_resume_min, t_fault.u8_resume_sec);
		}
		else
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s_disp_get_string_pointer(183));
			memset(m_t_win_record.u8_special_name[i*4+2], 0, sizeof(m_t_win_record.u8_special_name[i*4+2]));
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+3]), sizeof(m_t_win_record.u8_special_name[i*4+3]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*4+3].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+4]), sizeof(m_t_win_record.u8_special_name[i*4+4]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*4+4].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
		}
	}

	win->u8_item_cnt = i*4+1;
}

/*************************************************************
��������: v_disp_his_fault_first_page		           				
��������: ��ʾ��ʷ���ϵ�һҳ�����ô˺���ǰ��Ҫ��ȡ������g_mut_fetch_his_fault						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_his_fault_first_page(void)
{
	U16_T num;
	
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	
	v_fault_reset_his_fault_state();
	num  = u16_fault_read_his_fault_cnt();
	if (num > 0)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_INFO;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		if (num > MMI_HIS_FAULT_CNT)
			m_t_win_record.t_curr_win.u8_icon_cnt = 2;
		else
			m_t_win_record.t_curr_win.u8_icon_cnt = 0;

		v_disp_his_falut_page();
	}
	else
	{
		m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_HIS_FAULT; // ����ʷ���ϼ�¼
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
��������: v_disp_no_resume_fault_page		           				
��������: ����ĩ���������ʾ������������ʾ���ݴ�ӡ����������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_no_resume_fault_page(void)
{
	U16_T i, num;
	CURR_FAULT_DATA_T t_fault;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	char *s1, *s2;
	
	num = u16_fault_read_no_resume_fault_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d", 
					s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);

	num -= m_t_win_record.u_ordinal.u16_exception;
	
	for (i=0; (i<MMI_EXCEPTION_CNT && i<num); i++)
	{
		v_fault_read_no_resume_fault(&t_fault, m_t_win_record.u_ordinal.u16_exception+i);

		if (s32_disp_judge_no_resume_fault(&t_fault) == 0)
		{
			s1 = s_disp_get_fault_name_by_id(t_fault.u16_id);
			s2 = s_lcd_lcd_cut_string(s1, MMI_FAULT_NAME_WIDTH);
			 
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+1]), sizeof(m_t_win_record.u8_special_name[i*3+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s1);
			if (s2 != NULL)
				snprintf((char *)(m_t_win_record.u8_special_name[i*3+2]), sizeof(m_t_win_record.u8_special_name[i*3+2]),
						"    %s", s2);
			else
				memset(m_t_win_record.u8_special_name[i*3+2], 0, sizeof(m_t_win_record.u8_special_name[i*3+2]));

			snprintf((char *)(m_t_win_record.u8_special_name[i*3+3]), sizeof(m_t_win_record.u8_special_name[i*3+3]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*3+3].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_fault.u8_occur_year, t_fault.u8_occur_mon, t_fault.u8_occur_day, t_fault.u8_occur_hour,
					t_fault.u8_occur_min, t_fault.u8_occur_sec);
		}
		else
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+1]), sizeof(m_t_win_record.u8_special_name[i*3+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s_disp_get_string_pointer(183));
			memset(m_t_win_record.u8_special_name[i*3+2], 0, sizeof(m_t_win_record.u8_special_name[i*3+2]));
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+3]), sizeof(m_t_win_record.u8_special_name[i*3+3]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*3+3].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
		}
	}

	win->u8_item_cnt = i*3+1;
}

/*************************************************************
��������: v_disp_curr_falut_page		           				
��������: ��ʾ��ǰ����ҳ������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_mut_fault_data						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_curr_falut_page(void)
{
	U16_T i, num;
	CURR_FAULT_DATA_T t_fault;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	char *s1, *s2;
	
	num  = u16_fault_read_curr_fault_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d", 
				s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);
				
	num -= m_t_win_record.u_ordinal.u16_curr_fault;

	for (i=0; (i<MMI_CURR_FAULT_CNT && i<num); i++)
	{
		v_fault_read_curr_fault(&t_fault, m_t_win_record.u_ordinal.u16_curr_fault+i);
		
		s1 = s_disp_get_fault_name_by_id(t_fault.u16_id);
		s2 = s_lcd_lcd_cut_string(s1, MMI_FAULT_NAME_WIDTH);
		 
		snprintf((char *)(m_t_win_record.u8_special_name[i*3+1]), sizeof(m_t_win_record.u8_special_name[i*3+1]),
				"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s1);
		if (s2 != NULL)
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+2]), sizeof(m_t_win_record.u8_special_name[i*3+2]),
					"    %s", s2);
		else
			memset(m_t_win_record.u8_special_name[i*3+2], 0, sizeof(m_t_win_record.u8_special_name[i*3+2]));

		snprintf((char *)(m_t_win_record.u8_special_name[i*3+3]), sizeof(m_t_win_record.u8_special_name[i*3+3]),
				"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*3+3].u16_name_index & MMI_STR_ID_INDEX_MASK),
				t_fault.u8_occur_year, t_fault.u8_occur_mon, t_fault.u8_occur_day, t_fault.u8_occur_hour,
				t_fault.u8_occur_min, t_fault.u8_occur_sec);
	}

	win->u8_item_cnt = i*3+1;
}

/*************************************************************
��������: v_disp_curr_falut_first_page		           				
��������: ��ʾ��ǰ���ϵ�һҳ�����ô˺���ǰ��Ҫ��ȡ������g_mut_fault_data						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_curr_falut_first_page(void)
{
	U16_T num;
	
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	
	v_fault_reset_curr_fault_state();
	num  = u16_fault_read_curr_fault_cnt();
	if (num > 0)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CURR_FAULT_INFO;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		if (num > MMI_CURR_FAULT_CNT)
			m_t_win_record.t_curr_win.u8_icon_cnt = 2;
		else
			m_t_win_record.t_curr_win.u8_icon_cnt = 0;

		v_disp_curr_falut_page();
	}
	else
	{
		m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CURR_FAULT;   // �޵�ǰ����
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
��������: v_disp_record_page		           				
��������: �¼���¼��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_mut_log						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_record_page(void)
{
	U16_T i, num;
	LOG_DATA_T t_log_data;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = u16_log_read_log_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]),
				"%s%d", s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);
				
	num -= m_t_win_record.u_ordinal.u16_record;

	for (i=0; (i<MMI_RECORD_CNT && i<num); i++)
	{
		v_log_read_log_record(&t_log_data, m_t_win_record.u_ordinal.u16_record+i);

		if (s32_disp_judge_record(&t_log_data) == 0)
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+1]), sizeof(m_t_win_record.u8_special_name[i*2+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_record+i+1, s_disp_get_record_name_by_id(t_log_data.u16_log_id));
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+2]), sizeof(m_t_win_record.u8_special_name[i*2+2]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*2+2].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_log_data.u8_occur_year, t_log_data.u8_occur_mon, t_log_data.u8_occur_day, t_log_data.u8_occur_hour,
					t_log_data.u8_occur_min, t_log_data.u8_occur_sec);
		}
		else
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+1]), sizeof(m_t_win_record.u8_special_name[i*2+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_record+i+1, s_disp_get_string_pointer(183));
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+2]), sizeof(m_t_win_record.u8_special_name[i*2+2]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*2+2].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
		}
	}

	win->u8_item_cnt = i*2+1;
}

/*************************************************************
��������: v_disp_record_first_page		           				
��������: ��ʾ�¼���¼��һҳ�����ô˺���ǰ��Ҫ��ȡ������g_mut_log						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_record_first_page(void)
{
	U32_T num;
	
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	
	num = u16_log_read_log_cnt();
	if (num > 0)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		if (num > MMI_RECORD_CNT)
			m_t_win_record.t_curr_win.u8_icon_cnt = 2;
		else
			m_t_win_record.t_curr_win.u8_icon_cnt = 0;
			
		v_disp_record_page();
	}
	else
	{
		m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_RECORD;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
��������: v_disp_feeder_branch_info		           				
��������: ����֧·��Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_feeder_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T panel = m_u8_feeder_panel_ordianl;
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
								panel+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
	
	num = g_t_share_data.t_sys_cfg.t_feeder_panel[panel].u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*5+1]), sizeof(m_t_win_record.u8_special_name[i*5+1]), "%s", 
								s_disp_get_branch_name_by_id(panel, branch+i));
								
		win->t_item[i*5+2].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].u8_state);
		
		m_t_win_record.u32_set_value = 0;

		if (g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].u8_insu_state == 0)
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+3].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].f32_res);
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+4].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+4].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*5+5].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*5+1;
}

/*************************************************************
��������: v_disp_feeder_branch_info		           				
��������: ����֧·��Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_feeder_branch_info2(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T panel = m_u8_feeder_panel_ordianl;
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
								panel+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
	
	num = g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel].u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*5+1]), sizeof(m_t_win_record.u8_special_name[i*5+1]), "%s", 
								s_disp_get_branch_name_by_id(4+panel, branch+i));
								
		win->t_item[i*5+2].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].u8_state);
		
		m_t_win_record.u32_set_value = 0;

		if (g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].u8_insu_state == 0)
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+3].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].f32_res);
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+4].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+4].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*5+5].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*5+1;
}

/*************************************************************
��������: v_disp_rect_info		           				
��������: ����ģ����Ϣ��ʾ������������ʾ���ݴ�ӡ����������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_rect_info(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%02d%s", 
			m_t_win_record.u_ordinal.u16_rect_info+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));

	win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].f32_out_volt);
	win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].f32_out_curr);
	win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].f32_curr_percent);
	win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].b_ctl_mode);
	win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].e_module_state);
}

/*************************************************************
��������: v_disp_dcdc_module_info		           				
��������: ͨ��ģ����Ϣ��ʾ������������ʾ���ݴ�ӡ����������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dcdc_module_info(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%02d%s", 
			m_t_win_record.u_ordinal.u16_dcdc_module+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));

	win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].f32_out_volt);
	win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].f32_out_curr);
	win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].f32_curr_percent);
	win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].b_ctl_mode);
	win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].e_module_state);
}

/*************************************************************
��������: v_disp_dcac_module_info		           				
��������: UPSģ����Ϣ��ʾ������������ʾ���ݴ�ӡ����������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dcac_module_info(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%02d%s", 
			m_t_win_record.u_ordinal.u16_dcac_module+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));

	win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_volt);
	win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_curr);
	win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_freq);
	win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_inverter_volt);
	win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_bypass_input_volt);
	win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_active_power);
	win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_apparen_power);
	win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_power_factor);
	win->t_item[9].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].e_module_state);
	win->t_item[10].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].b_alarm_state);
}

/*************************************************************
��������: v_disp_dcdc_branch_info		           				
��������: ͨ����֧·��Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dcdc_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]), "%s", 
								s_disp_get_branch_name_by_id(8, branch+i));
								
		win->t_item[i*4+2].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[branch+i].u8_state);
		
		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			m_t_win_record.u32_set_value = 0;
			win->t_item[i*4+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*4+3].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*4+4].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*4+1;
}

/*************************************************************
��������: v_disp_dcac_branch_info		           				
��������: UPS��֧·��Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dcac_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]), "%s", 
								s_disp_get_branch_name_by_id(9, branch+i));
								
		win->t_item[i*4+2].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[branch+i].u8_state);
		
		if (g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			m_t_win_record.u32_set_value = 0;
			win->t_item[i*4+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*4+3].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*4+4].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*4+1;
}

/*************************************************************
��������: v_disp_ac_branch_info		           				
��������: ͨ����֧·��Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_ac_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	num = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]), "%s", 
								s_disp_get_branch_name_by_id(10, branch+i));
								
		win->t_item[i*4+2].pv_val = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[branch+i].u8_state);
		
		if (g_t_share_data.t_rt_data.t_ac_panel.t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			m_t_win_record.u32_set_value = 0;
			win->t_item[i*4+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*4+3].pv_val = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*4+4].pv_val = &(g_t_share_data.t_rt_data.t_ac_panel.u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*4+1;
}

/*************************************************************
��������: v_disp_cell1_info		           				
��������: һ�鵥������Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_cell1_info(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num - m_t_win_record.u_ordinal.u16_cell;

	for (i=0; (i<MMI_CELL_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%03d:", m_t_win_record.u_ordinal.u16_cell+i+1);
		win->t_item[i+1].pv_val = &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_cell_volt[m_t_win_record.u_ordinal.u16_cell+i]);
	}

	win->u8_item_cnt = i + 1;
}

/*************************************************************
��������: v_disp_cell2_info		           				
��������: ���鵥������Ϣ��ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_cell2_info(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num - m_t_win_record.u_ordinal.u16_cell;

	for (i=0; (i<MMI_CELL_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%03d:", m_t_win_record.u_ordinal.u16_cell+i+1);
		win->t_item[i+1].pv_val = &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_cell_volt[m_t_win_record.u_ordinal.u16_cell+i]);
	}

	win->u8_item_cnt = i + 1;
}

/*************************************************************
��������: v_disp_special_swt_info		           				
��������: ���ݿ����������ã�����ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_special_swt_info(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt - m_t_win_record.u_ordinal.u16_swt;

	for (i=0; (i<MMI_SWT_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%s",
						s_disp_get_swt_name_by_index(i+m_t_win_record.u_ordinal.u16_swt));
		win->t_item[i+1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[i+m_t_win_record.u_ordinal.u16_swt]);
	}

	win->u8_item_cnt = i+1;
	
	if (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt > MMI_SWT_CNT)
		win->u8_icon_cnt = 2;
	else
		win->u8_icon_cnt = 0;
}

/*************************************************************
��������: v_disp_ec_swt_info		           				
��������: ���ݵ�ٿ����������ã�����ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_ec_swt_info(void)
{
	U16_T num, i, index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = u16_public_get_ctrl_swt_num() - m_t_win_record.u_ordinal.u16_swt;
	index = m_u16_swt_index;

	for (i=0; (i<MMI_SWT_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%s",
					s_disp_get_swt_string_pointer(index));
		win->t_item[i+1].pv_val = (g_t_swt_sheet[index].p_u8_swt_state);

		index++;
		while ( (!g_t_swt_sheet[index].u8_swt_valid) && 
		        (index < FACT_SWT_CTRL_MAX) )
		{
			index++;
		}
	}

	win->u8_item_cnt = i+1;
	
	if (u16_public_get_ctrl_swt_num() > MMI_SWT_CNT)
		win->u8_icon_cnt = 2;
	else
		win->u8_icon_cnt = 0;
}

/*************************************************************
��������: v_disp_rect_on_off		           				
��������: ����ģ�鿪�ػ���ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_rect_on_off(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num - m_t_win_record.u_ordinal.u16_rect_on_off;

	for (i=0; (i<MMI_RECT_ON_OFF_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%02d%s",
					m_t_win_record.u_ordinal.u16_rect_on_off+i+1,
					s_disp_get_string_pointer(win->t_item[i+1].u16_name_index & MMI_STR_ID_INDEX_MASK));
		win->t_item[i+1].pv_val = &(g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl[m_t_win_record.u_ordinal.u16_rect_on_off+i]);
	}

	win->u8_item_cnt = i+1;
}

/*************************************************************
��������: v_disp_swt_on_off		           				
��������: ��ٿ�����ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_swt_on_off(void)
{
	U16_T num, i, index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = u16_public_get_ctrl_swt_num() - m_t_win_record.u_ordinal.u16_swt_on_off;
	index = m_u16_swt_index;

	for (i=0; (i<MMI_SWT_ON_OFF_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%s",
					s_disp_get_swt_string_pointer(index));
		win->t_item[i+1].pv_val = (g_t_swt_sheet[index].p_u8_swt_ctrl);

		index++;
		while ( (!g_t_swt_sheet[index].u8_swt_valid) && 
		        (index < FACT_SWT_CTRL_MAX) )
		{
			index++;
		}
	}

	win->u8_item_cnt = i+1;
}

/*************************************************************
��������: v_disp_feeder_panel_module_num_set		           				
��������: ����������ģ������������ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_feeder_panel_module_num_set(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
				m_u8_feeder_panel_ordianl+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
			
	win->t_item[1].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num);
	
	if (g_u8_product_type == PRODUCT_TYPE_SC22)
		m_t_win_record.t_curr_win.t_item[1].u32_val_max = 1;
}

/*************************************************************
��������: v_disp_feeder_panel_module_num_set2		           				
��������: ��������������ģ������������ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_feeder_panel_module_num_set2(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
				m_u8_feeder_panel_ordianl+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
			
	win->t_item[1].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num);
	
	if (g_u8_product_type == PRODUCT_TYPE_SC22)
		m_t_win_record.t_curr_win.t_item[1].u32_val_max = 1;
}

/*************************************************************
��������: v_disp_acfeeder_panel_module_num_set		           				
��������: ����������ģ������������ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_acfeeder_panel_module_num_set(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s", 
				s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
			
	win->t_item[1].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num);
	
	if (g_u8_product_type == PRODUCT_TYPE_SC22)
		m_t_win_record.t_curr_win.t_item[1].u32_val_max = 1;
}

/*************************************************************
��������: v_disp_feeder_panel_module_set		           				
��������: ����������ģ��������ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_feeder_panel_module_set(void)
{
	U16_T panel_index, module_index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	panel_index = m_u8_feeder_panel_ordianl;
	module_index = m_t_win_record.u_ordinal.u16_feeder_module;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s%d%s", 
					panel_index+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
					module_index+1, s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
	snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
					s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK),
					MMI_FEEDER_PANEL_MODULE_ADDR+ panel_index * 4 + module_index);
			
	win->t_item[2].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_alarm_feeder_num);
	win->t_item[3].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].e_alarm_type);
	win->t_item[4].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_state_feeder_num);
	win->t_item[5].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].e_state_type);
	win->t_item[6].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_insu_feeder_num);
	win->t_item[7].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_curr_feeder_num);
}

/*************************************************************
��������: v_disp_feeder_panel_module_set2		           				
��������: ����������ģ��������ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_feeder_panel_module_set2(void)
{
	U16_T panel_index, module_index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	panel_index = m_u8_feeder_panel_ordianl;
	module_index = m_t_win_record.u_ordinal.u16_feeder_module;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s%d%s", 
					panel_index+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
					module_index+1, s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
	snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
					s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK),
					MMI_FEEDER_PANEL_MODULE_ADDR+ (4+panel_index) * 4 + module_index);
			
	win->t_item[2].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_alarm_feeder_num);
	win->t_item[3].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].e_alarm_type);
	win->t_item[4].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_state_feeder_num);
	win->t_item[5].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].e_state_type);
	win->t_item[6].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_insu_feeder_num);
	win->t_item[7].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_curr_feeder_num);
}

/*************************************************************
��������: v_disp_acfeeder_panel_module_set		           				
��������: ����������ģ��������ʾ������������ʾ���ݴ�ӡ���������У����ô˺���ǰ��Ҫ��ȡ������g_t_share_data						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_acfeeder_panel_module_set(void)
{
	U16_T  module_index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	module_index = m_t_win_record.u_ordinal.u16_feeder_module;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d%s", 
					s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
					module_index+1, s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
	snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
					s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK),
					MMI_AC_FEEDER_MODULE_ADDR + module_index);
			
	win->t_item[2].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].u8_alarm_feeder_num);
	win->t_item[3].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].e_alarm_type);
	win->t_item[4].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].u8_state_feeder_num);
	win->t_item[5].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].e_state_type);
	win->t_item[6].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].u8_curr_feeder_num);
}

/*************************************************************
��������: v_disp_display_update		           				
��������: ��ʾҳ����£����ݵ�ǰ��ҳ��IDˢ��ҳ��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_display_update(void)
{
	MMI_ITEM_T *t_item;
	STR_T str;
	char buf[25];
	U32_T i, j;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);

	if ( (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW) ||
	     (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2) )            // ������ˢ��
	{
		if (u8_ctime_get_time(&(m_t_win_record.t_time)) == TIME_IRIGB)	      // ˢ��������ʱ��
			win->t_icon[0].u16_index = ICON_SIGNAL;
		else
			win->t_icon[0].u16_index = ICON_CLOCK;
		snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%4d-%d-%d %d:%02d:%02d",
					m_t_win_record.t_time.year, m_t_win_record.t_time.month, m_t_win_record.t_time.day,
					m_t_win_record.t_time.hour, m_t_win_record.t_time.min, m_t_win_record.t_time.sec);
		
		//�����Ƿ��кϿ�ĸ����ʾ��ͬ�ĵ�ѹ����
		if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
		{
			if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW)
			{
				win->t_item[8].u16_name_index = 5;
				win->u8_item_cnt = 10;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt);
			}
			else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2)
			{
				win->t_item[8].u16_name_index = 579;
				win->u8_item_cnt = 10;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_volt);
			}	
		}
		else
		{
			if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW)
			{
				win->t_item[8].u16_name_index = 6;
				win->u8_item_cnt = 9;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt);
			}
			else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2)
			{
				win->t_item[8].u16_name_index = 580;
				win->u8_item_cnt = 9;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_volt);
			}
		}
		
		
		//���ݵ�ص�ǰ״̬�����õ����Ŀ��ID
		if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
		{
			if (g_t_share_data.t_rt_data.t_batt.e_state[0] == DIS)
				win->t_item[6].u16_name_index = 10; 
			else if ((g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] < -1.0)
					|| (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] < -1.0))
				win->t_item[6].u16_name_index = 11;				
			else if (g_t_share_data.t_rt_data.t_batt.e_state[0] == EQU)
				win->t_item[6].u16_name_index = 9;	
			else
				win->t_item[6].u16_name_index = 8;
		}
		else
		{
			if (g_t_share_data.t_rt_data.t_batt.e_state[0] == DIS)
				win->t_item[6].u16_name_index = 440; 
			else if ((g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] < -1.0)
					|| (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] < -1.0))
				win->t_item[6].u16_name_index = 441;				
			else if (g_t_share_data.t_rt_data.t_batt.e_state[0] == EQU)
				win->t_item[6].u16_name_index = 13;	
			else
				win->t_item[6].u16_name_index = 12;
		}

		//��ѯ��ǰ���ϣ����ݹ�����Ŀ��ID
		switch (u8_fault_get_fault_class())
		{
			case AC_FAULT_CLASS:                      //��������
				win->t_item[7].u16_name_index = 15;
				break;

			case DC_FAULT_CLASS:                      //ֱ��ĸ�߹���
				win->t_item[7].u16_name_index = 16;
				break;

			case BATT_FAULT_CLASS:                    //��ع���
				win->t_item[7].u16_name_index = 17;
				break;

			case INSU_FAULT_CLASS:                    //��Ե����
				win->t_item[7].u16_name_index = 18;
				break;

			case RECT_FAULT_CLASS:                    //���ģ�����
				win->t_item[7].u16_name_index = 19;
				break;

			case FUSE_FAULT_CLASS:                    //�۶�������
				win->t_item[7].u16_name_index = 20;
				break;

			case DCAC_FAULT_CLASS:                    //UPSģ�����
				win->t_item[7].u16_name_index = 21;
				break;

			case FEEDER_FAULT_CLASS:                  //����֧·����
				win->t_item[7].u16_name_index = 22;
				break;
				
			case SYSTEM_NORMAL:                       //ϵͳ���� 
			default:
				win->t_item[7].u16_name_index = 14;
				break;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TOTAL_INFO)
	{
		//���ݵ�ص�ǰ״̬���������һ����Ŀ��ID
		if (g_t_share_data.t_rt_data.t_batt.e_state[0] == DIS)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 99; 			
		else if (g_t_share_data.t_rt_data.t_batt.e_state[0] == EQU)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 98;	
		else
			win->t_item[win->u8_item_cnt-1].u16_name_index = 97;
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_GROUP_INFO)
	{
		//���ݵ�ص�ǰ״̬���������һ����Ŀ��ID
		if (g_t_share_data.t_rt_data.t_batt.e_state[1] == DIS)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 99; 			
		else if (g_t_share_data.t_rt_data.t_batt.e_state[1] == EQU)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 98;	
		else
			win->t_item[win->u8_item_cnt-1].u16_name_index = 97;
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP1_INFO)
	{
		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num; i++)  // ������еĵ��Ѳ��ͨ���жϣ�����
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_comm_state[i] == 0)
			{
				break;
			}
		}

		if (i < g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num)
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP2_INFO)
	{
		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num; i++)  // ������еĵ��Ѳ��ͨ���жϣ�����
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_comm_state[i] == 0)
			{
				break;
			}
		}

		if (i < g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num)
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS1_CELL_INFO)                // �����ص�ѹ����ˢ��
	{
		U32_T ordianl, end_num, start_num = 0;

		for (j=0; j<win->u8_item_cnt-1; j++)                                          //��������ʾ���е���Ŀ
		{
			m_t_win_record.u8_item_inverse[j+1] = MMI_NORMAL_DISP;
		}

		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num; i++)  // ������Ѳ��ͨ���жϣ����������ĵ����ص�ѹ����
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_comm_state[i] != 0)
			{
				end_num = start_num + g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[i];

				for (j=0; j<win->u8_item_cnt-1; j++)
				{
					ordianl = m_t_win_record.u_ordinal.u16_cell + j;
					if ((ordianl >= start_num) && (ordianl < end_num))
					{
						m_t_win_record.u8_item_inverse[j+1] = MMI_INVERSE_DISP;
					}
				}
			}

			start_num += g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[i];
		}

		for (i=0; i<win->u8_item_cnt-1; i++)                                   // ��������ѹ�쳣�����Զ�Ӧ����Ŀ
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_cell_state[m_t_win_record.u_ordinal.u16_cell+i] != 0)
			{
				m_t_win_record.u8_item_inverse[i+1] = MMI_INVERSE_DISP;
			}
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS2_CELL_INFO)                // �����ص�ѹ����ˢ��
	{
		U32_T ordianl, end_num, start_num = 0;

		for (j=0; j<win->u8_item_cnt-1; j++)                                          //��������ʾ���е���Ŀ
		{
			m_t_win_record.u8_item_inverse[j+1] = MMI_NORMAL_DISP;
		}

		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num; i++)  // ������Ѳ��ͨ���жϣ����������ĵ����ص�ѹ����
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_comm_state[i] != 0)
			{
				end_num = start_num + g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i];

				for (j=0; j<win->u8_item_cnt-1; j++)
				{
					ordianl = m_t_win_record.u_ordinal.u16_cell + j;
					if ((ordianl >= start_num) && (ordianl < end_num))
					{
						m_t_win_record.u8_item_inverse[j+1] = MMI_INVERSE_DISP;
					}
				}
			}

			start_num += g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i];
		}

		for (i=0; i<win->u8_item_cnt-1; i++)                                   // ��������ѹ�쳣�����Զ�Ӧ����Ŀ
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_cell_state[m_t_win_record.u_ordinal.u16_cell+i] != 0)
			{
				m_t_win_record.u8_item_inverse[i+1] = MMI_INVERSE_DISP;
			}
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_INFO)
	{
		if (g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].u8_comm_state != 0)    // ����ģ��ͨ���жϣ�����ģ����ص�ʵʱ����
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO)          // ����֧·��ѯҳ�棬�������ģ��ͨ���жϣ�������ص�����
	{
		DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl]);
		
		v_disp_feeder_branch_info();

		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
		
		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch+1 >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch+1 < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO2)          // ����֧·��ѯҳ�棬�������ģ��ͨ���жϣ�������ص�����
	{
		DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl]);
		
		v_disp_feeder_branch_info2();

		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[4+m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
		
		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch+1 >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch+1 < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[4+m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_INFO)
	{
		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].u8_comm_state != 0)    // DCDCģ��ͨ���жϣ�����ģ����ص�ʵʱ����
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_INFO)
	{
		v_disp_dcdc_branch_info();

		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[0].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_INFO)
	{
		if (g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].u8_comm_state != 0)    // DCDCģ��ͨ���жϣ�����ģ����ص�ʵʱ����
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_INFO)
	{
		v_disp_dcac_branch_info();

		if (g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[0].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_INFO)
	{
		AC_FEEDER_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed);

		v_disp_ac_branch_info();

		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
		}
		
		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch+1 >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch+1 < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
		}
	}
	else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_CURR_FAULT_INFO)
			|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_NO_CURR_FAULT))
	{
		os_mut_release(g_mut_share_data);            //����������ͬʱ�������������������
		os_mut_wait(g_mut_fault_data, 0xFFFF);       //��д��ǰ������Ҫ���л�����
		if (u8_fault_read_curr_fault_state() == FAULT_CHANGE)
		{
			v_disp_curr_falut_first_page();
			win = &(m_t_win_record.t_curr_win);
		}
		os_mut_release(g_mut_fault_data);            //������ǰ���ϻ�����
		os_mut_wait(g_mut_share_data, 0xFFFF);       //��������������������
	}
	else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_HIS_FAULT_INFO)
			|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_NO_HIS_FAULT))
	{
		os_mut_release(g_mut_share_data);            //����������ͬʱ�������������������
		os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);  //��д��ʷ������Ҫ���л�����
		if (u8_fault_read_his_fault_state() == FAULT_CHANGE)
		{
			v_disp_his_fault_first_page();
			win = &(m_t_win_record.t_curr_win);
		}
		os_mut_release(g_mut_fetch_his_fault);       //������ʷ���ϻ�����
		os_mut_wait(g_mut_share_data, 0xFFFF);       //��������������������
	}
	else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RECORD)
			|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_NO_RECORD))
	{
		os_mut_release(g_mut_share_data);            //����������ͬʱ�������������������
		os_mut_wait(g_mut_log, 0xFFFF);              //��д�¼���¼��Ҫ���л�����
		if (u8_log_read_log_state() == LOG_CHANGE)
		{
			v_disp_record_first_page();
			win = &(m_t_win_record.t_curr_win);
		}
		os_mut_release(g_mut_log);                   //�ͷŻ�����
		os_mut_wait(g_mut_share_data, 0xFFFF);       //��������������������
	}

	v_lcd_lcd_clear();
	v_lcd_lcd_draw_rectangle(0, 0, 160, 160, 1);                           // ����Ļ�ܵľ���
	if( (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW) ||
	    (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2) )              // ����������ڣ���໭4������
	{
		v_lcd_lcd_draw_hline(0, 15, 160, 1);
		v_lcd_lcd_draw_hline(0, 17, 160, 1);
		v_lcd_lcd_draw_hline(0, 128, 160, 1);
		v_lcd_lcd_draw_hline(0, 130, 160, 1);
	}

	for (i=0; i<win->u8_icon_cnt; i++)                                     // ��ͼ��
	{
		v_lcd_lcd_draw_icon(win->t_icon[i].u8_x, win->t_icon[i].u8_y, win->t_icon[i].u16_index);
	}

	for (i=0; i<win->u8_item_cnt; i++)                                     // ѭ����������Ŀ
	{
		t_item = &(win->t_item[i]);

		memset(buf, 0, sizeof(buf));
		if ((t_item->u16_name_index & MMI_STR_ID_TYPE_MASK) == MMI_STR_ID_NORMAL_NAME)
		{
			str = s_disp_get_string_pointer(t_item->u16_name_index);                   // ��ͨ�ַ�����ͨ��g_s_string��ȡ�ַ���������
		}
		else
		{
			str = (char *)(m_t_win_record.u8_special_name[i]);             // �����ַ�����ͨ��m_t_win_record.u8_special_name��ȡ����
		}

		snprintf(buf, sizeof(buf), "%s", str);       //Ϊ��ֹ����Խ�磬��memcpy��Ϊsnprintf
		//memcpy(buf, str, strlen(str));

		if ((t_item->u8_val_type==MMI_VAL_TYPE_NONE) &&                             // �������Ƶ�ǰ������Ŀû����ֵ
			(((m_t_win_record.u8_sel_index==i) && (m_t_win_record.u8_sel_index!=0)) // ѡ����Ŀ������0����Ҫ��Ӧ�ڲ˵�����
				|| (m_t_win_record.u8_item_inverse[i]==MMI_INVERSE_DISP)))          // ��Ŀ���ԣ���Ҫ���ڱ��⡢�쳣��Ŀ�ķ���
		{
			v_lcd_lcd_draw_string(t_item->u8_name_x, t_item->u8_name_y, buf, 0, strlen(buf));
		}
		else
		{
			v_lcd_lcd_draw_string(t_item->u8_name_x, t_item->u8_name_y, buf, 0, 0);
		}
			
		
		if (t_item->u8_val_type != MMI_VAL_TYPE_NONE)
		{
			memset(buf, 0, sizeof(buf));

			/* �������úͷ�����״̬����ֵ��ʾ�ֲ�ͬ���������Ҫԭ���ǲ���ֱ���ø������������ã����ֱ���ø������������ã�
			   ���ھ��ȷ�������⣬�ᵼ�����ݲ�׼ȷ�����Խ�����������1000��ת����������������ʾ��ʱ���ٳ���1000ת���ɸ����� */
			if (((m_t_win_record.u8_win_status==MMI_WIN_SET) && (i==m_t_win_record.u8_sel_index))
					|| ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD))
			{
				switch (t_item->u8_val_type)
				{
					case MMI_VAL_TYPE_ENUM:
						str = s_disp_get_string_pointer(t_item->u16_val_index+(U8_T)(m_t_win_record.u32_set_value));
						memcpy(buf, str, strlen(str));
						break;
	
					case MMI_VAL_TYPE_U8:
						snprintf(buf, sizeof(buf), "%d", (U8_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U8_2BIT:
						snprintf(buf, sizeof(buf), "%02d", (U8_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U8_3BIT:
						snprintf(buf, sizeof(buf), "%03d", m_t_win_record.u32_set_value); //������ʾ3λ��ʱ����ܳ���255�����Բ���ǿ��ת����U8_T
						break;


					case MMI_VAL_TYPE_U16:
						snprintf(buf, sizeof(buf), "%d", (U16_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U16_2BIT:
						snprintf(buf, sizeof(buf), "%02d", (U16_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U16_3BIT:
						snprintf(buf, sizeof(buf), "%03d", (U16_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U16_4BIT:
						snprintf(buf, sizeof(buf), "%04d", (U16_T)(m_t_win_record.u32_set_value));
						break;

				

					case MMI_VAL_TYPE_U32:
						snprintf(buf, sizeof(buf), "%d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_2BIT:
						snprintf(buf, sizeof(buf), "%02d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_3BIT:
						snprintf(buf, sizeof(buf), "%03d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_4BIT:
						snprintf(buf, sizeof(buf), "%04d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_5BIT:
						snprintf(buf, sizeof(buf), "%05d", (U32_T)(m_t_win_record.u32_set_value));
						break;


					case MMI_VAL_TYPE_F32_1P:
						snprintf(buf, sizeof(buf), "%.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_3W1P:
						snprintf(buf, sizeof(buf), "%03.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_4W1P:
						snprintf(buf, sizeof(buf), "%04.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_5W1P:
						snprintf(buf, sizeof(buf), "%05.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_6W1P:
						snprintf(buf, sizeof(buf), "%06.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;


					case MMI_VAL_TYPE_F32_2P:
						snprintf(buf, sizeof(buf), "%.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_4W2P:
						snprintf(buf, sizeof(buf), "%04.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_5W2P:
						snprintf(buf, sizeof(buf), "%05.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_6W2P:
						snprintf(buf, sizeof(buf), "%06.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;


					case MMI_VAL_TYPE_F32_3P:
						snprintf(buf, sizeof(buf), "%.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_5W3P:
						snprintf(buf, sizeof(buf), "%05.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_6W3P:
						snprintf(buf, sizeof(buf), "%06.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_7W3P:
						snprintf(buf, sizeof(buf), "%07.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;

					default:
						break;
				}  // end of switch (t_item->u8_val_type)
			}
			else
			{
				switch (t_item->u8_val_type)
				{
					case MMI_VAL_TYPE_ENUM:
						str = s_disp_get_string_pointer(t_item->u16_val_index+*((U8_T *)(t_item->pv_val)));
						memcpy(buf, str, strlen(str));
						break;
	
					case MMI_VAL_TYPE_U8:
						snprintf(buf, sizeof(buf), "%d", *((U8_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U8_2BIT:
						snprintf(buf, sizeof(buf), "%02d", *((U8_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U8_3BIT:
						snprintf(buf, sizeof(buf), "%03d", *((U8_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_U16:
						snprintf(buf, sizeof(buf), "%d", *((U16_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U16_2BIT:
						snprintf(buf, sizeof(buf), "%02d", *((U16_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U16_3BIT:
						snprintf(buf, sizeof(buf), "%03d", *((U16_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U16_4BIT:
						snprintf(buf, sizeof(buf), "%04d", *((U16_T *)(t_item->pv_val)));
						break;

				

					case MMI_VAL_TYPE_U32:
						snprintf(buf, sizeof(buf), "%d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_2BIT:
						snprintf(buf, sizeof(buf), "%02d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_3BIT:
						snprintf(buf, sizeof(buf), "%03d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_4BIT:
						snprintf(buf, sizeof(buf), "%04d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_5BIT:
						snprintf(buf, sizeof(buf), "%05d", *((U32_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_F32_1P:
						snprintf(buf, sizeof(buf), "%.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_3W1P:
						snprintf(buf, sizeof(buf), "%03.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_4W1P:
						snprintf(buf, sizeof(buf), "%04.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_5W1P:
						snprintf(buf, sizeof(buf), "%05.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_6W1P:
						snprintf(buf, sizeof(buf), "%06.1f", *((F32_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_F32_2P:
						snprintf(buf, sizeof(buf), "%.2f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_4W2P:
						snprintf(buf, sizeof(buf), "%04.2f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_5W2P:
						snprintf(buf, sizeof(buf), "%05.2f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_6W2P:
						snprintf(buf, sizeof(buf), "%06.2f", *((F32_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_F32_3P:
						snprintf(buf, sizeof(buf), "%.3f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_5W3P:
						snprintf(buf, sizeof(buf), "%05.3f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_6W3P:
						snprintf(buf, sizeof(buf), "%06.3f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_7W3P:
						snprintf(buf, sizeof(buf), "%07.3f", *((F32_T *)(t_item->pv_val)));
						break;

					default:
						break;
				}  // end of switch (t_item->u8_val_type)
			}  /* enf of if (((m_t_win_record.u8_win_status==MMI_WIN_SET) && (i==m_t_win_record.u8_sel_index))
					          || ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD)) */

		
			if (((m_t_win_record.u8_win_status == MMI_WIN_SET) && (m_t_win_record.u8_sel_index==i)) //������Ŀ����˸��ʾ��ǰ���õ�����λ
					|| ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD))
			{
				if (m_t_win_record.u8_set_blink == MMI_INVERSE_DISP)   // ���Ե�ǰ���õ�����λ
				{
					if (t_item->u8_val_type == MMI_VAL_TYPE_ENUM)      // ö����������ȫ������
					{
						v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, strlen(buf));
					}
					else if ((t_item->u8_val_type & 0xF0) != 0x30)     // �������ݷ��Ե�ǰ���õ�����λ
					{
						v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, m_t_win_record.u8_bit_index, 1);
					}
					else
					{
						if (                                           // �������ݷ���ʱ����С����λ
								(
									(m_t_win_record.u8_bit_index >= 1)
									&&
									(
										(t_item->u8_val_type == MMI_VAL_TYPE_F32_3W1P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_4W2P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_5W3P)
									)
								)
								||
								(
									(m_t_win_record.u8_bit_index >= 2)
									&&
									(
										(t_item->u8_val_type == MMI_VAL_TYPE_F32_4W1P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_5W2P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_6W3P)
									)
								)
								||
								(
									(m_t_win_record.u8_bit_index >= 3)
									&&
									(
										(t_item->u8_val_type == MMI_VAL_TYPE_F32_5W1P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_6W2P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_7W3P)
									)
								)
								||
								(
									(m_t_win_record.u8_bit_index >= 4) && (t_item->u8_val_type == MMI_VAL_TYPE_F32_6W1P)
								)
							)
						{
							v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, m_t_win_record.u8_bit_index+1, 1);
						}
						else
						{
							v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, m_t_win_record.u8_bit_index, 1);
						}
					}
				}
				else
				{
					v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, 0);            // �����Ե�ǰ���õ�����λ
				}  // end of if (m_t_win_record.u8_set_blink == MMI_INVERSE_DISP)
			}
			else if (((m_t_win_record.u8_sel_index==i)&&(m_t_win_record.u8_sel_index!=0))
						|| (m_t_win_record.u8_item_inverse[i] == MMI_INVERSE_DISP))

			{
				v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, strlen(buf));
			}
			else
			{
				v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, 0);
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
	
	if (m_e_lcd_direction == LCD_HORIZONTAL)
		v_lcd_lcd_flush(LCD_ROTATE_0);
	else
		v_lcd_lcd_flush(LCD_ROTATE_90);
}

/*************************************************************
��������: v_disp_cancel_key_handler		           				
��������: CANCEL���������������ںܶ�ҳ��CANCEL��������һ�µģ������������װ��һ������						
�������: ��      		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_cancel_key_handler(void)
{
	U16_T id_back;
	U8_T index_back;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	id_back = win->u16_id_father;
	index_back = win->u8_sel_father;

	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	m_t_win_record.u16_curr_id = id_back;
	m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

	if ((id_back>>8) == MMI_WIN_TYPE_MENU)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u8_sel_index = index_back;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);		
		if (id_back == MMI_WIN_ID_SET_MENU)
			win->u8_item_cnt = m_u8_set_menu_item_cnt;
		else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
	    else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
		else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
		else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU2)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
		else if (id_back == MMI_WIN_ID_DC_RUN_INFO_MENU)        // ������Ϣ��ѯ�˵�
		{
			if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
			{
				win->u8_item_cnt = 8;
			}
		}
		os_mut_release(g_mut_share_data);
	}
	else if ((id_back>>8) == MMI_WIN_TYPE_MAIN_WIN)
	{
		m_t_win_record.u8_sel_index = index_back;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		if (id_back == MMI_WIN_ID_MAIN_WINDOW)
		{
			if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
			{
				win->u16_id_next = MMI_WIN_ID_NULL;
				win->u8_icon_cnt = 1;
			}
		}
		os_mut_release(g_mut_share_data);
	}
}


/*************************************************************
��������: v_disp_menu_key_handler		           				
��������: �˵�ҳ�水��������						
�������: u16_key_val -- ����ֵ       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_menu_key_handler(U16_T u16_key_val)
{
	U8_T index_back;
	U32_T i, num = 0;
	U8_T feeder_flag = 0;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (u16_key_val == g_u16_key_enter)                                              // "ENTER"������
	{
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_MENU)  // ���˵�
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:                                          //����ϵͳ��Ϣ��ѯ
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 1;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_AC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 1;
						}
					}
					break;

				case 2:                                          // ֱ��������Ϣ��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_RUN_INFO_MENU;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
					{
						m_t_win_record.t_curr_win.u8_item_cnt = 8;
					}
					break;
					
				case 3:                                          //ͨ��ϵͳ��Ϣ��ѯ
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 3;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 3;
						}
					}
					break;
					
				case 4:                                          //UPSϵͳ��Ϣ��ѯ
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 4;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 4;
						}
					}
					break;
					
				case 5:                                          // ��ǰ���ϲ�ѯ
					os_mut_release(g_mut_share_data);            // �ͷŹ���������������������ͬʱ�ӳ����������������
					os_mut_wait(g_mut_fault_data, 0xFFFF);       // ��д��ǰ������Ҫ���л�����
					v_disp_curr_falut_first_page();
					os_mut_release(g_mut_fault_data);            // �ͷŵ�ǰ���ϻ�����
					os_mut_wait(g_mut_share_data, 0xFFFF);       // �ӳֹ���������������
					break;


				case 6:                                          // ��ʷ���ϲ�ѯ
					os_mut_release(g_mut_share_data);            // �ͷŹ���������������������ͬʱ�ӳ����������������
					os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);  // ��д��ʷ������Ҫ���л�����
					v_disp_his_fault_first_page();
					os_mut_release(g_mut_fetch_his_fault);       // �ͷ���ʷ���ϻ�����
					os_mut_wait(g_mut_share_data, 0xFFFF);       // �ӳֹ���������������
					break;

				case 7:                                          // ����ĩ������ϲ�ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));

					num  = u16_fault_read_no_resume_fault_cnt();
					if (num > 0)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_EXCEPTION_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

						if (num > MMI_EXCEPTION_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
							
						v_disp_no_resume_fault_page();
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_EXCEPTION;   // �޵���ĩ�������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					}
					break;

				case 8:                                          //�¼���¼
					os_mut_release(g_mut_share_data);            //�ͷŹ���������������������ͬʱ�ӳ����������������
					os_mut_wait(g_mut_log, 0xFFFF);              //��д�¼���¼��Ҫ���л�����
					u8_log_read_log_state();
					v_disp_record_first_page();
					os_mut_release(g_mut_log);                   //�ͷŻ�����
					os_mut_wait(g_mut_share_data, 0xFFFF);       //�ӳֹ���������������
					break;


				case 9:                                         // ��������
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_INPUT_PASSWORD;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
					
				case 10:                                         // ά��ָ��
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_GUIDELINE;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					break;


				case 11:                                        // ����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ABOUT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);
					
					//��ȡ�����̡���Ӳ���汾�ŵ������Ϣ�����record.u8_special_name
					for (i=0; i<8; i++)
					{
						snprintf((char *)(m_t_win_record.u8_special_name[i]), sizeof(m_t_win_record.u8_special_name[i]),
							"%s", s_disp_get_about_name_by_index(i));
					}

					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-4]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-4]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-4].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_product_type_name());
					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-3]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-3]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-3].u16_name_index & MMI_STR_ID_INDEX_MASK), HW_VERSION);
					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-2]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-2]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-2].u16_name_index & MMI_STR_ID_INDEX_MASK), SW_VERSION);
					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-1]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-1]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-1].u16_name_index & MMI_STR_ID_INDEX_MASK), TOOL_VERSION);
					break;				

				default:
					break;
			}  //end of switch (m_t_win_record.u8_sel_index)
		}  //end of if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_RUN_INFO_MENU)        // ������Ϣ��ѯ�˵�
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:                         // ������Ϣ��ʾҳ��
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;

					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase == AC_3_PHASE) // ����
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_TRIPHASE_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							m_t_win_record.t_curr_win.u8_item_cnt = 5;
					}
					else                                                             // ����
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_UNIPHASE_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							m_t_win_record.t_curr_win.u8_item_cnt = 3;
					}
					break;

				case 2:                        // ���״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)	            // �����е���飬����ʾ��ز�ѯҳ��
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_TOTAL_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						win = &(m_t_win_record.t_curr_win);
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
						{
							if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
								win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
							else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
								win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
							else
								win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;

							win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
						}
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
						{
							if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
								win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
							else
								win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;

							if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
								win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
							else
								win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
						}
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;

				case 3:                        // ��Ե״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
					{
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num > 0)
						{
							feeder_flag = 1;
							break;
						}
					}
					
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_PB_CB_INSU_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
							
						win = &(m_t_win_record.t_curr_win);
						if (feeder_flag == 1)
						{
							win->u8_item_cnt = 8;     // ����������ģ�飬����ʾ��Ե�Եص�ѹ�ͶԵص���
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_pos_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt);
							win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);	
							win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
						else
						{
							win->u8_item_cnt = 8;//4;     //ĩ��������ģ�飬��ֻ��ʾ��Ե�Եص�ѹ
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_cb_pos_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt);
							//����Ϊ2014.10.28������ʾ
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);	
							win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_BUS_INSU_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
							
						win = &(m_t_win_record.t_curr_win);
						if (feeder_flag == 1)
						{
							win->u8_item_cnt = 7;     // ����������ģ�飬����ʾ��Ե�Եص�ѹ�ͶԵص���
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt);
							win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
						else
						{
							win->u8_item_cnt = 7;//3;     //ĩ��������ģ�飬��ֻ��ʾ��Ե�Եص�ѹ
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_cb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt);
							//����Ϊ2014.10.28������ʾ
							win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
					}
					break;

				case 4:                        // ����ģ����Ϣ
					num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_RECT_INFO;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);

					if (num > 1)
					{
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_RECT_INFO;               // ��ǰһҳ
						win->u16_id_next = MMI_WIN_ID_RECT_INFO;               // ����һҳ
					}
					else
					{
						win->u8_icon_cnt = 0;
						win->u16_id_prev = MMI_WIN_ID_NULL;                    // û��ǰһҳ
						win->u16_id_next = MMI_WIN_ID_NULL;                    // û����һҳ
					}
					
					v_disp_rect_info();
					break;
					
				case 5:                        // ����״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt > 0)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_SWITCH_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						v_disp_special_swt_info();
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 5;
					}
					break;
				
				case 6:                        // һ�����߹���Ϣ��ѯ
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 6;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 6;
						}
					}
					break;

				case 7:                        // ��ٿ���״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_u16_swt_index = 0;
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num > 0)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_ECSWT_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						v_disp_ec_swt_info();
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 7;
					}
					break;

				case 8:                        // ���ξ�Ե״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
					{
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num > 0)
						{
							feeder_flag = 1;
							break;
						}
					}
					
					if (feeder_flag == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_PB_CB_INSU_INFO2;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
								
							win = &(m_t_win_record.t_curr_win);
						
							win->u8_item_cnt = 8;     // ����������ģ�飬����ʾ��Ե�Եص�ѹ�ͶԵص���
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_pos_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_neg_to_gnd_volt);
							win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_to_gnd_ac_volt);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_pos_to_gnd_res);	
							win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_neg_to_gnd_res);
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_BUS_INSU_INFO;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
								
							win = &(m_t_win_record.t_curr_win);
							win->u8_item_cnt = 7;     // ����������ģ�飬����ʾ��Ե�Եص�ѹ�ͶԵص���
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt);
							win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 8;
					}
					break;

				case 9:                        // �������߹���Ϣ��ѯ
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 9;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_RUN_INFO_MENU2;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 9;
						}
					}
					break;

				default:
					break;
			}  //end of switch (m_t_win_record.u8_sel_index)
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SET_MENU)
		{	
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    // ����ϵͳ����
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 1;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_MODULE_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;

						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
						}
						else
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
						}
					}
					break;

				case 2:    // ֱ��ϵͳ����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_SET_MENU;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					break;

				case 3:    // ͨ��ģ���������
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					else
					{	
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_MODULE_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
						
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
						}
						else
						{	
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
						}
					}
					break;

				case 4:    // UPSģ���������
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 3;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_MODULE_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
						
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
						}
						else
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
						}
					}
					break;

				case 5:    // �澯�趨
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_RELAY_OUT_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 2;
					break;

				case 6:    // ��̨ͨѶ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_BACKSTAGE_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u8_item_cnt = 5;
					break;

				case 7:    // ʱ������
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_RTC_PASSWORD_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					u8_ctime_get_time(&m_t_win_record.t_time);
					m_t_win_record.t_time.year -= 2000;
					m_t_win_record.u8_sel_index = 1;
					break;
					
				case 8:    // �����ָ�
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_CLEAR;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;

				case 9:    // ����У׼
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_ADJUST;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						win->u8_item_cnt = 4;
						win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
					}

					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu*10))/10.0;
					break;
					
				case 10:
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DISPLAY;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 11:
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_MANUAL_LIMIT_CURR_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				default:
					break;
			}  //end of switch (m_t_win_record.u8_sel_index)
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SET_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
		{
			index_back = m_t_win_record.u8_sel_index;
			
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_branch_num > 0)
			{
				m_u8_feeder_panel_ordianl = index_back - 1;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_FEEDER_INFO;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
				
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				else
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
				v_disp_feeder_branch_info();	
			}
			else
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_FEEDER_RUN_INFO_MENU;
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
		{
			index_back = m_t_win_record.u8_sel_index;
			
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_branch_num > 0)
			{
				m_u8_feeder_panel_ordianl = index_back - 1;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_FEEDER_INFO2;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
				
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				else
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
				v_disp_feeder_branch_info2();	
			}
			else
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_FEEDER_RUN_INFO_MENU2;
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_RUN_INFO_MENU)
		{	
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    //DC/DCģ����Ϣ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_INFO;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

					if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 1)
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_DCDC_INFO;               // ��ǰһҳ
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_DCDC_INFO;               // ����һҳ
					}
					else
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 0;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // û��ǰһҳ
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // û����һҳ
					}
					
					v_disp_dcdc_module_info();
					break;
					
				case 2:    //ͨ�ſ���״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					
					if ((g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
						&& (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > 0))
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_FEEDER_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
						v_disp_dcdc_branch_info();	
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DCDC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_RUN_INFO_MENU)
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    //UPSģ����Ϣ��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_INFO;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

					if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 1)
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_DCAC_INFO;               // ��ǰһҳ
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_DCAC_INFO;               // ����һҳ
					}
					else
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 0;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // û��ǰһҳ
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // û����һҳ
					}
					
					v_disp_dcac_module_info();
					break;
					
				case 2:    //UPSϵͳ����״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					
					if ((g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
						&& (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > 0))
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_FEEDER_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
						v_disp_dcac_branch_info();	
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DCAC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_AC_RUN_INFO_MENU)
		{	
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    //����ϵͳĸ����Ϣ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_AC_RUN_INFO; 					
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];					
					break;
					
				case 2:    //��������״̬��ѯ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					
					if ((g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
						&& (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > 0))
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_FEEDER_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
						v_disp_ac_branch_info();	
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_ACS_AC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_AC_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SET_MENU)
		{
			U16_T u16_ctrl_swt_num;
			U8_T  u8_rc10_num;
				
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:     //ֱ��ϵͳ����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_SYSTEM_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u8_item_cnt = 5;
					else if (g_u8_product_type == PRODUCT_TYPE_SC22)
						win->t_item[5].u32_val_max = 1;

					if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
					{
						win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
						win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
					}
					else
					{
						win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
						win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
					}
					
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
						win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
						win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
					break;
					
				case 2:    //���߹�����
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_SET_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 2;
						}
					}
					break;
					
				case 3:   //����ֵ����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_THR_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					break;
					
				case 4:   //��س�ŵ��������
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_CHARGE_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					//�������ѹ������Ϊ��ع�ѹֵ������Ϊ���Ƿѹֵ
					win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
					win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
					win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					
					//��������������Ϊ��������
					win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
					break;
					
				case 5:   //ϵͳ����
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)     //�����е���飬��ʾϵͳ����ҳ��
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_SYSTEM_CTL_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;

						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						else
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
					}
					else            //ĩ���õ���飬��ʾģ�鿪�ػ�����ҳ��
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_RECT_ON_OFF;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;

						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num > MMI_RECT_ON_OFF_CNT)
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						}
						else
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
						}
						
						v_disp_rect_on_off();
					}
					break;

				case 6:   //��ٿ��ؿ���
					u16_ctrl_swt_num = u16_public_get_ctrl_swt_num();
					u8_rc10_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num;					
					if ((u16_ctrl_swt_num > 0) && (u8_rc10_num > 0))     //������RC10����ʾ��ٿ��ؿ���ҳ��
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_SWT_ON_OFF;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						m_u16_swt_index = 0;

						if (u16_ctrl_swt_num > MMI_SWT_ON_OFF_CNT)
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_SWT_ON_OFF;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_SWT_ON_OFF;
						}
						else
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
						}
						
						v_disp_swt_on_off();
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 6;
					}
					break;

				case 7:    //�������߹�����
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // ��ʾû������
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 7;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_SET_MENU2;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // ��ʾû������������
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 7;
						}
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SET_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU)
		{
			index_back = m_t_win_record.u8_sel_index;
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.t_curr_win.u8_sel_father = index_back;
			m_u8_feeder_panel_ordianl = index_back - 1;
			m_t_win_record.u8_sel_index = 1;

			if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_module_num > 0)
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ��ǰһҳ
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ����һҳ
			}
			else
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 0;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // ��ǰһҳ
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // ����һҳ
			}
			
			v_disp_feeder_panel_module_num_set();
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU2)
		{
			index_back = m_t_win_record.u8_sel_index;
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.t_curr_win.u8_sel_father = index_back;
			m_u8_feeder_panel_ordianl = index_back - 1;
			m_t_win_record.u8_sel_index = 1;

			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_module_num > 0)
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ��ǰһҳ
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ����һҳ
			}
			else
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 0;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // ��ǰһҳ
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // ����һҳ
			}
			
			v_disp_feeder_panel_module_num_set2();
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU2)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_GUIDELINE)
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:     //��ͨ
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_START;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 2:     //ͨѶ�쳣����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_COMM_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 3:     //��Ƿѹ����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_VOLT_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 4:     //��Ե����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_INSU_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 5:     //��̨ͨѶ�쳣
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_BACKSTAGE_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 6:     //��ϵ����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_CONTACT_VENDER;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 7:     //�����豸����
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_REPAIR;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				default:
					break;
			}			
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_GUIDELINE)
	}

	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u8_sel_index > 1)
		{
			m_t_win_record.u8_sel_index--;
		}
		else
		{
			m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
		}
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (m_t_win_record.u8_sel_index < win->u8_item_cnt-1)
		{
			m_t_win_record.u8_sel_index++;
		}
		else
		{
			m_t_win_record.u8_sel_index = 1;
		}
	}
	
	os_mut_release(g_mut_share_data);
}


/*************************************************************
��������: v_disp_run_info_key_handler		           				
��������: ������Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_run_info_key_handler(U16_T u16_key_val)
{
	U16_T id_back, id_old;
	U8_T index_back;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (u16_key_val == g_u16_key_cancel)
	{
		id_old = m_t_win_record.u16_curr_id;
		id_back = win->u16_id_father;
		index_back = win->u8_sel_father;

		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = id_back;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;

		if ((id_back>>8) == MMI_WIN_TYPE_MENU)                  //������ص��˵�ҳ�棬ѡ����Ӧ����Ŀ
		{
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_sel_index = index_back;
			
			if (id_back == MMI_WIN_ID_SET_MENU)
				win->u8_item_cnt = m_u8_set_menu_item_cnt;
			else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU2)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_DC_RUN_INFO_MENU)        // ������Ϣ��ѯ�˵�
			{
				if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
				{
					win->u8_item_cnt = 8;
				}
			}
		}
		else if ((id_back == MMI_WIN_ID_AC_ADJUST)
			|| (id_back == MMI_WIN_ID_DC_VOLT_ADJUST)
			|| (id_back == MMI_WIN_ID_LOAD_CURR_ADJUST)
			|| (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)
			|| (id_back == MMI_WIN_ID_BATT2_CURR_ADJUST))          //��ǰҳ����У׼�ɹ�ʧ����ʾҳ�棬����У׼ҳ��
		{
			if (id_back != MMI_WIN_ID_AC_ADJUST)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			}
			
			if (g_u8_product_type == PRODUCT_TYPE_SC12)
			{
				if (id_back == MMI_WIN_ID_AC_ADJUST)
				{
					win->u8_item_cnt = 4;
					win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
				}
				else if (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)
				{
					win->u16_id_next = MMI_WIN_ID_AC_ADJUST;
				}
			}
			
			m_t_win_record.u8_sel_index = index_back;
			if (id_old == MMI_WIN_ID_PARAM_OUT_RANGE)
			{
				m_t_win_record.u8_win_status = MMI_WIN_SET;
				m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*10)) * 100;
				m_t_win_record.pv_back_item_val = win->t_item[index_back].pv_val;     //��������ָ���ֵ
				win->t_item[index_back].pv_val = &(m_t_win_record.u32_set_value);     //����ָ��ָ����ʱ��������
			}
		}
		else if ((id_back >>8) == MMI_WIN_TYPE_SET)             //��ǰҳ�����������ݳ�����Χ��ʾҳ�棬���ص�����ҳ��
		{
			m_t_win_record.u8_sel_index = index_back;
			m_t_win_record.u8_win_status = MMI_WIN_SET;

			if (id_back == MMI_WIN_ID_RTC_PASSWORD_SET)
			{ 
				u8_ctime_get_time(&m_t_win_record.t_time);
				m_t_win_record.t_time.year -= 2000;
			}
			else if (id_back == MMI_WIN_ID_DC_SYSTEM_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->u8_item_cnt = 5;
				else if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[5].u32_val_max = 1;
					
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
				}
				else
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
				}
				
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
				else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
				else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
					win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
			}
			else if (id_back == MMI_WIN_ID_BATT_PARAM_SET)
			{
				if (g_u8_product_type != PRODUCT_TYPE_SC32)
					win->t_item[1].u32_val_max = 1;
			}
			if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
			{
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			}
			else if (id_back == MMI_WIN_ID_DC_PARAM_SET)
			{
				m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				//m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			}
			else if (id_back == MMI_WIN_ID_RECT_PARAM_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->t_item[1].u32_val_max = 3;
				else if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[1].u32_val_max = 8;
					
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt;
				}
				else
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt / 2;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt / 2;
				}
				
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
					win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;
			}
			else if (id_back == MMI_WIN_ID_BATT_METER1_SET)
			{
				v_disp_batt1_data_init();
			}
			else if (id_back == MMI_WIN_ID_BATT_METER2_SET)
			{
				v_disp_batt2_data_init();
			}
			else if (id_back == MMI_WIN_ID_DC_THR_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt;
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt;
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt ;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt ;
					win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt;
					win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt;
					win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt;
					win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt;
				}
				else
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt / 2;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt / 2;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt / 2;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt / 2;
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt / 2;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt / 2;
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt / 2;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt / 2;
					win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt / 2;
					win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt / 2;
					win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt / 2;
					win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt / 2;
				}
			}
			else if (id_back == MMI_WIN_ID_BATT_THR_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit;
				}
				else
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit / 2;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit / 2;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit / 2;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit / 2;
				}
			}
			else if (id_back == MMI_WIN_ID_INSU_THR_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->u8_item_cnt = 3;
				else
					win->u8_item_cnt = 4;
			}
			else if (id_back == MMI_WIN_ID_BATT_CHARGE_SET)
			{
				//�������ѹ������Ϊ��ع�ѹֵ������Ϊ���Ƿѹֵ
				win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
				win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
				win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
				win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
				
				//��������������Ϊ��������
				win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
			}
			else if (id_back == MMI_WIN_ID_BATT_TO_EQU_SET)  //���ת������������ֵӦ���ǵ�صĳ��������
			{
				win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * 1000);
			}
			else if (id_back == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt;
				}
				else
				{
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt / 2;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt / 2;
				}
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)
			{
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;

				if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_module_num > 0)
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ��ǰһҳ
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ����һҳ
				}
				else
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // ��ǰһҳ
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // ����һҳ
				}
			
				v_disp_feeder_panel_module_num_set();
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
				m_t_win_record.u_ordinal.u16_feeder_module = m_u16_feeder_module_ordianl;
				
				v_disp_feeder_panel_module_set();
					
				win->u8_icon_cnt = 2;
				if (m_u16_feeder_module_ordianl > 0)
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
				else
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;	
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)
			{
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;

				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_module_num > 0)
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ��ǰһҳ
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ����һҳ
				}
				else
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // ��ǰһҳ
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // ����һҳ
				}
			
				v_disp_feeder_panel_module_num_set2();
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
				m_t_win_record.u_ordinal.u16_feeder_module = m_u16_feeder_module_ordianl;
				
				v_disp_feeder_panel_module_set2();
					
				win->u8_icon_cnt = 2;
				if (m_u16_feeder_module_ordianl > 0)
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
				else
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
				else
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;	
			}
			else if (id_back == MMI_WIN_ID_DCDC_MODULE_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[1].u32_val_max = 4;
					
				if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
					win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
			}
			else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
					win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
				else
					win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;
			}
			else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				
				snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
						s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
						s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
				snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
						s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCDC_FEEDER_MODULE_ADDR);
			}
			else if (id_back == MMI_WIN_ID_DCAC_MODULE_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[1].u32_val_max = 4;
				
				if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
					win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
			}
			else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
					win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
				else
					win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;
			}
			else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				
				snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
						s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
						s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
				snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
						s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCAC_FEEDER_MODULE_ADDR);
			}

			else if (id_back == MMI_WIN_ID_AC_MODULE_SET)
			{			
				if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
			}
			else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
			{
					win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;
			}
			else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u16_curr_id = id_back;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

				m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num - 1;
				v_disp_acfeeder_panel_module_set();

				m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
				win->u8_icon_cnt = 2;
				win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
				if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 1)
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
			}

			if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0)
			{
				m_t_win_record.u32_set_value = *((U8_T *)(win->t_item[index_back].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x10)
			{
				m_t_win_record.u32_set_value = *((U16_T *)(win->t_item[index_back].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x20)
			{
				m_t_win_record.u32_set_value = *((U32_T *)(win->t_item[index_back].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x30)
			{
				//�Ѹ���������1000ת��Ϊ��������
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W1P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*10)) * 100;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W2P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*100)) * 10;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_7W3P))
				{
					m_t_win_record.u32_set_value = (U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*1000);
				}
			}

			m_t_win_record.pv_back_item_val = win->t_item[index_back].pv_val;     //��������ָ���ֵ
			win->t_item[index_back].pv_val = &(m_t_win_record.u32_set_value);     //����ָ��ָ����ʱ��������
		}
	}  //end of if (u16_key_val == g_u16_key_cancel)

	else if (u16_key_val == g_u16_key_up)
	{
		id_back = win->u16_id_prev;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			
			switch (id_back)
			{
				case MMI_WIN_ID_BATT_TOTAL_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;

						win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;

						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					break;
					
				case MMI_WIN_ID_BATT_GROUP_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{						
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP2_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{				
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}					
					break;

				case MMI_WIN_ID_BMS_GROUP1_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS_GROUP2_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
						win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS1_CELL_INFO:               // һ�鵥���ص�ѹ���棬��ҳ����
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u_ordinal.u16_cell = ((g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num - 1)
														/ MMI_CELL_CNT) * MMI_CELL_CNT; 

					v_disp_cell1_info();
					break;
					
				case MMI_WIN_ID_BMS2_CELL_INFO:               // ���鵥���ص�ѹ���棬��ҳ����
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u_ordinal.u16_cell = ((g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num - 1)
														/ MMI_CELL_CNT) * MMI_CELL_CNT; 

					v_disp_cell2_info();
					break;

				default:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP1_INFO)
					{
						m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					}
					
					break;
			}  //end of switch (id_back>>8)
		}  //end of if (id_back != MMI_WIN_ID_NULL)
	}  //end of else if (u16_key_val == g_u16_key_up)

	else if (u16_key_val == g_u16_key_down)
	{
		id_back = win->u16_id_next;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			
			switch (id_back)
			{
				case MMI_WIN_ID_BATT_TOTAL_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;

						win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;

						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}

					break;
					
				case MMI_WIN_ID_BATT_GROUP_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{						
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP2_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{							
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					break;

				case MMI_WIN_ID_BMS_GROUP1_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS_GROUP2_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
						win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS1_CELL_INFO:               // һ�鵥���ص�ѹ���棬��ҳ����
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					v_disp_cell1_info();
					break;
					
				case MMI_WIN_ID_BMS2_CELL_INFO:               // һ�鵥���ص�ѹ���棬��ҳ����
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					v_disp_cell2_info();
					break;

				default:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP1_INFO)
					{
						m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					}
					
					break;
			}  //end of switch (id_back>>8)
		}
	}  //end of else if (u16_key_val == g_u16_key_down)
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_cell1_volt_key_handler		           				
��������: һ�鵥���ص�ѹҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_cell1_volt_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if ((m_t_win_record.u_ordinal.u16_cell+MMI_CELL_CNT) < num)
		{
			m_t_win_record.u_ordinal.u16_cell += MMI_CELL_CNT;
			v_disp_cell1_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			if ((g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
				&& (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0))
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_BMS_GROUP2_INFO;
				m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;

				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
			}
			else
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_TOTAL_INFO;

				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
				{
					win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
				}
				else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
				{
					win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
				}
			}
			
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_cell >= MMI_CELL_CNT)
		{
			m_t_win_record.u_ordinal.u16_cell -= MMI_CELL_CNT;
			v_disp_cell1_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BMS_GROUP1_INFO;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
			if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
			{
				win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
			}
			else
			{
				win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_cell2_volt_key_handler		           				
��������: ���鵥���ص�ѹҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_cell2_volt_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if ((m_t_win_record.u_ordinal.u16_cell+MMI_CELL_CNT) < num)
		{
			m_t_win_record.u_ordinal.u16_cell += MMI_CELL_CNT;
			v_disp_cell2_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_TOTAL_INFO;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			
			win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
			if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
				win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
			else
				win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
				
		}
	}


	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_cell >= MMI_CELL_CNT)
		{
			m_t_win_record.u_ordinal.u16_cell -= MMI_CELL_CNT;
			v_disp_cell2_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BMS_GROUP2_INFO;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
			
			if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
				win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
			else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
				win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
			else
				win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_rect_info_key_handler		           				
��������: ����ģ����Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static v_disp_rect_info_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
	os_mut_release(g_mut_share_data);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		m_t_win_record.u_ordinal.u16_rect_info += 1;
		m_t_win_record.u_ordinal.u16_rect_info %= num;
		v_disp_rect_info();	
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_rect_info > 0)
		{
			m_t_win_record.u_ordinal.u16_rect_info -= 1;
		}
		else
		{
			m_t_win_record.u_ordinal.u16_rect_info = num - 1;
		}

		v_disp_rect_info();
	}
}

/*************************************************************
��������: v_disp_swt_info_key_handler		           				
��������: ������Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_swt_info_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (num > MMI_SWT_CNT)
		{
			if ((m_t_win_record.u_ordinal.u16_swt+MMI_SWT_CNT) < num)
				m_t_win_record.u_ordinal.u16_swt += MMI_SWT_CNT;
			else
				m_t_win_record.u_ordinal.u16_swt = 0;
	
			v_disp_special_swt_info();
		}
	}


	else if (u16_key_val == g_u16_key_up)
	{
		if (num > MMI_SWT_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_swt >= MMI_SWT_CNT)
				m_t_win_record.u_ordinal.u16_swt -= MMI_SWT_CNT;
			else
				m_t_win_record.u_ordinal.u16_swt = ((num-1) / MMI_SWT_CNT) * MMI_SWT_CNT;

			v_disp_special_swt_info();
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_ecswt_info_key_handler		           				
��������: ��ٿ�����Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_ecswt_info_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = u16_public_get_ctrl_swt_num();

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (num > MMI_SWT_CNT)
		{
			if ((m_t_win_record.u_ordinal.u16_swt + MMI_SWT_CNT) < num)
			{
				m_t_win_record.u_ordinal.u16_swt += MMI_SWT_CNT;
				u16_public_next_mv_swt_index(&m_u16_swt_index, MMI_SWT_CNT);
			}
			else
			{
				m_t_win_record.u_ordinal.u16_swt = 0;
				m_u16_swt_index = u16_public_get_first_swt_index();
			}			
			v_disp_ec_swt_info();
		}
	}


	else if (u16_key_val == g_u16_key_up)
	{
		if (num > MMI_SWT_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_swt >= MMI_SWT_CNT)
			{
				m_t_win_record.u_ordinal.u16_swt -= MMI_SWT_CNT;
				u16_public_pre_mv_swt_index(&m_u16_swt_index, MMI_SWT_CNT);
			}
			else
			{
				m_t_win_record.u_ordinal.u16_swt = ((num-1) / MMI_SWT_CNT) * MMI_SWT_CNT;
				m_u16_swt_index = u16_public_get_swt_index_from_no(m_t_win_record.u_ordinal.u16_swt + 1);
			}

			v_disp_ec_swt_info();
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_curr_fault_key_handler		           				
��������: ��ǰ������Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_curr_fault_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		os_mut_wait(g_mut_fault_data, 0xFFFF);       //��д��ǰ������Ҫ���л�����

		if (u8_fault_read_curr_fault_state() == FAULT_CHANGE)
		{
			v_disp_curr_falut_first_page();
		}
		else
		{
			num  = u16_fault_read_curr_fault_cnt();
			
			if (num > MMI_CURR_FAULT_CNT)
			{
				m_t_win_record.u_ordinal.u16_curr_fault += MMI_CURR_FAULT_CNT;
				if (m_t_win_record.u_ordinal.u16_curr_fault >= num)
					m_t_win_record.u_ordinal.u16_curr_fault = 0;

				v_disp_curr_falut_page();
			}
		}

		os_mut_release(g_mut_fault_data);
	}

	else if (u16_key_val == g_u16_key_up)
	{
		os_mut_wait(g_mut_fault_data, 0xFFFF);       //��д��ǰ������Ҫ���л�����

		if (u8_fault_read_curr_fault_state() == FAULT_CHANGE)
		{
			v_disp_curr_falut_first_page();
		}
		else
		{
			num  = u16_fault_read_curr_fault_cnt();
			if (num > MMI_CURR_FAULT_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_curr_fault >= MMI_CURR_FAULT_CNT)
				{
					m_t_win_record.u_ordinal.u16_curr_fault -= MMI_CURR_FAULT_CNT;
				}
				else
				{
					m_t_win_record.u_ordinal.u16_curr_fault = ((num-1) / MMI_CURR_FAULT_CNT) * MMI_CURR_FAULT_CNT;
				}

				v_disp_curr_falut_page();
			}
		}
		
		os_mut_release(g_mut_fault_data);
	}
}


/*************************************************************
��������: v_disp_his_fault_key_handler		           				
��������: ��ʷ������Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_his_fault_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);                 //��д��ʷ������Ҫ���л�����

		if (u8_fault_read_his_fault_state() == FAULT_CHANGE)
		{
			v_disp_his_fault_first_page();
		}
		else
		{
			num  = u16_fault_read_his_fault_cnt();
			if (num > MMI_HIS_FAULT_CNT)
			{
				m_t_win_record.u_ordinal.u16_his_fault += MMI_HIS_FAULT_CNT;
				if (m_t_win_record.u_ordinal.u16_his_fault >= num)
					m_t_win_record.u_ordinal.u16_his_fault = 0;
				
				v_disp_his_falut_page();
			}
		}

		os_mut_release(g_mut_fetch_his_fault);
	}

	else if (u16_key_val == g_u16_key_up)
	{
		os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);                 //��д��ʷ������Ҫ���л�����

		if (u8_fault_read_his_fault_state() == FAULT_CHANGE)
		{
			v_disp_his_fault_first_page();
		}
		else
		{
			num  = u16_fault_read_his_fault_cnt();
			if (num > MMI_HIS_FAULT_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_curr_fault >= MMI_HIS_FAULT_CNT)
					m_t_win_record.u_ordinal.u16_curr_fault -= MMI_HIS_FAULT_CNT;
				else
					m_t_win_record.u_ordinal.u16_curr_fault = ((num-1) / MMI_HIS_FAULT_CNT) * MMI_HIS_FAULT_CNT;

				v_disp_his_falut_page();
			}
		}

		os_mut_release(g_mut_fetch_his_fault);
	}
}

/*************************************************************
��������: v_disp_excption_fault_key_handler		           				
��������: ����ĩ���������Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_excption_fault_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		num  = u16_fault_read_no_resume_fault_cnt();

		if (num > MMI_EXCEPTION_CNT)
		{
			m_t_win_record.u_ordinal.u16_exception += MMI_EXCEPTION_CNT;
			
			if (m_t_win_record.u_ordinal.u16_exception >= num)
				m_t_win_record.u_ordinal.u16_exception = 0;

			v_disp_no_resume_fault_page();
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		num  = u16_fault_read_no_resume_fault_cnt();

		if (num > MMI_EXCEPTION_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_exception >= MMI_EXCEPTION_CNT)
				m_t_win_record.u_ordinal.u16_exception -= MMI_EXCEPTION_CNT;
			else
				m_t_win_record.u_ordinal.u16_exception = ((num-1) / MMI_EXCEPTION_CNT) * MMI_EXCEPTION_CNT;

			v_disp_no_resume_fault_page();
		}
	}
}

/*************************************************************
��������: v_disp_record_key_handler		           				
��������: �¼���¼ҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_record_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		os_mut_wait(g_mut_log, 0xFFFF);                 //��д�¼���¼��Ҫ���л�����

		if (u8_log_read_log_state() == LOG_CHANGE)
		{
			v_disp_record_first_page();
		}
		else
		{
			num = u16_log_read_log_cnt();
			if (num > MMI_RECORD_CNT)
			{
				m_t_win_record.u_ordinal.u16_record += MMI_RECORD_CNT;
				if (m_t_win_record.u_ordinal.u16_record >= num)
					m_t_win_record.u_ordinal.u16_record = 0;

				v_disp_record_page();
			}
		}	
		os_mut_release(g_mut_log);                      //�ͷŻ�����
	}

	else if (u16_key_val == g_u16_key_up)
	{
		os_mut_wait(g_mut_log, 0xFFFF);                 //��д�¼���¼��Ҫ���л�����

		if (u8_log_read_log_state() == LOG_CHANGE)
		{
			v_disp_record_first_page();
		}
		else
		{
			num = u16_log_read_log_cnt();
			if (num > MMI_RECORD_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_record >= MMI_RECORD_CNT)
					m_t_win_record.u_ordinal.u16_record -= MMI_RECORD_CNT;
				else
					m_t_win_record.u_ordinal.u16_record = ((num-1) / MMI_RECORD_CNT) * MMI_RECORD_CNT;
			
				v_disp_record_page();
			}
		}
		os_mut_release(g_mut_log);                      //�ͷŻ�����
	}
}

/*************************************************************
��������: v_disp_dc_feeder_key_handler		           				
��������: ����֧·��Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dc_feeder_key_handler(U16_T u16_key_val)
{
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
				if (m_t_win_record.u_ordinal.u16_feeder_branch
					>= g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 0;
				}
				v_disp_feeder_branch_info();	
			}
		}
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO2)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
				if (m_t_win_record.u_ordinal.u16_feeder_branch
					>= g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 0;
				}
				v_disp_feeder_branch_info2();	
			}
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
				}
				else
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 
						((g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
							* MMI_FEEDER_BRANCH_CNT;
				}
	
				v_disp_feeder_branch_info();	
			}
		}
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO2)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
				}
				else
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 
						((g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
							* MMI_FEEDER_BRANCH_CNT;
				}
	
				v_disp_feeder_branch_info2();	
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_dcdc_module_key_handler		           				
��������: ͨ��ģ����Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static v_disp_dcdc_module_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	os_mut_release(g_mut_share_data);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		m_t_win_record.u_ordinal.u16_dcdc_module += 1;
		m_t_win_record.u_ordinal.u16_dcdc_module %= num;
		v_disp_dcdc_module_info();	
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_dcdc_module > 0)
		{
			m_t_win_record.u_ordinal.u16_dcdc_module -= 1;
		}
		else
		{
			m_t_win_record.u_ordinal.u16_dcdc_module = num - 1;
		}

		v_disp_dcdc_module_info();
	}
}

/*************************************************************
��������: v_disp_dcdc_feeder_key_handler		           				
��������: ͨ����֧·��Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dcdc_feeder_key_handler(U16_T u16_key_val)
{
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
			if (m_t_win_record.u_ordinal.u16_feeder_branch
				>= g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 0;
			}
			v_disp_dcdc_branch_info();	
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
			}
			else
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 
					((g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
						* MMI_FEEDER_BRANCH_CNT;
			}

			v_disp_dcdc_branch_info();	
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_dcac_module_key_handler		           				
��������: UPSģ����Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static v_disp_dcac_module_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		m_t_win_record.u_ordinal.u16_dcac_module += 1;
		m_t_win_record.u_ordinal.u16_dcac_module %= num;
		v_disp_dcac_module_info();	
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_dcac_module > 0)
		{
			m_t_win_record.u_ordinal.u16_dcac_module -= 1;
		}
		else
		{
			m_t_win_record.u_ordinal.u16_dcac_module = num - 1;
		}

		v_disp_dcac_module_info();
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_dcac_feeder_key_handler		           				
��������: UPS��֧·��Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_dcac_feeder_key_handler(U16_T u16_key_val)
{	
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
			if (m_t_win_record.u_ordinal.u16_feeder_branch
				>= g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 0;
			}
			v_disp_dcac_branch_info();	
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
			}
			else
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 
					((g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
						* MMI_FEEDER_BRANCH_CNT;
			}

			v_disp_dcac_branch_info();	
		}
	}
	
	os_mut_release(g_mut_share_data);
}


/*************************************************************
��������: v_disp_ac_feeder_key_handler		           				
��������: ������֧·��Ϣҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_ac_feeder_key_handler(U16_T u16_key_val)
{	
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
			if (m_t_win_record.u_ordinal.u16_feeder_branch
				>= g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 0;
			}
			v_disp_ac_branch_info();	
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
			}
			else
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 
					((g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
						* MMI_FEEDER_BRANCH_CNT;
			}

			v_disp_ac_branch_info();	
		}
	}
	
	os_mut_release(g_mut_share_data);
}


#define MMI_1000000_BIT   0   //����λ
#define MMI_100000_BIT    1   //ʮ��λ
#define MMI_10000_BIT     2   //��λ
#define MMI_1000_BIT      3	  //ǧλ
#define MMI_100_BIT       4   //��λ
#define MMI_10_BIT        5   //ʮλ
#define MMI_1_BIT         6   //��λ


/*************************************************************
��������: u8_disp_get_integer_bit		           				
��������: ��ȡu8_flagָ��������λ						
�������: u32_data -- Ҫ��ȡ������
          u8_flag  -- ָ����Ӧ������λ       		   				
�������: ��
����ֵ  ��u8_flagָ��λ����ֵ														   				
**************************************************************/
static U8_T u8_disp_get_integer_bit(U32_T u32_data, U8_T u8_flag)
{
	U8_T bit = 0;

	switch (u8_flag)
	{
		case MMI_1000000_BIT:
			bit = (u32_data%10000000)/1000000;
			break;

		case MMI_100000_BIT:
			bit = (u32_data%1000000)/100000;
			break;

		case MMI_10000_BIT:
			bit = (u32_data%100000)/10000;
			break;

		case MMI_1000_BIT:
			bit = (u32_data%10000)/1000;
			break;

		case MMI_100_BIT:
			bit = (u32_data%1000)/100;
			break;

		case MMI_10_BIT:
			bit = (u32_data%100)/10;
			break;

		case MMI_1_BIT:
			bit = (u32_data%10);
			break;
		default:
			break;
	}

	return bit;
}

/*************************************************************
��������: v_disp_data_bit_add		           				
��������: ���u8_flag��Ӧ������λС�����ѣ�����Ӧ����λ��1�����u8_flag��Ӧ������λ����9������Ӧ������λ��9						
�������: u8_flag -- ָ����Ӧ������λ       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_data_bit_add(U8_T u8_flag)
{
	U32_T val_tab[7] = { 1000000, 100000, 10000, 1000, 100, 10, 1};

	if (u8_disp_get_integer_bit(m_t_win_record.u32_set_value, u8_flag) < 9)
	{
		m_t_win_record.u32_set_value += val_tab[u8_flag];
	}
	else
	{
		m_t_win_record.u32_set_value -= (val_tab[u8_flag] * 9);
	}
}

/*************************************************************
��������: v_disp_data_bit_sub		           				
��������: ���u8_flag��Ӧ������λ����0������Ӧ����λ��1�����u8_flag��Ӧ������λ����0������Ӧ������λ��9						
�������: u8_flag -- ָ����Ӧ������λ       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_data_bit_sub(U8_T u8_flag)
{
	U32_T val_tab[7] = { 1000000, 100000, 10000, 1000, 100, 10, 1};

	if (u8_disp_get_integer_bit(m_t_win_record.u32_set_value, u8_flag) > 0)
	{
		m_t_win_record.u32_set_value -= val_tab[u8_flag];
	}
	else
	{
		m_t_win_record.u32_set_value += (val_tab[u8_flag] * 9);
	}
}

/*************************************************************
��������: v_disp_bit_index_add		           				
��������: ���λ���������ֵ�����Ϊ��Сֵ������������ֵ��������λ������1						
�������: u8_bit_max -- ���λ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_bit_index_add(U8_T bit_max)
{
	if (m_t_win_record.u8_bit_index < (bit_max-1))
	{
		m_t_win_record.u8_bit_index++;
	}
	else
	{
		m_t_win_record.u8_bit_index = 0;
	}
}

/*************************************************************
��������: v_disp_bit_index_sub		           				
��������: ���λ��������Сֵ�����Ϊ���ֵ�����������Сֵ��������λ������1					
�������: u8_bit_max -- ���λ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_bit_index_sub(U8_T u8_bit_max)
{
	if (m_t_win_record.u8_bit_index > 0)
	{
		m_t_win_record.u8_bit_index--;
	}
	else
	{
		m_t_win_record.u8_bit_index = u8_bit_max - 1;
	}
}

/*************************************************************
��������: v_disp_password_key_handler		           				
��������: ��������ҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_password_key_handler(U16_T u16_key_val)
{
	U32_T password;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_enter)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		password = g_t_share_data.t_sys_cfg.t_sys_param.u32_password;
		os_mut_release(g_mut_share_data);
		
		if ((m_t_win_record.u32_set_value == password)                                              //�û���������ȷ����ʾ���ò˵�ҳ��
			|| (m_t_win_record.u32_set_value == MMI_SUPER_PASSWORD))
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_SET_MENU;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_sel_index = 1;

			win->u8_item_cnt = 8;                                                                  //�û������ò˵���7��
			m_u8_set_menu_item_cnt = win->u8_item_cnt;
		}
		else if (m_t_win_record.u32_set_value == MMI_DEFEND_PASSWORD)                               //ά����������ȷ����ʾ���ò˵�ҳ��
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_SET_MENU;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_sel_index = 1;

			win->u8_item_cnt = 12;                                                                   //ά�������ò˵���10��
			m_u8_set_menu_item_cnt = win->u8_item_cnt;
		}
		else                                                                                        //���������ʾ���������ʾҳ��
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_PASSWORD_ERROR;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		v_disp_bit_index_sub(5);
	}

	else if (u16_key_val == g_u16_key_down)
	{
		v_disp_bit_index_add(5);
	}

	else if (u16_key_val == g_u16_key_add)
	{
		if (m_t_win_record.u8_bit_index == 0)	       // ��λ     
		{
			v_disp_data_bit_add(MMI_10000_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 1)	    // ǧλ     
		{
			v_disp_data_bit_add(MMI_1000_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 2)    // ��λ     
		{
			v_disp_data_bit_add(MMI_100_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 3)	   // ʮλ     
		{
			v_disp_data_bit_add(MMI_10_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 4)    // ��λ
		{
			v_disp_data_bit_add(MMI_1_BIT);
		}
	}

	else if (u16_key_val == g_u16_key_sub)
	{
		if (m_t_win_record.u8_bit_index == 0)	       // ��λ     
		{
			v_disp_data_bit_sub(MMI_10000_BIT);
		}                   
		else if (m_t_win_record.u8_bit_index == 1)	   // ǧλ     
		{
			v_disp_data_bit_sub(MMI_1000_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 2)    // ��λ     
		{
			v_disp_data_bit_sub(MMI_100_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 3)	   // ʮλ     
		{
			v_disp_data_bit_sub(MMI_10_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 4)    // ��λ
		{
			v_disp_data_bit_sub(MMI_1_BIT);
		}
	}
}

/*************************************************************
��������: v_disp_set_key_handler		           				
��������: ����ҳ�水��������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_set_key_handler(U16_T u16_key_val)
{
	U16_T id_back;
	U8_T index_back;
	U32_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (m_t_win_record.u8_win_status == MMI_WIN_NORMAL)     // �鿴״̬
	{
		if (u16_key_val == g_u16_key_enter)                                 // ��������״̬
		{
			m_t_win_record.u8_win_status = MMI_WIN_SET;
			m_t_win_record.u8_bit_index = 0;

			if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0)
			{
				m_t_win_record.u32_set_value = *((U8_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x10)
			{
				m_t_win_record.u32_set_value = *((U16_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x20)
			{
				m_t_win_record.u32_set_value = *((U32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x30)
			{
				//�Ѹ���������1000ת��Ϊ��������
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W1P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val))*10)) * 100;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W2P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val))*100)) * 10;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_7W3P))
				{
					m_t_win_record.u32_set_value = (U32_T)(*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val))*1000);
				}
			}

			m_t_win_record.pv_back_item_val = win->t_item[m_t_win_record.u8_sel_index].pv_val;
			win->t_item[m_t_win_record.u8_sel_index].pv_val = &(m_t_win_record.u32_set_value);
		}

		else if (u16_key_val == g_u16_key_cancel)
		{
			v_disp_cancel_key_handler();
		}

		else if (u16_key_val == g_u16_key_up)
		{
			if (m_t_win_record.u8_sel_index <= 1)
			{
				id_back = win->u16_id_prev;

				if (id_back == MMI_WIN_ID_BATT_CTL_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num < 2)
					{
						m_t_win_record.t_curr_win.u8_item_cnt = 2;
						m_t_win_record.u8_sel_index = 1;
					}
					else
					{
						m_t_win_record.u8_sel_index = 2;
					}
				}
				else if (id_back == MMI_WIN_ID_RECT_ON_OFF)
				{
					num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;

					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u_ordinal.u16_rect_on_off = ((num - 1) / MMI_RECT_ON_OFF_CNT) * MMI_RECT_ON_OFF_CNT;

						v_disp_rect_on_off();
						m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						
						win->u8_icon_cnt = 2;
						win->u16_id_next = MMI_WIN_ID_SYSTEM_CTL_SET;
						if (num > MMI_RECT_ON_OFF_CNT)
						{
							win->u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;
						}
						else
						{
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
								win->u16_id_prev = MMI_WIN_ID_SYSTEM_CTL_SET;
							else
								win->u16_id_prev = MMI_WIN_ID_BATT_CTL_SET;
						}
					}
					else  // else of if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
					{
						if (m_t_win_record.u_ordinal.u16_rect_on_off == 0)
							m_t_win_record.u_ordinal.u16_rect_on_off = ((num - 1) / MMI_RECT_ON_OFF_CNT) * MMI_RECT_ON_OFF_CNT;
						else
							m_t_win_record.u_ordinal.u16_rect_on_off -= MMI_RECT_ON_OFF_CNT;

						v_disp_rect_on_off();
						m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						
						win->u8_icon_cnt = 2;
						win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						win->u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;

						if (m_t_win_record.u_ordinal.u16_rect_on_off == 0)
						{
							if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)
							{
								if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
									win->u16_id_prev = MMI_WIN_ID_SYSTEM_CTL_SET;
								else
									win->u16_id_prev = MMI_WIN_ID_BATT_CTL_SET;
							}
						}
					}  // end of if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
				}  //end of if (id_back == MMI_WIN_ID_RECT_ON_OFF)
				else if (id_back == MMI_WIN_ID_SWT_ON_OFF)
				{
					num = u16_public_get_ctrl_swt_num();

					/*if (m_t_win_record.u_ordinal.u16_swt_on_off == 0)
					{
						m_t_win_record.u_ordinal.u16_swt_on_off = ((num - 1) / MMI_SWT_ON_OFF_CNT) * MMI_SWT_ON_OFF_CNT;
						m_u16_swt_index = u16_public_get_swt_index_from_no(m_t_win_record.u_ordinal.u16_swt_on_off + 1);
					}
					else
					{
						m_t_win_record.u_ordinal.u16_swt_on_off -= MMI_SWT_ON_OFF_CNT;
						u16_public_pre_mv_swt_index(&m_u16_swt_index, MMI_SWT_ON_OFF_CNT);
					}*/
					if (m_t_win_record.u_ordinal.u16_swt_on_off >= MMI_SWT_ON_OFF_CNT)
				  {
						m_t_win_record.u_ordinal.u16_swt_on_off -= MMI_SWT_ON_OFF_CNT;
						u16_public_pre_mv_swt_index(&m_u16_swt_index, MMI_SWT_ON_OFF_CNT);
					}
					else
					{
						m_t_win_record.u_ordinal.u16_swt_on_off = ((num - 1) / MMI_SWT_ON_OFF_CNT) * MMI_SWT_ON_OFF_CNT;
						m_u16_swt_index = u16_public_get_swt_index_from_no(m_t_win_record.u_ordinal.u16_swt_on_off + 1);
					}

					v_disp_swt_on_off();
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_SWT_ON_OFF;
					win->u16_id_prev = MMI_WIN_ID_SWT_ON_OFF;					
				}  //end of if (id_back == MMI_WIN_ID_SWT_ON_OFF)
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
					m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num - 1;
					v_disp_feeder_panel_module_set();
					
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
					if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
					m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num - 1;
					v_disp_feeder_panel_module_set2();
					
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
					if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
				}
				else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCDC_FEEDER_MODULE_ADDR);
					
				}
				else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCAC_FEEDER_MODULE_ADDR);
				}
				else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

					m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num - 1;
					v_disp_acfeeder_panel_module_set();

					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 1)
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;					
				}
				else if (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)      //�����ص���У׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u16_id_next = MMI_WIN_ID_AC_ADJUST;
						
					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_BATT2_CURR_ADJUST)      //�����ص���У׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;

					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_RELAY_OUT_SET)        //�ɽ������趨ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
				} 
				else if (id_back != MMI_WIN_ID_NULL)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					{
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
						//m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
						{
							win->t_item[2].u32_val_max = 1;
							win->u8_item_cnt = 3;
							m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						}
						else
						{
							if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path == ONE_PATH)
							{
								win->t_item[3].u32_val_max = 1;
								if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac >= 2)
									g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;
							}
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_PARAM_SET)
					{
						if (g_u8_product_type != PRODUCT_TYPE_SC32)
							win->t_item[1].u32_val_max = 1;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_CHARGE_SET)   //��س���������ҳ��
					{
						//�������ѹ������Ϊ��ع�ѹֵ������Ϊ���Ƿѹֵ
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
						win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					
						//��������������Ϊ��������
						win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)  //���ת������������ֵӦ���ǵ�صĳ��������
					{
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER1_SET)
					{
						v_disp_batt1_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER2_SET)
					{
						v_disp_batt2_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt;
						}
						else
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->t_item[1].u32_val_max = 3;
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 8;
					
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt ;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt ;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt / 2;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt / 2;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt / 2;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt / 2;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt / 2;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt / 2;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_INSU_THR_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->u8_item_cnt = 3;
						else
							win->u8_item_cnt = 4;
							
						m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
						{
							win->u8_item_cnt = 5;
							m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						}
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
						{
							win->t_item[5].u32_val_max = 1;
						}
							
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SYSTEM_CTL_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
							win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
							
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
					{
						win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;  // ����һҳ
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
								
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
					{
						win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;  // ����һҳ
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
								
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
					{
						win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;  // ����һҳ
					}
				}
				else
				{
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
				}
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_VOLT_ADJUST) && (m_t_win_record.u8_sel_index == 2))
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
				{
					win->u8_item_cnt = 4;
					win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
				}
						
				m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;

				g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_LOAD_CURR_ADJUST) && (m_t_win_record.u8_sel_index == 2))        //���ص���У׼ҳ��
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_VOLT_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_array[MMI_WIN_ID_DC_VOLT_ADJUST & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;

				g_t_share_data.t_dc_adjust_data.f32_batt_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_pb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_cb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT1_CURR_ADJUST) && (m_t_win_record.u8_sel_index == 2))       //һ���ص���У׼ҳ��
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_LOAD_CURR_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_array[MMI_WIN_ID_LOAD_CURR_ADJUST & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;

				g_t_share_data.t_dc_adjust_data.f32_load_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_load_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT2_CURR_ADJUST) && (m_t_win_record.u8_sel_index == 2))       //�����ص���У׼ҳ��
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT1_CURR_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_array[MMI_WIN_ID_BATT1_CURR_ADJUST & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
				
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->u16_id_next = MMI_WIN_ID_AC_ADJUST;

				g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RELAY_OUT_SET) && (m_t_win_record.u8_sel_index == 2))           //�ɽ������趨ҳ��
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_ALARM_SET;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_record.t_curr_win.u8_item_cnt - 1;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET) && (m_t_win_record.u8_sel_index == 2)) //���߹�����ģ���趨ҳ��
			{
				if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
				{
					m_t_win_record.u_ordinal.u16_feeder_module--;

					v_disp_feeder_panel_module_set();
					
					win->u8_icon_cnt = 2;
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
					if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
				}
				else
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ��ǰһҳ
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ����һҳ

					v_disp_feeder_panel_module_num_set();
				}
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2) && (m_t_win_record.u8_sel_index == 2)) //���߹�����ģ���趨ҳ��
			{
				if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
				{
					m_t_win_record.u_ordinal.u16_feeder_module--;

					v_disp_feeder_panel_module_set2();
					
					win->u8_icon_cnt = 2;
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
					if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
				}
				else
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ��ǰһҳ
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ����һҳ

					v_disp_feeder_panel_module_num_set2();
				}
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET) && (m_t_win_record.u8_sel_index == 2))  //ͨ�ſ�������ҳ��
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_sel_index = 1;

				win->u8_icon_cnt = 2;
				win->u16_id_prev = MMI_WIN_ID_DCDC_MODULE_SET;         // ��ǰһҳ
				win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;  // ����һҳ
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET) && (m_t_win_record.u8_sel_index == 2))  //UPS��������ҳ��
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_sel_index = 1;

				win->u8_icon_cnt = 2;
				win->u16_id_prev = MMI_WIN_ID_DCAC_MODULE_SET;         // ��ǰһҳ
				win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;  // ����һҳ
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_SET) && (m_t_win_record.u8_sel_index == 2))  //����ϵͳ��������ҳ��
			{
				if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
				{
					m_t_win_record.u_ordinal.u16_feeder_module--;

					v_disp_acfeeder_panel_module_set();
					
					win->u8_icon_cnt = 2;
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
					if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
				}
				else
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;  // ��ǰһҳ
					win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;  // ����һҳ

					v_disp_acfeeder_panel_module_num_set();
				}
			}
			else  // else of if (m_t_win_record.u8_sel_index <= 1)
			{
				m_t_win_record.u8_sel_index--;

				//������������ת�����оݡ����ת�����оݡ���غ������ã���Щ����������Ŀû����ֵ����������ƶ����ʱ����������Ŀ
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_NONE)
				   && ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_FLO_SET)
				      || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)))
				{
					m_t_win_record.u8_sel_index--;
				}

				/*//�������������ģ��������ý��棬ͨѶЭ��̶�ΪMODBUS�������趨��������Ҫ��������Ŀ
				if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET) && (m_t_win_record.u8_sel_index == 3))
				{
					m_t_win_record.u8_sel_index--;
				}*/
			}  // end of if (m_t_win_record.u8_sel_index <= 1)
		}

		else if (u16_key_val == g_u16_key_down)
		{
			if (m_t_win_record.u8_sel_index >= win->u8_item_cnt-1)
			{
				id_back = win->u16_id_next;

				if (id_back == MMI_WIN_ID_BATT_CTL_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num < 2)
					{
						m_t_win_record.t_curr_win.u8_item_cnt = 2;
					}
				}
				else if (id_back == MMI_WIN_ID_RECT_ON_OFF)      // ����ģ�鿪�ػ�����ҳ��
				{
					num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;

					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
					{
						U16_T id_curr_back = m_t_win_record.u16_curr_id;

						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						win->u16_id_prev = id_curr_back;

						v_disp_rect_on_off();
					}
					else
					{
						m_t_win_record.u8_sel_index = 1;
						m_t_win_record.u_ordinal.u16_rect_on_off += MMI_RECT_ON_OFF_CNT;
						if (m_t_win_record.u_ordinal.u16_rect_on_off >= num)
							m_t_win_record.u_ordinal.u16_rect_on_off = 0;

						v_disp_rect_on_off();
						win->u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;
					}

					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;

					if (m_t_win_record.u_ordinal.u16_rect_on_off + MMI_RECT_ON_OFF_CNT >= num)
					{
					   	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)
							win->u16_id_next = MMI_WIN_ID_SYSTEM_CTL_SET;
					}
				}  // end of if (id_back == MMI_WIN_ID_RECT_ON_OFF)
				else if (id_back == MMI_WIN_ID_SWT_ON_OFF)      // ��ٿ��ؿ���ҳ��
				{
					num = u16_public_get_ctrl_swt_num(); 
										
					m_t_win_record.u8_sel_index = 1;
					m_t_win_record.u_ordinal.u16_swt_on_off += MMI_SWT_ON_OFF_CNT;
					u16_public_next_mv_swt_index(&m_u16_swt_index, MMI_SWT_ON_OFF_CNT);
					if (m_t_win_record.u_ordinal.u16_swt_on_off >= num)
					{
						m_t_win_record.u_ordinal.u16_swt_on_off = 0;
						m_u16_swt_index = u16_public_get_first_swt_index();
					}

					v_disp_swt_on_off();
					win->u16_id_prev = MMI_WIN_ID_SWT_ON_OFF;

					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_SWT_ON_OFF;
				}  // end of if (id_back == MMI_WIN_ID_SWT_ON_OFF)
				else if (id_back == MMI_WIN_ID_AC_ADJUST)       //����У׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						win->u8_item_cnt = 4;
						win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
					}

					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_DC_VOLT_ADJUST)       //ֱ����ѹУ׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;

					g_t_share_data.t_dc_adjust_data.f32_batt_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_pb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_cb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_LOAD_CURR_ADJUST)       //���ص���У׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;

					g_t_share_data.t_dc_adjust_data.f32_load_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_load_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)       //��ص���У׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u16_id_next = MMI_WIN_ID_AC_ADJUST;

					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_BATT2_CURR_ADJUST)       //��������У׼ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;

					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_RELAY_OUT_SET)        //�ɽ������趨ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)   //���������ģ����������ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ��ǰһҳ
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // ����һҳ

					v_disp_feeder_panel_module_num_set();
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)       //���������ģ�������趨ҳ��
				{	
					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
						m_t_win_record.u8_sel_index = 2;
						v_disp_feeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;	
					}
					else
					{
						m_t_win_record.u_ordinal.u16_feeder_module++;
						m_t_win_record.u8_sel_index = 2;

						v_disp_feeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						if ((m_t_win_record.u_ordinal.u16_feeder_module+1) < g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
					}
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)   //���������ģ����������ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ��ǰһҳ
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // ����һҳ

					v_disp_feeder_panel_module_num_set2();
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)       //���������ģ�������趨ҳ��
				{	
					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
						m_t_win_record.u8_sel_index = 2;
						v_disp_feeder_panel_module_set2();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;	
					}
					else
					{
						m_t_win_record.u_ordinal.u16_feeder_module++;
						m_t_win_record.u8_sel_index = 2;

						v_disp_feeder_panel_module_set2();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						if ((m_t_win_record.u_ordinal.u16_feeder_module+1) < g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
					}
				}
				else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET)     //ͨ��������ģ����������ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCDC_FEEDER_MODULE_ADDR);
				}
				else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET)     //UPS������ģ����������ҳ��
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCAC_FEEDER_MODULE_ADDR);
				}
				else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)     //����������ģ����������ҳ��
				{
					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
					{	//�������ģ�������е�һ������
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 2;
						v_disp_acfeeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 1)
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;//MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;	
					}
					else
					{   //�������ģ�������зǵ�һ������
						m_t_win_record.u_ordinal.u16_feeder_module++;
						m_t_win_record.u8_sel_index = 2;

						v_disp_acfeeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						if ((m_t_win_record.u_ordinal.u16_feeder_module+1) < g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num)
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;//MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					}

//					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
//					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
//					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
//					m_t_win_record.u8_sel_index = 2;
//					m_t_win_record.u16_curr_id = id_back;
//					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
//					
//					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
//							 s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
//							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
//					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
//							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_AC_FEEDER_MODULE_ADDR);
				}
				else if (id_back != MMI_WIN_ID_NULL)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					{
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
						//m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
						{
							win->t_item[2].u32_val_max = 1;
							win->u8_item_cnt = 3;
						}
						else
						{
							if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path == ONE_PATH)
							{
								win->t_item[3].u32_val_max = 1;
								if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac >= 2)
									g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;
							}
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_PARAM_SET)
					{
						if (g_u8_product_type != PRODUCT_TYPE_SC32)
							win->t_item[1].u32_val_max = 1;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_CHARGE_SET)   //��س���������ҳ��
					{
						//�������ѹ������Ϊ��ع�ѹֵ������Ϊ���Ƿѹֵ
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
						win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					
						//��������������Ϊ��������
						win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)  //���ת������������ֵӦ���ǵ�صĳ��������
					{
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER1_SET)
					{
						v_disp_batt1_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER2_SET)
					{
						v_disp_batt2_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt;
						}
						else
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->t_item[1].u32_val_max = 3;
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 8;
					
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt ;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt ;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt / 2;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt / 2;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt / 2;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt / 2;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt / 2;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt / 2;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_INSU_THR_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->u8_item_cnt = 3;
						else
							win->u8_item_cnt = 4;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->u8_item_cnt = 5;
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[5].u32_val_max = 1;
					
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SYSTEM_CTL_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
							win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
								
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
							
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
					{  		
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;
					}

					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
					{  		
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;
					}
						
				} // end of else if (id_back != MMI_WIN_ID_NULL)
				else
				{
					m_t_win_record.u8_sel_index = 1;
				}
			}
			else  // else of if (m_t_win_record.u8_sel_index >= win->u8_item_cnt-1)
			{
				m_t_win_record.u8_sel_index++;

				//������������ת�����оݡ����ת�����оݡ���غ������ã���Щ�������������Ŀû����ֵ����������ƶ����ʱ����������Ŀ
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_NONE)
				   && ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_FLO_SET)
				      || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)))
				{
					m_t_win_record.u8_sel_index++;
				}

				/*//�������������ģ��������ý��棬ͨѶЭ��̶�ΪMODBUS�������趨��������Ҫ��������Ŀ
				if ((win->u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET) && (m_t_win_record.u8_sel_index == 3))
				{
					m_t_win_record.u8_sel_index++;
				}*/
			}  //end of if (m_t_win_record.u8_sel_index >= win->u8_item_cnt-1)
		}
	}
	else                                                    // ����״̬
	{
		if (u16_key_val == g_u16_key_cancel)
		{
			m_t_win_record.u8_win_status = MMI_WIN_NORMAL;
			m_t_win_record.u8_bit_index = 0;
			win->t_item[m_t_win_record.u8_sel_index].pv_val = m_t_win_record.pv_back_item_val;
		}

		else if (u16_key_val == g_u16_key_enter)
		{
			if (m_t_win_record.u32_set_value >= win->t_item[m_t_win_record.u8_sel_index].u32_val_min &&
				m_t_win_record.u32_set_value <= win->t_item[m_t_win_record.u8_sel_index].u32_val_max)
			{
				m_t_win_record.u8_win_status = MMI_WIN_NORMAL;
				m_t_win_record.u8_bit_index = 0;

				win->t_item[m_t_win_record.u8_sel_index].pv_val = m_t_win_record.pv_back_item_val;

				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0)
				{
					*((U8_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = (U8_T)(m_t_win_record.u32_set_value);
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x10)
				{
					*((U16_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = (U16_T)(m_t_win_record.u32_set_value);
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x20)
				{
					*((U32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = (U32_T)(m_t_win_record.u32_set_value);
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x30)
				{
					*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = ((F32_T)(m_t_win_record.u32_set_value) / 1000.0);
				}

				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SYSTEM_CTL_SET)
				{
					if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
					{
						win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
					}
					else
					{
						win->u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
						g_t_share_data.t_sys_cfg.t_ctl.u16_batt[0] = (U16_T)(g_t_share_data.t_rt_data.t_batt.e_state[0]);
						g_t_share_data.t_sys_cfg.t_ctl.u16_batt[1] = (U16_T)(g_t_share_data.t_rt_data.t_batt.e_state[1]);
					}
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_THR_SET)
				{
					if ((m_t_win_record.u8_sel_index == 1) || (m_t_win_record.u8_sel_index == 2))
					{
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
						{
							//��������ѹ����/С�ڵ�ع�ѹ/Ƿѹֵ��������ѹ���ڹ�ѹ/Ƿѹֵ
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit;
							else if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit;

							//���������ѹ����/С�ڵ�ع�ѹ/Ƿѹֵ���򸡳��ѹ���ڹ�ѹ/Ƿѹֵ
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit;
							else if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit;
						}
					}
					else if ((m_t_win_record.u8_sel_index == (win->u8_item_cnt - 1)))  //��س������������ֵӦ���ǵ�صĳ�������
					{
						//��������������ڳ������㣬��ѳ���������ֵ��Ϊ��������
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit)
						{
							g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit;

							//���ת����������ڳ�������㣬���ת���������ֵ��Ϊ���������
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_CHARGE_SET)    //���ת������������ֵӦ���ǵ�صĳ��������
					&& (m_t_win_record.u8_sel_index == (win->u8_item_cnt-1)))
				{
					//���ת����������ڳ�������㣬���ת���������ֵ��Ϊ���������
					if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr)
						g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr;
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RTC_PASSWORD_SET)
					&& (m_t_win_record.u8_sel_index >= 1) && (m_t_win_record.u8_sel_index <= 6))
				{
					m_t_win_record.t_time.year += 2000; // RTCʱ�����õ�ʱ����ʾ���ͣ�����дRTCʱ��ʱ��Ҫ��������ݣ����Ը�дRTC��ʱ���2000����д��ɼ�2000
					v_rtc_rtc_set_time(&m_t_win_record.t_time);
					m_t_win_record.t_time.year -= 2000;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
				{
					if ((m_t_win_record.u8_sel_index == 1) && (m_e_volt_level != g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level))
					{
						m_e_volt_level = g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level;
						v_disp_volt_init_by_level();
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_PARAM_SET)
					&& (m_t_win_record.u8_sel_index == 2) && (g_u8_product_type != PRODUCT_TYPE_SC12))
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path == ONE_PATH)
					{
						win->t_item[3].u32_val_max = 1;
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac >= 2)
							g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;
					}
					else
					{
						win->t_item[3].u32_val_max = 3;
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET))
				{
					DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl]);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_feeder_panel->t_feeder_module[m_t_win_record.u_ordinal.u16_feeder_module]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_insu_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_insu_feeder_num > u8_max_num)
						pt_feeder_module->u8_insu_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_insu_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_insu_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//�����֧·����
					pt_feeder_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						if (pt_feeder_panel->u8_feeder_branch_num + pt_feeder_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_feeder_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_feeder_panel->u8_feeder_branch_num;
							pt_feeder_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_feeder_panel->u8_feeder_branch_num += pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
					}
					
					pt_feeder_panel->u8_alarm_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_state_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_insu_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_curr_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					
					/*for (i=pt_feeder_panel->u8_feeder_module_num; i<FEEDER_PANEL_MODULE_MAX; i++)
					{
						memset(&(pt_feeder_panel->t_feeder_module[i]), 0, sizeof(FEEDER_MODULE_CFG_T));
					}*/
						
					pt_feeder_panel->u8_feeder_start_num[0] = 0;
					for (i=1; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						pt_feeder_panel->u8_feeder_start_num[i] = pt_feeder_panel->u8_feeder_start_num[i-1]
																	+ pt_feeder_panel->t_feeder_module[i-1].u8_feeder_num;
						
						if (pt_feeder_panel->u8_feeder_start_num[i] > FEEDER_BRANCH_MAX)
							pt_feeder_panel->u8_feeder_start_num[i] = FEEDER_BRANCH_MAX;
					}
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)
					{		
						if (pt_feeder_panel->u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						}
						else
						{
							win->u8_icon_cnt = 0;
							win->u16_id_prev = MMI_WIN_ID_NULL;
							win->u16_id_next = MMI_WIN_ID_NULL;
						}
					}	
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2))
				{
					DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl]);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_feeder_panel->t_feeder_module[m_t_win_record.u_ordinal.u16_feeder_module]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_insu_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_insu_feeder_num > u8_max_num)
						pt_feeder_module->u8_insu_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_insu_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_insu_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//�����֧·����
					pt_feeder_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						if (pt_feeder_panel->u8_feeder_branch_num + pt_feeder_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_feeder_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_feeder_panel->u8_feeder_branch_num;
							pt_feeder_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_feeder_panel->u8_feeder_branch_num += pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
					}
					
					pt_feeder_panel->u8_alarm_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_state_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_insu_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_curr_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					
					/*for (i=pt_feeder_panel->u8_feeder_module_num; i<FEEDER_PANEL_MODULE_MAX; i++)
					{
						memset(&(pt_feeder_panel->t_feeder_module[i]), 0, sizeof(FEEDER_MODULE_CFG_T));
					}*/
						
					pt_feeder_panel->u8_feeder_start_num[0] = 0;
					for (i=1; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						pt_feeder_panel->u8_feeder_start_num[i] = pt_feeder_panel->u8_feeder_start_num[i-1]
																	+ pt_feeder_panel->t_feeder_module[i-1].u8_feeder_num;
						
						if (pt_feeder_panel->u8_feeder_start_num[i] > FEEDER_BRANCH_MAX)
							pt_feeder_panel->u8_feeder_start_num[i] = FEEDER_BRANCH_MAX;
					}
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)
					{		
						if (pt_feeder_panel->u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						}
						else
						{
							win->u8_icon_cnt = 0;
							win->u16_id_prev = MMI_WIN_ID_NULL;
							win->u16_id_next = MMI_WIN_ID_NULL;
						}
					}	
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET))
				{
					DCDC_PANEL_CFG_T *pt_dcdc_panel = &(g_t_share_data.t_sys_cfg.t_dcdc_panel);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_dcdc_panel->t_feeder_module[0]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//�����֧·����
					pt_dcdc_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_dcdc_panel->u8_feeder_module_num; i++)
					{
						if (pt_dcdc_panel->u8_feeder_branch_num + pt_dcdc_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_dcdc_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_dcdc_panel->u8_feeder_branch_num;
							pt_dcdc_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_dcdc_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_dcdc_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcdc_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcdc_panel->t_feeder_module[i].u8_state_feeder_num > pt_dcdc_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcdc_panel->t_feeder_module[i].u8_state_feeder_num = pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcdc_panel->t_feeder_module[i].u8_curr_feeder_num > pt_dcdc_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcdc_panel->t_feeder_module[i].u8_curr_feeder_num = pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_dcdc_panel->u8_feeder_branch_num += pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
						}

					}
					
					pt_dcdc_panel->u8_state_feeder_num = pt_dcdc_panel->u8_feeder_branch_num;
					pt_dcdc_panel->u8_alarm_feeder_num = pt_dcdc_panel->u8_feeder_branch_num;
					pt_dcdc_panel->u8_curr_feeder_num = pt_dcdc_panel->u8_feeder_branch_num;
					
						
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
					{
						if (pt_dcdc_panel->u8_feeder_module_num > 0)
						{
							win->u16_id_prev = MMI_WIN_ID_DCDC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
							
						}
						else
						{
							win->u16_id_prev = MMI_WIN_ID_DCDC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET))
				{
					DCAC_PANEL_CFG_T *pt_dcac_panel = &(g_t_share_data.t_sys_cfg.t_dcac_panel);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_dcac_panel->t_feeder_module[0]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//�����֧·����
					pt_dcac_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_dcac_panel->u8_feeder_module_num; i++)
					{
						if (pt_dcac_panel->u8_feeder_branch_num + pt_dcac_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_dcac_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_dcac_panel->u8_feeder_branch_num;
							pt_dcac_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_dcac_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_dcac_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcac_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcac_panel->t_feeder_module[i].u8_state_feeder_num > pt_dcac_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcac_panel->t_feeder_module[i].u8_state_feeder_num = pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcac_panel->t_feeder_module[i].u8_curr_feeder_num > pt_dcac_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcac_panel->t_feeder_module[i].u8_curr_feeder_num = pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_dcac_panel->u8_feeder_branch_num += pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
						}

					}
					
					pt_dcac_panel->u8_state_feeder_num = pt_dcac_panel->u8_feeder_branch_num;
					pt_dcac_panel->u8_alarm_feeder_num = pt_dcac_panel->u8_feeder_branch_num;
					pt_dcac_panel->u8_curr_feeder_num = pt_dcac_panel->u8_feeder_branch_num;
					
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
					{
						if (pt_dcac_panel->u8_feeder_module_num > 0)
						{
							win->u16_id_prev = MMI_WIN_ID_DCAC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
							
						}
						else
						{
							win->u16_id_prev = MMI_WIN_ID_DCAC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_SET))
				{
					AC_FEEDER_CFG_T *pt_ac_panel = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_ac_panel->t_feeder_module[m_t_win_record.u_ordinal.u16_feeder_module]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//�����֧·����
					pt_ac_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_ac_panel->u8_feeder_module_num; i++)
					{
						if (pt_ac_panel->u8_feeder_branch_num + pt_ac_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_ac_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_ac_panel->u8_feeder_branch_num;
							pt_ac_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_ac_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_ac_panel->t_feeder_module[i].u8_feeder_num)
								pt_ac_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_ac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_ac_panel->t_feeder_module[i].u8_state_feeder_num > pt_ac_panel->t_feeder_module[i].u8_feeder_num)
								pt_ac_panel->t_feeder_module[i].u8_state_feeder_num = pt_ac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_ac_panel->t_feeder_module[i].u8_curr_feeder_num > pt_ac_panel->t_feeder_module[i].u8_feeder_num)
								pt_ac_panel->t_feeder_module[i].u8_curr_feeder_num = pt_ac_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_ac_panel->u8_feeder_branch_num += pt_ac_panel->t_feeder_module[i].u8_feeder_num;
						}

					}
					
					pt_ac_panel->u8_state_feeder_num = pt_ac_panel->u8_feeder_branch_num;
					pt_ac_panel->u8_alarm_feeder_num = pt_ac_panel->u8_feeder_branch_num;
					pt_ac_panel->u8_curr_feeder_num = pt_ac_panel->u8_feeder_branch_num;
					
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
					{
						if (pt_ac_panel->u8_feeder_module_num > 0)
						{
							win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;							
						}
						else
						{
							win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;
						}
					}
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER1_SET)
				{
					v_disp_batt1_data_init();
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER2_SET)
				{
					BATT_CFG_T *pt_batt_cfg = &(g_t_share_data.t_sys_cfg.t_batt);
					
					pt_batt_cfg->t_batt_group[1].u8_cell_total_num = 0;
					for (i=0; i<pt_batt_cfg->t_batt_group[1].u8_bms_num; i++)
						pt_batt_cfg->t_batt_group[1].u8_cell_total_num += pt_batt_cfg->t_batt_group[1].u8_cell_num[i];
					
					if (pt_batt_cfg->t_batt_group[1].u8_cell_total_num > BATT_CELL_MAX)
						pt_batt_cfg->t_batt_group[1].u8_cell_total_num = BATT_CELL_MAX;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_ADJUST)      //����У׼ҳ��
				{
					os_evt_set(1 << (m_t_win_record.u8_sel_index - 1), g_tid_ac_sample);
					index_back = m_t_win_record.u8_sel_index;
				
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //��ʾУ׼���ڽ���
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_AC_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_VOLT_ADJUST)  //ֱ����ѹУ׼ҳ��
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_BATT_VOLT, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_PB_VOLT, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 4)
						os_evt_set(DC_ADJUST_CB_VOLT, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 5)
						os_evt_set(DC_ADJUST_BUS_NEG_TO_END_VOLT, g_tid_dc_sample);

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //��ʾУ׼���ڽ���
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_DC_VOLT_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_LOAD_CURR_ADJUST)    //���ص���У׼ҳ��
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_LOAD_CURR_1, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_LOAD_CURR_2, g_tid_dc_sample);

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //��ʾУ׼���ڽ���
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_LOAD_CURR_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT1_CURR_ADJUST)   //һ���ص���У׼ҳ��
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_BATT1_CURR_1, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_BATT1_CURR_2, g_tid_dc_sample);

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //��ʾУ׼���ڽ���
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_BATT1_CURR_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT2_CURR_ADJUST)   //�����ص���У׼ҳ��
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_BATT2_CURR_1, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_BATT2_CURR_2, g_tid_dc_sample);
					
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //��ʾУ׼���ڽ���
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_BATT2_CURR_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DISPLAY)   // ��ʾ���ý���
				{
					if (m_t_win_record.u8_sel_index == 1)
					{
						m_e_lang = g_t_share_data.t_sys_cfg.t_sys_param.e_lang;
					}
					else if (m_t_win_record.u8_sel_index == 2)
					{
						m_e_lcd_direction = g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection;
						
						if (m_e_lcd_direction == LCD_HORIZONTAL)   //������ʾ�����ʼ������ֵ
						{
							g_u16_key_cancel = KEY_K7;
							g_u16_key_enter = KEY_K6;
							g_u16_key_up = KEY_K4;
							g_u16_key_down = KEY_K5;
							g_u16_key_add = KEY_K3;
							g_u16_key_sub = KEY_K2;
						}
						else
						{
							g_u16_key_cancel = KEY_K7;
							g_u16_key_enter = KEY_K6;
							g_u16_key_up = KEY_K3;
							g_u16_key_down = KEY_K2;
							g_u16_key_add = KEY_K5;
							g_u16_key_sub = KEY_K4;
						}
					}
				}

				v_fetch_save_cfg_data();                                   //�����������ݵ�dataflash                                  
			}
			else              //�������ݷ�Χ���ޣ���ʾ���ݳ�����Χ
			{
				U32_T u32_val_min, u32_val_max;
				U8_T u8_val_type;
				
				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
					m_u16_feeder_module_ordianl = m_t_win_record.u_ordinal.u16_feeder_module;

				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
					m_u16_feeder_module_ordianl = m_t_win_record.u_ordinal.u16_feeder_module;

				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
					m_u16_feeder_module_ordianl = m_t_win_record.u_ordinal.u16_feeder_module;
				
				id_back = m_t_win_record.u16_curr_id;
				index_back = m_t_win_record.u8_sel_index;
				win->t_item[index_back].pv_val = m_t_win_record.pv_back_item_val;
				u32_val_min = win->t_item[index_back].u32_val_min;
				u32_val_max = win->t_item[index_back].u32_val_max;
				u8_val_type = win->t_item[index_back].u8_val_type;

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_OUT_RANGE;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				
				if ((u8_val_type == MMI_VAL_TYPE_F32_1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_3W1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_4W1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_6W1P))	    //�����ӡ��1λС��
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%.1f", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_max/1000.0);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]),
							"%s%.1f", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_min/1000.0);
				}
				else if ((u8_val_type == MMI_VAL_TYPE_F32_2P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_4W2P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W2P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_6W2P))	    //�����ӡ��2λС��
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%.2f", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_max/1000.0);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]), 
							"%s%.2f", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_min/1000.0);
				}
				else if ((u8_val_type == MMI_VAL_TYPE_F32_3P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W3P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_6W3P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W3P))	    //�����ӡ��3λС��
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%.3f", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_max/1000.0);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]),
							"%s%.3f", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_min/1000.0);
				}
				else
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%d", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), u32_val_max);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]),
							"%s%d", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), u32_val_min);
				}

				win = &(m_t_win_record.t_curr_win);
				win->u16_id_father = id_back;
				win->u8_sel_father = index_back;
			}
		}

		else if (u16_key_val == g_u16_key_up)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_U8_2BIT: 
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:
				case MMI_VAL_TYPE_F32_3W1P: //����4���������2������λ���ϲ�����
					v_disp_bit_index_sub(2);
					break;

				case MMI_VAL_TYPE_U8_3BIT: 
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:
				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_4W2P: //����5���������3������λ���ϲ�����
					v_disp_bit_index_sub(3);
					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:
				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_5W2P:
				case MMI_VAL_TYPE_F32_5W3P: //����5���������4������λ���ϲ�����
					v_disp_bit_index_sub(4);
					break;

				case MMI_VAL_TYPE_U32_5BIT:
				case MMI_VAL_TYPE_F32_6W1P:
				case MMI_VAL_TYPE_F32_6W2P:
				case MMI_VAL_TYPE_F32_6W3P: //����3���������5������λ���ϲ�����
					v_disp_bit_index_sub(5);
					break;

				case MMI_VAL_TYPE_F32_7W3P: //6������λ
					v_disp_bit_index_sub(6);
					break;
				
				default:
					break;
			}
		}

		else if (u16_key_val == g_u16_key_down)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_U8_2BIT: 
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:
				case MMI_VAL_TYPE_F32_3W1P: //����4���������2������λ���ϲ�����
					v_disp_bit_index_add(2);
					break;

				case MMI_VAL_TYPE_U8_3BIT:
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:
				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_4W2P: //����5���������3������λ���ϲ�����
					v_disp_bit_index_add(3);
					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:
				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_5W2P:
				case MMI_VAL_TYPE_F32_5W3P: //����5���������4������λ���ϲ�����
					v_disp_bit_index_add(4);
					break;

				case MMI_VAL_TYPE_U32_5BIT:
				case MMI_VAL_TYPE_F32_6W1P:
				case MMI_VAL_TYPE_F32_6W2P:
				case MMI_VAL_TYPE_F32_6W3P: //����3���������5������λ���ϲ�����
					v_disp_bit_index_add(5);
					break;

				case MMI_VAL_TYPE_F32_7W3P: //6������λ
					v_disp_bit_index_add(6);
					break;
				
				default:
					break;
			}
		}

		else if (u16_key_val == g_u16_key_add)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_ENUM:
					m_t_win_record.u32_set_value += 1;
					if (m_t_win_record.u32_set_value > win->t_item[m_t_win_record.u8_sel_index].u32_val_max)
					{
						m_t_win_record.u32_set_value = win->t_item[m_t_win_record.u8_sel_index].u32_val_min;
					}
					break;

				case MMI_VAL_TYPE_U8:
				case MMI_VAL_TYPE_U16:
				case MMI_VAL_TYPE_U32:	                      // ����3�����ֻ��1������λ���ϲ�����
					if (m_t_win_record.u32_set_value < 9)
					{
						m_t_win_record.u32_set_value += 1;
					}
					else
					{
						m_t_win_record.u32_set_value = 0;
					}	
					break;

				case MMI_VAL_TYPE_U8_2BIT:
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:                   // ����3�������2������λ���ϲ�����	       
					if (m_t_win_record.u8_bit_index == 0)	      // ʮλ     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)   // ��λ
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U8_3BIT:
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:	                  // ����3�������3������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	      // ��λ     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	   // ʮλ     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // ��λ
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:	                   // ����2�������4������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ǧλ     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // ��λ     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // ʮλ     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // ��λ
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_U32_5BIT:	                   // 5������λ����������Ҫ������������
					if (m_t_win_record.u8_bit_index == 0)	       // ��λ     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // ǧλ     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // ��λ     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // ʮλ     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // ��λ
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;
					
				case MMI_VAL_TYPE_F32_3W1P:
				case MMI_VAL_TYPE_F32_4W2P:         // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
				case MMI_VAL_TYPE_F32_5W3P:         // ����3�������1������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	  // ��λ*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // ʮ��λ*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // ǧ��λ*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_5W2P:             // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
				case MMI_VAL_TYPE_F32_6W3P:             // ����3�������2������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ʮλ*1000     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // ��λ*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // ʮ��λ*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // ǧ��λ*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_6W2P:             // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
				case MMI_VAL_TYPE_F32_7W3P:             // ����3�������3������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ��λ*1000     
					{
						v_disp_data_bit_add(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // ʮλ*1000     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // ��λ*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // ʮ��λ*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)    // ǧ��λ*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_6W1P:             // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
					if (m_t_win_record.u8_bit_index == 0)	       // ǧλ*1000     
					{
						v_disp_data_bit_add(MMI_1000000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	       // ��λ*1000     
					{
						v_disp_data_bit_add(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // ʮλ*1000     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	    // ��λ*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // ʮ��λ*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 6)    // ǧ��λ*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				default:
					break;
			}
		}																																																				

		else if (u16_key_val == g_u16_key_sub)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_ENUM:
					if (m_t_win_record.u32_set_value > win->t_item[m_t_win_record.u8_sel_index].u32_val_min)
					{
						m_t_win_record.u32_set_value -= 1;
					}
					else
					{
						m_t_win_record.u32_set_value = win->t_item[m_t_win_record.u8_sel_index].u32_val_max;
					}
					break;

				case MMI_VAL_TYPE_U8:
				case MMI_VAL_TYPE_U16:
				case MMI_VAL_TYPE_U32:	                      // ����3�����ֻ��1������λ���ϲ�����
					if (m_t_win_record.u32_set_value > 0)
					{
						m_t_win_record.u32_set_value -= 1;
					}
					else
					{
						m_t_win_record.u32_set_value = 9;
					}	
					break;

				case MMI_VAL_TYPE_U8_2BIT:
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:                   // ����3�������2������λ���ϲ�����	       
					if (m_t_win_record.u8_bit_index == 0)	      // ʮλ     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)   // ��λ
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U8_3BIT:
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:	                  // ����3�������3������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	      // ��λ     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	   // ʮλ     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // ��λ
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:	                   // ����2�������4������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ǧλ     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // ��λ     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // ʮλ     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // ��λ
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_U32_5BIT:                    // 5������λ����������Ҫ������������
					if (m_t_win_record.u8_bit_index == 0)	       // ��λ     
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}                   
					else if (m_t_win_record.u8_bit_index == 1)	   // ǧλ     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // ��λ     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // ʮλ     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // ��λ
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;
					
				case MMI_VAL_TYPE_F32_3W1P:
				case MMI_VAL_TYPE_F32_4W2P:			   // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
				case MMI_VAL_TYPE_F32_5W3P:              // ����3�������1������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // ʮ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // ǧ��λ*1000
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_5W2P:			   // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
				case MMI_VAL_TYPE_F32_6W3P:              // ����3�������2������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ʮλ*1000     
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // ʮ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // ǧ��λ*1000
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_6W2P:			   // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
				case MMI_VAL_TYPE_F32_7W3P:              // ����3�������3������λ���ϲ�����
					if (m_t_win_record.u8_bit_index == 0)	       // ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // ʮλ*1000    
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // ʮ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)    // ǧ��λ*1000
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_6W1P:			   // ����ֱ���ø��㴦����о�����ʧ��������ݲ��ԣ�������*1000ת��Ϊ���ν��д���
					if (m_t_win_record.u8_bit_index == 0)	       // ǧλ*1000     
					{
						v_disp_data_bit_sub(MMI_1000000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)     // ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // ʮλ*1000    
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	    // ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // ʮ��λ*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)	   // �ٷ�λ*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 6)    // ǧ��λ*1000
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				default:
					break;
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
��������: v_disp_main_win_key_handler		           				
��������: �����ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_main_win_key_handler(U16_T u16_key_val)
{
	U16_T id_back;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_MAIN_MENU;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u8_sel_index = 1;
	}
	else if (u16_key_val == g_u16_key_up)
	{
		id_back = win->u16_id_prev;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));			
			m_t_win_record.u16_curr_id = id_back;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
	else if (u16_key_val == g_u16_key_down)
	{
		id_back = win->u16_id_next;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));			
			m_t_win_record.u16_curr_id = id_back;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
}

/*************************************************************
��������: v_disp_about_key_handler		           				
��������: ���ڴ��ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_about_key_handler(U16_T u16_key_val)
{
	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
}

/*************************************************************
��������: v_disp_restore_param_key_handler		           				
��������: �ָ�Ĭ�ϲ������ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_restore_param_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		win->u16_id_father = MMI_WIN_ID_PARAM_RESTORE;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		U32_T num;

		os_mut_wait(g_mut_share_data, 0xFFFF);
		num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		os_mut_release(g_mut_share_data);
				
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;

		if (num > 0)
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_RESTORE;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
		else
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD_CLEAR;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
}

/*************************************************************
��������: v_disp_restore_batt_cap_key_handler		           				
��������: �ָ�����������ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_restore_batt_cap_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_BATT_RESTORE;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_RESTORE;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
��������: v_disp_clear_record_key_handler		           				
��������: �����س�ŵ��¼���ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_clear_record_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_RECORD_CLEAR;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		U32_T num;

		os_mut_wait(g_mut_share_data, 0xFFFF);
		num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		os_mut_release(g_mut_share_data);

		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;

		if (num > 0)
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_RESTORE;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
		else
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_RESTORE;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_EXCEPTION_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
��������: v_disp_clear_his_record_key_handler		           				
��������: �����ʷ��¼���ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_clear_his_fault_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_HIS_FAULT_CLEAR;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_EXCEPTION_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_RESTORE;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}


/*************************************************************
��������: v_disp_clear_no_resume_falut_key_handler		           				
��������: �����ʷ��¼���ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_clear_no_resume_falut_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_EXCEPTION_CLEAR;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}


/*************************************************************
��������: v_disp_confirm_key_handler		           				
��������: ȷ�ϲ�����ʾ���ڰ���������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_confirm_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U16_T u16_id_father = win->u16_id_father;

	if (u16_key_val == g_u16_key_enter)
	{
		switch (u16_id_father)
		{
			case MMI_WIN_ID_PARAM_RESTORE:
				v_disp_cfg_data_restore();      //ִ�лָ�Ĭ�ϲ�������

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_PARAM_RESTORE;
				break;

			case MMI_WIN_ID_HIS_FAULT_CLEAR:
				v_fault_clear_his_fault();      //ִ�������ʷ�澯��¼����

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_HIS_FAULT_CLEAR;
				break;

			case MMI_WIN_ID_EXCEPTION_CLEAR:
				v_fault_clear_no_resume_fault();      //ִ���������ĩ����澯��¼����

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_EXCEPTION_CLEAR;
				break;

			case MMI_WIN_ID_BATT_RESTORE:
				os_evt_set(BATT_CAPACITY_RESTORE, g_tid_batt);   //���͵�������ָ��¼���־����ع�������

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_BATT_RESTORE;
				break;

			case MMI_WIN_ID_RECORD_CLEAR:
				v_log_clear_record();            //ִ������¼���¼����

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_RECORD_CLEAR;
				break;

			default:
				break;
		}
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = u16_id_father;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
	}
}


/*************************************************************
��������: v_disp_key_handler		           				
��������: ����������						
�������: u16_key_val -- ����ֵ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_key_handler(U16_T u16_key_val)
{
	switch (m_t_win_record.u16_curr_id >> 8)             //����ҳ�����ͣ����ദ��
	{
		case MMI_WIN_TYPE_MAIN_WIN:
			v_disp_main_win_key_handler(u16_key_val);
			break;
		
		case MMI_WIN_TYPE_MENU:
			v_disp_menu_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_PASSWORD:
			v_disp_password_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_SET:
			v_disp_set_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_ABOUT:
			v_disp_about_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_PARAM_RESTORE:
			v_disp_restore_param_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_BATT_CAP_RESTORE:
			v_disp_restore_batt_cap_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_RECORD_CLEAR:
			v_disp_clear_record_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_HIS_FAULT_CLEAR:
			v_disp_clear_his_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_EXCEPTION_CLEAR:
			v_disp_clear_no_resume_falut_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_RUN_INFO:
			v_disp_run_info_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_CELL1_VOLT:
			v_disp_cell1_volt_key_handler(u16_key_val);
			break;
		
		case MMI_WIN_TYPE_CELL2_VOLT:
			v_disp_cell2_volt_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_RECT:
			v_disp_rect_info_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_SWITCH:
			v_disp_swt_info_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_EC_SWITCH:
			v_disp_ecswt_info_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_CURR_FAULT:
			v_disp_curr_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_HIS_FAULT:
			v_disp_his_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_EXCEPTION:
			v_disp_excption_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_BATT_RECORD:
			v_disp_record_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_CONFIRM:
			v_disp_confirm_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DC_FEEDER:
			v_disp_dc_feeder_key_handler(u16_key_val);
			break;
		
		case MMI_WIN_TYPE_DCDC_MODULE:
			v_disp_dcdc_module_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DCAC_MODULE:
			v_disp_dcac_module_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DCDC_FEEDER:
			v_disp_dcdc_feeder_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DCAC_FEEDER:
			v_disp_dcac_feeder_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_AC_FEEDER:
			v_disp_ac_feeder_key_handler(u16_key_val);
			break;
			
		default:
			break;
	}
}

/*************************************************************
��������: v_disp_adjust_donig_handler		           				
��������: У׼���ڽ���ҳ���¼�����						
�������: u16_flags -- У׼�ɹ�����ʧ�ܱ�־        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_disp_adjust_donig_handler(U16_T u16_flag)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U16_T u16_id_father = win->u16_id_father;
	U8_T  u8_sel_index = win->u8_sel_father;

	if (u16_id_father == MMI_WIN_ID_AC_ADJUST)
	{
		if (u16_flag & AC_ADJUST_SCUESS)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_SCUESS;  //��ʾУ׼�ɹ�
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = MMI_WIN_ID_AC_ADJUST;
			win->u8_sel_father = u8_sel_index;
		}
		else if (u16_flag & AC_ADJUST_FAIL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_FAIL;   //��ʾУ׼ʧ��
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = MMI_WIN_ID_AC_ADJUST;
			win->u8_sel_father = u8_sel_index;
		}
	}
	else if ((u16_id_father == MMI_WIN_ID_DC_VOLT_ADJUST)
	         || (u16_id_father == MMI_WIN_ID_LOAD_CURR_ADJUST)
			 || (u16_id_father == MMI_WIN_ID_BATT1_CURR_ADJUST)
			 || (u16_id_father == MMI_WIN_ID_BATT2_CURR_ADJUST)) 
	{
		if (u16_flag & DC_ADJUST_SCUESS)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_SCUESS;  //��ʾУ׼�ɹ�
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = u16_id_father;
			win->u8_sel_father = u8_sel_index;
		}
		else if (u16_flag & DC_ADJUST_FAIL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_FAIL;  //��ʾУ׼ʧ��
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = u16_id_father;
			win->u8_sel_father = u8_sel_index;
		}
		else if (u16_flag & DC_ADJUST_FIRST_CURR_COMPLETE)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_NEXT_CURR;  //��ʾУ׼��һ��������
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = u16_id_father;
			win->u8_sel_father = u8_sel_index;
		}
	}
}

/*************************************************************
��������: v_disp_display_task		           				
��������: ��ʾ�Ͱ����������񣬸������ͨ��RTX����os_tsk_create������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
__task void v_disp_display_task(void)
{
	U16_T flags;
	U32_T blink_cnt = 0;
	U32_T u32_time;
	BOOL_T b_backlight = 1;
	BOOL_T b_buzzer = 0;

	v_disp_display_init();

	while (1)
	{
		os_evt_set(DISPLAY_FEED_DOG, g_tid_wdt);             //����ι���¼���־
		
		//У׼���ڽ�����ʾҳ�棬��ȡУ׼�ɹ�����ʧ�ܱ�־
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ADJUST_DOING)
		{
			if (os_evt_wait_or(ADJUST_FLAGS, 0) == OS_R_EVT)
			{ 
				flags = os_evt_get();
				v_disp_adjust_donig_handler(flags);
			}
		}

		//��ȡ���Ϸ������¼���־
		if (os_evt_wait_or(BUZZER_EVT_FLAGS, 0) == OS_R_EVT)
		{
			flags = os_evt_get();

			if (flags & FAULT_BUZZER_BEEP)
			{
				b_buzzer = 1;
			}
			else if (flags & FAULT_BUZZER_QUEIT)
			{
				b_buzzer = 0;
			}

			if (flags & FAULT_OCCURE)
			{
				u32_time = u32_delay_get_timer_val();        //�澯��������������ǹرյģ���򿪱���
				if (b_backlight == 0)
				{
					v_relay_relay_operation(ENABLE_BACKLIGHT);
					b_backlight = 1;
				}
			}
		}
		
		//�ȴ������¼���־		 
		if (os_evt_wait_or(KEY_EVT_FLAGS, MMI_REFRESH_INTERVAL) == OS_R_EVT)
		{ 
			flags = os_evt_get();

			if ((b_buzzer == 0) && (b_backlight == 1))
			{
				if (flags & g_u16_key_cancel)	
					v_disp_key_handler(g_u16_key_cancel);

				if (flags & g_u16_key_enter)
					v_disp_key_handler(g_u16_key_enter);

				if (flags & g_u16_key_down)
					v_disp_key_handler(g_u16_key_down);

				if (flags & g_u16_key_up)
					v_disp_key_handler(g_u16_key_up);

				if (flags & g_u16_key_sub)
					v_disp_key_handler(g_u16_key_sub);

				if (flags & g_u16_key_add)
					v_disp_key_handler(g_u16_key_add);
			}

			if (b_buzzer == 1)                                //����������������У��رշ�����
			{
				v_relay_relay_operation(BEEP_OFF);
				b_buzzer = 0;
			}

			if (b_backlight == 0)                            //����Ǻ���״̬����򿪱���
			{
				v_relay_relay_operation(ENABLE_BACKLIGHT);
				b_backlight = 1;
				u32_time = u32_delay_get_timer_val();
			}

			u32_time = u32_delay_get_timer_val();
		}
		else
		{
			if ((b_backlight == 1)                  //��ʱ��û�а������£����عر���������
				&& (u32_delay_time_elapse(u32_time, u32_delay_get_timer_val()) >= MMI_RETURN_MAIN_WIN_TIME))
			{
				if (m_t_win_record.u16_curr_id != MMI_WIN_ID_MAIN_WINDOW)
				{
					if (m_t_win_record.u8_win_status == MMI_WIN_SET)       //���������״̬��������Ŀ������ָ����Ҫ�ָ�
					{
						MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
						win->t_item[m_t_win_record.u8_sel_index].pv_val = m_t_win_record.pv_back_item_val;
					}

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_MAIN_WINDOW;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
					{
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
						m_t_win_record.t_curr_win.u8_icon_cnt = 1;
					}
				}

				v_relay_relay_operation(DISABLE_BACKLIGHT);
				b_backlight = 0;

				if (b_buzzer == 1)                                   //����������������У��رշ�����
				{
					v_relay_relay_operation(BEEP_OFF);
					b_buzzer = 0;
				}
			}
		}

		if ((m_t_win_record.u8_win_status == MMI_WIN_SET)
			|| ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD))
		{
			if ((++blink_cnt) % 3 == 0)
			{
				blink_cnt = 0;
				if (m_t_win_record.u8_set_blink == MMI_NORMAL_DISP)
				{
			 		m_t_win_record.u8_set_blink = MMI_INVERSE_DISP;
				}
				else
				{
			 		m_t_win_record.u8_set_blink = MMI_NORMAL_DISP;
				}
			}
		} 

		v_disp_display_update();
	}
}
