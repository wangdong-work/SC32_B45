/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：FaultId.h
版    本：1.00
创建日期：2012-05-15
作    者：郭数理
功能描述：故障ID宏定义头文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-15  1.00     创建
**************************************************************/

#ifndef __FAULT_ID_H_
#define __FAULT_ID_H_


/* 分组号 */
#define FAULT_GROUP_OFFSET     10        //分组号偏移
#define FAULT_AC_GROUP         0         //交流
#define	FAULT_DC_BUS_GROUP     1         //直流母线
#define FAULT_BATT1_GROUP      2         //1组电池异常
#define FAULT_BATT2_GROUP      3         //2组电池异常
#define	FAULT_BMS_GROUP        4         //电池巡检故障
#define	FAULT_RECT_GROUP       5         //整流模块故障
#define FAULT_DCAC_GROUP       6         //逆变模块故障
#define	FAULT_DCDC_GROUP       7         //通信模块故障
#define FAULT_FC_GROUP         8         //馈线模块故障
#define FAULT_RC_GROUP         9         //RC10模块故障
#define FAULT_SWT_GROUP        10        //自带开关量故障
#define	FAULT_DC_PANEL1_GROUP  11        //分电屏01故障
#define FAULT_DC_PANEL2_GROUP  12        //分电屏02故障
#define FAULT_DC_PANEL3_GROUP  13        //分电屏03故障
#define	FAULT_DC_PANEL4_GROUP  14        //分电屏04故障
#define FAULT_DC_PANEL5_GROUP  15        //分电屏05故障
#define FAULT_DC_PANEL6_GROUP  16        //分电屏06故障
#define	FAULT_DC_PANEL7_GROUP  17        //分电屏07故障
#define FAULT_DC_PANEL8_GROUP  18        //分电屏08故障
#define FAULT_DCDC_PANEL_GROUP 19        //通信屏故障
#define FAULT_DCAC_PANEL_GROUP 20        //UPS屏故障
#define FAULT_AC_PANEL_GROUP   21        //交流屏故障


/* 交流分组序号 */
#define FAULT_AC_PATH1_OFF         0     //直流系统交流一路停电
#define FAULT_AC_PATH1_UNDER_VOLT  1     //直流系统交流一路欠压
#define FAULT_AC_PATH1_OVER_VOLT   2     //直流系统交流一路过压
#define FAULT_AC_PATH1_LACK_PHASE  3     //直流系统交流一路缺相
#define FAULT_AC_PATH2_OFF         4     //直流系统交流二路停电
#define FAULT_AC_PATH2_UNDER_VOLT  5     //直流系统交流二路欠压
#define FAULT_AC_PATH2_OVER_VOLT   6     //直流系统交流二路过压
#define FAULT_AC_PATH2_LACK_PHASE  7     //直流系统交流二路缺相
#define FAULT_AC2_PATH1_OFF         8    //交流系统交流一路停电
#define FAULT_AC2_PATH1_UNDER_VOLT  9    //交流系统交流一路欠压
#define FAULT_AC2_PATH1_OVER_VOLT   10   //交流系统交流一路过压
#define FAULT_AC2_PATH1_LACK_PHASE  11   //交流系统交流一路缺相
#define FAULT_AC2_PATH2_OFF         12   //交流系统交流二路停电
#define FAULT_AC2_PATH2_UNDER_VOLT  13   //交流系统交流二路欠压
#define FAULT_AC2_PATH2_OVER_VOLT   14   //交流系统交流二路过压
#define FAULT_AC2_PATH2_LACK_PHASE  15   //交流系统交流二路缺相
#define FAULT_AC_FC1_COMM_FAIL      16   //交流屏1#馈线模块通信中断
#define FAULT_AC_FC2_COMM_FAIL      17   //交流屏2#馈线模块通信中断
#define FAULT_AC_FC3_COMM_FAIL      18   //交流屏3#馈线模块通信中断
#define FAULT_AC_NUM               19    //交流分组故障数量


/* 直流母线分组序号 */
#define FAULT_DC_PB_OVER_VOLT        0     //一段合母过压     
#define FAULT_DC_PB_UNDER_VOLT       1     //一段合母欠压
#define FAULT_DC_CB_OVER_VOLT        2     //一段控母过压
#define FAULT_DC_CB_UNDER_VOLT       3     //一段控母欠压
#define FAULT_DC_BUS_OVER_VOLT       4     //一段母线过压
#define FAULT_DC_BUS_UNDER_VOLT      5     //一段母线欠压
#define FAULT_BATT_OVER_VOLT         6     //一段电池过压
#define FAULT_BATT_UNDER_VOLT        7     //一段电池欠压
#define FAULT_DC10_COMM_FAIL         8     //1#DC10模块通信中断//电流传感器异常
#define FAULT_BATT1_OVER_CURR        9     //一组电池过流
#define FAULT_BATT2_OVER_CURR        10    //二组电池过流
#define FAULT_DC_BUS_CURR_IMBALANCE  11    //一段母线电流不平衡
#define FAULT_DC_BUS_VOLT_IMBALANCE  12    //一段母线电压不平衡
#define FAULT_DC_BUS_INSU_FAULT      13    //一段母线绝缘下降
#define FAULT_INSU_RELAY_FAULT       14    //一段绝缘继电器故障
#define FAULT_DC_BUS_INPUT_AC        15    //一段直流母线窜入交流告警

#define FAULT_DC_PB2_OVER_VOLT       16    //二段合母过压     
#define FAULT_DC_PB2_UNDER_VOLT      17    //二段合母欠压
#define FAULT_DC_CB2_OVER_VOLT       18    //二段控母过压
#define FAULT_DC_CB2_UNDER_VOLT      19    //二段控母欠压
#define FAULT_DC_BUS2_OVER_VOLT      20    //二段母线过压
#define FAULT_DC_BUS2_UNDER_VOLT     21    //二段母线欠压
#define FAULT_BATT2_OVER_VOLT        22    //二段电池过压
#define FAULT_BATT2_UNDER_VOLT       23    //二段电池欠压
#define FAULT_DC_BUS2_CURR_IMBALANCE 24    //二段母线电流不平衡
#define FAULT_DC_BUS2_VOLT_IMBALANCE 25    //二段母线电压不平衡
#define FAULT_DC_BUS2_INSU_FAULT     26    //二段母线绝缘下降
#define FAULT_INSU2_RELAY_FAULT      27    //二段绝缘继电器故障
#define FAULT_DC_BUS2_INPUT_AC       28    //二段直流母线窜入交流告警
#define FAULT_AC10_COMM_FAIL         29    //2#DC10模块通信中断
                                           //30~31预留
#define FAULT_DC_NUM                 30    //直流母线分组故障数量


/* 一组电池异常序号 */
/*
	0~119：120节单体电池过压
	120~239：120节单体电池欠压
*/
#define FAULT_BATT1_NUM                240    //一组电池故障数量


/* 二组电池异常序号 */
/*
	0~119：120节单体电池过压
	120~239：120节单体电池欠压
*/
#define FAULT_CELL_OVER_VOLT_BASE_NUM  0      //单体电池过压起始序号
#define FAULT_CELL_UNDER_VOLT_BASE_NUM 120    //单体电池欠压起始序号
#define FAULT_BATT2_NUM                240    //一组电池故障数量


/* 电池巡检故障序号 */
/*
	0~4：一组电池的5个电池巡检通信中断
	5~9：二组电池的5个电池巡检通信中断
*/
#define FAULT_BMS_NUM            10    //电池巡检故障数量


/* 整流模块故障序号 */
/*
	0~4：1号整流模块通讯中断、过温、交流异常、过压保护、模块故障
	5~10：2号整流模块通讯中断、过温、交流异常、过压保护、模块故障
	...
*/
#define FAULT_RECT_COMM_FAIL          0     //整流模块通信中断
#define FAULT_RECT_VOER_TEMPERATURE   1     //整流模块过温
#define FAULT_RECT_AC_EXCEPTION       2     //整流模块交流异常
#define FAULT_RECT_OVER_VOLT_PROTECT  3     //整流模块过压保护
#define FAULT_RECT_FAULT              4     //整流模块故障
#define FAULT_RECT_CNT                5     //整流模块故障数量
#define FAULT_RECT_NUM                120   //整流模块分组故障数量


/* 逆变模块故障序号 */
/*
	0~5：1号逆变模块通讯中断、模块故障、过载、过温、电池欠压、旁路异常
	6~11：2号逆变模块通讯中断、模块故障、过载、过温、电池欠压、旁路异常
	...
*/
#define FAULT_DCAC_COMM_FAIL          0     //逆变模块通信中断
#define FAULT_DCAC_FAULT              1     //逆变模块故障
#define FAULT_DCAC_VOERLOAD           2     //逆变模块过载
#define FAULT_DCAC_VOER_TEMPERATURE   3     //逆变模块过温
#define FAULT_DCAC_BATT_UNDERVOLT     4     //逆变模块电池欠压
#define FAULT_DCAC_BYPASS_EXCEPTION   5     //逆变模块旁路异常
#define FAULT_DCAC_CNT                6     //逆变模块故障数量
#define FAULT_DCAC_NUM                48    //逆变模块分组故障数量




/* 通信模块故障序号 */
/*
	0~2：1号通信模块通讯中断、保护、模块故障
	3~5：2号通信模块通讯中断、保护、模块故障
	...
*/
#define FAULT_DCDC_COMM_FAIL          0     //通信模块通信中断
#define FAULT_DCDC_PROTECT            1     //通信模块保护
#define FAULT_DCDC_FAULT              2     //通信模块故障
#define FAULT_DCDC_CNT                3     //通信模块故障数量
#define FAULT_DCDC_NUM                24    //通信模块分组故障数量


/* 馈线模块通信中断 */
/*
	0~3：1#馈电柜1#~4#馈线模块通信中断
	...
	28~31：8#馈电柜1#~4#馈线模块通信中断
	32: 通信屏馈线模块通信中断
	33：UPS屏馈线模块通信中断
*/
#define FAULT_DCDC_FC_COMM_FAIL 32
#define FAULT_DCAC_FC_COMM_FAIL 33
#define FAULT_FC_COMM_FIAL_NUM  34    //馈线模块通信中断故障数量

/* RC10模块通信中断 */
/*
	0~15：1#~16#RC10模块通信中断
*/
#define FAULT_RC10_COMM_FAIL    0     //1#RC10模块通信中断
#define FAULT_RC_COMM_FIAL_NUM  16    //RC模块通信中断故障数量


/* 自带开关量故障 */
/*
	 0~19：SC32自测1~20开关跳闸
	20~35：1#DC10自测1~16开关跳闸
	36~51：2#DC10自测1~16开关跳闸
*/
#define FAULT_SC32_SWT_BEGIN	0     //SC32自带开关跳闸开始索引
#define FAULT_DC10_SWT_BEGIN	20    //1#DC10开关跳闸开始索引
#define FAULT_AC10_SWT_BEGIN	36    //2#DC10开关跳闸开始索引
#define FAULT_SWT_NUM           (20+16+16)    //自带开关量分组数量

/* 分电屏01故障序号 */
/* 分电屏02故障序号 */
/* 分电屏03故障序号 */
/* 分电屏04故障序号 */
/* 分电屏05故障序号 */
/* 分电屏06故障序号 */
/* 分电屏07故障序号 */
/* 分电屏08故障序号 */
/*
	0~2：1#支路开关跳闸、绝缘下降、传感器异常
	3~5：2#支路开关跳闸、绝缘下降、传感器异常
	...
	189~191：64#支路开关跳闸、绝缘下降、传感器异常
*/
#define FAULT_FEEDER_TOATL_SWT_FAULT  0     //总开关故障
#define FAULT_FEEDER_BRANCH_BASE      1     //支路故障起始序号
#define FAULT_FEEDER_BRANCH_SWT       0     //开关跳闸
#define FAULT_FEEDER_BRANCH_INSU      1     //绝缘下降
#define FAULT_FEEDER_BRANCH_SENSOR    2     //传感器异常
#define FAULT_FEEDER_BRANCH_FAULT_CNT 3     //每个馈线支路的故障数量
#define FAULT_PANEL_BRANCH_NUM        64    //每个分电屏的最大支路数
#define FAULT_DC_PANEL_NUM            193   //分屏组故障数量


/* 通信屏07故障序号 */
/* UPS屏08故障序号 */
/*
	0~1：1#支路开关跳闸、传感器异常
	2~3：2#支路开关跳闸、传感器异常
	...
	126~127：64#支路开关跳闸、传感器异常
*/
#define FAULT_DCDC_TOATL_SWT_FAULT    0     //总开关故障
#define FAULT_DCDC_BRANCH_BASE        1     //支路故障起始序号
#define FAULT_DCDC_BRANCH_SWT         0     //开关跳闸
#define FAULT_DCDC_BRANCH_SENSOR      1     //传感器异常
#define FAULT_DCDC_BRANCH_FAULT_CNT   2     //每个馈线支路的故障数量
#define FAULT_DCDC_PANEL_NUM          129   //故障数量

#define FAULT_DCAC_TOATL_SWT_FAULT    0     //总开关故障
#define FAULT_DCAC_BRANCH_BASE        1     //支路故障起始序号
#define FAULT_DCAC_BRANCH_SWT         0     //开关跳闸
#define FAULT_DCAC_BRANCH_SENSOR      1     //传感器异常
#define FAULT_DCAC_BRANCH_FAULT_CNT   2     //每个馈线支路的故障数量
#define FAULT_DCAC_PANEL_NUM          129   //故障数量

#define FAULT_AC_TOATL_SWT_FAULT    0     //总开关故障
#define FAULT_AC_BRANCH_BASE        1     //支路故障起始序号
#define FAULT_AC_BRANCH_SWT         0     //开关跳闸
#define FAULT_AC_BRANCH_SENSOR      1     //传感器异常
#define FAULT_AC_BRANCH_FAULT_CNT   2     //每个馈线支路的故障数量
#define FAULT_AC_PANEL_NUM          129   //故障数量



/************************ 下面是通过邮箱发送故障ID的相关函数 ****************************/

/*************************************************************
函数名称: v_fauid_fault_mbx_init		           				
函数功能: 初始化故障ID的邮箱，需在RTX启动后，其它任务启动前调用						
输入参数: 无         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fauid_fault_mbx_init(void);


/*************************************************************
函数名称: v_fauid_send_fault_id_occur		           				
函数功能: 故障发生时，调用此函数通过邮箱发送故障ID						
输入参数: u16_fault_id -- 故障ID         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fauid_send_fault_id_occur(U16_T u16_fault_id);


/*************************************************************
函数名称: v_fauid_send_fault_id_resume		           				
函数功能: 故障恢复时，调用此函数通过邮箱发送故障ID						
输入参数: u16_fault_id -- 故障ID         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fauid_send_fault_id_resume(U16_T u16_fault_id);


/*************************************************************
函数名称: u32_fauid_recv_fault_id		           				
函数功能: 接收故障ID，此函数由故障处理任务调用，获取其它任务发送过来的任务ID						
输入参数: timeout -- 超时值         		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
U32_T u32_fauid_recv_fault_id(U16_T timeout);


/*********** 以下常量只在故障处理任务和FaultId.c中调用，其它任务不应该调用 **************/

#define FAULT_INVALID           0xFFFFFFFF      //无效故障
#define FAULT_OCCUR             0x00000000      //故障发生
#define FAULT_RESUME            0x00010000      //故障恢复
#define FAULT_OCCUR_MASK        0xFFFF0000      //发生恢复屏蔽码
#define FAULT_ID_MASK           0x0000FFFF      //故障ID屏蔽码
#define FAULT_ID_GROUP_MASK     0x7C00          //故障ID组屏蔽码
#define FAULT_ID_INDEX_MASK     0x3FF           //故障ID序号屏蔽码


#endif
