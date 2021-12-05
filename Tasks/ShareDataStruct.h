/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：ShareDataStruct.h
版    本：1.00
创建日期：2012-04-12
作    者：郭数理
功能描述：共享数据结构定义头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-04-12  1.00     创建
**************************************************************/


#ifndef __SHARE_DATA_STRUCT_H_
#define __SHARE_DATA_STRUCT_H_

#include <rtl.h>


/*
缩写     全称         翻译
volt     voltage      电压
curr     current      电流/现在
res      resistance   电阻
batt     battery      电池
thr      threshold    阀值
in       input        输入
out      output       输出
insu     insulation   绝缘
num      number       数量
cfg      configure    配置
def      default      默认
comm     commucation  通信
mgmt     management   管理
lang     language     语言
ctl      control      控制
bms      battery monitor set 电池巡检
dur      duration     持续
comp     compensation 补偿
dis      discharge    放电
pos      positive     正
neg      negative     负
rt       real time    实时
freq     frequency    频率
pf       power factor 功率因数
*/

#define CFG_DATA_VERSION_L  4     //配置数据区版本的低字节
#define CFG_DATA_VERSION_H  1     //配置数据区版本的高字节

#define ACS_FEEDER_MODULE_MAX 3   // 交流屏馈线模块最大数量
#define DC10_SWT_BRANCH_MAX  16    // 开关量最大支路数

#define BATT_METER_MAX  5     // 电池巡检最大数量
#define BATT_GROUP_MAX  2     // 电池最大组数
#define BATT_CELL_MAX   120   // 单体电池最在数量
#define RECT_CNT_MAX    24    // 整流模块最大数量
#define SWT_BRANCH_MAX  20    // 开关量最大支路数
#define	SWT_NAME_CHAR_LEN 32  //可配开关名称字符长度

#define FEEDER_PANEL_MODULE_MAX 4  // 一个屏的馈线模块最大数量
#define FEEDER_PANEL_MAX  8        // 馈线屏最大数量
#define FEEDER_BRANCH_MAX 64       // 馈出支路最大数量

#define DCDC_MODULE_MAX        8   // 通信模块最大数量
#define DCDC_FEEDER_MODULE_MAX 1   // 通信屏馈线模块最大数量

#define DCAC_MODULE_MAX        8   // 逆变模块最大数据
#define DCAC_FEEDER_MODULE_MAX 1   // 逆变屏馈线模块最大数量

#define RELAY_MAX       6     // 继电器最大数量

#define RC10_MODULE_MAX        16   // 系统允许带RC10模块最大数量
#define RC10_NODE_MAX          16   // RC10模块干结点最大数量
#define SWT_CTRL_MAX           (RC10_NODE_MAX / 2)   // RC10模块开关控制最大数量
#define TTL_SWT_CTRL_MAX       (RC10_MODULE_MAX * SWT_CTRL_MAX)     // 理论上可通过RC10控制的最大开关总数量

#define FACT_SWT_CTRL_MAX      (3 * SWT_CTRL_MAX)	//实际定义可控开关路数

#define	BATT_CURR_FROM_SENSOR	0		//1表示电池电流由DC10的传感器测量，	0表示由DC10的分流器测量

/* 系统电压等级 */
typedef enum
{
	VOLT_LEVEL_220=0,      //220v
	VOLT_LEVEL_110,        //110v
}SYS_VOLT_LEVEL_E;

/* 电池管理方式 */
typedef enum
{
	AUTO_MODE=0,	       //自动
	MANUAL_MODE,           //手动
}BATT_MGMT_MODE_E;

/* 系统显示语言 */
typedef enum
{
	CHINESE=0,             //中文
	ENGLISH,               //英文
}LANG_TYPE_E;

/* LCD方向 */
typedef enum
{
	LCD_HORIZONTAL=0,      //水平
	LCD_VERTICAL,          //竖显
}LCD_DIRECTION_E;

/* 蜂鸣器告警设置 */
typedef enum
{
	BEEP=0,	               //鸣叫
	QUIET,                 //静音
}BUZZER_ALM_SET_E;

/* 是否保存 */
typedef enum
{
	NO_SAVE=0,             //不保存
	SAVE,                  //保存
}SAVE_E;

/* 开启/关闭枚举 */
typedef enum
{
	CLOSE=0,               //关闭
	OPEN,                  //开启
}OPEN_E;

/* 母线段数枚举 */
typedef enum
{
	ONE=0,                //一段
	TWO,                  //两段
}BUS_NUM_E;

/* 有/无枚举 */
typedef enum
{
	NO=0,                  //无
	HAVE,                  //有
}HAVE_E;

/* 硅链配置枚举 */
typedef enum
{
	NO_DIODE_CHAIN=0,      //无硅链控制
	STEP_5_4V,             //5级4V
	STEP_5_7V,             //5级7V
	STEP_7_3V,             //7级3V
	STEP_7_5V,             //7级5V
}DIODE_CHAIN_E;

/* 开关输入配置 */
typedef enum
{
	NORMALLY_OPEN=0,       //常开
	NORMALLY_CLOSE,        //常闭
}SWT_INPUT_TYPE_E;	

/* 后台通信协议 */
typedef enum
{
	BS_MODBUS=0,           //MODBUS
	BS_CDT,                //CDT
	BS_ADJUST,             //校准
	BS_IEC103,             //IEC103
}BACKSTAGE_PROTROL_E;

/* 串口通信速率 */
typedef enum
{
	COM_BAUD_1200=0,
	COM_BAUD_2400,
	COM_BAUD_4800,
	COM_BAUD_9600,
	COM_BAUD_19200
}COM_BAUD_E;

/* CAN口通信速率 */
typedef enum
{
	CAN_BAUD_125K=0,
	CAN_BAUD_50K,
	CAN_BAUD_20K,
	CAN_BAUD_10K
}CAN_BAUD_E;

/* 串口校验类型 */
typedef enum
{
	ODD_PARITY=0,          //奇校验
	EVEN_PARITY,           //偶校验
	NONE_PARITY,           //无校验
}COM_PARITY_E;

/* 交流相数 */
typedef enum
{
	AC_3_PHASE=0,          //三相
	AC_1_PHASE,            //单相
}AC_INPUT_TYPE_E;

/* 系统内部通信协议书 */
typedef enum
{
	MODBUS=0,
}INTERNAL_PROTOCOL_E;

/* 充电模块额定电流 */
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

/* 交流供电输入 */
typedef enum
{
	NONE_PATH=0,           //无
	ONE_PATH,              //1路
	TWO_PATH,              //2路
}AC_INPUT_NUM_E;

/* 分流器额定电压 */
typedef enum
{
	VOLT_75MV=0,           //75mv
	VOLT_50MV,             //50mv
}SHUNT_RATED_VOLT_E;

/* 电池巡检类型	*/
typedef enum
{
	B21=0,
	B3,
	B4,
}BMS_TYPE_E;

/* 电池状态 */
typedef enum
{
	FLO=0,                 // 浮充
	EQU,                   // 均充
	DIS,                   // 核容
}BATT_STATE_E;


/* 模块状态 */
typedef enum
{
	START_UP=0,
	SHUT_DOWN,
	EXCEPTION,
}MODULE_STATE_E;

/* 逆变模块状态 */
typedef enum
{
	INVERT,
	BYPASS,
	SHUT,
}DCAC_STATE_E;

/* 限流方式 */
typedef enum
{
	AUTO_LIMIT_CURR=0,	   //自动
	MANUAL_LIMIT_CURR,     //手动
}LIMIT_CURR_E;

/* 通信模块协议 */
typedef enum
{
	DCDC_MODBUS=0,	       //MODBUS协议
	DCDC_CAN,              //CAN协议
}DCDC_PROTOCOL_E;

/* 通信模块额定电流 */
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

/* 绝缘测量方式 */
typedef enum
{
    INSU_PROJECT = 0,  //工程模式
    INSU_DEBUG         //调试模式
}INSU_MEAS_WAY_E;

/* 参数值符号 */
typedef enum
{
    POSITIVE = 0,  //正
    NEGATIVE       //负
}SIGN_E;

/* 遥控开关对像结构 */
typedef struct                                           /* 遥控开关属性 */
{
	U8_T              u8_ctrl_old;                       // 原来的开关控制值，范围0~1(0表示分闸，1表示合闸)，默认0
	U8_T              u8_state;                          // 当前开关状态，范围0~1(0表示分闸，1表示合闸)，默认0
} SWT_OBJ_T;
	
/******************************** 系统配置参数定义 *********************************/
//g_t_share_data.t_sys_cfg.t_batt_mgmt
typedef struct                                           /* 电池管理部分参数 */
{
	BATT_MGMT_MODE_E  e_mode[2];		                 // 电池管理方式 手动、自动
	U16_T             u16_rate_c10;                      // 电池标称容量，范围30~3000Ah，默认100Ah
	F32_T             f32_equ_volt;                      // 电池均充电压，范围电池组欠压点~过压点，默认245V
	F32_T             f32_flo_volt;                      // 电池浮充电压，范围电池组欠压点~过压点，默认235V
	F32_T             f32_limit_curr;                    // 电池充电限流点，范围0.05~充电过流点，默认0.1C10
	
	//转均充判据1
	F32_T             f32_to_equ_curr;		             // 电池转均充电流，范围0.04~充电限流点，默认0.08C10
	U16_T             u16_to_equ_dur_time;               // 电流满足转均充条件的持续时间，范围1~999秒，默认60秒
	//转均充判据2
	U16_T             u16_equ_cycle;	    	         // 电池定时均充周期，范围1~9999小时，默认4320小时
	//转均充判据3
	U16_T             u16_ac_fail_time;                  // 交流停电时长，范围1~999分钟，默认20分钟
	//转均充判据4
	F32_T             f32_low_cap;                       // 电池容量低于，范围0.8~0.95C10，默认0.8C10
	
	//转浮充判据1
	F32_T             f32_to_flo_curr;			         // 电池转浮充电流，范围0.01~0.03C10，默认0.02C10
	U16_T             u16_curr_go_time;                  // 电池转浮充倒计时时间，范围1~360分钟，默认180分钟
	//转浮充判据2
	U16_T             u16_max_equ_time;	                 // 电池均充保护时间，范围1~999分钟，默认720分钟
	
	//电池放电终止判据1
	U16_T             u16_total_end_volt;                // 电池放电终止电压，范围190~220V，默认200V
	//电池放电终止判据2
	F32_T             f32_cell_end_volt;                 // 电池放电终止单体电压，范围1.8~12V，默认1.8V
	//电池放电终止判据3
	U16_T             u16_max_dis_time;                  // 电池放电终止时间，范围1~999分钟，默认600分钟
	
	//电池均充补偿电压
	F32_T             f32_batt_temp_comp_volt;           // 温度补偿电压，范围0~500mV，默认0mV
	
	//电池组电压告警门限
	F32_T             f32_high_volt_limit;               // 电池组过压点，范围220~320V，默认264V
	F32_T             f32_low_volt_limit;                // 电池组欠压点，范围186~220，默认187V
	F32_T             f32_high_curr_limit;               // 电池充电过流点，范围0.1~1.0C10，默认0.2
	
	/* 放电曲线 */
	F32_T             f32_batt_10c_dis_rate;             // 电池1.0C放电率   [0~10.0], 默认0.3
	F32_T             f32_batt_09c_dis_rate;             // 电池0.9C放电率   [0~10.0], 默认0.4
	F32_T             f32_batt_08c_dis_rate;             // 电池0.8C放电率   [0~10.0], 默认0.5
	F32_T             f32_batt_07c_dis_rate;             // 电池0.7C放电率   [0~10.0], 默认0.7
	F32_T             f32_batt_06c_dis_rate;             // 电池0.6C放电率   [0~10.0], 默认0.8
	F32_T             f32_batt_05c_dis_rate;             // 电池0.5C放电率   [0~10.0], 默认0.9
	F32_T             f32_batt_04c_dis_rate;             // 电池0.4C放电率   [0~10.0], 默认2.2
	F32_T             f32_batt_03c_dis_rate;             // 电池0.3C放电率   [0~10.0], 默认3.3
	F32_T             f32_batt_02c_dis_rate;             // 电池0.2C放电率   [0~10.0], 默认4
	F32_T             f32_batt_01c_dis_rate;             // 电池0.1C放电率   [0~10.0], 默认10
}BATT_MGMT_CFG_T;

//g_t_share_data.t_sys_cfg.t_sys_param
typedef struct                                           /* 系统参数配置 */
{
	/* 直流系统类型 */
	SYS_VOLT_LEVEL_E        e_volt_level;                // 系统类型，220/110V
	
	/* 屏数设置部分参数 */
	BUS_NUM_E               e_bus_seg_num;			     // 直流母线段数，范围ONE/TWO，默认ONE
	U8_T                    u8_dc_feeder_panel_num;      // 一段直流馈电屏面数，范围0~4，默认1
	U8_T                    u8_seg1_fdl_master_add;      // 一段绝缘主机地址，范围14~111，默认14
	U8_T                    u8_dc2_feeder_panel_num;     // 二段直流馈电屏面数，范围0~4，默认0
	U8_T                    u8_seg2_fdl_master_add;      // 二段绝缘主机地址，范围14~111，默认30
	U8_T                    u8_batt_group_num;           // 电池组数，范围0~2，默认1
	U8_T                    u8_rc10_module_num;          // RC10模块数，范围0~16，默认0
	
	/* 语言 */
	LANG_TYPE_E             e_lang;                      // 语言，中文/英文，默认中文
	LCD_DIRECTION_E         e_lcd_driection;             // 显示屏方向，水平/竖显，默认水平
	
	/* 手动限流调节 */
	LIMIT_CURR_E            e_limit_curr;                // 限流方式，自动/手动，默认手动
	F32_T                   f32_manual_limit_curr;       // 手动限流点，5%~110%，默认100%
	
	/* 通讯报警条件 */
	U8_T                    u8_fc10_offline_time;        // 馈线模块通讯中断时间，范围5~60S，默认10
	U8_T                    u8_bms_offline_cnt;          // 电池巡检通讯中断命令个数，范围4~99个，默认10
	U8_T                    u8_rect_offline_cnt;         // 整流模块通讯中断命令个数，范围4~99个，默认10
	U8_T                    u8_dcdc_offline_cnt;         // 通信模块通讯中断命令个数，范围4~99个，默认10
	U8_T                    u8_dcac_offline_cnt;         // 逆变模块通讯中断命令个数，范围4~99个，默认10
	U8_T                    u8_ats_offline_cnt;          // ATS模块通讯中断命令个数，范围4~99个，默认10       xj-2020-6-23  重新定义为交流故障确认时间  
	U8_T                    u8_acm_offline_cnt;          // 多功能电表通讯中断命令个数，范围4~99个，默认10
	U8_T                    u8_dc10_offline_cnt;         // DC10模块通讯中断命令个数，范围4~99个，默认10
	U8_T                    u8_rc10_offline_cnt;         // RC10模块通讯中断命令个数，范围4~99个，默认10

	/* 绝缘测量参数设置 */
	CAN_BAUD_E              e_can_baud;		             // CAN通信速率，125K｜50K｜20K｜10K，默认125K
	INSU_MEAS_WAY_E         e_insu_meas_way;             // 母线电阻测量方式，工程模式和调试模式，默认工程模式
    U8_T 					u8_res_switch_delay;         // 平衡及不平衡电桥投切延时，1~120可设，以秒为单位，默认2秒
    U16_T                   u16_insu_sensor_range;       // 支路传感器量程，1~500可设，以mA为单位，默认10mA
    U8_T                    u8_insu_bus_err_confirm;     // 母线绝缘压差报警确认时间，1~180可设，以秒为单位，默认3秒
    U8_T                    u8_insu_meas_period;         // 绝缘定期测量周期时间，0~180可设，以小时为单位，默认24小时
    U8_T                    u8_insu_meas_hour;           // 强制启动绝缘测量的时间-时，0~23小时，默认8点
    U8_T                    u8_insu_meas_min;            // 强制启动绝缘测量的时间-分，0~59分钟，默认0分

	/* 电流修正参数设置 */
	SIGN_E                  e_load_sign;		         // 负载电流零点修正值符号，正｜负, 默认正
	F32_T                   f32_load_zero;               // 负载电流零点修正绝对值，范围[0~2.0], 默认0.0
    SIGN_E                  e_bat1_sign;		         // 电池1电流零点修正值符号，正｜负, 默认正
	F32_T                   f32_bat1_zero;               // 电池1电流零点修正绝对值，范围[0~2.0], 默认0.0
    SIGN_E                  e_bat2_sign;		         // 电池2电流零点修正值符号，正｜负, 默认正
	F32_T                   f32_bat2_zero;               // 电池2电流零点修正绝对值，范围[0~2.0], 默认0.0
	
	/* 其它 */
	BUZZER_ALM_SET_E        e_buzzer_state;              // 蜂鸣器告警设置：鸣叫/静音，默认鸣叫
	SAVE_E                  e_minor_fault_save;          // 次要告警保存设置：0-不保存，1-保存
	SAVE_E                  e_general_fault_save;        // 一般告警保存设置：0-不保存，1-保存
	OPEN_E                  e_curr_imbalance_alm;        // 电流不平衡报警：关闭/开启，默认关闭
	U32_T                   u32_password; 				 // 用户操作密码设置，5位密码，默认为11111，超级密码为02051，维护级密码02012
	U8_T                    u8_local_addr;               // 本机地址，范围1~255，默认1
	BACKSTAGE_PROTROL_E     e_backstage_protrol;         // 后台通信协议，MODBUS/CDT/IEC101/IEC102，默认MODBUS
	COM_BAUD_E              e_backstage_baud;            // 波特率，默认9600
	COM_PARITY_E            e_backstage_parity;          // 校验，默认奇检验
	HAVE_E                  e_compare_time;              // B码对时：无/有，默认无
	
	/* 故障输出 */
	U8_T                    u8_ac_fault_output;          //交流进线故障，范围0~6，默认1
	U8_T                    u8_dc_bus_fault_output;      //直流母线故障，范围0~6，默认2
	U8_T                    u8_batt_fault_output;        //电池异常故障，范围0~6，默认3
	U8_T                    u8_insu_fault_output;        //绝缘下降故障，范围0~6，默认4
	U8_T                    u8_rect_fault_output;        //充电模块故障，范围0~6，默认0
	U8_T                    u8_fuse_fault_output;        //电池熔断器故障，范围0~6，默认0
	U8_T                    u8_dcac_fault_output;        //通信及逆变电源故障，范围0~6，默认0
	U8_T                    u8_feeder_fault_output;      //馈线支路故障，范围0~6，默认0
	U8_T                    u8_batt_equ_output;          //均充指示，范围0~6，默认5
	U8_T                    u8_batt_dis_output;          //核容指示，范围0~6，默认6
}MISC_PARAM_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_ac
typedef struct                                           /* 交流参数配置 */
{
	AC_INPUT_NUM_E   e_path;                             // 交流输入路数，1路/2路/无，默认2路
	AC_INPUT_TYPE_E  e_phase;                            // 交流输入相数，三相/单相，默认三相
	                                                     
	U16_T            u16_high_volt;                      // 交流过压点，范围220~530V，默认456V
	U16_T            u16_low_volt;                       // 交流欠压点，范围187~380V，默认327V
	U16_T            u16_lack_phase;                     // 交流缺相点，范围0~380V，默认200V，在单相情况下，此项无效
}AC_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect
typedef struct                                           /* 整流模块配置 */
{
	U8_T                 u8_d21_num;                     // D21模块数量，范围0~1，默认0
	U8_T                 u8_rect_num;                    // 模块数量，范围1~24，默认4
	INTERNAL_PROTOCOL_E  e_protocol;                     // 通信协议，协议固定为modbus，不可设
	RATED_CURR_E         e_rated_curr;                   // 额定电流，5A/7A/10A/20A/30A/35A/40A/50A可选，默认10A
	F32_T                f32_out_volt[2];                // 输出电压，这个值界面不能设定，由电池管理任务修改
	F32_T                f32_curr_percent[2];            // 限流百分比，这个值界面不能设定，由电池管理任务修改
	F32_T                f32_max_out_volt;               // 输出电压上限，范围220~286V，默认286V，这个值界面没有相关的设定项，采用默认值
	F32_T                f32_min_out_volt;               // 输出电压下限，范围176~220V，默认176V，这个值界面没有相关的设定项，采用默认值
	F32_T                f32_offline_out_volt;           // 默认输出电压，范围176~286V，默认220V
	F32_T                f32_offline_curr_percent;       // 默认输出限流比，范围5~110%，默认110%
}RECT_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[]
typedef struct
{
	U8_T u8_join_way;	                //开关接入方式，0：常开，1：常闭
	U8_T u8_meas_type;	                //开关测量类型，0：告警，1：状态
	U8_T u8_fault_type;	                //开关故障类型，只有配置为告警量测量，才需根据此值归类输出
	                                    //0：交流故障类
	                                    //1：直流母线故障类
	                                    //2：电池故障类
	                                    //3：绝缘故障类
	                                    //4：充电模块故障类
	                                    //5：通信模块故障类
	                                    //6：UPS模块故障类
	                                    //7：馈线支路故障类

    U8_T u8_reserve;		            //预留
	char s_ch_name[SWT_NAME_CHAR_LEN];	//开关测量中文名称，最多16个汉字
	char s_en_name[SWT_NAME_CHAR_LEN];	//开关测量英文名称，最多32个字母
} SWT_CFG_ITEM_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_swt
typedef struct                                           /* 馈出开关配置 */
{
	SWT_CFG_ITEM_T t_swt_item[SWT_BRANCH_MAX];           //开关配置数组
	U8_T           u8_state_cnt;                         //开关状态量的个数
}SWT_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc
typedef struct                                           /* 直流监测数据配置 */
{
	HAVE_E              e_have_cb;                       // 控母配置，无/有，默认有
	DIODE_CHAIN_E       e_diode_chain_ctl;               // 硅链控制，无/5级4伏/5级7伏/7级3伏/7级5伏，默认无

	SHUNT_RATED_VOLT_E  e_shunt_rated_volt;              // 分流器额定电压，75mv/50mv，默认75mv
	U16_T               u16_load_shunt_range;            // 负载分流器量程，范围25~2000A，默认100A
	U16_T               u16_batt1_shunt_range;           // 电池1分流器量程，范围25~2000A，默认100A
	U16_T               u16_batt2_shunt_range;           // 电池2分流器量程，范围25~2000A，默认100A
	U16_T               u16_feeder_shunt_range;          // 支路电流传感器量程，范围10~1000A，默认100A
	                                                     
	U16_T               u16_pb_high_volt;                // 合母过压门限，范围220~320V，默认286V
	U16_T               u16_pb_low_volt;                 // 合母欠压门限，范围186~220V，默认187V
	U16_T               u16_cb_high_volt;                // 控母过压门限，范围220~242V，默认235V
	U16_T               u16_cb_low_volt;                 // 控母欠压门限，范围198~220V，默认205V
	U16_T               u16_bus_high_volt;               // 母线过压门限，范围220~320V，默认286V
	U16_T               u16_bus_low_volt;                // 母线欠压门限，范围186~220V，默认187V
	U16_T               u16_cb_output_volt;              // 控母输出电压（硅链输出电压），范围210~230V，默认220V

	U16_T               u16_insu_volt_imbalance;         // 绝缘电压不平衡，范围20~100V，默认50V
	U16_T               u16_insu_res_thr;                // 绝缘电阻门限，范围5~99K，默认25K
	U16_T               u16_dc_bus_input_ac_thr;         // 交流窜直流报警门限值，范围1~50V，默认10V
}DC_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10
typedef struct                                           /* DC10模块配置 */
{
	U8_T                 u8_dc10_module_num;			 //DC10模块个数，范围0~1，默认0

	U16_T                u16_shunt_a1_rated_volt;        // 分流器额定电压，范围1~500mv，默认75mv
	U16_T                u16_a1_s1_shunt_range;          // A1或S1电流量程(分流器及传感器共用)，范围25~2000A，默认100A
	U16_T                u16_shunt_a2_rated_volt;        // 分流器额定电压，范围1~500mv，默认75mv
	U16_T                u16_a2_s2_shunt_range;          // A2或S2电流量程(分流器及传感器共用)，范围25~2000A，默认100A
	U16_T                u16_shunt_a3_rated_volt;        // 分流器额定电压，范围1~500mv，默认75mv	
	U16_T                u16_a3_s3_shunt_range;          // A3或S3电流量程(分流器及传感器共用)，范围25~2000A，默认100A

	U16_T                u16_cb_output_volt;             //二段直流控母输出

	//DC10模块主要用来测量第二段直流母线电压、电流，以及硅链控制，所以以下几个量暂未用到
	U16_T                u16_ac_meas_modle;              //DC10模块交流测量模式，默认0x0100一路三相
	U16_T                u16_ac_meas_num;                //DC10模块交流测量路数，默认0x0002固定一路	

#define	RELAY_OUT_NUM    9
	U8_T                 u8_relay_out[RELAY_OUT_NUM];    // 故障输出路数
}DC10_CFG_T;

//g_t_share_data.t_sys_cfg.t_feeder_panel[].t_feeder_module[]
//g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[]
//g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[]
//g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[]
typedef struct                                           /* 馈线模块配置 */
{
	U8_T               u8_feeder_num;                    // 该馈线模块的实际支路数
	
	U8_T               u8_alarm_feeder_num;              // 告警量支路数，范围0~64，默认21
	SWT_INPUT_TYPE_E   e_alarm_type;                     // 告警量输入方式，常开｜常闭，默认常开

	U8_T               u8_state_feeder_num;              // 状态量支路数，范围0~64，默认21
	SWT_INPUT_TYPE_E   e_state_type;                     // 状态量输入方式，常开｜常闭，默认常开

	U8_T               u8_insu_feeder_num;               // 绝缘量支路数，范围0~64，默认21
	U8_T               u8_curr_feeder_num;               // 电流量支路数，范围0~64，默认0	
}FEEDER_MODULE_CFG_T;


//g_t_share_data.t_sys_cfg.t_feeder_panel[]
typedef struct                                           /* 馈线屏配置 */
{
	U8_T                 u8_feeder_module_num;           //馈线监测模块个数，范围0~4，默认1
	U8_T                 u8_feeder_branch_num;           //馈线支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_state_feeder_num;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_insu_feeder_num;             //绝缘量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_alarm_feeder_num;            //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_curr_feeder_num;             //电流量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_feeder_start_num[FEEDER_PANEL_MODULE_MAX];//馈线模块支路在馈电屏支路中的起始支路号
	FEEDER_MODULE_CFG_T  t_feeder_module[FEEDER_PANEL_MODULE_MAX]; //馈线模块配置数据                
}DC_FEEDER_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_dc_panel
typedef struct                                           /* 直流充电屏配置 */
{
	RECT_CFG_T  t_rect;                                  // 整流模块配置
	AC_CFG_T    t_ac;                                    // 直流系统交流参数配置
	DC_CFG_T    t_dc;                                    // 直流监测数据配置
	SWT_CFG_T   t_swt;                                   // 馈出开关配置 
	DC10_CFG_T  t_dc10;                                  // 1#DC10配置数据，用于二段直流母线相关测量                                              
}DC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_batt
typedef struct                                           /* 电池组配置 */
{
	BMS_TYPE_E   e_bms_type;                             // 电池仪种类，B21/B3/B4，默认B21
	                                                     
	F32_T        f32_cell_high_volt;                     // 单体过压，范围2~15V，默认12.05V
	F32_T        f32_cell_low_volt;                      // 单体欠压，范围1.8~12V，默认10.08V
	F32_T        f32_tail_high_volt;                     // 末端单体过压，范围2~15V，默认12.05V
	F32_T        f32_tail_low_volt;                      // 末端单体欠压，范围1.8~12V，默认10.08V
	                                                     
	struct                                               
	{                                                    
		U8_T     u8_bms_num;                             // 电池仪个数，范围0~5，默认0
		U8_T     u8_cell_total_num;                      // 实际单体电池节数
		U8_T     u8_cell_num[BATT_METER_MAX];            // 电池仪的电池单体数
		                                                 // B21最多可测24节电池
													     // B3最多可测54节电池，只有u8_cell_num[0]和u8_cell_num[1]有效
		                                                 // B4最多可测120节电池，只有u8_cell_num[0]有效
		                                                 // u8_cell_num[0]的范围0~120，默认24
														 // u8_cell_num[1]的范围0~54，默认24
														 // u8_cell_num[2]的范围0~24，默认24
														 // u8_cell_num[3]的范围0~24，默认24
														 // u8_cell_num[4]的范围0~24，默认24
	}t_batt_group[BATT_GROUP_MAX];
}BATT_CFG_T;

//g_t_share_data.t_sys_cfg.t_dcdc_panel
typedef struct                                           /* 通信电源屏配置 */                                           
{
	U8_T                 u8_dcdc_module_num;             //通信模块个数，范围0~8，默认0
	U8_T                 u8_feeder_module_num;           //馈线监测模块个数，范围0~1，默认0
	U8_T                 u8_feeder_branch_num;           //馈线支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_state_feeder_num;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_alarm_feeder_num;            //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_curr_feeder_num;             //电流量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U16_T                u16_feeder_shunt_range;         // 支路电流传感器量程，范围10~1000A，默认100A

	DCDC_PROTOCOL_E      e_protocol;                     // 通信协议，协议为modbus/can，默认modbus
	DCDC_RATED_CURR_E    e_rated_curr;                   // 额定电流，5A/10A/20A/30A/40A/50A/60A/80A/100A可选，默认100A
	F32_T                f32_out_volt;                   // 输出电压，范围40~60，默认48
	F32_T                f32_curr_percent;               // 限流百分比，范围10%~100%，默认100

	FEEDER_MODULE_CFG_T  t_feeder_module[DCDC_FEEDER_MODULE_MAX];                // 馈线模块配置
}DCDC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_dcac_panel
typedef struct                                           /* 逆变电源屏配置 */
{
	U8_T                 u8_dcac_module_num;             //逆变模块个数，范围0~8，默认0
	U8_T                 u8_feeder_module_num;           //馈线监测模块个数，范围0~1，默认0
	U8_T                 u8_feeder_branch_num;           //馈线支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_state_feeder_num;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_alarm_feeder_num;            //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_curr_feeder_num;             //电流量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U16_T                u16_feeder_shunt_range;         // 支路电流传感器量程，范围10~1000A，默认100A

	INTERNAL_PROTOCOL_E  e_protocol;                     // 通信协议，协议固定为modbus，不可设

	FEEDER_MODULE_CFG_T  t_feeder_module[DCAC_FEEDER_MODULE_MAX];                // 馈线模块配置


}DCAC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_ac_panel.t_feed
typedef struct                                           /* 馈线屏配置 */
{
	//交流系统馈线模块配置参数
	U8_T                 u8_feeder_module_num;           //馈线监测模块个数，范围0~3，默认0
	U8_T                 u8_feeder_branch_num;           //馈线支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_state_feeder_num;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_alarm_feeder_num;            //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_curr_feeder_num;             //电流量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	U8_T                 u8_feeder_start_num[ACS_FEEDER_MODULE_MAX];//馈线模块支路在馈电屏支路中的起始支路号
	U16_T                u16_feeder_shunt_range;         // 支路电流传感器量程，范围10~1000A，默认100A

	FEEDER_MODULE_CFG_T  t_feeder_module[ACS_FEEDER_MODULE_MAX];                // 馈线模块配置               
}AC_FEEDER_CFG_T;

//g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.
typedef struct                                           /* DC10模块配置 */
{
	//交流系统DC10模块配置参数
	U16_T            	 u16_high_volt;                  // 交流过压点，范围220~530V，默认456V
	U16_T            	 u16_low_volt;                   // 交流欠压点，范围187~380V，默认327V
	U16_T            	 u16_lack_phase;                 // 交流缺相点，范围0~380V，默认200V，在单相情况下，此项无效

	U8_T                 u8_ac10_module_num;			 //DC10模块个数，范围0~1，默认0

	U16_T                u16_ac_meas_modle;              //DC10模块交流测量模式，默认0x0200两路三相
	U16_T                u16_ac_meas_num;                //DC10模块交流测量路数，默认0x0000一路优先

//	U16_T                u16_shunt_a1_rated_volt;        // 分流器额定电压，范围1~500mv，默认75mv
	U16_T                u16_a1_s1_shunt_range;          // A1或S1电流量程(分流器及传感器共用)，范围25~2000A，默认100A
//	U16_T                u16_shunt_a2_rated_volt;        // 分流器额定电压，范围1~500mv，默认75mv
	U16_T                u16_a2_s2_shunt_range;          // A2或S2电流量程(分流器及传感器共用)，范围25~2000A，默认100A
//	U16_T                u16_shunt_a3_rated_volt;        // 分流器额定电压，范围1~500mv，默认75mv	
	U16_T                u16_a3_s3_shunt_range;          // A3或S3电流量程(分流器及传感器共用)，范围25~2000A，默认100A	

//#define	RELAY_OUT_NUM    9
//	U8_T                 u8_relay_out[RELAY_OUT_NUM];    // 故障输出路数
}AC10_CFG_T;

//g_t_share_data.t_sys_cfg.t_ac_panel
typedef struct                                           /* 直流充电屏配置 */
{
	AC10_CFG_T  	t_ac10;                                  // 2#DC10配置数据，用于二段直流母线相关测量
	AC_FEEDER_CFG_T t_feed;                                  // 交流馈线参数配置 	                                              
}AC_PANEL_CFG_T;

//g_t_share_data.t_sys_cfg.t_swt_ctrl[]
typedef struct                                           /* 系统开关控制配置 */
{
	U8_T                 u8_swt_ctrl[SWT_CTRL_MAX];      //RC10模块开关控制值，范围0~1(0分闸，1合闸)，默认0
}SWT_CTRL_T;

//g_t_share_data.t_sys_cfg.t_ctl
typedef struct                                           /* 系统控制数据 */
{
                                                         // 电池手动控制
	U16_T u16_batt[2];                                   // bit0~1:手动充电控制，0:浮充，1:均充，2:核容
	                                                     // bit2~15:保留   
	              
	U16_T u16_ac;                                        // bit0~1:交流工作策略，0：一路优先，1：固定一路，2：二路优先，3：固定二路
														 // bit2~15:保留
	
	U16_T u16_rect_ctrl[RECT_CNT_MAX];                   // 充电模块状态控制，1：关机，0：开机

	U8_T  u8_sys_relay;                                  // 系统总故障继电器，0：不动作；1：动作
	U8_T  u8_relay[RELAY_MAX];                           // 6个继电器，0：不动作；1：动作

	U8_T  u8_swt_ctrl[SWT_CTRL_MAX];                     // 由界面设置的开关控制值，0：分闸；1：合闸
}SYS_CTL_T;

//g_t_share_data.t_sys_cfg
typedef struct                                           /*配置数据*/
{
	BATT_MGMT_CFG_T        t_batt_mgmt;                       // 电池管理配置
	MISC_PARAM_CFG_T       t_sys_param;                       // 系统参数配置

	AC_PANEL_CFG_T         t_ac_panel;                        // 交流屏配置 
	DC_PANEL_CFG_T         t_dc_panel;                        // 直流屏配置
	DC_FEEDER_PANEL_CFG_T  t_feeder_panel[FEEDER_PANEL_MAX];  // 馈电屏配置
	DCDC_PANEL_CFG_T       t_dcdc_panel;                      // 通信电源屏配置
	DCAC_PANEL_CFG_T       t_dcac_panel;                      // 逆变电源屏配置

	BATT_CFG_T             t_batt;                            // 电池组配置
	SYS_CTL_T              t_ctl;                             // 系统控制

	SWT_CTRL_T             t_swt_ctrl[RC10_MODULE_MAX];       // 电操开关控制
}SYS_CFG_T;


/***************************** 实时数据定义 *********************************************/
//g_t_share_data.t_rt_data.t_batt.t_batt_group[]
typedef struct                                           /* 电池组实时数据 */
{
	F32_T   f32_max_cell_volt;                           // 单体电压最高值
	F32_T   f32_min_cell_volt;                           // 单体电压最低值
	F32_T   f32_temperature1;                            // 环境温度1
	F32_T   f32_temperature2;                            // 环境温度2
	F32_T   f32_cell_volt[BATT_CELL_MAX];                // 单体电压
	F32_T   f32_cell_max_volt_id;                        // 最高电压单体号，后台用
	F32_T   f32_cell_min_volt_id;                        // 最低电压单体号，后台用
	U8_T    u8_cell_max_volt_id;                         // 最高电压单体号，显示用
	U8_T    u8_cell_min_volt_id;                         // 最低电压单体号，显示用
	U8_T    u8_cell_state[BATT_CELL_MAX];                // 单体过欠压，0：正常，1：过压，2：欠压
	U8_T    u8_comm_state[BATT_METER_MAX];               // [电池组电池仪个数]，每个元素代表一个电池仪通信状态，0：正常，1：中断
	F32_T   f32_capacity;                                // 电池组当前容量，0~C10Ah
}BATT_GROUP_RT_DATA_T;

//g_t_share_data.t_rt_data.t_batt
typedef struct                                           /* 电池实时数据通信*/
{
	BATT_STATE_E   e_state[2];                              // 电池组当前状态
	U16_T          u16_time_from_change_state[2];           // 从上次电池状态转变开始，到现在所过去时间，浮充状态单位为小时，均充和核容状态单位是分钟
	F32_T          f32_total_capacity;                   // 电池总容量，各组电池容量之和
	BATT_GROUP_RT_DATA_T t_batt_group[BATT_GROUP_MAX];   // 电池组实时数据
}BATT_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel.t_rect[]
typedef struct                                           /* 整流模块实时数据 */
{
	U32_T   u32_hw_ver;                                  // 硬件版本号
	U32_T   u32_sw_ver;                                  // 软件版本号
	U32_T	u32_barcode;                                 // 设备条形码
	F32_T   f32_out_volt;                                // 输出电压
	F32_T   f32_out_curr;                                // 输出电流
	F32_T   f32_curr_percent;                            // 模块限流百分数
	F32_T   f32_max_out_volt;                            // 输出电压上限
	F32_T   f32_min_out_volt;                            // 输出电压下限
	//F32_T   f32_offline_out_volt;                        // 模块默认输出电压，范围176~286V，默认220V
	//F32_T   f32_offline_out_curr;                        // 模块默认输出电流，范围5~110%，默认110%
	
	U16_T   u16_state;                                   // 模块状态信息
	                                                     // bit0:模块开关机状态      1：关机，0：开机
										                 // bit1:模块自动/手动状态   1：手动，0：自动
	                                                     // bit2:保护状态            1：保护，0：正常
	                                                     // bit3:模块故障状态        1：故障，0：正常
	                                                     // bit4:软件过压状态        1：有软件过压，0：正常
	                                                     // bit5:保留			
	                                                     // bit6:保留			
	                                                     // bit7:保留			
	                                                     // bit8:模块开关机状态      1：关机，0：开机
	                                                     // bit9:模块自动/手动状态   1：手动，0：自动
	                                                     // bit10:模块输出欠压状态   1：欠压，0：正常
	                                                     // bit11:模块温度状态       1：过温，0：正常
	                                                     // bit12:模块输入状态       1：过压或欠压，0：正常
	                                                     // bit13:模块输入状态       1：缺相，0：正常
	                                                     // bit14:模块开关机状态     1：关机，0：开机
	                                                     // bit15:模块输出过压状态   1：过压，0：正常

	BOOL_T  b_ctl_mode;                                  // 模块控制方式             0：自动，1：手动
	MODULE_STATE_E  e_module_state;                      // 模块开关机、异常状态     0：开机，1：关机，2：异常
	U8_T    u8_comm_state;                               // 通讯状态，0：正常，1：中断
}RECT_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel.t_ac
typedef struct                                           /* 交流采集单元实时数据 */
{
	// 状态量实时数据
	U16_T   u16_state;                                   // 交流状态信息，16位，每位代表一个状态
	                                                     // bit0:交流一路停电		
	                                                     // bit1:交流一路欠压		
	                                                     // bit2:交流一路过压		
	                                                     // bit3:交流一路缺相		
	                                                     // bit4:交流二路停电		
	                                                     // bit5:交流二路欠压		
	                                                     // bit6:交流二路过压		
	                                                     // bit7:交流二路缺相
														 // bit8:原分流器故障改为d21模块通信中断
	
	// 模拟量实时数据
	F32_T   f32_first_path_volt_uv;                      // 1路UV线电压
	F32_T   f32_first_path_volt_vw;                      // 1路VW线电压
	F32_T   f32_first_path_volt_wu;                      // 1路WU线电压
	        
	F32_T   f32_second_path_volt_uv;                     // 2路UV线电压
	F32_T   f32_second_path_volt_vw;                     // 2路VW线电压
	F32_T   f32_second_path_volt_wu;                     // 2路WU线电压
}AC_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel.t_dc
typedef struct                                           /* 直流采集单元实时数据 */
{
	// 状态信息                                          
	U16_T   u16_state;		                             // 直流状态信息
	                                                     // bit0:一段合母过压		
	                                                     // bit1:一段合母欠压		
	                                                     // bit2:一段控母过压		
	                                                     // bit3:一段控母欠压
														 // bit4:一段母线过压		
	                                                     // bit5:一段母线欠压		
	                                                     // bit6:一组电池过压		
	                                                     // bit7:一组电池欠压				
	                                                     // bit8:DC10模块通信中断//电流传感器异常	
	                                                     // bit9: 一组电池过流		
	                                                     // bit10:二组电池过流		
	                                                     // bit11:一段母线电流不平衡		
	                                                     // bit12:一段母线电压不平衡		
	                                                     // bit13:一段母线绝缘下降		
	                                                     // bit14:母线绝缘继电器告警（直流采样使用）		
	                                                     // bit15:直流窜入交流告警	
														 	
	U16_T   u16_state2;		                             // 直流状态信息
	                                                     // bit0:二段合母过压		
	                                                     // bit1:二段合母欠压		
	                                                     // bit2:二段控母过压		
	                                                     // bit3:二段控母欠压
														 // bit4:二段母线过压		
	                                                     // bit5:二段母线欠压		
	                                                     // bit6:二组电池过压		
	                                                     // bit7:二组电池欠压				
	                                                     // bit8:预留
	                                                     // bit9: 预留		
	                                                     // bit10:预留		
	                                                     // bit11:二段母线电流不平衡		
	                                                     // bit12:二段母线电压不平衡		
	                                                     // bit13:二段母线绝缘下降		
	                                                     // bit14:二母线绝缘继电器告警（直流采样使用）		
	                                                     // bit15:二段直流窜入交流告警	                                                     
	                                                     
	// 直流采集单元实时数据
	//一段母线实时数据-由SC32自身采集                              
	F32_T   f32_pb_volt;                                 // 一段合母电压
	F32_T   f32_cb_volt;                                 // 一段控母电压
	F32_T   f32_load_curr;                               // 一段控母电流，也称负载电流	                                                     
	F32_T   f32_batt_volt;                               // 一组电池电压

	//二段母线实时数据-由DC10模块采集
	F32_T   f32_pb2_volt;                                // 二段合母电压
	F32_T   f32_cb2_volt;                                // 二段控母电压
	F32_T   f32_load2_curr;                              // 二段控母电流，也称负载电流	                                                     
	F32_T   f32_batt2_volt;                              // 二组电池电压

	F32_T   f32_batt_curr[BATT_GROUP_MAX];               // 一、二组电池电流
	F32_T   f32_batt_total_curr;                         // 电池总电流，两组电池电流之和
	                                                     
	F32_T   f32_temperature;                             // 环境温度

	// 馈线模块测量的母线绝缘信息（显示优先使用馈线模块测出的信息）
	//一段母线绝缘实时数据
	F32_T   f32_cb_pos_to_gnd_volt;                      // 一段控母正对地电压
	F32_T   f32_pb_pos_to_gnd_volt;                      // 一段合母正对地电压	                                                     
	F32_T   f32_bus_neg_to_gnd_volt;                     // 一段母线负对地电压

	F32_T   f32_bus_pos_to_gnd_res;                      // 一段母线正对地电阻
	F32_T   f32_bus_neg_to_gnd_res;                      // 一段母线负对地电阻

	F32_T   f32_feeder_min_to_gnd_res;                   // 一段支路对地绝缘电阻最小值
	F32_T   f32_bus_to_gnd_ac_volt;                      // 一段母线对地交流电压有效值

	//二段母线绝缘实时数据
	F32_T   f32_cb2_pos_to_gnd_volt;                     // 二段控母正对地电压
	F32_T   f32_pb2_pos_to_gnd_volt;                     // 二段合母正对地电压	                                                     
	F32_T   f32_bus2_neg_to_gnd_volt;                    // 二段母线负对地电压

	F32_T   f32_bus2_pos_to_gnd_res;                     // 二段母线正对地电阻
	F32_T   f32_bus2_neg_to_gnd_res;                     // 二段母线负对地电阻

	F32_T   f32_feeder2_min_to_gnd_res;                  // 二段支路对地绝缘电阻最小值
	F32_T   f32_bus2_to_gnd_ac_volt;                     // 二段母线对地交流电压有效值
	
	//自带直流测量的母线绝缘信息
	F32_T   f32_dc_cb_pos_to_gnd_volt;                   // 一段控母正对地电压
	F32_T   f32_dc_pb_pos_to_gnd_volt;                   // 一段合母正对地电压	                                                     
	F32_T   f32_dc_bus_neg_to_gnd_volt;                  // 一段母线负对地电压                                                 
}DC_RT_DATA_T;  

//g_t_share_data.t_rt_data.t_dc_panel.t_dc10.
typedef struct                                           /* DC10模块实时数据 */
{
	F32_T   f32_temperature;                             // 环境温度
		
	F32_T   f32_v1_volt;                                 // 控母电压
	F32_T   f32_a1_curr;                                 // 控母电流，也称负载电流	                                                     
	F32_T   f32_v2_volt;                                 // 电池电压
	F32_T   f32_a2_curr;                                 // 电池电流
	F32_T   f32_v3_volt;                                 // 合母电压
	F32_T   f32_a3_curr;                                 // 电池2电流

	F32_T   f32_s1_curr;                                 // 控母电流，也称负载电流
	F32_T   f32_s2_curr;                                 // 电池电流
	F32_T   f32_s3_curr;                                 // 电池2电流

	U16_T   u16_swt_state;                               //DC10开关量信息
	                                                     //bit0~15:开关01~16状态（0：正常，1：故障）
	U8_T    u8_swt_state[DC10_SWT_BRANCH_MAX];	         //直流系统DC10模块开关状态量，1：闭合，0：断开，16路开关量的状态，用于后台通信

	U8_T    u8_comm_state;                               //通讯状态，0：正常，1：中断
}DC10_RT_DATA_T;

//g_t_share_data.t_rt_data.t_feeder_panel[].t_feeder[]
//g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[]
//g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[]
//g_t_share_data.t_rt_data.t_ac_panel.t_feeder[]
typedef struct                                           /* 支路实时数据 */
{
	U8_T u8_alarm;                                       // 开关告警，00本路无开关告警检测，01表示正常，02表示告警
	U8_T u8_state;                                       // 开关状态，00本路无开关状态检测，01表示断开，02表示闭合
	U8_T  u8_insu_state;                                 // 支路绝缘状态，00本路无绝缘检测，01表示正常，02表示绝缘异常，03表示绝缘传感器异常
	U8_T  u8_curr_state;                                 // 支路电流传感器状态，00本路无支路电流检测，01表示正常，03表示电流传感器异常
	
	F32_T f32_res;                                       // 支路绝缘电阻
	F32_T f32_curr;                                      // 支路电流
}FEEDER_RT_DATA_T;

//g_t_share_data.t_rt_data.t_feeder_panel[].t_feeder_module[]
//g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[]
//g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[]
//g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[]
typedef struct                                           /* 馈线模块实时数据 */
{
	U32_T   u32_hw_ver;                                  // 硬件版本号
	U32_T   u32_sw_ver;                                  // 软件版本号
	U32_T	u32_barcode;                                 // 设备条形码
	U8_T    u8_comm_state;                               // 通讯状态，0：正常，1：中断
}FEEDER_MODULE_RT_DATA_T;

//g_t_share_data.t_rt_data.t_ac_panel.t_ac10.
typedef struct                                           /* 用于交流柜测量的2#DC10模块实时数据 */
{	//一路为三相进线	
	F32_T   f32_ac1_uv_volt;                             // 一路UV线电压
	F32_T   f32_ac1_vw_volt;                             // 一路VW线电压	                                                     
	F32_T   f32_ac1_wu_volt;                             // 一路WU线电压
	F32_T   f32_s1_curr;                                 // 一路交流电流

	//二路为单相进线
	F32_T   f32_ac2_uv_volt;                             // 二路UV线电压
	F32_T   f32_ac2_vw_volt;                             // 二路VW线电压	                                                     
	F32_T   f32_ac2_wu_volt;                             // 二路WU线电压		
	F32_T   f32_s3_curr;                                 // 二路交流电流

	U16_T   u16_ac_state;                                // 交流故障状态

	U16_T   u16_swt_state;                               //AC10开关量信息
	                                                     //bit0~15:开关01~16状态（0：正常，1：故障）
	U8_T    u8_swt_state[DC10_SWT_BRANCH_MAX];	         //交流系统DC10模块开关状态量，1：闭合，0：断开，16路开关量的状态，用于后台通信

	U8_T    u8_comm_state;                               //通讯状态，0：正常，1：中断
} AC10_RT_DATA_T;

//g_t_share_data.t_rt_data.t_ac_panel.
typedef struct                                           /* 用于交流柜测量的2#DC10模块实时数据 */
{	
	AC10_RT_DATA_T			t_ac10;

	//交流系统馈线实时数据
	FEEDER_MODULE_RT_DATA_T t_feeder_module[ACS_FEEDER_MODULE_MAX];   //馈线模块数据
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //馈出支路数据
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //支路传感器状态，00本路无传感器，01表示传感器正常，02表示传感器异常
	                                                                  //把u8_sensor_state提到FEEDER_RT_DATA_T结构外面可节省内存4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX

	U8_T                    u8_total_swt_falut;                       //总开关报警状态，00表示正常，01表示告警
}AC_PANEL_RT_DATA_T;//                                         

//g_t_share_data.t_rt_data.t_dc_panel.t_swt
typedef struct
{
	U8_T u8_raw_swt_state[SWT_BRANCH_MAX];               //开关状态量，1：闭合，0：断开，自带20路开关量的状态，用于模拟DC10通信
	U8_T u8_swt_state[SWT_BRANCH_MAX];                   //开关状态量，1：闭合，0：断开，自带20路开关量的状态，用于后台通信
	U8_T u8_state[SWT_BRANCH_MAX];                       //开关状态量，1：闭合，0：断开
	U8_T u8_alm[SWT_BRANCH_MAX];                         //开关告警量，1：告警，0：不告警

#define	ECSWT_MAX_NUM_FROM_FDL		(11 * SWT_CTRL_MAX)
	U8_T u8_fdl_swt_state[ECSWT_MAX_NUM_FROM_FDL];       //开关状态量，1：闭合，0：断开，由FC10模块测量到的开关状态值
}SWT_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dc_panel
typedef struct                                           /* 直流屏实时数据 */
{
	// 状态量实时数据
	U16_T   u16_total_state;                             // 直流总状态信息，16位，每位代表一个状态
	                                                     // bit0:交流故障		
	                                                     // bit1:直流母线故障		
	                                                     // bit2:电池故障		
	                                                     // bit3:绝缘故障		
	                                                     // bit4:充电模块故障		
	                                                     // bit5:通信电源故障		
	                                                     // bit6:逆变电源故障		
	                                                     // bit7:馈出开关故障
	                                                     // bit8:直流系统直流母线绝缘异常
	                                                     // bit9:直流系统馈出支路绝缘异常
	                                                     // bit10:直流系统蓄电池组电压异常
	                                                     
	AC_RT_DATA_T    t_ac;                                // 直流系统中交流进线实时数据
	DC_RT_DATA_T    t_dc;                                // 直流实时数据
	DC10_RT_DATA_T  t_dc10;                              // 1#DC10模块测量的二段直流母线相关实时数据	 
	RECT_RT_DATA_T  t_rect[RECT_CNT_MAX];                // 模块实时数据
	SWT_RT_DATA_T   t_swt;                               // 开关量实时数据              
}DC_PANEL_RT_DATA_T;


//g_t_share_data.t_rt_data.t_feeder_panel[]
typedef struct                                                        /* 馈线屏实时数据 */
{
	FEEDER_MODULE_RT_DATA_T t_feeder_module[FEEDER_PANEL_MODULE_MAX]; //馈线模块数据
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //馈出支路数据
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //支路传感器状态，00本路无传感器，01表示传感器正常，02表示传感器异常
	                                                                  //把u8_sensor_state提到FEEDER_RT_DATA_T结构外面可节省内存4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX
	
	U8_T                    u8_total_swt_falut;                       //总开关报警状态，00表示正常，01表示告警
}FEEDER_PANEL_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[]
typedef struct                                           /* 通信模块实时数据 */
{
	U32_T   u32_hw_ver;                                  // 硬件版本号
	U32_T   u32_sw_ver;                                  // 软件版本号
	U32_T	u32_barcode;                                 // 设备条形码
	F32_T   f32_out_volt;                                // 输出电压
	F32_T   f32_out_curr;                                // 输出电流
	F32_T   f32_curr_percent;                            // 模块限流百分数
	F32_T   f32_max_out_volt;                            // 输出电压上限
	F32_T   f32_min_out_volt;                            // 输出电压下限
	//F32_T   f32_offline_out_volt;                        // 模块默认输出电压，范围176~286V，默认220V
	//F32_T   f32_offline_out_curr;                        // 模块默认输出电流，范围5~110%，默认110%
	
	U16_T   u16_state;                                   // 模块状态信息
	                                                     // bit0:模块开关机状态      1：关机，0：开机
										                 // bit1:模块自动/手动状态   1：手动，0：自动
	                                                     // bit2:保护状态            1：保护，0：正常
	                                                     // bit3:模块故障状态        1：故障，0：正常
	                                                     // bit4~15:保留

	BOOL_T  b_ctl_mode;                                  // 模块控制方式             0：自动，1：手动
	MODULE_STATE_E  e_module_state;                      // 模块开关机、异常状态     0：开机，1：关机，2：异常
	U8_T    u8_comm_state;                               // 通讯状态，0：正常，1：中断
}DCDC_MODULE_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dcdc_panel
typedef struct                                                        /* 通信电源屏实时数据 */
{
	F32_T                   f32_dcdc_bus_volt;                        //通信系统母线电压，所有通信模块中的最高输出电压为母线电压
	F32_T                   f32_dcdc_bus_curr;                        //通信系统母线电流，所有通信模块输出电流之和为母线电流
	DCDC_MODULE_RT_DATA_T   t_dcdc_module[DCDC_MODULE_MAX];           //通信模块数据
	FEEDER_MODULE_RT_DATA_T t_feeder_module[DCDC_FEEDER_MODULE_MAX];  //馈线模块数据
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //馈出支路数据
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //支路传感器状态，00本路无传感器，01表示传感器正常，02表示传感器异常
	                                                                  //把u8_sensor_state提到FEEDER_RT_DATA_T结构外面可节省内存4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX

	U8_T                    u8_total_swt_falut;                       //总开关报警状态，00表示正常，01表示告警
}DCDC_PANEL_RT_DATA_T;


//g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[]
typedef struct                                           /* 逆变模块实时数据 */
{
	U32_T   u32_hw_ver;                                  // 硬件版本号
	U32_T   u32_sw_ver;                                  // 软件版本号
	U32_T	u32_barcode;                                 // 设备条形码
	F32_T   f32_out_volt;                                // 逆变模块输出电压
	F32_T   f32_out_curr;                                // 逆变模块输出电流
	F32_T   f32_out_freq;                                // 逆变模块输出频率
	F32_T   f32_out_power_factor;                        // 逆变模块输出功率因数
	F32_T   f32_inverter_volt;                           // 逆变模块逆变电压
	F32_T   f32_bypass_input_volt;                       // 旁路输入电压
	F32_T   f32_bypass_input_freq;                       // 旁路输入频率
	F32_T   f32_batt_input_volt;                         // 电池输入电压   
	F32_T   f32_active_power;                            // 逆变模块输出有功功率
	F32_T   f32_apparen_power;                           // 逆变模块输出视在功率
	F32_T   f32_load_ratio;                              // 逆变模块输出负载率
	F32_T   f32_temperature;                             // 模块温度
	F32_T   f32_outage_capacity_ratio;                   // 逆变模块容量降额比
	F32_T   f32_bypass_high_volt;                        // 旁路电压上限
	F32_T   f32_bypass_low_volt;                         // 旁路电压下限
	
	U16_T   u16_state;                                   // 模块状态信息
	                                                     // bit0:模块开关机状态      1：关机，0：开机
										                 // bit1:模块工作方式        1：在线，0：后备
	                                                     // bit2:模块故障状态        1：故障，0：正常
														 // bit3:模块过载状态        1：过载，0：正常
														 // bit4:模块温度状态        1：过温，0：正常
														 // bit5:模块电池欠压        1：欠压，0：正常
														 // bit6:模块旁路输入状态    1：过压或欠压，0：正常
														 // bit7:输出方式            1：逆变输出    0：旁路输出
	                                                     // bit8~15:保留

	BOOL_T  b_alarm_state;                               // 模块报警状态             0：正常，1：报警
	DCAC_STATE_E  e_module_state;                      // 模块状态     0：开机/逆变，1：开机/旁路，2：关机
	U8_T    u8_comm_state;                               // 通讯状态，0：正常，1：中断
}DCAC_MODULE_RT_DATA_T;

//g_t_share_data.t_rt_data.t_dcac_panel
typedef struct                                                        /* 通信电源屏实时数据 */
{
	DCAC_MODULE_RT_DATA_T   t_dcac_module[DCAC_MODULE_MAX];           //通信模块数据
	FEEDER_MODULE_RT_DATA_T t_feeder_module[DCAC_FEEDER_MODULE_MAX];  //馈线模块数据
	FEEDER_RT_DATA_T        t_feeder[FEEDER_BRANCH_MAX];              //馈出支路数据
	U8_T                    u8_sensor_state[FEEDER_BRANCH_MAX];       //支路传感器状态，00本路无传感器，01表示传感器正常，02表示传感器异常
	                                                                  //把u8_sensor_state提到FEEDER_RT_DATA_T结构外面可节省内存4*FEEDER_BRANCH_MAX*FEEDER_PANEL_MAX

	U8_T                    u8_total_swt_falut;                       //总开关报警状态，00表示正常，01表示告警
}DCAC_PANEL_RT_DATA_T;

//g_t_share_data.t_rt_data.t_rc10[]
typedef struct                                                        /* RC10模块实时数据 */
{
	U8_T                    u8_comm_state;                       //RC10模块通信状态，00表示正常，01表示告警
}RC10_RT_DATA_T;

//g_t_share_data.t_rt_data
typedef struct                                                /*实时数据*/
{
	BATT_RT_DATA_T 	        t_batt;                           // 电池组
	AC_PANEL_RT_DATA_T      t_ac_panel;                       // 交流屏2#DC10模块测量的交流柜电压及电流相关实时数据
	DC_PANEL_RT_DATA_T      t_dc_panel;                       // 直流屏
	FEEDER_PANEL_RT_DATA_T  t_feeder_panel[FEEDER_PANEL_MAX]; // 馈线屏
	DCDC_PANEL_RT_DATA_T    t_dcdc_panel;                     // 通信屏
	DCAC_PANEL_RT_DATA_T    t_dcac_panel;                     // UPS屏
	RC10_RT_DATA_T          t_rc10[RC10_MODULE_MAX];          // RC10状态
	U8_T                    t_u8_dc10_fault_out[7];           // 用于DC10干接点输出
}RT_DATA_T;


/************************ 电力版直流校准数据定义 ************************************/
//g_t_share_data.t_dc_adjust_data
typedef struct
{
	F32_T   f32_batt_volt;                 //电池电压                    
	F32_T   f32_pb_volt;                   //合母电压
	F32_T   f32_cb_volt;                   //控母电压
	F32_T   f32_bus_neg_to_gnd_volt;       //母线负对地电压

	F32_T   f32_load_curr_1;                //负载电流1
	F32_T   f32_load_curr_2;                //负载电流2
	F32_T   f32_batt1_curr_1;               //一组电池电流1
	F32_T   f32_batt1_curr_2;               //一组电池电流2
	F32_T   f32_batt2_curr_1;               //二组电池电流1
	F32_T   f32_batt2_curr_2;               //二组电池电流2
}DC_ADJUST_DATA_T;


/************************ 交流校准数据定义 ************************************/
//g_t_share_data.t_ac_adjust_data
typedef struct
{
	F32_T   f32_first_path_volt_uv;        //1路UV线电压
	F32_T   f32_first_path_volt_vw;        //1路VW线电压
	F32_T   f32_first_path_volt_wu;        //1路WU线电压
	        
	F32_T   f32_second_path_volt_uv;       //2路UV线电压
	F32_T   f32_second_path_volt_vw;       //2路VW线电压
	F32_T   f32_second_path_volt_wu;       //2路WU线电压
}AC_ADJUST_DATA_T;

/***************************** 校准系数数据 *********************************************/
//g_t_share_data.t_coeff_data.
typedef struct
{
	F32_T			f32_ac1_uv_slope;      // 交流1路UV斜率
	F32_T			f32_ac1_vw_slope;      // 交流1路VW斜率
	F32_T			f32_ac1_wu_slope;      // 交流1路WU斜率
	F32_T			f32_ac2_uv_slope;      // 交流2路UV斜率
	F32_T			f32_ac2_vw_slope;      // 交流2路VW斜率
	F32_T			f32_ac2_wu_slope;      // 交流2路WU斜率

	F32_T			f32_v1_vol_slope;      // V1电压测量斜率
	S16_T			s16_v1_vol_zero;       // V1电压测量零点
	F32_T			f32_v2_vol_slope;      // V2电压测量斜率
	S16_T			s16_v2_vol_zero;       // V2电压测量零点
	F32_T			f32_v3_vol_slope;      // V3电压测量斜率
	S16_T			s16_v3_vol_zero;       // V3电压测量零点

	S16_T			s16_a1_fixed_zero;     // A1电路固定零点
	S16_T			s16_a2_fixed_zero;     // A2电路固定零点
	S16_T			s16_a3_fixed_zero;     // A3电路固定零点

	F32_T			f32_a1_curr_slope;     // A1电流测量斜率
	F32_T			f32_a1_curr_zero;      // A1电流测量零点
	F32_T			f32_a2_curr_slope;     // A2电流测量斜率
	F32_T			f32_a2_curr_zero;      // A2电流测量零点
	F32_T			f32_a3_curr_slope;     // A3电流测量斜率
	F32_T			f32_a3_curr_zero;      // A3电流测量零点

	F32_T			f32_ref_volt;          // AD测量基准电压
	F32_T			f32_neg_vol_slope;     // 负对地电压测量斜率
	S16_T			s16_neg_vol_zero;      // 负对地电压测量零点
	
}COEFF_DATA_T;

/***************************** 校准用AD值 *********************************************/
//g_t_share_data.t_ad_data.
typedef struct
{
	U16_T	u16_ac1_uv_ad;                 // 交流1路UV采样AD值
	U16_T	u16_ac1_vw_ad;                 // 交流1路VW采样AD值
	U16_T	u16_ac1_wu_ad;                 // 交流1路WU采样AD值
	U16_T	u16_ac2_uv_ad;                 // 交流2路UV采样AD值
	U16_T	u16_ac2_vw_ad;                 // 交流2路VW采样AD值
	U16_T	u16_ac2_wu_ad;                 // 交流2路WU采样AD值
	U16_T	u16_temp_ad;                   // 环境温度采样AD值
	U16_T	u16_v1_ad;                     // V1采样AD值
	U16_T	u16_v2_ad;                     // V2采样AD值
	U16_T	u16_v3_ad;                     // V3采样AD值
	U16_T	u16_a1_ad;                     // A1采样AD值
	U16_T	u16_a2_ad;                     // A2采样AD值
	U16_T	u16_a3_ad;                     // A3采样AD值
	U16_T	u16_neg_v_ad;                     //负对地电压采样AD
}AD_DATA_T;

/************************* 共享数据定义 ***************************************/
//g_t_share_data
typedef struct
{
	SYS_CFG_T         t_sys_cfg;           // 系统配置
	RT_DATA_T         t_rt_data;           // 实时数据
	AC_ADJUST_DATA_T  t_ac_adjust_data;    // 交流校准输入实际数值
	DC_ADJUST_DATA_T  t_dc_adjust_data;    // 直流校准输入实际数值
	COEFF_DATA_T      t_coeff_data;        // 校准系数
	AD_DATA_T         t_ad_data;           // 校准AD值

	SWT_OBJ_T         t_swt_obj[SWT_CTRL_MAX]; //
}SHARE_DATA_T;


/* 电操开关是否有效类型 */
typedef enum
{	
	INVALID = 0,     //无效
	VALID,           //有效
}SWT_VALID_E;

/* 电操开关对象结构类型 */
typedef struct
{
	U8_T u8_swt_valid;          //该电操开关是否有效标识，0无效，1有效
	U8_T *p_u8_swt_ctrl;        //每个电操开关控制值指针
	U8_T *p_u8_swt_state;       //每个电操开关当前合分状态值指针
}RC10_SWT_ITEM_T;

/* 由馈线模块采样的电操开关对应转换结构类型 */
typedef struct
{
	U8_T *p_u8_swt_dest;        //每个电操开关当前状态值
	U8_T *p_u8_swt_sorce;       //每个电操开关由FC10模块采样值
	U8_T data_sorce;			//0:来自FC10模块，1:来自SC32模块
#define FC10_SWT		0
#define SC32_SWT		1
}FDL_SWT_PAIR_T;

#endif
