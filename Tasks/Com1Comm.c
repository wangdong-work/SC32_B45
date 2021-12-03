/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Com1Comm.c
��    ����1.00
�������ڣ�2012-08-01
��    �ߣ�������
�����������¼�ģ���ͨ�ż����ݴ�������ʵ��


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-08-01  1.00     ����
**************************************************************/


#include <rtl.h>
#include <string.h>
#include <math.h>

#include "Type.h"
#include "PublicData.h"
#include "Com1Comm.h"
#include "Crc.h"
#include "FaultId.h"

#include "../Drivers/uart_device.h"
#include "../Drivers/Delay.h"


//#define COM1_DEBUG

#ifdef COM1_DEBUG
	#define DEBUG(fmt, ...) u32_usb_debug_print(fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif

#define COM1_RC10_CTRL_SPACE_TM     (2*OSC_SECOND)   //2s��ٿ���������ʱ��
#define COM1_SWT_SYNC_UPDATE_TM     (30*OSC_SECOND)  //10s��ٿ���������ʱ��

#define COM1_RECV_WAIT_TICK         5      //ÿ�εȴ�5��TICK��Ҳ����50ms
#define COM1_RECV_TIMEOUT_CNT       20     //��ʱ�������ֵΪ20����ʱֵΪ1s

#define COM1_COMM_WAIT_TICK         10     //������ڻ�����д����ȥ���ݣ��ȴ�100MS������
#define COM1_BROADCAST_WAIT_TICK    30     //���͹㲥����ȴ�300MS��Ȼ������һ������

//#define COM1_FAIL_MAX_NUM           10     //����10��ͨ�Ų��ϣ���ͨ�Ź���
#define COM1_COM_PORT_NUM           1      //���ڶ˿ں�

#define COM1_RX_BUF_SIZE            256    //�������Ĵ�С
#define COM1_TX_BUF_SIZE            56     //�������Ĵ�С

#define COM1_GROUP1_RECT			0		//��һ����ģ��
#define COM1_GROUP2_RECT			1		//�ڶ�����ģ��

/* ���ͺͽ��ջ��������� */
static U8_T m_u8_com1_rx_buf[COM1_RX_BUF_SIZE];
static U8_T m_u8_com1_tx_buf[COM1_TX_BUF_SIZE];



/* ����� */
typedef struct
{
	U8_T u8_func_code;          //�����룬03--��ѯ���ݣ�06--��������
	U16_T u16_reg_addr;         //�Ĵ�����ַ�����ֽ��ȷ�
	U16_T u16_data_or_num;      //������Ϊ03ʱ����ʾ��ѯ�ļĴ�������
	                            //������Ϊ06ʱ����ʾ���õ�����
}MODULE_CMD_T;

/* ģ�����ݼ�¼ */
typedef struct
{
	U8_T u8_module_index;       //ģ������
	U8_T u8_cmd_index;          //��������
	U8_T u8_com1_fail_cnt;      //ͨ��ʧ�ܼ�����ʧ�ܴ�������10�α�ͨ���жϹ���
}MODULE_RECORD_T;



/******************************* ���Ѳ����ض��忪ʼ ***************************/

typedef struct
{
	U8_T u8_total_index;        //���������е��Ѳ���е�����
	U8_T u8_start_addr;         //������ڵ��Ѳ����ʼ��ַ
	U8_T u8_group_index;        //���������
	U8_T u8_module_index;       //ģ������
	U8_T u8_cmd_index;          //��������
	U8_T u8_com1_fail_cnt;      //ͨ��ʧ�ܼ�����ʧ�ܴ�������10�α�ͨ���жϹ���
}BMS_RECORD_T;
	
#define BMS_B21_MAX_CELL_NUM              24     //B21����������ؽ���
#define BMS_START_ADDR                    0x70   //���Ѳ����ʼ��ַ
#define BMS_READ_CMD                      0      //��ѯ������������
#define BMS_CMD_CNT                       1      //���Ѳ������ĸ���


static U8_T m_u8_bms_offline_cnt;          // ���Ѳ��ͨѶ�ж��������

static MODULE_CMD_T m_t_bms_cmd[BMS_CMD_CNT] =
{
	{ 3, 0, 114 },   //��ѯ������ݣ�114�����Ĵ������������ɰ�ʵ�����ã���Ӧ�ļ�С
};

static BMS_RECORD_T m_t_bms_group1_record[BATT_METER_MAX] =
{
	{ 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 1, 0, 0 },
	{ 2, 0, 0, 2, 0, 0 },
	{ 3, 0, 0, 3, 0, 0 },
	{ 4, 0, 0, 4, 0, 0 },
};

static BMS_RECORD_T m_t_bms_group2_record[BATT_METER_MAX] =
{
	{ 5, 5, 1, 0, 0, 0 },
	{ 6, 5, 1, 1, 0, 0 },
	{ 7, 5, 1, 2, 0, 0 },
	{ 8, 5, 1, 3, 0, 0 },
	{ 9, 5, 1, 4, 0, 0 }
};
/******************************* ���Ѳ����ض������ ***************************/


/******************************* ����ģ����ض��忪ʼ ***************************/
#define RECT_MODULE_START_ADDR             0x00   //����ģ����ʼ��ַ
#define RECT_MODULE_BROADCAST_ADDR         0xFF   //һ������ģ��㲥��ַ
                                           
#define RECT_MODULE_READ_CMD               0      //����ģ���ѯ������������
#define RECT_MODULE_SET_STATE_CMD          1      //����ģ������ģ��״̬��������
#define RECT_MODULE_NORMAL_CMD_CNT         2      //����ģ������ĸ���

#define RECT_MODULE_SET_VOLT_CMD           0      //����ģ���ѹ�����������㲥����
#define RECT_MODULE_SET_CURR_CMD           1      //����ģ�����������������㲥����
#define RECT_MODULE_SET_HIGH_VOLT_CMD      2      //����ģ������ģ�������ѹ���ޣ��㲥����
#define RECT_MODULE_SET_LOW_VOLT_CMD       3      //����ģ������ģ�������ѹ���ޣ��㲥����
#define RECT_MODULE_SET_DEF_VOLT_CMD       4      //����ģ������Ĭ�������ѹ�����������㲥����       
#define RECT_MODULE_BROADCAST_CMD_CNT      5      //����ģ������ĸ���

#define RECT_MODULE_OVER_VOLT_PROTECT_MASK 0x8000 //����ģ���ѹ����״̬������
#define RECT_MODULE_FAULT_MASK             0x041C //����ģ�����״̬�����룬�������Ƿѹ״̬�������ѹ״̬λ��ģ�����״̬������״̬λ
#define RECT_MODULE_VOER_TEMPERATURE_MASK  0x0800 //����ģ�����״̬������
#define RECT_MODULE_AC_EXCEPTION_MASK      0x3000 //����ģ�齻���쳣������
#define RECT_MODULE_EXCEPTION_MASK         (RECT_MODULE_OVER_VOLT_PROTECT_MASK | RECT_MODULE_FAULT_MASK \
											| RECT_MODULE_VOER_TEMPERATURE_MASK | RECT_MODULE_AC_EXCEPTION_MASK)


static U8_T m_u8_rect_offline_cnt;         // ����ģ��ͨѶ�ж��������

static MODULE_CMD_T m_t_rect_module_normal_cmd[RECT_MODULE_NORMAL_CMD_CNT] = 
{
	{ 3, 0, 6    },              //��ѯ��������
	{ 6, 5, 0    },              //����ģ�鿪��״̬������
};

static MODULE_CMD_T m_t_rect_module_broadcast_cmd[RECT_MODULE_BROADCAST_CMD_CNT] =
{
	{ 6, 0, 2200 },              //����ģ�������ѹ���㲥���Ĭ��220V��ϵ��Ϊ0.1��220/0.1=2200
	{ 6, 2, 1100 },              //����ģ�������㣬�㲥�����Χ5%~110%��Ĭ��110%��ϵ��Ϊ0.1��110/0.1=1100
	{ 6, 3, 2860 },              //����ģ�������ѹ���ޣ��㲥���Ĭ��286V��ϵ��Ϊ0.1��286/0.1=2860
	{ 6, 4, 1870 },              //����ģ�������ѹ���ޣ��㲥���Ĭ��187V��ϵ��Ϊ0.1��187/0.1=1870
	{ 6, 6, 2200 },              //����Ĭ�������ѹ����㲥���Ĭ��220V��ϵ��Ϊ0.1��220/0.1=2200
};

static MODULE_RECORD_T m_t_rect_module_record[RECT_CNT_MAX] =
{
	{ 0,  0, 0 },
	{ 1,  0, 0 },
	{ 2,  0, 0 },
	{ 3,  0, 0 },
	{ 4,  0, 0 },
	{ 5,  0, 0 },
	{ 6,  0, 0 },
	{ 7,  0, 0 },
	{ 8,  0, 0 },
	{ 9,  0, 0 },
	{ 10, 0, 0 },
	{ 11, 0, 0 },
	{ 12, 0, 0 },
	{ 13, 0, 0 },
	{ 14, 0, 0 },
	{ 15, 0, 0 },
	{ 16, 0, 0 },
	{ 17, 0, 0 },
	{ 18, 0, 0 },
	{ 19, 0, 0 },
	{ 20, 0, 0 },
	{ 21, 0, 0 },
	{ 22, 0, 0 },
	{ 23, 0, 0 },
};

//��������ģ���һЩ��������
static F32_T  m_f32_rect_out_volt[2];              //�����ѹ
static F32_T  m_f32_rect_curr_percent[2];          //�����ٷֱ�
static F32_T  m_u16_pb_high_volt[2];               //�����ѹ����
static F32_T  m_u16_pb_low_volt[2];                //�����ѹ����
static F32_T  m_f32_rect_offline_out_volt[2];      //Ĭ�������ѹ
static U16_T  m_u16_rect_ctrl[RECT_CNT_MAX];    //���ģ��״̬���ƣ�1���ػ���0������

/******************************* ����ģ����ض������ ***************************/


/******************************* ͨ��ģ����ض��忪ʼ ***************************/
#define DCDC_MODULE_START_ADDR             0x90   //ͨ��ģ����ʼ��ַ
#define DCDC_MODULE_BROADCAST_ADDR         0xF2   //һ��ͨ��ģ��㲥��ַ
                                           
#define DCDC_MODULE_READ_CMD               0      //ͨ��ģ���ѯ������������
#define DCDC_MODULE_NORMAL_CMD_CNT         1      //ͨ��ģ������ĸ���

#define DCDC_MODULE_SET_VOLT_CMD           0      //ͨ��ģ���ѹ�����������㲥����
#define DCDC_MODULE_SET_CURR_CMD           1      //ͨ��ģ�����������������㲥����
#define DCDC_MODULE_SET_DEF_VOLT_CMD       2      //ͨ��ģ������Ĭ�������ѹ�����������㲥����       
#define DCDC_MODULE_BROADCAST_CMD_CNT      3      //ͨ��ģ������ĸ���


static U8_T m_u8_dcdc_offline_cnt;         // ͨ��ģ��ͨѶ�ж��������

static MODULE_CMD_T m_t_dcdc_module_normal_cmd[DCDC_MODULE_NORMAL_CMD_CNT] = 
{
	{ 3, 0, 6    },              //��ѯ��������
};

static MODULE_CMD_T m_t_dcdc_module_broadcast_cmd[DCDC_MODULE_BROADCAST_CMD_CNT] =
{
	{ 6, 0, 480  },              //����ģ�������ѹ���㲥���Ĭ��48V��ϵ��Ϊ0.1��48/0.1=480
	{ 6, 2, 1000 },              //����ģ�������㣬�㲥�����Χ5%~110%��Ĭ��10%��ϵ��Ϊ0.1��100/0.1=1000
	{ 6, 6, 480 },               //����Ĭ�������ѹ����㲥���Ĭ��48V��ϵ��Ϊ0.1��48/0.1=480
};

static MODULE_RECORD_T m_t_dcdc_module_record[DCDC_MODULE_MAX] =
{
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
};

//����ͨ��ģ���һЩ��������
static F32_T  m_f32_dcdc_out_volt;              //�����ѹ
static F32_T  m_f32_dcdc_curr_percent;          //�����ٷֱ�

/******************************* ͨ��ģ����ض������ ***************************/


/******************************* ���ģ����ض��忪ʼ ***************************/
#define DCAC_MODULE_START_ADDR             0x80   //���ģ����ʼ��ַ
#define DCAC_MODULE_BROADCAST_ADDR         0xF1   //һ�����ģ��㲥��ַ
                                           
#define DCAC_MODULE_READ_CMD               0      //���ģ���ѯ������������
#define DCAC_MODULE_NORMAL_CMD_CNT         1      //���ģ������ĸ���

#define DCAC_MODULE_FAULT_MASK             0x0004
#define DCAC_MODULE_OVERLOAD_MASK          0x0008
#define DCAC_MODULE_VOER_TEMPERATURE_MASK  0x0010
#define DCAC_MODULE_BATT_UNDERVOLT_MASK    0x0020
#define DCAC_MODULE_BYPASS_EXCEPTION_MASK  0x0040
#define DCAC_MODULE_EXCEPTION_MASK         (DCAC_MODULE_FAULT_MASK | DCAC_MODULE_OVERLOAD_MASK \
											| DCAC_MODULE_VOER_TEMPERATURE_MASK | DCAC_MODULE_BATT_UNDERVOLT_MASK \
											| DCAC_MODULE_BYPASS_EXCEPTION_MASK)


static U8_T m_u8_dcac_offline_cnt;         // ���ģ��ͨѶ�ж��������

static MODULE_CMD_T m_t_dcac_module_normal_cmd[DCAC_MODULE_NORMAL_CMD_CNT] = 
{
	{ 3, 0, 16 },              //��ѯ��������
};

static MODULE_RECORD_T m_t_dcac_module_record[DCAC_MODULE_MAX] =
{
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
};
/******************************* ���ģ����ض������ ***************************/


/******************************* D21ģ����ض��忪ʼ ***************************/
//#define D21_MODULE_START_ADDR             0x40   //DC10ģ����ʼ��ַ
//                                           
//#define D21_MODULE_READ_CMD               0      //DC10ģ���ѯ������������
//#define D21_MODULE_NORMAL_CMD_CNT         1      //DC10ģ������ĸ���	
//
//static U8_T m_u8_d21_offline_cnt = 10;         // DC10ģ��ͨѶ�ж��������
//
//static MODULE_CMD_T m_t_d21_module_normal_cmd[D21_MODULE_NORMAL_CMD_CNT] = 
//{
//	{ 3, 4, 5 },              //��ѯ��������
//};
//
//static MODULE_RECORD_T m_t_d21_module_record;
/******************************* D21ģ����ض������ ***************************/


/******************************* DC10ģ����ض��忪ʼ ***************************/
/* DC10ģ�����ݼ�¼ */
typedef struct
{
	U8_T u8_cmd_index;          //��������
	U8_T u8_com1_fail_cnt;      //ͨ��ʧ�ܼ�����ʧ�ܴ�������10�α�ͨ���жϹ���
}DC10_RECORD_T;

#define DC10_MODULE_START_ADDR             0x40   //DC10ģ����ʼ��ַ
                                           
#define DC10_MODULE_READ_YC_CMD            0      //DC10ģ���ѯ������������
#define DC10_MODULE_READ_YX_CMD            1      //DC10ģ���ѯ������������
#define DC10_MODULE_WRITE_YK_CMD           2      //DC10ģ��ң����������
#define DC10_MODULE_NORMAL_CMD_CNT         3      //DC10ģ������ĸ���

#define DC10_MODULE_FUSE_MASK              0x0080
#define DC10_MODULE_EXCEPTION_MASK         (DC10_MODULE_FUSE_MASK)

static U8_T m_u8_dc10_offline_cnt;         // DC10ģ��ͨѶ�ж��������

static MODULE_CMD_T m_t_dc10_module_normal_cmd[DC10_MODULE_NORMAL_CMD_CNT] = 
{
	{ 0x04, 0x100, 16 },              //ң���ѯ��������
	{ 0x02, 0x500, 64 },              //ң�Ų�ѯ��������
	{ 0x06, 0x1000, 0 },              //��ǰ�·���ң����������
};

static 	MODULE_CMD_T m_t_dc10_yk_cmd[] = 
{
	{ 0x06, 0x100E,  75 },         //A1�������������ѹmV��ң����������
	{ 0x06, 0x100F, 100 },         //A1����������̣�ң����������
	{ 0x06, 0x1010,  75 },         //A2�������������ѹmV��ң����������
	{ 0x06, 0x1011, 100 },         //A2����������̣�ң����������
	{ 0x06, 0x1012,  75 },         //A3�������������ѹmV��ң����������
	{ 0x06, 0x1013, 100 },         //A3����������̣�ң����������
	{ 0x06, 0x1017, 100 },         //S1���������̣�ң����������
	{ 0x06, 0x1018, 100 },         //S2���������̣�ң����������
	{ 0x06, 0x1019, 100 },         //S3���������̣�ң����������
	{ 0x06, 0x1020, 2200 },        //�������������ѹ��ң����������
	{ 0x06, 0x1021,   0 },         //�������ң����������
	{ 0x06, 0x1022,   0 },         //ϵͳ���ϣ�ң����������
	{ 0x06, 0x1023,   0 },         //��������ɽ��1��ң����������
	{ 0x06, 0x1024,   0 },         //��������ɽ��2��ң����������
	{ 0x06, 0x1025,   0 },         //��������ɽ��3��ң����������
	{ 0x06, 0x1026,   0 },         //��������ɽ��4��ң����������
	{ 0x06, 0x1027,   0 },         //��������ɽ��5��ң����������
	{ 0x06, 0x1028,   0 },         //��������ɽ��6��ң����������
	{ 0x06, 0x1029,   0 },         //��������ɽ��7��ң����������
	{ 0x06, 0x102A,   0 },         //��������ɽ��8��ң����������
};
#define DC10_YK_CMD_NUM		(sizeof(m_t_dc10_yk_cmd)/sizeof(MODULE_CMD_T))
static U8_T  m_u8_dc10_yk_idx = 0;
static DC10_RECORD_T m_t_dc10_module_record = { 0,  0 };


/******************************* AC10ģ����ض������ ***************************/


/******************************* AC10ģ����ض��忪ʼ ***************************/
/* AC10ģ�����ݼ�¼ */
typedef struct
{
	U8_T u8_cmd_index;          //��������
	U8_T u8_com1_fail_cnt;      //ͨ��ʧ�ܼ�����ʧ�ܴ�������10�α�ͨ���жϹ���
}AC10_RECORD_T;

#define AC10_MODULE_START_ADDR             0x41   //AC10ģ����ʼ��ַ
                                           
#define AC10_MODULE_READ_YC_CMD            0      //AC10ģ���ѯ������������
#define AC10_MODULE_READ_YX_CMD            1      //AC10ģ���ѯ������������
#define AC10_MODULE_WRITE_YK_CMD           2      //AC10ģ��ң����������
#define AC10_MODULE_NORMAL_CMD_CNT         3      //AC10ģ������ĸ���

#define AC10_MODULE_FUSE_MASK              0x0080
#define AC10_MODULE_EXCEPTION_MASK         (AC10_MODULE_FUSE_MASK)

static U8_T m_u8_ac10_offline_cnt;         // AC10ģ��ͨѶ�ж��������

static MODULE_CMD_T m_t_ac10_module_normal_cmd[AC10_MODULE_NORMAL_CMD_CNT] = 
{
	{ 0x04, 0x100, 16 },              //ң���ѯ��������
	{ 0x02, 0x500, 64 },              //ң�Ų�ѯ��������
	{ 0x06, 0x1000, 0 },              //��ǰ�·���ң����������
};

static 	MODULE_CMD_T m_t_ac10_yk_cmd[] = 
{
	{ 0x06, 0x1000, 0x0200 },      //�������뷽ʽ(Ĭ�ϣ���·����)��ң����������
	{ 0x06, 0x1017, 100 },         //S1���������̣�ң����������
	{ 0x06, 0x1018, 100 },         //S2���������̣�ң����������
	{ 0x06, 0x1019, 100 },         //S3���������̣�ң����������
};
#define AC10_YK_CMD_NUM		(sizeof(m_t_ac10_yk_cmd)/sizeof(MODULE_CMD_T))
static U8_T  m_u8_ac10_yk_idx = 0;
static AC10_RECORD_T m_t_ac10_module_record = { 0,  0 };
U32_T	m_u32_ac_ov_start1;     //һ·������ѹ
U32_T	m_u32_ac_ov_start2;		//��·������ѹ

/******************************* AC10ģ����ض������ ***************************/

/******************************* RC10ģ����ض��忪ʼ ***************************/
#define RC10_MODULE_START_ADDR             0xC0   //RC10ģ����ʼ��ַ
                                           
#define RC10_MODULE_WRITE_CMD              0      //RC10ģ���ѯ������������
#define RC10_MODULE_NORMAL_CMD_CNT         1      //RC10ģ������ĸ���      

static U8_T m_u8_rc10_offline_cnt;         // ͨ��ģ��ͨѶ�ж��������

static MODULE_CMD_T m_t_rc10_module_normal_cmd[RC10_MODULE_NORMAL_CMD_CNT] = 
{
	{ 16, 0x0100, 16   },              //д��������
};

static U8_T m_u8_rc10_write_data[RC10_NODE_MAX] = { 0 };

static MODULE_RECORD_T m_t_rc10_module_record[RC10_MODULE_MAX] =
{
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
	{ 8, 0, 0 },
	{ 9, 0, 0 },
	{10, 0, 0 },
	{11, 0, 0 },
	{12, 0, 0 },
	{13, 0, 0 },
	{14, 0, 0 },
	{15, 0, 0 },
};

static U32_T m_u32_rc10_module_ctrl_tm[RC10_MODULE_MAX] = {0};
//static U32_T m_u32_rc10_module_retry[RC10_MODULE_MAX] = {0};
static U8_T  m_u8_rc10_swt_bak[FACT_SWT_CTRL_MAX] = {0};

static U32_T m_u32_switch_sync_time = 0;

/******************************* RC10ģ����ض������ ***************************/


/******************************* ���Ѳ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_bms_send_cmd		           				
��������: ���Ѳ�췢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_bms_send_cmd(BMS_RECORD_T *pt_record)
{
	U16_T crc, num;

#ifdef BIC_DEBUG
	U32_T i;
#endif

	if (pt_record->u8_cmd_index != BMS_READ_CMD)
		return;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B21)
		num = BMS_B21_MAX_CELL_NUM + 1;        //B21�¶����ݾݷ��ڵ����ѹ����
	else
		num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_num[pt_record->u8_module_index] + 2;
	os_mut_release(g_mut_share_data);
		
	m_u8_com1_tx_buf[0] = BMS_START_ADDR + pt_record->u8_start_addr + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_bms_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_bms_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_bms_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (num >> 8);
	m_u8_com1_tx_buf[5] = num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef BIC_DEBUG
	DEBUG("BMS send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_RECV_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
��������: u16_com1_create_cell_fault_id		           				
��������: ���ݵ�ؽںż������ID						
�������: u8_group_index -- ������
          u8_batt_index -- �����غ�
          u8_fault_type -- �������ͣ�0����ѹ��1��Ƿѹ     		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_create_cell_fault_id(U8_T u8_group_index, U8_T u8_batt_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	if (u8_group_index == 0)
		u16_fault_id = (FAULT_BATT1_GROUP<<FAULT_GROUP_OFFSET);
	else
		u16_fault_id = (FAULT_BATT2_GROUP<<FAULT_GROUP_OFFSET);

	if (u8_fault_type == 0)
	{
		u16_fault_id |= (FAULT_CELL_OVER_VOLT_BASE_NUM + u8_batt_index);
	}
	else
	{
		u16_fault_id |= (FAULT_CELL_UNDER_VOLT_BASE_NUM + u8_batt_index);
	}

	return u16_fault_id;
}

/*************************************************************
��������: u16_com1_create_bms_fault_id		           				
��������: ���ݵ����ż������ID						
�������: u8_module_index -- ���Ѳ��������������Ѳ���е����   		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_create_bms_fault_id(U8_T u8_module_index)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_BMS_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= u8_module_index;

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_bms_comm_fault_alm		           				
��������: ���Ѳ��ͨ�Ź��ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_bms_comm_fault_alm(BMS_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_bms_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_bms_offline_cnt)
		{
			//��ͨ�Ź���
			v_fauid_send_fault_id_occur(u16_com1_create_bms_fault_id(pt_record->u8_total_index));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_batt.t_batt_group[pt_record->u8_group_index].u8_comm_state[pt_record->u8_module_index] = 1;
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
��������: v_com1_bms_unpacket		           				
��������: ���Ѳ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_bms_unpacket(BMS_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case BMS_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timout < 30)

			if (crc_scuess == 1)
			{
				BATT_GROUP_RT_DATA_T *batt_group = &(g_t_share_data.t_rt_data.t_batt.t_batt_group[pt_record->u8_group_index]);
				U8_T batt_num, bms_num;
				F32_T f32_cell_over_volt, f32_cell_under_volt;

#ifdef BIC_DEBUG
				DEBUG("BMS recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				i += 3;

				if (pt_record->u8_module_index == 0)      //����ǵ�һ�����Ѳ�죬����µ�����¶�
				{
					if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type != B21)
					{
						if ((m_u8_com1_rx_buf[i] & 0x80) != 0)
							batt_group->f32_temperature1 = -((((m_u8_com1_rx_buf[i]<<8) + m_u8_com1_rx_buf[i+1]) & 0x7FFF) / 10.0);
						else
							batt_group->f32_temperature1 = ((m_u8_com1_rx_buf[i]<<8) + m_u8_com1_rx_buf[i+1]) / 10.0;
							
						if ((m_u8_com1_rx_buf[i+2] & 0x80) != 0)
							batt_group->f32_temperature2 = -((((m_u8_com1_rx_buf[i+2]<<8) + m_u8_com1_rx_buf[i+3]) &0x7FFF) / 10.0);
						else
							batt_group->f32_temperature2 = ((m_u8_com1_rx_buf[i+2]<<8) + m_u8_com1_rx_buf[i+3]) / 10.0;
					}
					else
					{
						if ((m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i] & 0x80) != 0)
							batt_group->f32_temperature1 = -((((m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i]<<8)
															+ m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i+1]) & 0x7FFF) / 10.0);
						else
							batt_group->f32_temperature1 = ((m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i]<<8)
															+ m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i+1]) / 10.0;
					}
				}

				if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type != B21)   //B3��B4ǰ�����Ĵ������¶����ݣ���������ƶ������Ĵ���
				{
					i += 4;
				}

				batt_num = 0;
				for (j=0; j<pt_record->u8_module_index; j++)
				{
					batt_num += g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_num[j];
				}
				
				bms_num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_num[pt_record->u8_module_index];

				for (j=0; j<bms_num; j++)
				{
					if (batt_num+j > BATT_CELL_MAX-1)    //��ֹ����Խ��
						break;

					if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type != B21)
						batt_group->f32_cell_volt[batt_num+j] = ((m_u8_com1_rx_buf[i+j*2]<<8) + m_u8_com1_rx_buf[i+j*2+1]) / 1000.0;
					else
						batt_group->f32_cell_volt[batt_num+j] = ((m_u8_com1_rx_buf[i+j*2]<<8) + m_u8_com1_rx_buf[i+j*2+1]) * 20.0 / 65536.0;

					//�жϵ������Ƿ��Ƿѹ�������͹���ID
					if ((pt_record->u8_module_index ==
						(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_bms_num-1)) && (j == (bms_num-1)))
					{
						f32_cell_over_volt = g_t_share_data.t_sys_cfg.t_batt.f32_tail_high_volt;
						f32_cell_under_volt = g_t_share_data.t_sys_cfg.t_batt.f32_tail_low_volt;
					}
					else
					{
						f32_cell_over_volt = g_t_share_data.t_sys_cfg.t_batt.f32_cell_high_volt;
						f32_cell_under_volt = g_t_share_data.t_sys_cfg.t_batt.f32_cell_low_volt;
					}

					if (batt_group->f32_cell_volt[batt_num+j] <= f32_cell_under_volt)   //Ƿѹ
					{
						if (batt_group->u8_cell_state[batt_num+j] == 0)
						{
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1));  //Ƿѹ���Ϸ���
							batt_group->u8_cell_state[batt_num+j] = 2;
						}
						else if (batt_group->u8_cell_state[batt_num+j] == 1)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0)); //��ѹ���ϻָ�
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1));  //Ƿѹ���Ϸ���
							batt_group->u8_cell_state[batt_num+j] = 2;
						}
					}
					else if (batt_group->f32_cell_volt[batt_num+j] >= f32_cell_over_volt)  //��ѹ
					{
						if (batt_group->u8_cell_state[batt_num+j] == 0)
						{
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0));  //��ѹ���Ϸ���
							batt_group->u8_cell_state[batt_num+j] = 1;
						}
						else if (batt_group->u8_cell_state[batt_num+j] == 2)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1)); //Ƿѹ���ϻָ�
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0));  //��ѹ���Ϸ���
							batt_group->u8_cell_state[batt_num+j] = 1;
						}
					}
					else
					{
						if (batt_group->u8_cell_state[batt_num+j] == 1)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0)); //��ѹ���ϻָ�
							batt_group->u8_cell_state[batt_num+j] = 0;
						}
						else if (batt_group->u8_cell_state[batt_num+j] == 2)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1)); //Ƿѹ���ϻָ�
							batt_group->u8_cell_state[batt_num+j] = 0;
						}
					}
				}

				//�жϵ����������͵�ѹ
				batt_group->f32_min_cell_volt = batt_group->f32_cell_volt[0];
				batt_group->u8_cell_min_volt_id = 1;
				batt_group->f32_max_cell_volt = batt_group->f32_cell_volt[0];
				batt_group->u8_cell_max_volt_id = 1;

				for (j=1; j<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_total_num; j++)
				{
					if (batt_group->f32_cell_volt[j] < batt_group->f32_min_cell_volt)
					{
						batt_group->f32_min_cell_volt = batt_group->f32_cell_volt[j];
						batt_group->u8_cell_min_volt_id = j + 1;
					}
					else if (batt_group->f32_cell_volt[j] > batt_group->f32_max_cell_volt)
					{
						batt_group->f32_max_cell_volt = batt_group->f32_cell_volt[j];
						batt_group->u8_cell_max_volt_id = j + 1;
					}
				}
				batt_group->f32_cell_min_volt_id = batt_group->u8_cell_min_volt_id;
				batt_group->f32_cell_max_volt_id = batt_group->u8_cell_max_volt_id;

				batt_group->u8_comm_state[pt_record->u8_module_index] = 0;   //���ͨ���жϱ�־
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_bms_offline_cnt)    //����ͨ�Ź���
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_create_bms_fault_id(pt_record->u8_total_index));
				}
				pt_record->u8_com1_fail_cnt = 0;                             //���ͨ���жϼ�����
			}
			else
			{
				v_com1_bms_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_bms_comm_handle		           				
��������: ���Ѳ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_bms_comm_handle(BMS_RECORD_T *pt_record)
{
	v_com1_bms_send_cmd(pt_record);
	v_com1_bms_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= BMS_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* ���Ѳ����غ���������� ***************************/


/******************************* ����ģ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_rect_module_send_cmd		           				
��������: ����ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, data_or_num;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	switch (pt_record->u8_cmd_index)
	{

		case RECT_MODULE_READ_CMD:
			data_or_num = m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case RECT_MODULE_SET_STATE_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);
			data_or_num = g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl[pt_record->u8_module_index];
			os_mut_release(g_mut_share_data);
			break;

		default:
			break;
	}

	m_u8_com1_tx_buf[0] = RECT_MODULE_START_ADDR + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}


/*************************************************************
��������: v_com1_rect_module_broadcast_send		           				
��������: ����ģ�鷢�͹㲥�����						
�������: u8_cmd_index -- �㲥��������       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_broadcast_send(U8_T no, U8_T u8_cmd_index, U16_T set_data)
{
	U16_T crc;

	//һ��ģ��㲥���������
	m_u8_com1_tx_buf[0] = RECT_MODULE_BROADCAST_ADDR - no;    //�㲥��ַ
	m_u8_com1_tx_buf[1] = m_t_rect_module_broadcast_cmd[u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_rect_module_broadcast_cmd[u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_rect_module_broadcast_cmd[u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (set_data >> 8);
	m_u8_com1_tx_buf[5] = set_data;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
��������: v_com1_rect_module_send_broadcast_cmd		           				
��������: ����ģ�鷢�͹㲥�����						
�������: u8_cmd_index -- �㲥��������       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_send_broadcast_cmd(U8_T no, U8_T u8_cmd_index)
{
	U16_T set_data;	

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);
	switch (u8_cmd_index)
	{
		case RECT_MODULE_SET_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[no] * 10);
			break;

		case RECT_MODULE_SET_CURR_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[no] * 10);
			break;

		case RECT_MODULE_SET_HIGH_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt * 10);
			break;

		case RECT_MODULE_SET_LOW_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt * 10);
			break;
			
		case RECT_MODULE_SET_DEF_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt * 10);
			break;

		default:
			break;
	}
	os_mut_release(g_mut_share_data);
	
	if (no == 0)
	{
		//һ��ģ��㲥���������
		v_com1_rect_module_broadcast_send(COM1_GROUP1_RECT, u8_cmd_index, set_data);
		if (u8_cmd_index == RECT_MODULE_SET_CURR_CMD)
			os_evt_set(RECT_SET_CURR_SCUESS1, g_tid_batt);      //��������������ɣ������¼���־����ع�������
	}

	else
	{	
		//����ģ��㲥���������
		v_com1_rect_module_broadcast_send(COM1_GROUP2_RECT, u8_cmd_index, set_data);
		if (u8_cmd_index == RECT_MODULE_SET_CURR_CMD)
			os_evt_set(RECT_SET_CURR_SCUESS2, g_tid_batt);      //��������������ɣ������¼���־����ع�������
	}
	
	os_dly_wait(COM1_BROADCAST_WAIT_TICK);   //�ȴ�300ms����ģ�鴦�����Ȼ����ܷ�����һ������

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);                                 //����ι���¼���־
}

/*************************************************************
��������: u16_com1_rect_create_fault_id		           				
��������: ��������ģ�������͹������ͼ������ID						
�������: u8_module_index -- ģ������
          u8_fault_type   -- ��������    		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_rect_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_RECT_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (FAULT_RECT_CNT * u8_module_index + u8_fault_type);

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_rect_module_send_fault_id		           				
��������: ����ģ�鷢�͹���ID����						
�������: u16_state_old -- �ɵ�ģ��״ֵ̬
          u16_state_new -- �µ�ģ��״ֵ̬       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new)
{
	U16_T u16_fault_id;

	if (((u16_state_old & RECT_MODULE_OVER_VOLT_PROTECT_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_OVER_VOLT_PROTECT_MASK) != 0))   //��ѹ�������Ϸ���
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_OVER_VOLT_PROTECT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_OVER_VOLT_PROTECT_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_OVER_VOLT_PROTECT_MASK) == 0))   //��ѹ�������ϸ���
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_OVER_VOLT_PROTECT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & RECT_MODULE_FAULT_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_FAULT_MASK) != 0))        //ģ����Ϸ���
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_FAULT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_FAULT_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_FAULT_MASK) == 0))        //ģ����ϸ���
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_FAULT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & RECT_MODULE_VOER_TEMPERATURE_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_VOER_TEMPERATURE_MASK) != 0))    //���¹��Ϸ���
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_VOER_TEMPERATURE);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_VOER_TEMPERATURE_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_VOER_TEMPERATURE_MASK) == 0))    //���¹��ϸ���
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_VOER_TEMPERATURE);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & RECT_MODULE_AC_EXCEPTION_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_AC_EXCEPTION_MASK) != 0))        //�����쳣����
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_AC_EXCEPTION);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_AC_EXCEPTION_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_AC_EXCEPTION_MASK) == 0))        //�����쳣����
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_AC_EXCEPTION);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
}

/*************************************************************
��������: v_com1_rect_module_comm_fault_alm		           				
��������: ����ģ��ͨ�Ź��ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_rect_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_rect_offline_cnt)
		{
			//��ͨ�Ź���
			v_fauid_send_fault_id_occur(u16_com1_rect_create_fault_id(pt_record->u8_module_index, FAULT_RECT_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dc_panel.t_rect[pt_record->u8_module_index].u8_comm_state = 1;   //����ͨ���жϱ�־
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
��������: v_com1_rect_module_unpacket		           				
��������: ����ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case RECT_MODULE_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				RECT_RT_DATA_T *rect = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[pt_record->u8_module_index]);
				U16_T u16_state_old, u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				rect->f32_out_volt = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]) / 10.0;
				rect->f32_out_curr = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]) / 10.0;
				rect->f32_curr_percent = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]) / 10.0;
				rect->f32_max_out_volt = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]) / 10.0;
				rect->f32_min_out_volt = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]) / 10.0;
				u16_state_old = rect->u16_state;
				u16_state_new = (m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14];
				rect->u16_state = u16_state_new;
				rect->b_ctl_mode = ((u16_state_new & 0x0002) ? 1 : 0);
				rect->e_module_state = ((u16_state_new & 0x0001) ? SHUT_DOWN : START_UP);
				if ((u16_state_new & RECT_MODULE_EXCEPTION_MASK) != 0)
					rect->e_module_state = EXCEPTION;
				rect->u8_comm_state = 0;                                                             //���ͨ���жϱ�־
				os_mut_release(g_mut_share_data);

				v_com1_rect_module_send_fault_id(pt_record->u8_module_index, u16_state_old, u16_state_new);     //���͸澯ID

				if (pt_record->u8_com1_fail_cnt >= m_u8_rect_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_rect_create_fault_id(pt_record->u8_module_index, FAULT_RECT_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_rect_module_comm_fault_alm(pt_record);
			}

			break;

		case RECT_MODULE_SET_STATE_CMD:
			rx_len = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)  //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < 8)
				{
					continue;
				}
				else
				{
					for (i=0; i<(rx_len-8+1); i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == m_u8_com1_tx_buf[2])
							&& (m_u8_com1_rx_buf[i+3] == m_u8_com1_tx_buf[3])
							&& (m_u8_com1_rx_buf[i+4] == m_u8_com1_tx_buf[4])
							&& (m_u8_com1_rx_buf[i+5] == m_u8_com1_tx_buf[5])
							&& (m_u8_com1_rx_buf[i+6] == m_u8_com1_tx_buf[6])
							&& (m_u8_com1_rx_buf[i+7] == m_u8_com1_tx_buf[7]))
						{
							if (pt_record->u8_com1_fail_cnt >= m_u8_rect_offline_cnt)
							{
								//����ͨ�Ź��ϱ���
								v_fauid_send_fault_id_resume(u16_com1_rect_create_fault_id(pt_record->u8_module_index, FAULT_RECT_COMM_FAIL));

								os_mut_wait(g_mut_share_data, 0xFFFF);
								g_t_share_data.t_rt_data.t_dc_panel.t_rect[pt_record->u8_module_index].u8_comm_state = 0;   //���ͨ���жϱ�־
								os_mut_release(g_mut_share_data);
							}

							pt_record->u8_com1_fail_cnt = 0;

#ifdef COM1_DEBUG
							DEBUG("recv data: ");
							for (j=0; j<8; j++)
							{
								DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
							}
							DEBUG("\r\n");
#endif

							break;
						}
					}

					if (i<(rx_len-8+1))
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}
			}

			if (timeout >= COM1_RECV_TIMEOUT_CNT)
			{
				v_com1_rect_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_rect_module_comm_handle		           				
��������: ����ģ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rect_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	v_com1_rect_module_send_cmd(pt_record);
	v_com1_rect_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= RECT_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* ����ģ����غ���������� ***************************/



/******************************* ͨ��ģ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_dcdc_module_send_cmd		           				
��������: ͨ��ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcdc_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, data_or_num;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	switch (pt_record->u8_cmd_index)
	{

		case DCDC_MODULE_READ_CMD:
			data_or_num = m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = DCDC_MODULE_START_ADDR + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
��������: v_com1_dcdc_module_send_broadcast_cmd		           				
��������: ͨ��ģ�鷢�͹㲥�����						
�������: u8_cmd_index -- �㲥��������       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcdc_module_send_broadcast_cmd(U8_T u8_cmd_index)
{
	U16_T crc, set_data;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);
	switch (u8_cmd_index)
	{
		case DCDC_MODULE_SET_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt * 10);
			break;

		case DCDC_MODULE_SET_CURR_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent * 10);
			break;

		case DCDC_MODULE_SET_DEF_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt * 10);
			break;

		default:
			break;
	}
	os_mut_release(g_mut_share_data);
	

	m_u8_com1_tx_buf[0] = DCDC_MODULE_BROADCAST_ADDR;    //�㲥��ַ
	m_u8_com1_tx_buf[1] = m_t_dcdc_module_broadcast_cmd[u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dcdc_module_broadcast_cmd[u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dcdc_module_broadcast_cmd[u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (set_data >> 8);
	m_u8_com1_tx_buf[5] = set_data;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send broadcast cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
	
	os_dly_wait(COM1_BROADCAST_WAIT_TICK);   //�ȴ�300ms����ģ�鴦�����Ȼ����ܷ�����һ������

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);                                 //����ι���¼���־
}

/*************************************************************
��������: u16_com1_dcdc_create_fault_id		           				
��������: ����ͨ��ģ�������͹������ͼ������ID						
�������: u8_module_index -- ģ������
          u8_fault_type   -- ��������    		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_dcdc_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DCDC_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (FAULT_DCDC_CNT * u8_module_index + u8_fault_type);

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_dcdc_module_send_fault_id		           				
��������: ͨ��ģ�鷢�͹���ID����						
�������: u16_state_old -- �ɵ�ģ��״ֵ̬
          u16_state_new -- �µ�ģ��״ֵ̬       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_com1_dcdc_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new)
{
	U16_T u16_fault_id;
	
	if (((u16_state_old & DCDC_MODULE_PROTECT_MASK) == 0)
		&& ((u16_state_new & DCDC_MODULE_PROTECT_MASK) != 0))     //�������Ϸ���
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_PROTECT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCDC_MODULE_PROTECT_MASK) != 0)
		&& ((u16_state_new & DCDC_MODULE_PROTECT_MASK) == 0))      //�������ϸ���
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_PROTECT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & DCDC_MODULE_FAULT_MASK) == 0)
		&& ((u16_state_new & DCDC_MODULE_FAULT_MASK) != 0))        //ģ����Ϸ���
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_FAULT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCDC_MODULE_FAULT_MASK) != 0)
		&& ((u16_state_new & DCDC_MODULE_FAULT_MASK) == 0))        //ģ����ϸ���
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_FAULT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
}

/*************************************************************
��������: v_com1_dcdc_module_comm_fault_alm		           				
��������: ͨ��ģ��ͨ�Ź��ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcdc_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_dcdc_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_dcdc_offline_cnt)
		{
			//��ͨ�Ź���
			v_fauid_send_fault_id_occur(u16_com1_dcdc_create_fault_id(pt_record->u8_module_index, FAULT_DCDC_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[pt_record->u8_module_index].u8_comm_state = 1;   //����ͨ���жϱ�־
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
��������: v_com1_dcdc_module_unpacket		           				
��������: ͨ��ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcdc_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case DCDC_MODULE_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DCDC_MODULE_RT_DATA_T *dcdc = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[pt_record->u8_module_index]);
				U16_T u16_state_old, u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				dcdc->f32_out_volt = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]) / 10.0;
				dcdc->f32_out_curr = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]) / 10.0;
				dcdc->f32_curr_percent = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]) / 10.0;
				dcdc->f32_max_out_volt = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]) / 10.0;
				dcdc->f32_min_out_volt = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]) / 10.0;
				u16_state_old = dcdc->u16_state;
				u16_state_new = (m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14];
				dcdc->u16_state = u16_state_new;
				dcdc->b_ctl_mode = ((u16_state_new & 0x0002) ? 1 : 0);
				dcdc->e_module_state = ((u16_state_new & 0x0001) ? SHUT_DOWN : START_UP);
				if ((u16_state_new & DCDC_MODULE_EXCEPTION_MASK) != 0)
					dcdc->e_module_state = EXCEPTION;
				dcdc->u8_comm_state = 0;                                                             //���ͨ���жϱ�־
				
				//����ͨ��ĸ�ߵ�ѹ������
				g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_curr = 0;
				g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt = 0;
				for (j=0; j<g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num; j++)
				{
					g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_curr += 
								g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[j].f32_out_curr;
					
					if (g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt
							< g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[j].f32_out_volt)
					{
						g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt = 
							g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[j].f32_out_volt;
					}
				}
				
				
				os_mut_release(g_mut_share_data);

				v_com1_dcdc_module_send_fault_id(pt_record->u8_module_index, u16_state_old, u16_state_new);     //���͸澯ID

				if (pt_record->u8_com1_fail_cnt >= m_u8_dcdc_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_dcdc_create_fault_id(pt_record->u8_module_index, FAULT_DCDC_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dcdc_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_dcdc_module_comm_handle		           				
��������: ͨ��ģ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcdc_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	v_com1_dcdc_module_send_cmd(pt_record);
	v_com1_dcdc_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= DCDC_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* ͨ��ģ����غ���������� ***************************/


/******************************* ���ģ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_dcac_module_send_cmd		           				
��������: ���ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcac_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, data_or_num;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	switch (pt_record->u8_cmd_index)
	{
		case DCAC_MODULE_READ_CMD:
			data_or_num = m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = DCAC_MODULE_START_ADDR + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
��������: u16_com1_dcac_create_fault_id		           				
��������: �������ģ�������͹������ͼ������ID						
�������: u8_module_index -- ģ������
          u8_fault_type   -- ��������    		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_dcac_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DCAC_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (FAULT_DCAC_CNT * u8_module_index + u8_fault_type);

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_dcac_module_send_fault_id		           				
��������: ���ģ�鷢�͹���ID����						
�������: u16_state_old -- �ɵ�ģ��״ֵ̬
          u16_state_new -- �µ�ģ��״ֵ̬       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcac_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new)
{
	U16_T u16_fault_id;
											
	if (((u16_state_old & DCAC_MODULE_FAULT_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_FAULT_MASK) != 0))                   //ģ����Ϸ���
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_FAULT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_FAULT_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_FAULT_MASK) == 0))                   //ģ����ϸ���
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_FAULT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & DCAC_MODULE_OVERLOAD_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_OVERLOAD_MASK) != 0))                //ģ����ط���
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOERLOAD);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_OVERLOAD_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_OVERLOAD_MASK) == 0))                //ģ����ظ���
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOERLOAD);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
	
	if (((u16_state_old & DCAC_MODULE_VOER_TEMPERATURE_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_VOER_TEMPERATURE_MASK) != 0))        //ģ����·���
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOER_TEMPERATURE);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_VOER_TEMPERATURE_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_VOER_TEMPERATURE_MASK) == 0))        //ģ����¸���
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOER_TEMPERATURE);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
	
	if (((u16_state_old & DCAC_MODULE_BATT_UNDERVOLT_MASK) == 0)              //���Ƿѹ����
		&& ((u16_state_new & DCAC_MODULE_BATT_UNDERVOLT_MASK) != 0))          //���Ƿѹ����
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BATT_UNDERVOLT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_BATT_UNDERVOLT_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_BATT_UNDERVOLT_MASK) == 0))          //���Ƿѹ����
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BATT_UNDERVOLT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
	
	if (((u16_state_old & DCAC_MODULE_BYPASS_EXCEPTION_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_BYPASS_EXCEPTION_MASK) != 0))        //��·�쳣����
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BYPASS_EXCEPTION);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_BYPASS_EXCEPTION_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_BYPASS_EXCEPTION_MASK) == 0))        //��·�쳣����
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BYPASS_EXCEPTION);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
}

/*************************************************************
��������: v_com1_dcac_module_comm_fault_alm		           				
��������: ���ģ��ͨ�Ź��ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcac_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_dcac_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_dcac_offline_cnt)
		{
			//��ͨ�Ź���
			v_fauid_send_fault_id_occur(u16_com1_dcac_create_fault_id(pt_record->u8_module_index, FAULT_DCAC_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[pt_record->u8_module_index].u8_comm_state = 1;   //����ͨ���жϱ�־
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
��������: v_com1_dcac_module_unpacket		           				
��������: ���ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcac_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case DCAC_MODULE_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DCAC_MODULE_RT_DATA_T *dcac = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[pt_record->u8_module_index]);
				U16_T u16_state_old, u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				dcac->f32_out_volt = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]) / 10.0;
				dcac->f32_out_curr = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]) / 10.0;
				dcac->f32_out_freq = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]) / 100.0;
				dcac->f32_out_power_factor = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]) / 100.0;
				dcac->f32_inverter_volt = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]) / 10.0;
				dcac->f32_bypass_input_volt = ((m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14]) / 10.0;
				dcac->f32_bypass_input_freq = ((m_u8_com1_rx_buf[i+15]<<8) + m_u8_com1_rx_buf[i+16]) / 100.0;
				dcac->f32_batt_input_volt = ((m_u8_com1_rx_buf[i+17]<<8) + m_u8_com1_rx_buf[i+18]) / 10.0;
				dcac->f32_active_power = ((m_u8_com1_rx_buf[i+19]<<8) + m_u8_com1_rx_buf[i+20]) / 100.0;
				dcac->f32_apparen_power = ((m_u8_com1_rx_buf[i+21]<<8) + m_u8_com1_rx_buf[i+22]) / 100.0;
				dcac->f32_load_ratio = ((m_u8_com1_rx_buf[i+23]<<8) + m_u8_com1_rx_buf[i+24]) / 10.0;
				dcac->f32_temperature = (((m_u8_com1_rx_buf[i+25]&0x7F)<<8) + m_u8_com1_rx_buf[i+26]) / 10.0;
				if ((m_u8_com1_rx_buf[i+25] & 0x80) != 0)
					dcac->f32_temperature = -dcac->f32_temperature;
				dcac->f32_outage_capacity_ratio = ((m_u8_com1_rx_buf[i+27]<<8) + m_u8_com1_rx_buf[i+28]) / 10.0;
				dcac->f32_bypass_high_volt = ((m_u8_com1_rx_buf[i+29]<<8) + m_u8_com1_rx_buf[i+30]) / 10.0;
				dcac->f32_bypass_low_volt = ((m_u8_com1_rx_buf[i+31]<<8) + m_u8_com1_rx_buf[i+32]) / 10.0;
				
				u16_state_old = dcac->u16_state;
				u16_state_new = (m_u8_com1_rx_buf[i+33]<<8) + m_u8_com1_rx_buf[i+34];
				if ((u16_state_new & DCAC_MODULE_BATT_UNDERVOLT_MASK) != 0)
					u16_state_new &= ~DCAC_MODULE_BATT_UNDERVOLT_MASK;
				else
					u16_state_new |= DCAC_MODULE_BATT_UNDERVOLT_MASK;
				dcac->u16_state = u16_state_new;
				if (u16_state_new & 0x0001)
				{
					dcac->e_module_state = SHUT;
				}
				else
				{
					if (u16_state_new & 0x0080)
						dcac->e_module_state = INVERT;
					else
						dcac->e_module_state = BYPASS;
				}
								
				if ((u16_state_new & DCAC_MODULE_EXCEPTION_MASK) != 0)
					dcac->b_alarm_state = 1;
				else
					dcac->b_alarm_state = 0;
					
				dcac->u8_comm_state = 0; 
				                                                            //���ͨ���жϱ�־
				os_mut_release(g_mut_share_data);

				v_com1_dcac_module_send_fault_id(pt_record->u8_module_index, u16_state_old, u16_state_new);     //���͸澯ID

				if (pt_record->u8_com1_fail_cnt >= m_u8_dcac_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_dcac_create_fault_id(pt_record->u8_module_index, FAULT_DCAC_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dcac_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_dcac_module_comm_handle		           				
��������: ���ģ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dcac_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	v_com1_dcac_module_send_cmd(pt_record);
	v_com1_dcac_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= DCAC_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* ���ģ����غ���������� ***************************/



/******************************* D21ģ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_d21_module_send_cmd		           				
��������: D21ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
//static void v_com1_d21_module_send_cmd(MODULE_RECORD_T *pt_record)
//{
//	U16_T crc, data_or_num;
//
//#ifdef COM1_DEBUG
//	U32_T i;
//#endif
//
//	switch (pt_record->u8_cmd_index)
//	{
//		case D21_MODULE_READ_CMD:
//			data_or_num = m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
//			break;
//
//		default:
//			return;
//	}
//
//	m_u8_com1_tx_buf[0] = D21_MODULE_START_ADDR + pt_record->u8_module_index;
//	m_u8_com1_tx_buf[1] = m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
//	m_u8_com1_tx_buf[2] = (m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
//	m_u8_com1_tx_buf[3] = m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
//	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
//	m_u8_com1_tx_buf[5] = data_or_num;
//	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
//	m_u8_com1_tx_buf[6] = (U8_T)crc;
//	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);
//
//#ifdef COM1_DEBUG
//	DEBUG("send cmd: ");
//	for (i=0; i<8; i++)
//	{
//		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
//	}
//	DEBUG("\r\n");
//#endif
//
//	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
//	{
//		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
//		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
//	}
//}

/*************************************************************
��������: v_com1_d21_module_unpacket		           				
��������: D21ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
//static void v_com1_d21_module_unpacket(MODULE_RECORD_T *pt_record)
//{
//	U16_T byte_len, rx_len, timeout = 0;
//	U32_T i, j, crc_scuess;
//	U16_T id, u16_data,u16_load_shunt_range;
//	DC_RT_DATA_T *d21 = NULL;
//
//	os_mut_wait(g_mut_share_data, 0xFFFF);
//	d21 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc);
//	u16_load_shunt_range = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range;
//	os_mut_release(g_mut_share_data);
//
//	switch (pt_record->u8_cmd_index)
//	{
//		case D21_MODULE_READ_CMD:
//			rx_len = 0;
//			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�
//
//			crc_scuess = 0;
//			timeout = 0;
//
//			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
//			{
//				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
//				os_dly_wait(COM1_RECV_WAIT_TICK);
//				timeout++;
//
//				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
//				if (rx_len < byte_len)
//				{
//					continue;
//				}
//				else
//				{
//					for (i=0; i<rx_len-2; i++)
//					{
//						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
//							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
//							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
//						{
//							if (rx_len - i < byte_len)
//							{
//								break;
//							}
//							else
//							{
//								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
//								{
//									continue;
//								}
//								else
//								{
//									crc_scuess = 1;
//									break;
//								}
//							}
//						}
//					}
//
//					if (crc_scuess == 1)
//					{
//						break;
//					}
//					else
//					{
//						for (j=0; j<(rx_len-i); j++)
//							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];
//
//						rx_len -= i;
//					}
//				}  // end of if (rx_len < byte_len)
//			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)
//
//			if (crc_scuess == 1)
//			{
//#ifdef COM1_DEBUG
//				DEBUG("recv data: ");
//				for (j=0; j<byte_len; j++)
//				{
//					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
//				}
//				DEBUG("\r\n");
//#endif
//
//				os_mut_wait(g_mut_share_data, 0xFFFF);
//				u16_data = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]);
//				d21->f32_pb_volt = (F32_T)u16_data / 10.0;
//				u16_data = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]);
//				d21->f32_cb_volt = (F32_T)u16_data / 10.0;
//				u16_data = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]);
//				//D21�����ϵĸ��ص�������������Ϊ50A�������ڴ˽��л���
//				d21->f32_load_curr = (F32_T)u16_data * u16_load_shunt_range / 500.0;
//					
//				d21->u16_state &= ~0x0100;				   //���ͨ���жϱ�־
//				os_mut_release(g_mut_share_data); 				
//
//				if (pt_record->u8_com1_fail_cnt >= m_u8_d21_offline_cnt)
//				{
//					//����ͨ�Ź��ϱ���
//					id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC10_COMM_FAIL;
//					v_fauid_send_fault_id_resume(id);
//				}
//				pt_record->u8_com1_fail_cnt = 0;
//			}
//			else
//			{
//				if (pt_record->u8_com1_fail_cnt < m_u8_d21_offline_cnt)
//				{
//					pt_record->u8_com1_fail_cnt++;
//					if (pt_record->u8_com1_fail_cnt >= m_u8_d21_offline_cnt)
//					{
//						d21->u16_state |= 0x0100;
//						id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC10_COMM_FAIL;
//						v_fauid_send_fault_id_occur(id);
//					}
//				}
//			}
//
//			break;
//					
//		default:
//			break;
//	}
//}

/*************************************************************
��������: v_com1_d21_module_comm_handle		           				
��������: D21ģ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
//static void v_com1_d21_module_comm_handle(MODULE_RECORD_T *pt_record)
//{ 
//	v_com1_d21_module_send_cmd(pt_record);
//	v_com1_d21_module_unpacket(pt_record);
//
//	pt_record->u8_cmd_index++;
//	pt_record->u8_cmd_index %= D21_MODULE_NORMAL_CMD_CNT;
//
//	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
//}

/******************************* DC10ģ����غ���������� ***************************/

/******************************* DC10ģ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_dc10_module_send_cmd		           				
��������: DC10ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dc10_module_send_cmd(DC10_RECORD_T *pt_record)
{
	U16_T crc, data_or_num, u16_cfg_changed = 0;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);

	//���øı估ʱ�·�����
	if(m_t_dc10_yk_cmd[0].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt)
	{
		m_t_dc10_yk_cmd[0].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[0].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[0].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[0].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[1].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range)
	{
		m_t_dc10_yk_cmd[1].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[1].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[1].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[1].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[2].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt)
	{
		m_t_dc10_yk_cmd[2].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[2].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[2].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[2].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[3].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range)
	{
		m_t_dc10_yk_cmd[3].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[3].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[3].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[3].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[4].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt)
	{
		m_t_dc10_yk_cmd[4].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[4].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[4].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[4].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[5].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range)
	{
		m_t_dc10_yk_cmd[5].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[5].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[5].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[5].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[6].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range)
	{
		m_t_dc10_yk_cmd[6].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[6].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[6].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[6].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[7].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range)
	{
		m_t_dc10_yk_cmd[7].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[7].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[7].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[7].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[8].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range)
	{
		m_t_dc10_yk_cmd[8].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[8].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[8].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[8].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[9].u16_data_or_num != (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt * 10))
	{
		m_t_dc10_yk_cmd[9].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt * 10;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[9].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[9].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[9].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[10].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl)
	{
		m_t_dc10_yk_cmd[10].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[10].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[10].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[10].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[11].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[0])
	{
		m_t_dc10_yk_cmd[11].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[0];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[11].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[11].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[11].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[12].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[1])
	{
		m_t_dc10_yk_cmd[12].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[1];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[12].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[12].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[12].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}	
	else if(m_t_dc10_yk_cmd[13].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[2])
	{
		m_t_dc10_yk_cmd[13].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[2];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[13].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[13].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[13].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[14].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[3])
	{
		m_t_dc10_yk_cmd[14].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[3];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[14].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[14].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[14].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}	
	else if(m_t_dc10_yk_cmd[15].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[4])
	{
		m_t_dc10_yk_cmd[15].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[4];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[15].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[15].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[15].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[16].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[5])
	{
		m_t_dc10_yk_cmd[16].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[5];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[16].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[16].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[16].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[17].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[6])
	{
		m_t_dc10_yk_cmd[17].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[6];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[17].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[17].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[17].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}	
	
	os_mut_release(g_mut_share_data);


	switch (pt_record->u8_cmd_index)
	{
		case DC10_MODULE_READ_YC_CMD:
			data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case DC10_MODULE_READ_YX_CMD:
			data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case DC10_MODULE_WRITE_YK_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);            
			if ( !u16_cfg_changed )
			{	//�����·�
				m_u8_dc10_yk_idx++;
				m_u8_dc10_yk_idx %= DC10_YK_CMD_NUM;
				m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[m_u8_dc10_yk_idx].u8_func_code;	
				m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[m_u8_dc10_yk_idx].u16_reg_addr;				
				m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[m_u8_dc10_yk_idx].u16_data_or_num;
				data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			}
			os_mut_release(g_mut_share_data);
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = DC10_MODULE_START_ADDR;
	m_u8_com1_tx_buf[1] = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
��������: u16_com1_dc10_create_fault_id		           				
��������: ����DC10ģ�������͹������ͼ������ID						
�������: u8_module_index -- ģ������
          u8_fault_type   -- ��������    		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_dc10_create_fault_id(U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET);//(FAULT_FEEDER_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= u8_fault_type;

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_dc10_module_comm_fault_alm		           				
��������: DC10ģ��ͨ�Ź��ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dc10_module_comm_fault_alm(DC10_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_dc10_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
		{
			//��ͨ�Ź���
			v_fauid_send_fault_id_occur(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dc_panel.t_dc10.u8_comm_state = 1;   //����ͨ���жϱ�־
			os_mut_release(g_mut_share_data);
		}
	}
}

/****************************************************************************
 *  �������ƣ�  v_com1_dc10_bus2_fault_judge(void) 
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:	��	
 *  ���ܽ���:   �жϲɼ������Ķ���ֱ��ĸ�������Ƿ��쳣���쳣������Ӧ��־λ 
 ***************************************************************************/
void v_com1_dc10_bus2_fault_judge(void)
 {
 	U16_T id;
 	DC_RT_DATA_T *dc_p_rt_data = NULL;
	DC_CFG_T *dc_p_cfg = NULL;
	BATT_MGMT_CFG_T *batt_p_mgmt_cfg = NULL;

	os_mut_wait(g_mut_share_data, 0xFFFF);

	dc_p_rt_data = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc);
	dc_p_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc);
	batt_p_mgmt_cfg = &(g_t_share_data.t_sys_cfg.t_batt_mgmt);

	if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb)
	{	//��ĸ��ѹ�ж�
		if((dc_p_rt_data->f32_pb2_volt) >= (dc_p_cfg->u16_pb_high_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0001))
			{
				dc_p_rt_data->u16_state2 |= 0x0001;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0001)
			{
				dc_p_rt_data->u16_state2 &= (~0x0001);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//��ĸǷѹ�ж�
		if((dc_p_rt_data->f32_pb2_volt) <= (dc_p_cfg->u16_pb_low_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0002))
			{
				dc_p_rt_data->u16_state2 |= 0x0002;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0002)
			{
				dc_p_rt_data->u16_state2 &= (~0x0002);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}

		//��ĸ��ѹ�ж�
		if((dc_p_rt_data->f32_cb2_volt) >= (dc_p_cfg->u16_cb_high_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0004))
			{
				dc_p_rt_data->u16_state2 |= 0x0004;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0004)
			{
				dc_p_rt_data->u16_state2 &= (~0x0004);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//��ĸǷѹ�ж�
		if((dc_p_rt_data->f32_cb2_volt) <= (dc_p_cfg->u16_cb_low_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0008))
			{
				dc_p_rt_data->u16_state2 |= 0x0008;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0008)
			{
				dc_p_rt_data->u16_state2 &= (~0x0008);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	else
	{
		//ĸ�߹�ѹ�ж�
		if((dc_p_rt_data->f32_cb2_volt) >= (dc_p_cfg->u16_bus_high_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0010))
			{
				dc_p_rt_data->u16_state2 |= 0x0010;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0010)
			{
				dc_p_rt_data->u16_state2 &= (~0x0010);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//ĸ��Ƿѹ�ж�
		if((dc_p_rt_data->f32_cb2_volt) <= (dc_p_cfg->u16_bus_low_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0020))
			{
				dc_p_rt_data->u16_state2 |= 0x0020;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0020)
			{
				dc_p_rt_data->u16_state2 &= (~0x0020);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//��ع�ѹ�ж�
	if((dc_p_rt_data->f32_batt2_volt) >= (batt_p_mgmt_cfg->f32_high_volt_limit))
	{
		if(!((dc_p_rt_data->u16_state2)&0x0040))
		{
			dc_p_rt_data->u16_state2 |= 0x0040;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_OVER_VOLT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else 
	{
		if((dc_p_rt_data->u16_state2)&0x0040)
		{
			dc_p_rt_data->u16_state2 &= (~0x0040);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	//���Ƿѹ�ж�
	if((dc_p_rt_data->f32_batt2_volt) <= (batt_p_mgmt_cfg->f32_low_volt_limit))
	{
		if(!((dc_p_rt_data->u16_state2)&0x0080))
		{
			dc_p_rt_data->u16_state2 |= 0x0080;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_UNDER_VOLT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else
	{
		if((dc_p_rt_data->u16_state2)&0x0080)
		{
			dc_p_rt_data->u16_state2 &= (~0x0080);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_UNDER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	os_mut_release(g_mut_share_data);		
 }

/*************************************************************
��������: v_com1_dc10_module_unpacket		           				
��������: DC10ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
#define		LINE_VOL_TO_PHASE_VOL		1.732	   //�ߵ�ѹת��Ϊ���ѹ����ϵ��
static void v_com1_dc10_module_unpacket(DC10_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0, u16_data;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case DC10_MODULE_READ_YC_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DC10_RT_DATA_T *dc10 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc10);

				os_mut_wait(g_mut_share_data, 0xFFFF);
//				//һ·����
//				u16_data = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]);
//				f32_ac_ua = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]);
//				f32_ac_ub = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]);
//				f32_ac_uc = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;

                //��·����
//				u16_data = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]);
//				f32_ac_ua = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]);
//				f32_ac_ub = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14]);
//				f32_ac_uc = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;

                //�����¶�
				u16_data = ((m_u8_com1_rx_buf[i+15]<<8) + m_u8_com1_rx_buf[i+16]);

				//V1��ĸ��ѹ
				u16_data = ((m_u8_com1_rx_buf[i+17]<<8) + m_u8_com1_rx_buf[i+18]);
				dc10->f32_v1_volt = ((F32_T)(S16_T)u16_data) / 10.0;
				//A1���������ص���
				u16_data = ((m_u8_com1_rx_buf[i+19]<<8) + m_u8_com1_rx_buf[i+20]);
				dc10->f32_a1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (dc10->f32_a1_curr < 0)
				{
					dc10->f32_a1_curr = 0;
				}
				//V2��ص�ѹ
				u16_data = ((m_u8_com1_rx_buf[i+21]<<8) + m_u8_com1_rx_buf[i+22]);
				dc10->f32_v2_volt = ((F32_T)(S16_T)u16_data) / 10.0;
				//A2��������ص���
				u16_data = ((m_u8_com1_rx_buf[i+23]<<8) + m_u8_com1_rx_buf[i+24]);
				dc10->f32_a2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//V3��ĸ��ѹ
				u16_data = ((m_u8_com1_rx_buf[i+25]<<8) + m_u8_com1_rx_buf[i+26]);
				dc10->f32_v3_volt = ((F32_T)(S16_T)u16_data) / 10.0;
				//A3���������2����
				u16_data = ((m_u8_com1_rx_buf[i+27]<<8) + m_u8_com1_rx_buf[i+28]);
				dc10->f32_a3_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S1���������ص���
				u16_data = ((m_u8_com1_rx_buf[i+29]<<8) + m_u8_com1_rx_buf[i+30]);
				dc10->f32_s1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (dc10->f32_s1_curr < 0)
				{
					dc10->f32_s1_curr = 0;
				}
				//S2��������ص���
				u16_data = ((m_u8_com1_rx_buf[i+31]<<8) + m_u8_com1_rx_buf[i+32]);
				dc10->f32_s2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S3���������2����
				u16_data = ((m_u8_com1_rx_buf[i+33]<<8) + m_u8_com1_rx_buf[i+34]);
				dc10->f32_s3_curr = ((F32_T)(S16_T)u16_data) / 10.0;

				//����ĸ��������ȡ
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_volt = dc10->f32_v3_volt;   // ���κ�ĸ��ѹ
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_volt = dc10->f32_v1_volt;   // ���ο�ĸ��ѹ
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr = dc10->f32_a1_curr; // ���ο�ĸ������Ҳ�Ƹ��ص���	                                                     
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt2_volt = dc10->f32_v2_volt; // �����ص�ѹ

#if (BATT_CURR_FROM_SENSOR)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] = dc10->f32_s1_curr;// һ���ص���
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] = dc10->f32_s2_curr;// �����ص���
#else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] = dc10->f32_a2_curr;// �����ص���
#endif

				//����ĸ���쳣�жϴ���
				v_com1_dc10_bus2_fault_judge();								

				//��ͨ���ж�	
				dc10->u8_comm_state = 0;                                    //���ͨ���жϱ�־				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dc10_module_comm_fault_alm(pt_record);
			}

			break;

		case DC10_MODULE_READ_YX_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])/8 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DC10_RT_DATA_T *dc10 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc10);
				U16_T u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				//state1 = ((m_u8_com1_rx_buf[i+4]<<8) + m_u8_com1_rx_buf[i+3]);
				//state2 = ((m_u8_com1_rx_buf[i+6]<<8) + m_u8_com1_rx_buf[i+5]);
				//state3 = ((m_u8_com1_rx_buf[i+8]<<8) + m_u8_com1_rx_buf[i+7]);
				u16_state_new = (U16_T)((m_u8_com1_rx_buf[i+10]<<8) + m_u8_com1_rx_buf[i+9]);
				dc10->u16_swt_state = u16_state_new;				
//				if(u16_state_new & 0x0001)
//				{	//�۶�					
//					if( !(dc10->u16_sys_state & 0x0008) )
//					{
//						dc10->u16_sys_state |= 0x0008;
//						v_fauid_send_fault_id_occur(u16_com1_dc10_create_fault_id(FAULT_DCAC_DC10_FUSE_FAULT));
//					}
//				}
//				else
//				{	//����
//					if( (dc10->u16_sys_state & 0x0008) )
//					{
//						dc10->u16_sys_state &= ~0x0008;
//						v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DCAC_DC10_FUSE_FAULT));
//					}
//				}				

				//��ͨ���ж�	
				dc10->u8_comm_state = 0;                                    //���ͨ���жϱ�־				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dc10_module_comm_fault_alm(pt_record);
			}

			break;

		case DC10_MODULE_WRITE_YK_CMD:
			rx_len = 0;
			byte_len = 8;  //��ַ��1���ֽڡ�������1���ֽڡ�2�ֽڼĴ�����ַ��2�ֽڼĴ������ݡ�CRC�����ֽڣ��ܹ���8���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i]   == m_u8_com1_tx_buf[0]) && 
						    (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1]))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DC10_RT_DATA_T *dc10 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc10);

				os_mut_wait(g_mut_share_data, 0xFFFF);	
				dc10->u8_comm_state = 0;                                    //���ͨ���жϱ�־				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dc10_module_comm_fault_alm(pt_record);
			}
			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_dc10_module_comm_handle		           				
��������: DC10ģ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_dc10_module_comm_handle(DC10_RECORD_T *pt_record)
{ 
	v_com1_dc10_module_send_cmd(pt_record);
	v_com1_dc10_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= DC10_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* DC10ģ����غ���������� ***************************/


/******************************* AC10ģ����غ������忪ʼ ***************************/

/*************************************************************
��������: v_com1_ac10_module_send_cmd		           				
��������: AC10ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_ac10_module_send_cmd(AC10_RECORD_T *pt_record)
{
	U16_T crc, data_or_num, u16_cfg_changed = 0;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);

	//���øı估ʱ�·�����
	if(m_t_ac10_yk_cmd[0].u16_data_or_num != 0x0200)
	{
		m_t_ac10_yk_cmd[0].u16_data_or_num = 0x0200;//g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_ac_meas_modle;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[0].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[0].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[0].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_ac10_yk_cmd[1].u16_data_or_num != g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range)
	{
		m_t_ac10_yk_cmd[1].u16_data_or_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[1].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[1].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[1].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		u16_cfg_changed = 1;
	}
	else if(m_t_ac10_yk_cmd[2].u16_data_or_num != g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range)
	{
		m_t_ac10_yk_cmd[2].u16_data_or_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[2].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[2].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[2].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		u16_cfg_changed = 1;
	}
	else if(m_t_ac10_yk_cmd[3].u16_data_or_num != g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range)
	{
		m_t_ac10_yk_cmd[3].u16_data_or_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[3].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[3].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[3].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		u16_cfg_changed = 1;
	}

	os_mut_release(g_mut_share_data);


	switch (pt_record->u8_cmd_index)
	{
		case AC10_MODULE_READ_YC_CMD:
			data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case AC10_MODULE_READ_YX_CMD:
			data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case AC10_MODULE_WRITE_YK_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);            
			if ( !u16_cfg_changed )
			{	//�����·�
				m_u8_ac10_yk_idx++;
				m_u8_ac10_yk_idx %= AC10_YK_CMD_NUM;
				m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[m_u8_ac10_yk_idx].u8_func_code;	
				m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[m_u8_ac10_yk_idx].u16_reg_addr;				
				m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[m_u8_ac10_yk_idx].u16_data_or_num;
				data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			}
			os_mut_release(g_mut_share_data);
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = AC10_MODULE_START_ADDR;
	m_u8_com1_tx_buf[1] = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
��������: u16_com1_ac10_create_fault_id		           				
��������: ����AC10ģ�������͹������ͼ������ID						
�������: u8_module_index -- ģ������
          u8_fault_type   -- ��������    		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_ac10_create_fault_id(U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET);//(FAULT_FEEDER_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= u8_fault_type;

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_ac10_module_comm_fault_alm		           				
��������: AC10ģ��ͨ�Ź��ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_ac10_module_comm_fault_alm(AC10_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_ac10_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
		{
			//��ͨ�Ź���
			v_fauid_send_fault_id_occur(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_comm_state = 1;   //����ͨ���жϱ�־
			os_mut_release(g_mut_share_data);
		}
	}
}

/****************************************************************************
 *  �������ƣ�  v_com1_ac10_bus_fault_judge(void) 
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:	��	
 *  ���ܽ���:   �жϲɼ������Ľ�����ĸ�������Ƿ��쳣���쳣������Ӧ��־λ 
 ***************************************************************************/
//	 static U32_T ac_ov_start1 = u32_delay_get_timer_val();
//	 static U32_T ac_ov_start2 = u32_delay_get_timer_val();
void v_com1_ac10_bus_fault_judge(void)
 {

	 U8_T   u8_ac_ov_delay ;	 
 	U16_T id;
 	AC10_RT_DATA_T *p_ac_rt_data = NULL;
	AC10_CFG_T *p_ac_cfg = NULL;
	 



	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_ac_ov_delay = g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt;
	p_ac_rt_data = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);
	p_ac_cfg = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10);

	//һ·������ѹ�ж�
	if( (p_ac_rt_data->f32_ac1_uv_volt >= p_ac_cfg->u16_high_volt) || 
		(p_ac_rt_data->f32_ac1_vw_volt >= p_ac_cfg->u16_high_volt) ||
		(p_ac_rt_data->f32_ac1_wu_volt >= p_ac_cfg->u16_high_volt) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0004))
		{	
			if(u32_delay_time_elapse(m_u32_ac_ov_start1,u32_delay_get_timer_val()) > u8_ac_ov_delay *1000000)
			{
				p_ac_rt_data->u16_ac_state |= 0x0004;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
	}
	else if ( (p_ac_rt_data->f32_ac1_uv_volt < (p_ac_cfg->u16_high_volt - 3.0)) && 
			  (p_ac_rt_data->f32_ac1_vw_volt < (p_ac_cfg->u16_high_volt - 3.0)) &&
			  (p_ac_rt_data->f32_ac1_wu_volt < (p_ac_cfg->u16_high_volt - 3.0)) )
	{
		m_u32_ac_ov_start1 = u32_delay_get_timer_val();
		if((p_ac_rt_data->u16_ac_state)&0x0004)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0004);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//һ·����ͣ���ж�
	if( (p_ac_rt_data->f32_ac1_uv_volt < p_ac_cfg->u16_lack_phase) && 
		(p_ac_rt_data->f32_ac1_vw_volt < p_ac_cfg->u16_lack_phase) &&
		(p_ac_rt_data->f32_ac1_wu_volt < p_ac_cfg->u16_lack_phase) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0001))
		{
			p_ac_rt_data->u16_ac_state |= 0x0001;
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OFF;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else if ( (p_ac_rt_data->f32_ac1_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) || 
			  (p_ac_rt_data->f32_ac1_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) ||
			  (p_ac_rt_data->f32_ac1_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
	{
		if((p_ac_rt_data->u16_ac_state)&0x0001)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0001);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OFF;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//һ·����ȱ���ж�
	if( !((p_ac_rt_data->u16_ac_state)&0x0001) )
	{	//��ͣ��ʱ�����Ƿ�ȱ��
		if( (p_ac_rt_data->f32_ac1_uv_volt < p_ac_cfg->u16_lack_phase) || 
			(p_ac_rt_data->f32_ac1_vw_volt < p_ac_cfg->u16_lack_phase) ||
			(p_ac_rt_data->f32_ac1_wu_volt < p_ac_cfg->u16_lack_phase) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0008))
			{
				p_ac_rt_data->u16_ac_state |= 0x0008;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_LACK_PHASE;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac1_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) && 
				  (p_ac_rt_data->f32_ac1_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) &&
				  (p_ac_rt_data->f32_ac1_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0008)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0008);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_LACK_PHASE;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//һ·����Ƿѹ�ж�
	if( !((p_ac_rt_data->u16_ac_state)&0x0009) )
	{	//��ͣ�磬�Ҳ�ȱ��ʱ�����Ƿ�Ƿѹ
		if( (p_ac_rt_data->f32_ac1_uv_volt < p_ac_cfg->u16_low_volt) || 
			(p_ac_rt_data->f32_ac1_vw_volt < p_ac_cfg->u16_low_volt) ||
			(p_ac_rt_data->f32_ac1_wu_volt < p_ac_cfg->u16_low_volt) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0002))
			{
				p_ac_rt_data->u16_ac_state |= 0x0002;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac1_uv_volt >= (p_ac_cfg->u16_low_volt + 3.0)) && 
				  (p_ac_rt_data->f32_ac1_vw_volt >= (p_ac_cfg->u16_low_volt + 3.0)) &&
				  (p_ac_rt_data->f32_ac1_wu_volt >= (p_ac_cfg->u16_low_volt + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0002)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0002);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}

	
	
	//��·������ѹ�ж�
	if( (p_ac_rt_data->f32_ac2_uv_volt >= p_ac_cfg->u16_high_volt) || 
		(p_ac_rt_data->f32_ac2_vw_volt >= p_ac_cfg->u16_high_volt) ||
		(p_ac_rt_data->f32_ac2_wu_volt >= p_ac_cfg->u16_high_volt) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0040))
		{
			if(u32_delay_time_elapse(m_u32_ac_ov_start2,u32_delay_get_timer_val()) > u8_ac_ov_delay *1000000)
			{
				p_ac_rt_data->u16_ac_state |= 0x0040;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
	}
	else if ( (p_ac_rt_data->f32_ac2_uv_volt < (p_ac_cfg->u16_high_volt - 3.0)) && 
			  (p_ac_rt_data->f32_ac2_vw_volt < (p_ac_cfg->u16_high_volt - 3.0)) &&
			  (p_ac_rt_data->f32_ac2_wu_volt < (p_ac_cfg->u16_high_volt - 3.0)) )
	{
		m_u32_ac_ov_start2 = u32_delay_get_timer_val();
		if((p_ac_rt_data->u16_ac_state)&0x0040)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0040);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//��·����ͣ���ж�
	if( (p_ac_rt_data->f32_ac2_uv_volt < p_ac_cfg->u16_lack_phase) && 
		(p_ac_rt_data->f32_ac2_vw_volt < p_ac_cfg->u16_lack_phase) &&
		(p_ac_rt_data->f32_ac2_wu_volt < p_ac_cfg->u16_lack_phase) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0010))
		{
			p_ac_rt_data->u16_ac_state |= 0x0010;
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OFF;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else if ( (p_ac_rt_data->f32_ac2_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) || 
			  (p_ac_rt_data->f32_ac2_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) ||
			  (p_ac_rt_data->f32_ac2_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
	{
		if((p_ac_rt_data->u16_ac_state)&0x0010)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0010);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OFF;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//��·����ȱ���жϣ�����������ȱ��
	if( !((p_ac_rt_data->u16_ac_state)&0x0010) )
	{	//��ͣ��ʱ�����Ƿ�ȱ��
		if( (p_ac_rt_data->f32_ac2_uv_volt < p_ac_cfg->u16_lack_phase) || 
			(p_ac_rt_data->f32_ac2_vw_volt < p_ac_cfg->u16_lack_phase) ||
			(p_ac_rt_data->f32_ac2_wu_volt < p_ac_cfg->u16_lack_phase) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0080))
			{
				p_ac_rt_data->u16_ac_state |= 0x0080;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_LACK_PHASE;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac2_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) && 
				  (p_ac_rt_data->f32_ac2_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) &&
				  (p_ac_rt_data->f32_ac2_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0080)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0080);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_LACK_PHASE;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//��·����Ƿѹ�ж�
	if( !((p_ac_rt_data->u16_ac_state)&0x0090) )
	{	//��ͣ�磬�Ҳ�ȱ��ʱ�����Ƿ�Ƿѹ
		if( (p_ac_rt_data->f32_ac2_uv_volt < p_ac_cfg->u16_low_volt) || 
			(p_ac_rt_data->f32_ac2_vw_volt < p_ac_cfg->u16_low_volt) ||
			(p_ac_rt_data->f32_ac2_wu_volt < p_ac_cfg->u16_low_volt) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0020))
			{
				p_ac_rt_data->u16_ac_state |= 0x0020;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac2_uv_volt >= (p_ac_cfg->u16_low_volt + 3.0)) && 
				  (p_ac_rt_data->f32_ac2_vw_volt >= (p_ac_cfg->u16_low_volt + 3.0)) &&
				  (p_ac_rt_data->f32_ac2_wu_volt >= (p_ac_cfg->u16_low_volt + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0020)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0020);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}	
	
	os_mut_release(g_mut_share_data);		
 }

/*************************************************************
��������: v_com1_ac10_module_unpacket		           				
��������: AC10ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
//#define		LINE_VOL_TO_PHASE_VOL		1.732	   //�ߵ�ѹת��Ϊ���ѹ����ϵ��
static void v_com1_ac10_module_unpacket(AC10_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0, u16_data;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case AC10_MODULE_READ_YC_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				AC10_RT_DATA_T *ac10 = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);

				os_mut_wait(g_mut_share_data, 0xFFFF);
				//һ·������������߲���
				u16_data = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]);
				ac10->f32_ac1_uv_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]);
				ac10->f32_ac1_vw_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]);
				ac10->f32_ac1_wu_volt = (F32_T)u16_data / 10.0;

                //��·������������߲���
				u16_data = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]);
				ac10->f32_ac2_uv_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]);
				//f32_ac_ub = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
				ac10->f32_ac2_vw_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14]);
				//f32_ac_uc = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
				ac10->f32_ac2_wu_volt = (F32_T)u16_data / 10.0;


                //�����¶�
//				u16_data = ((m_u8_com1_rx_buf[i+15]<<8) + m_u8_com1_rx_buf[i+16]);

				//V1��ĸ��ѹ
//				u16_data = ((m_u8_com1_rx_buf[i+17]<<8) + m_u8_com1_rx_buf[i+18]);
//				ac10->f32_v1_volt = ((F32_T)(S16_T)u16_data) / 10.0;
//				//A1���������ص���
//				u16_data = ((m_u8_com1_rx_buf[i+19]<<8) + m_u8_com1_rx_buf[i+20]);
//				ac10->f32_a1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
//				if (ac10->f32_a1_curr < 0)
//				{
//					ac10->f32_a1_curr = 0;
//				}
//				//V2��ص�ѹ
//				u16_data = ((m_u8_com1_rx_buf[i+21]<<8) + m_u8_com1_rx_buf[i+22]);
//				ac10->f32_v2_volt = ((F32_T)(S16_T)u16_data) / 10.0;
//				//A2��������ص���
//				u16_data = ((m_u8_com1_rx_buf[i+23]<<8) + m_u8_com1_rx_buf[i+24]);
//				ac10->f32_a2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
//				//V3��ĸ��ѹ
//				u16_data = ((m_u8_com1_rx_buf[i+25]<<8) + m_u8_com1_rx_buf[i+26]);
//				ac10->f32_v3_volt = ((F32_T)(S16_T)u16_data) / 10.0;
//				//A3���������2����
//				u16_data = ((m_u8_com1_rx_buf[i+27]<<8) + m_u8_com1_rx_buf[i+28]);
//				ac10->f32_a3_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S1������һ·��������
				u16_data = ((m_u8_com1_rx_buf[i+29]<<8) + m_u8_com1_rx_buf[i+30]);
				ac10->f32_s1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (ac10->f32_s1_curr < 0)
				{
					ac10->f32_s1_curr = 0;
				}
//				//S2������
//				u16_data = ((m_u8_com1_rx_buf[i+31]<<8) + m_u8_com1_rx_buf[i+32]);
//				ac10->f32_s2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S3��������·��������
				u16_data = ((m_u8_com1_rx_buf[i+33]<<8) + m_u8_com1_rx_buf[i+34]);
				ac10->f32_s3_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (ac10->f32_s3_curr < 0)
				{
					ac10->f32_s3_curr = 0;
				}

				//����ĸ���쳣�жϴ���
				v_com1_ac10_bus_fault_judge();								

				//��ͨ���ж�	
				ac10->u8_comm_state = 0;                                    //���ͨ���жϱ�־				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_ac10_module_comm_fault_alm(pt_record);
			}

			break;

		case AC10_MODULE_READ_YX_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])/8 + 5;  //��ַ��1���ֽڡ�������1���ֽڡ������ֽ���1���ֽڡ�CRC�����ֽڣ��ܹ���5���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				AC10_RT_DATA_T *ac10 = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);
				U16_T u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				//state1 = ((m_u8_com1_rx_buf[i+4]<<8) + m_u8_com1_rx_buf[i+3]);
				//state2 = ((m_u8_com1_rx_buf[i+6]<<8) + m_u8_com1_rx_buf[i+5]);
				//state3 = ((m_u8_com1_rx_buf[i+8]<<8) + m_u8_com1_rx_buf[i+7]);
				u16_state_new = (U16_T)((m_u8_com1_rx_buf[i+10]<<8) + m_u8_com1_rx_buf[i+9]);
				//ac10->u16_swt_state = u16_state_new;
				for(i=0; i<DC10_SWT_BRANCH_MAX; i++)
				{
					if( u16_state_new & (0x0001<<i) )
					{
						ac10->u8_swt_state[i] = 1;	//�պ�
					}
					else
					{
						ac10->u8_swt_state[i] = 0;	//�Ͽ�
					}
				}
				
				//1~3HK������բ��������
				for(i=3; i<DC10_SWT_BRANCH_MAX; i++)
				{				
					if( u16_state_new & (0x0001<<i) )
					{	//��բ					
						if( !(ac10->u16_swt_state & (0x0001<<i)) )
						{
							ac10->u16_swt_state |= (0x0001<<i);
							//v_fauid_send_fault_id_occur(u16_com1_ac10_create_fault_id(FAULT_AC10_SWT_BEGIN+i));
							v_fauid_send_fault_id_occur((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+(FAULT_AC10_SWT_BEGIN+i));
						}
					}
					else
					{	//����
						if( (ac10->u16_swt_state & (0x0001<<i)) )
						{
							ac10->u16_swt_state &= ~(0x0001<<i);
							//v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_SWT_BEGIN+i));
							v_fauid_send_fault_id_resume((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+(FAULT_AC10_SWT_BEGIN+i));
						}
					}
				}				

				//��ͨ���ж�	
				ac10->u8_comm_state = 0;                                    //���ͨ���жϱ�־				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_ac10_module_comm_fault_alm(pt_record);
			}

			break;

		case AC10_MODULE_WRITE_YK_CMD:
			rx_len = 0;
			byte_len = 8;  //��ַ��1���ֽڡ�������1���ֽڡ�2�ֽڼĴ�����ַ��2�ֽڼĴ������ݡ�CRC�����ֽڣ��ܹ���8���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i]   == m_u8_com1_tx_buf[0]) && 
						    (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1]))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				AC10_RT_DATA_T *ac10 = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);

				os_mut_wait(g_mut_share_data, 0xFFFF);	
				ac10->u8_comm_state = 0;                                    //���ͨ���жϱ�־				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
				{
					//����ͨ�Ź��ϱ���
					v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_ac10_module_comm_fault_alm(pt_record);
			}
			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_ac10_module_comm_handle		           				
��������: AC10ģ��ͨ�ż����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_ac10_module_comm_handle(AC10_RECORD_T *pt_record)
{ 
	v_com1_ac10_module_send_cmd(pt_record);
	v_com1_ac10_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= AC10_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* AC10ģ����غ���������� ***************************/

/******************************* RC10ģ����غ������忪ʼ ***************************/
/*************************************************************
��������: v_com1_motor_switch_sync		           				
��������: ϵͳ��ٿ��ص�ǰ״̬�ͼ�ؿ���״̬��������						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_motor_switch_sync()
{
	U16_T i;
	U32_T u32_cur_time;

	u32_cur_time = u32_delay_get_timer_val();
	if( u32_delay_time_elapse(m_u32_switch_sync_time, u32_cur_time) >= COM1_SWT_SYNC_UPDATE_TM )
	{	//���ڽ���ǰ��ٿ���״̬�������µ���ٿ��������Խ����ٵ�ǰ״̬���������ͬ�������ܿ������⡣
		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		for (i=0; i < FACT_SWT_CTRL_MAX; i++)
		{
			if ( g_t_swt_sheet[i].u8_swt_valid )
			{	//����ǰ��ٿ���״̬ͬ�����µ���ٿ��ؿ�����
				(*g_t_swt_sheet[i].p_u8_swt_ctrl) = (*g_t_swt_sheet[i].p_u8_swt_state);
				//ȷ����ٿ��ؿ�����ͬ�����º����·��������� 
				m_u8_rc10_swt_bak[i] = (*g_t_swt_sheet[i].p_u8_swt_ctrl);
			}
		}

		os_mut_release(g_mut_share_data);

		m_u32_switch_sync_time = u32_cur_time;//���¿�ʼ��ٿ���ͬ�����¼�ʱ
	} 
}

/*************************************************************
��������: u16_com1_rc10_module_send_cmd		           				
��������: RC10ģ�鷢�������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static U16_T u16_com1_rc10_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, update = 0;
	U16_T i, j, start_idx;
	U32_T u32_cur_time, u32_start_time;

	u32_cur_time = u32_delay_get_timer_val();
	u32_start_time = m_u32_rc10_module_ctrl_tm[pt_record->u8_module_index];
	if( !(u32_delay_time_elapse(u32_start_time, u32_cur_time) >= COM1_RC10_CTRL_SPACE_TM) )
	{	//����ͼ����������ֹƵ���·���������
		return update;
	}

	m_u32_rc10_module_ctrl_tm[pt_record->u8_module_index] = u32_delay_get_timer_val();
	memset(m_u8_rc10_write_data, 0, sizeof(m_u8_rc10_write_data));

	switch (pt_record->u8_cmd_index)
	{
		case RC10_MODULE_WRITE_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);
			start_idx = pt_record->u8_module_index * SWT_CTRL_MAX;		  
			for (j=0, i=start_idx; ((j < SWT_CTRL_MAX) && (i < FACT_SWT_CTRL_MAX));  j++, i++)
			{
				if ( g_t_swt_sheet[i].u8_swt_valid )
				{
					if ( m_u8_rc10_swt_bak[i] != (*g_t_swt_sheet[i].p_u8_swt_ctrl) )//(*g_t_swt_sheet[i].p_u8_swt_state)
					{		
						m_u8_rc10_swt_bak[i] = (*g_t_swt_sheet[i].p_u8_swt_ctrl);

						if ((*g_t_swt_sheet[i].p_u8_swt_ctrl) == 0)	//0��ʾ��բ��1��ʾ��բ
						{
							m_u8_rc10_write_data[2*j]   = 0;	//��բ����
							m_u8_rc10_write_data[2*j+1] = 1;	//��բ����
						}
						else
						{
							m_u8_rc10_write_data[2*j]   = 1;	//��բ����
							m_u8_rc10_write_data[2*j+1] = 0;	//��բ����
						}

						update = 1;
					}
				}				
			}			
			os_mut_release(g_mut_share_data);			
			break;

		default:
			break;
	}

	if (update)
	{
		m_u32_switch_sync_time = u32_delay_get_timer_val();//�������µ�ٿ��ؿ���ʱ�����¿�ʼ��ٿ���ͬ�����¼�ʱ

		i = 0;
		m_u8_com1_tx_buf[i++] = RC10_MODULE_START_ADDR + pt_record->u8_module_index;
		m_u8_com1_tx_buf[i++] = m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
		m_u8_com1_tx_buf[i++] = (m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
		m_u8_com1_tx_buf[i++] = m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
		m_u8_com1_tx_buf[i++] = (m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num >> 8);
		m_u8_com1_tx_buf[i++] = m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		m_u8_com1_tx_buf[i++] = (U8_T)(m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num * 2);
		for (j=0; j<RC10_NODE_MAX; j++)
		{
			m_u8_com1_tx_buf[i++] = 0;
			m_u8_com1_tx_buf[i++] = m_u8_rc10_write_data[j];
		}
		crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, i);
		m_u8_com1_tx_buf[i++] = (U8_T)crc;
		m_u8_com1_tx_buf[i++] = (U8_T)(crc >> 8);
	
		if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, i) != TRUE)
		{
			os_dly_wait(COM1_COMM_WAIT_TICK);     //����ʧ�ܣ����100ms������
			Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, i);
		}
	}

	return (update);
}

/*************************************************************
��������: u16_com1_rc10_create_fault_id		           				
��������: ����RC10ģ�������͹������ͼ������ID						
�������: u8_module_index -- ģ������
          u8_fault_type   -- ��������    		   				
�������: ��
����ֵ  ������ID														   				
**************************************************************/
static U16_T u16_com1_rc10_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_RC_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (u8_fault_type + u8_module_index);

	return u16_fault_id;
}

/*************************************************************
��������: v_com1_rc10_module_comm_fault_alm		           				
��������: RC10ģ��RC10���ϸ澯						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rc10_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_rc10_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_rc10_offline_cnt)
		{
			//��RC10����
			v_fauid_send_fault_id_occur(u16_com1_rc10_create_fault_id(pt_record->u8_module_index, FAULT_RC10_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_rc10[pt_record->u8_module_index].u8_comm_state = 1;   //����RC10�жϱ�־
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
��������: v_com1_rc10_module_unpacket		           				
��������: RC10ģ��������						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rc10_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case RC10_MODULE_WRITE_CMD:
			rx_len = 0;
			byte_len = 8;  //��ַ��1���ֽڡ�������1���ֽڡ��Ĵ�����ַ2���ֽڡ��Ĵ�������2���ֽڡ�CRC�����ֽڣ��ܹ���8���ֽ�

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //�Ƿ�ʱ
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1]))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				if (pt_record->u8_com1_fail_cnt >= m_u8_rc10_offline_cnt)
				{
					//����RC10���ϱ���
					v_fauid_send_fault_id_resume(u16_com1_rc10_create_fault_id(pt_record->u8_module_index, FAULT_RC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_rc10_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
��������: v_com1_rc10_module_comm_handle		           				
��������: RC10ģ��RC10�����ݴ�����						
�������: pt_record -- ģ���¼����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_rc10_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	if (u16_com1_rc10_module_send_cmd(pt_record))
	{
		v_com1_rc10_module_unpacket(pt_record);
	}

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= RC10_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //����ι���¼���־
}

/******************************* RC10ģ����غ���������� ***************************/


/*************************************************************
��������: v_com1_set_cmd_handle		           				
��������: ���ݱ��ر��ݵ����ݺ͹��������������ݱȽϣ��ж��¼�ģ������������Ƿ񼺷����ı䣬
          ������ı䣬������������ú�����ÿ����ѯ������ɺ󶼻����һ�Σ��Լӿ��������ݵ��·�							
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_com1_set_cmd_handle(void)
{
	F32_T f32_rect_out_volt[2];
	F32_T f32_rect_curr_percent[2];
	F32_T u16_pb_high_volt;
	F32_T u16_pb_low_volt;
	F32_T f32_rect_offline_out_volt;
	U16_T u16_rect_ctrl[RECT_CNT_MAX];
	F32_T f32_dcdc_out_volt;
	F32_T f32_dcdc_curr_percent;
	U32_T rect_module_num, dcdc_module_num ,i;
	U8_T  u8_batt_group_num;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	u8_batt_group_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
	rect_module_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
	u16_pb_high_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt;
	u16_pb_low_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt;
	f32_rect_offline_out_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt;
	memcpy(u16_rect_ctrl, g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl, sizeof(u16_rect_ctrl));
	
	dcdc_module_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	f32_dcdc_out_volt = g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt;
	f32_dcdc_curr_percent = g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent;
	
	os_mut_release(g_mut_share_data);
	
	for (i=0; i<u8_batt_group_num; i++)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		f32_rect_out_volt[i] = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[i];
		f32_rect_curr_percent[i] = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[i];		
		os_mut_release(g_mut_share_data);
		
		if (fabs(m_f32_rect_out_volt[i] - f32_rect_out_volt[i]) > 0.01)             //����ģ�������ѹ���ı䣬���͵�ѹ����
		{
			m_f32_rect_out_volt[i] = f32_rect_out_volt[i];
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_VOLT_CMD);
		}
		
		if (fabs(m_f32_rect_curr_percent[i] - f32_rect_curr_percent[i]) > 0.01)     //����ģ�������㱻�ı䣬������������
		{
			m_f32_rect_curr_percent[i] = f32_rect_curr_percent[i];
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_CURR_CMD);  //��������������ɣ������¼���־����ع�������
		}

		if (m_u16_pb_high_volt[i] != u16_pb_high_volt)                           //����ģ�������ѹ���ޱ��ı䣬��������ģ�������ѹ��������
		{
			m_u16_pb_high_volt[i] = u16_pb_high_volt;
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_HIGH_VOLT_CMD);
		}
		
		if (m_u16_pb_low_volt[i] != u16_pb_low_volt)                             //����ģ�������ѹ���ޱ��ı䣬��������ģ�������ѹ��������
		{
			m_u16_pb_low_volt[i] = u16_pb_low_volt;
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_LOW_VOLT_CMD);
		}
		
		if (fabs(m_f32_rect_offline_out_volt[i] - f32_rect_offline_out_volt) > 0.01)//����ģ��Ĭ�������ѹ���ı䣬��������ģ��Ĭ�������ѹ����
		{
			m_f32_rect_offline_out_volt[i] = f32_rect_offline_out_volt;
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_DEF_VOLT_CMD);
		}
	}

	for (i=0; i<rect_module_num; i++)                                                //����ģ�鿪�ػ�����
	{
		if (m_u16_rect_ctrl[i] != u16_rect_ctrl[i])
		{
			m_u16_rect_ctrl[i] = u16_rect_ctrl[i];
			
			m_t_rect_module_record[i].u8_cmd_index = RECT_MODULE_SET_STATE_CMD;
			v_com1_rect_module_comm_handle(&(m_t_rect_module_record[i]));
		}	
	}

	if (dcdc_module_num > 0)
	{
		if (fabs(m_f32_dcdc_out_volt - f32_dcdc_out_volt) > 0.01)         //ͨ��ģ�������ѹ���ı䣬���͵�ѹ����
		{
			m_f32_dcdc_out_volt = f32_dcdc_out_volt;
			v_com1_dcdc_module_send_broadcast_cmd(DCDC_MODULE_SET_VOLT_CMD);
			v_com1_dcdc_module_send_broadcast_cmd(DCDC_MODULE_SET_DEF_VOLT_CMD);
		}
		
		if (fabs(m_f32_dcdc_curr_percent - f32_dcdc_curr_percent) > 0.01) //ͨ��ģ�������㱻�ı䣬������������
		{
			m_f32_dcdc_curr_percent = f32_dcdc_curr_percent;
			v_com1_dcdc_module_send_broadcast_cmd(DCDC_MODULE_SET_CURR_CMD);
		}
	}

}


/*************************************************************
��������: v_com1_module_comm_task
��������: �¼�ģ��ͨ�ż����ݴ���������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
__task void v_com1_module_comm_task(void)
{
	U16_T i, batt_num, bms_num1, bms_num2, rect_num, cps_num; 
	U16_T ups_num, dc10_num, ac10_num, rc10_num;
	BMS_TYPE_E e_bms_type;
	U8_T u8_rect_broadcast_cmd_index = 0;     //�㲥��������
	U8_T u8_dcdc_broadcast_cmd_index = 0;     //�㲥��������

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);     //����ι���¼���־
	
	//�������´�������������ң�ؿ���ֵ
	os_mut_wait(g_mut_share_data, 0xFFFF);
	memset(g_t_share_data.t_sys_cfg.t_swt_ctrl, 0, (RC10_MODULE_MAX * sizeof(SWT_CTRL_T)));
	os_mut_release(g_mut_share_data);

	m_u32_switch_sync_time = u32_delay_get_timer_val();
	m_u32_ac_ov_start1 = u32_delay_get_timer_val();
	m_u32_ac_ov_start2 = u32_delay_get_timer_val();
	while (1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		m_u8_bms_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_bms_offline_cnt;
		m_u8_rect_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_offline_cnt;
		m_u8_dcdc_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt;
		m_u8_dcac_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_offline_cnt;
//		m_u8_d21_offline_cnt = m_u8_rect_offline_cnt;	//��������ģ���ͨ�ż����ο�ֵ
		m_u8_dc10_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt;
		m_u8_ac10_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt; 
		m_u8_rc10_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_offline_cnt; 

		//���Ѳ��ͨ�Ŵ���
		batt_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		bms_num1 = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num;
		bms_num2 = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num;
		e_bms_type = g_t_share_data.t_sys_cfg.t_batt.e_bms_type;
		rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
		if (g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol == DCDC_MODBUS)
			cps_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
		else
			cps_num = 0;
		ups_num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num;
		dc10_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u8_dc10_module_num;
		ac10_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num;
		rc10_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num;
		os_mut_release(g_mut_share_data);
		
		if (batt_num > 0)
		{
			for (i=0; i<bms_num1; i++)
			{
				v_com1_bms_comm_handle(&(m_t_bms_group1_record[i]));
				v_com1_set_cmd_handle();
			}
			
			if (batt_num > 1)
			{
				for (i=0; i<bms_num2; i++)
				{
					if (e_bms_type == B21)
						m_t_bms_group2_record[i].u8_start_addr = 5;
					else if (e_bms_type == B3)
						m_t_bms_group2_record[i].u8_start_addr = 2;
					else
						m_t_bms_group2_record[i].u8_start_addr = 1;
						
					m_t_bms_group2_record[i].u8_total_index = bms_num1 + i;
					v_com1_bms_comm_handle(&(m_t_bms_group2_record[i]));
					v_com1_set_cmd_handle();
				}
			}
		}
		
		//����ģ��ͨ�Ŵ���
		for (i=0; i<rect_num; i++)
		{
			v_com1_rect_module_comm_handle(&(m_t_rect_module_record[i]));
			v_com1_set_cmd_handle();
		}
		
		for (i=0; i<batt_num; i++)
		{
			v_com1_rect_module_send_broadcast_cmd(i, u8_rect_broadcast_cmd_index);
		}
		u8_rect_broadcast_cmd_index++;
		u8_rect_broadcast_cmd_index %= RECT_MODULE_BROADCAST_CMD_CNT;
		v_com1_set_cmd_handle();
		
		//ͨ��ģ��ͨ�Ŵ���
		if (cps_num > 0)
		{
			for (i=0; i<cps_num; i++)
			{
				v_com1_dcdc_module_comm_handle(&(m_t_dcdc_module_record[i]));
				v_com1_set_cmd_handle();
			}
			
			v_com1_dcdc_module_send_broadcast_cmd(u8_dcdc_broadcast_cmd_index);
			u8_dcdc_broadcast_cmd_index++;
			u8_dcdc_broadcast_cmd_index %= DCDC_MODULE_BROADCAST_CMD_CNT;
			v_com1_set_cmd_handle();
		}
		
		//���ģ��ͨ�Ŵ���
		for (i=0; i<ups_num; i++)
		{
			v_com1_dcac_module_comm_handle(&(m_t_dcac_module_record[i]));
			v_com1_set_cmd_handle();
		}

	    //D21ģ��ͨ�Ŵ���
//		os_mut_wait(g_mut_share_data, 0xFFFF);
//		num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num;
//		os_mut_release(g_mut_share_data);
//		if(num > 0)
//		{
//			v_com1_d21_module_comm_handle(&m_t_d21_module_record);
//		}

		//1#DC10ģ��ͨ�Ŵ���
		if (dc10_num)
		{
			v_com1_dc10_module_comm_handle(&m_t_dc10_module_record);
		}

		//2#DC10ģ��ͨ�Ŵ���
		if (ac10_num)
		{
			v_com1_ac10_module_comm_handle(&m_t_ac10_module_record);
		}

		//RC10ģ��ͨ�Ŵ���
		v_com1_motor_switch_sync();
		for (i=0; i<rc10_num; i++)
		{
			v_com1_rc10_module_comm_handle(&m_t_rc10_module_record[i]);
		}
	}
}
