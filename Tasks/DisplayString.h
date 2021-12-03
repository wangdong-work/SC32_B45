/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：DisplayString.h
版    本：1.00
创建日期：2012-05-09
作    者：郭数理
功能描述：显示字符串头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-09  1.00     创建
**************************************************************/

#ifndef __DISPLAY_STRING_H_
#define __DISPLAY_STRING_H_

/************************** 界面显示字符串定义 ********************************/
#define MMI_STR_ID_TYPE_MASK            0xF000       //字符串ID类型屏蔽码，高4位是类型码
#define MMI_STR_ID_INDEX_MASK           0x0FFF       //字符串ID索引屏蔽码，低12位是类型码
#define MMI_STR_ID_NORMAL_NAME          0x0000       //字符串ID码高4位为0表示一般字符串，可用字符串ID为索引在g_pu8_string数组中查到相应的字符串指针
#define MMI_STR_ID_SPECIAL_NAME         (1 << 12)    //字符串ID码高4位为1表示特殊字符串，字符串可在MMI_WIN_RECORD_T结构体的u8_special_name中查到


extern const STR_T g_s_string[][2];             //全局字符串数组
extern const STR_T g_s_batt_record[][2];        //电池充放电记录的名称
extern const STR_T g_s_ctrl_swt_name[][2];      //电操开关名称


#endif
