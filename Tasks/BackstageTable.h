/************************************************************
Copyright (C), 深圳英可瑞科技开发有限公司
文 件 名：BackstageTable.h
版    本：1.00
创建日期：2012-07-28
作    者：郭数理
功能描述：后台通信数据表头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-07-28  1.00     创建
***********************************************************/


#ifndef __BACKSTAGE_TABLE_H_
#define __BACKSTAGE_TABLE_H_


#include "Type.h"


#define BS_TYPE_U8  0
#define BS_TYPE_U16 1
#define BS_TYPE_F32 2
#define BS_TYPE_S16 3

#define BS_YC_SIZE  911//898      //遥测条目个数，实测的索引号总数：911,2021-12-05
#define BS_YX_SIZE  2439//2528//2304     //遥信条目个数，实测的索引号总数：2439,2021-12-05
#define BS_YK_SIZE  50//82//26       //遥控条目个数，实测的索引号总数：50,2021-12-05
#define BS_YT_SIZE  2        //遥调条目个数

#define BS_ITEM_SIZE 4       //点表条目的字节数
#define BS_ITEM_INVALID    2
#define BS_ITEM_UPLOAD     1
#define BS_ITEM_NO_UPLOAD  0

#define BS_CB_POS_TO_GND_VOLT_INDEX  19   //控母正对地电压索引
#define BS_PB_POS_TO_GND_VOLT_INDEX  20   //合母正对地电压索引
#define BS_BUS_NEG_TO_GND_VOLT_INDEX 21   //母线负对地电压索引



//点表条目
typedef struct
{
	U16_T u16_id;    //条目ID
	U16_T u16_flag;  //是否上传标志
}BS_ITEM_T;


//遥信组装条目
typedef struct
{
	void  *pv_val;                //指向遥信数据指针
	U16_T u16_mask;               //屏蔽码
	U8_T  u8_type;                //pv_val指向的数据类型
	
	U16_T  u16_id;                //数据ID
	
	U8_T   u8_major_val;          //主比较值，*pu8_major_condition>=u8_major_val，则满足条件
	U8_T   u8_second_val;         //次比较值，*pu8_second_condition>=u8_second_val，则满足条件
	U8_T   *pu8_major_condition;  //指向主条件变量
	U8_T   *pu8_second_condition; //指向次条件变量
}YX_ASSEMBLE_T;


//遥测组装条目
typedef struct
{
	F32_T  *pf32_val;             //指向遥测数据指针
	U16_T  u16_modbus_coeff;      //MODBUS变比
	U16_T  u16_cdt_coeff;         //CDT变比
	U16_T  u16_id;                //数据ID
	
	U8_T   u8_major_val;          //主比较值，*pu8_major_condition>=u8_major_val，则满足条件
	U8_T   u8_second_val;         //次比较值，*pu8_second_condition>=u8_second_val，则满足条件
	U8_T   *pu8_major_condition;  //指向主条件变量
	U8_T   *pu8_second_condition; //指向次条件变量
}YC_ASSEMBLE_T;


//遥控组装条目
typedef struct
{
	void  *pv_val;                //指向遥控数据指针
	U16_T  u16_id;                //数据ID
	U8_T  u8_type;                //pv_val指向的数据类型
	
	U8_T   u8_major_val;          //主比较值，*pu8_major_condition>=u8_major_val，则满足条件
	U8_T   *pu8_major_condition;  //指向主条件变量
}YK_ASSEMBLE_T;



/* 遥测数据 */
extern const YC_ASSEMBLE_T g_t_yc_assemble[BS_YC_SIZE];

/* 遥信数据 */
extern const YX_ASSEMBLE_T g_t_yx_assemble[BS_YX_SIZE];

/* 遥控数据 */
extern const YK_ASSEMBLE_T g_t_yk_assemble[BS_YK_SIZE];

/* 遥调数据 */
extern const YC_ASSEMBLE_T g_t_yt_assemble[BS_YT_SIZE];

#endif
