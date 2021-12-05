/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：DisplayString.c
版    本：1.00
创建日期：2012-05-09
作    者：郭数理
功能描述：显示字符串定义

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-09  1.00     创建
**************************************************************/

#include "Type.h"
#include "ShareDataStruct.h"
#include "DisplayString.h"


/************************** 界面显示字符串定义 ********************************/
const STR_T g_s_string[][2] =
{
	{ "环境温度(℃)",              "Temperature(℃)" },                     //0
	{ "一组电池电压(V)",           "Batt1 Vol(V)" },                        //1
	{ "一组电池电流(A)",           "Batt1 Cur(A)" },                        //2
	{ "一组电池容量(Ah)",          "Batt1 Capa(Ah)" },                      //3
	{ "一段负载电流(A)",           "Load1 Cur(A)" },                        //4
	{ "一段合母电压(V)",           "PB1  Vol(V)" },                         //5
	{ "一段母线电压(V)",           "Bus1 Vol(V)" },                         //6
	{ "一段控母电压(V)",           "CB1  Vol(V)" },                         //7
	{ "电池正在浮充/自动管理",     "Batt FLO/Auto" },                       //8
	{ "电池正在均充/自动管理",     "Batt EQU/Auto" },                       //9
	{ "电池正在核容/自动管理",     "Batt CHK/Auto" },                       //10
	{ "电池正在放电/自动管理",     "Batt DIS/Auto" },                       //11
	{ "电池正在浮充/手动管理",     "Batt FLO/Manu" },                       //12
	{ "电池正在均充/手动管理",     "Batt EQU/Manu" },                       //13
	{ "系统正常",                  "Sys normal" },                          //14
	{ "系统告警/交流故障",         "Sys alarm/AC fault" },                  //15   
	{ "系统告警/直流母线故障",     "Sys alarm/DC bus fault" },              //16
	{ "系统告警/电池故障",         "Sys alarm/Batt fault" },                //17
	{ "系统告警/绝缘故障",         "Sys alarm/Insu fault" },                //18
	{ "系统告警/充电模块故障",     "Sys alarm/Rect fault" },                //19
	{ "系统告警/熔断器故障",       "Sys alarm/Fuse fault" },                //20
	{ "系统告警/UPS|CPS故障",      "Sys alarm/UPS|CPS fault" },             //21
	{ "系统告警/馈线支路故障",     "Sys alarm/Feeder fault" },              //22

	{ "请选择:",                   "Select please:" },                      //23
	{ ".直流系统信息查询",         ".DC  System Info" },                    //24
	{ ".通信系统信息查询",         ".CPS System Info" },                    //25
	{ ".UPS系统信息查询",          ".UPS System Info" },                    //26
	{ ".当前告警查询",             ".CUR Alarm  Info" },                    //27
	{ ".历史告警查询",             ".HIS Alarm  Info" },                    //28
	{ ".掉电未复归告警查询",       ".UNR Alarm  Info" },                    //29
	{ ".事件记录查询",             ".Event Log  Info" },                    //30
	{ ".参数设置",                 ".Parameter Setup" },                    //31
	{ ".关于",                     ".About" },                              //32

	{ "直流系统信息查询:",         "DC System Info:       " },              //33
	{ ".交流信息查询",             ".AC Module Data Query " },              //34
	{ ".电池状态查询",             ".BAT Group Data Query " },              //35
	{ ".一段母线绝缘查询",         ".BUS1 Insu Data Query " },              //36
	{ ".整流模块信息",             ".REC Module Data Query" },              //37
	{ ".系统开关状态查询",         ".SWT State Data Query " },              //38
	{ ".一段馈线信息查询",         ".FDL1 Panel Data Query" },              //39

	{ "一段馈线柜查询:",           "SEG1 FDL panel Info:  " },              //40
	{ ".1#馈线柜信息查询",         ".1#FDL Panel Info     " },              //41
	{ ".2#馈线柜信息查询",         ".2#FDL Panel Info     " },              //42
	{ ".3#馈线柜信息查询",         ".3#FDL Panel Info     " },              //43
	{ ".4#馈线柜信息查询",         ".4#FDL Panel Info     " },              //44
	{ ".二段母线绝缘查询",         ".BUS2 Insu Data Query " },              //45
	{ ".二段馈线信息查询",         ".FDL2 Panel Data Query" },              //46
	{ "二段馈线柜查询:",           "SEG2 FDL panel Info:  " },              //47
	{ "二段馈电柜(面) ",           "Cabinet Num2          " },              //48

	{ "通信系统信息查询:",         "CPS Panel Info:       " },              //49
	{ ".DC/DC模块信息查询",        ".DC/DC Module Info    " },              //50
	{ ".通信开关状态查询",         ".CPS SWT State Info   " },              //51

	{ "UPS系统信息查询：",         "UPS Panel Info:       " },              //52
	{ ".UPS模块信息查询",          ".UPS Module Info      " },              //53
	{ ".UPS系统开关状态查询",      ".UPS SWT State Info   " },              //54

	{ "请选择:",                   "Select please:        " },              //55
	{ ".直流系统配置",             ".DC  System Config    " },              //56
	{ ".通信系统配置",             ".CPS System Config    " },              //57
	{ ".UPS系统配置",              ".UPS System Config    " },              //58
	{ ".告警设定",                 ".SYS Alarm  Config    " },              //59
	{ ".后台通讯",                 ".Remote Comm Config   " },              //60
	{ ".时间密码",                 ".Time&Password Setup  " },              //61
	{ ".系统控制",                 ".System Control       " },              //62
	{ ".参数恢复",                 ".Parameter Recovery   " },              //63
	{ ".采样校准",                 ".Parameter Calibrate  " },              //64
	{ ".显示设置",                 ".Display Setup        " },              //65

	{ "直流系统配置:",             "DC System Config:     " },              //66
	{ ".直流配置",                 ".DC Config            " },              //67
	{ ".一段馈线柜配置",           ".SEG1 FDL Panel Config" },              //68
	{ ".门限值设置",               ".ThresholdValue Config" },              //69
	{ ".电池充放电设置",           ".BAT Manage Config    " },              //70
	{ ".充电及模块控制",           ".Charge Control       " },              //71

	{ "一段馈线柜设置:",           "SEG1 FDL Panel Setup: " },              //72
	{ ".1#馈线柜配置",             ".1#FDL Panel Config   " },              //73
	{ ".2#馈线柜配置",             ".2#FDL Panel Config   " },              //74
	{ ".3#馈线柜配置",             ".3#FDL Panel Config   " },              //75
	{ ".4#馈线柜配置",             ".4#FDL Panel Config   " },              //76
	{ "二段馈线柜设置:",           "SEG2 FDL Panel Setup: " },              //77
	{ "二段对地电压参数(V)",       "SEG2 Vol to GND(V)    " },              //78
	{ "二段对地电阻参数(KΩ)",      "SEG2 Res to GND(KΩ)   " },              //79
	{ ".二段馈线柜配置",           ".SEG2 FDL Panel Config" },              //80

	{ "系统未配置此项信息，",      "Not config this info, " },              //81
	{ "如有需要请配置！",          "Reconfigure please!   " },              //82

	{ "交流运行数据",              "AC Realtime Data" },                    //83
	{ "交流一路线电压",            "AC1 Line Voltage" },                    //84
	{ "交流二路线电压",            "AC2 Line Voltage" },                    //85
	{ "UV电压(V)",                 "Uuv(V)" },                              //86
	{ "VW电压(V)",                 "Uvw(V)" },                              //87
	{ "WU电压(V)",                 "Uwu(V)" },                              //88
	{ "交流一路相电压",            "AC1 Phase Voltage" },                   //89
	{ "交流二路相电压",            "AC2 Phase Voltage" },                   //90
	{ "电压(V)",                   "Voltage(V)" },                          //91

	{ "电池组运行数据",            "BAT Realtime Data" },                   //92
	{ "当前容量(Ah)",              "BAT Capa(Ah)" },                        //93
	{ "电池电压(V)",               "BAT Vol(V)" },                          //94
	{ "电池电流(A)",               "BAT Cur(A)" },                          //95
	{ "电池状态",                  "BAT State" },                           //96
	{ "已浮充(hor)",               "FLO Time(h)" },                         //97
	{ "已均充(min)",               "EQU Time(m)" },                         //98
	{ "已核容(min)",               "DIS Time(m)" },                         //99
	{ "浮充",                      "FLO" },                                 //100
	{ "均充",                      "EQU" },                                 //101
	{ "核容",                      "DIS" },                                 //102

	{ "一组电池数据",              "BAT1 Data" },                           //103
	{ "二组电池数据",              "BAT2 Data" },                           //104
	{ "电池电流(A)",               "BAT Cur(A)" },                          //105
	{ "当前容量(Ah)",              "BAT Capa(Ah)" },                        //106

	{ "一组电池巡检数据",          "BAT1 Cell Data" },                      //107
	{ "二组电池巡检数据",          "BAT2 Cell Data" },                      //108
	{ "单体电压最高值信息",        "BAT Cell Vmax" },                       //109
	{ "单体电压最低值信息",        "BAT Cell Vmin" },                       //110
	{ "电池序号",                  "Cell No." },                            //111
	{ "电压值(V)",                 "Cell Vol" },                            //112
	{ "一组单体电压数据(V)",       "BAT1 Cell Vol(V)" },                    //113
	{ "二组单体电压数据(V)",       "BAT2 Cell Vol(V)" },                    //114

	{ "一段对地电压参数(V)",       "SEG1 Vol to GND(V)" },                  //115
	{ "合母正对地电压",            "PB Vol to GND" },                       //116
	{ "控母正对地电压",            "CB Vol to GND" },                       //117
	{ "母线正对地电压",            "P+ Vol to GND" },                       //118
	{ "母线负对地电压",            "P- Vol to GND" },                       //119
	{ "一段对地电阻参数(KΩ)",      "SEG1 Res to GND(KΩ)" },                 //120
	{ "正对地电阻",                "Res+ to GND" },                         //121
	{ "负对地电阻",                "Res- to GND" },                         //122

	{ "#整流模块信息",             "#Rect Module Info" },                   //123
	{ "#DC/DC模块信息",            "#DC/DC Module Info" },                  //124
	{ "输出电压(V)",               "Out Vol(V)" },                          //125
	{ "输出电流(A)",               "Out Cur(A)" },                          //126
	{ "限流点(%)",                 "Lmt Cur(%)" },                          //127
	{ "控制方式",                  "Control Way" },                         //128
	{ "模块状态",                  "State" },                               //129
	{ "自动",                      "Auto" },                                //130
	{ "手动",                      "Manu" },                                //131
	{ "开机",                      "On " },                                 //132
	{ "关机",                      "Off" },                                 //133
	{ "异常",                      "Fault" },                               //134

	{ "直流系统开关状态",          "DC System SWT State" },                 //135
	{ "充电机开关",                "Charger SWT" },                         //136
	{ "电池开关1",                 "BAT SWT1" },                            //137
	{ "母联开关",                  "Bus tie SWT" },                         //138		   
	{ "交流开关1",                 "AC SWT1" },                             //139
	{ "交流开关2",                 "AC SWT2" },                             //140
	{ "断开",                      "Off" },                                 //141
	{ "闭合",                      "ON " },                                 //142

	{ "#馈线柜输出开关状态",       "#FDL Panel SWT State" },                //143
	{ "通信输出开关状态",          "CPS Panel SWT State" },                 //144
	{ "UPS系统输出开关状态",       "UPS Panel SWT State" },                 //145
	{ "#输出开关",                 "#SWT State" },                          //146
	{ "开关状态",                  "SWT State" },                           //147
	{ "绝缘电阻(KΩ)",              "Insu Res(KΩ)" },                        //148
	{ "输出电流(A)",               "Out Cur(A)" },                          //149
	{ "传感器状态",                "Sensor State" },                        //150
	{ "---",                       "---" },                                 //151
	{ "分",                        "Off" },                                 //152
	{ "合",                        "On " },                                 //153
	{ "---",                       "---" },                                 //154
	{ "正常",                      "Normal" },                              //155
	{ "异常",                      "Fault" },                               //156

	{ "#UPS模块信息 ",             "#UPS Module Info" },                    //157
	{ "输出频率(Hz)",              "Out Freq(Hz)" },                        //158
	{ "逆变电压(V)",               "Invert Vol(V)" },                       //159
	{ "旁路电压(V)",               "Bypass Vol(V)" },                       //160
	{ "输出有功(KW)",              "Out power(KW)" },                       //161
	{ "视在功率(KVA)",             "App power(KVA)" },                      //162
	{ "功率因数",                  "Power Rate" },                          //163
	{ "工作方式",                  "Work Way " },                           //164
	{ "开机/逆变",                 "On /INV" },                             //165
	{ "开机/旁路",                 "On /BYP" },                             //166
	{ "关机",                      "Off " },                                //167
	{ "状态",                      "State" },                               //168
	{ "正常",                      "Normal" },                              //169
	{ "报警",                      "Alarm" },                               //170

	{ "当前告警: ",                "Current Alarm: " },                     //171
	{ "历史告警: ",                "History Alarm: " },                     //172
	{ "掉电未复归告警: ",          "Unrecovered Alarm: " },                 //173
	{ "事件记录: ",                "Event Log: " },                         //174
	{ "发生:",                     "Strt:" },                               //175
	{ "复归:",                     "End: " },                               //176
	{ "系统当前正常，无故障",      "System is normal," },                   //177
	{ "记录！",                    "no fault log！" },                      //178
	{ "无历史故障记录信息！",      "No history fault log！" },              //179
	{ "无掉电未复归故障",          "No Recovery fault log" },               //180
	{ "记录信息！",                "Log information！" },                   //181
	{ "无事件记录信息！",          "No Event Log info！" },                 //182
	{ "数据记录不完整",            "Record isn't complete" },               //183

	{ "请输入设置密码:",           "Please input password:" },              //184
	{ "密码错误，请重新输入！",    "mistake, enter again！" },              //185
	
	{ "参数设置超限提示",          "Setting transfinit tip" },              //186
	{ "所设参数超出门限值，",      "Para beyond the value," },              //187
	{ "按ESC键返回。",             "Press 'ESC' back." },                   //188
	{ "可设最大值: ",              "Set the max: " },                       //189
	{ "可设最小值: ",              "Set the min: " },                       //190

	{ "直流系统",                  "DC System" },                           //191
	{ "直流电压等级",              "DC Vol level" },                        //192
	{ "合控母分段",                "PB Subsection" },                       //193
	{ "硅链规格",                  "Silicon chain" },                       //194
	{ "控母输出(V)",               "CB Out(V)" },                           //195
	{ "一段馈电柜(面) ",           "Cabinet Num1" },                        //196
	{ "220V",                      "220V" },                                //197
	{ "110V",                      "110V" },                                //198
	{ "不分段",                    "Multiple" },                            //199
	{ "分段",                      "Single" },                              //200
	{ "无",                        "None" },                                //201
	{ "5级4V",                     "5L/4V" },                               //202
	{ "5级7V",                     "5L/7V" },                               //203
	{ "7级3V",                     "7L/3V" },                               //204
	{ "7级5V",                     "7L/5V" },                               //205

	{ "交流参数配置",              "AC Para Config" },                      //206
	{ "交流相数",                  "Phase Num" },                           //207
	{ "输入路数",                  "Input Num" },                           //208
	{ "交流控制",                  "AC Control" },                          //209
	{ "三相",                      "PH3 " },                                //210
	{ "单相",                      "PH1 " },                                //211
	{ "无",                        "None" },                                //212
	{ "1路",                       "1nd " },                                //213
	{ "2路",                       "2nd " },                                //214
	{ "一路优先",                  "1nd pri" },                             //215
	{ "固定一路",                  "1nd fix" },                             //216
	{ "二路优先",                  "2nd pri" },                             //217
	{ "固定二路",                  "2nd fix" },                             //218

	{ "电池配置",                  "Battery config" },                      //219
	{ "电池组数",                  "Group Num" },                           //220
	{ "单组容量(Ah)",              "Capa(Ah)" },                            //221

	{ "电流采集参数配置",          "Cur Sample config" },                   //222
	{ "电流采集(mV)",              "Cur Sample(mV)" },                      //223
	{ "75",                        "75" },                                  //224
	{ "50",                        "50" },                                  //225
	{ "负载量程(A)",               "Load Range" },                          //226
	{ "电池1量程(A)",              "BAT1 Range" },                          //227
	{ "电池2量程(A)",              "BAT2 Range" },                          //228
	{ "支路电流传感器系数",        "Sensor coefficient" },                  //229
	{ "量程(A)",                   "Range(A)" },                            //230

	{ "整流模块参数配置",          "Rect Module config" },                  //231
	{ "数量(个)",                  "Num" },                                 //232
	{ "额定电流",                  "Rated Cur" },                           //233
	{ "通讯协议",                  "Comm Pro" },                            //234
	{ "脱机电压(V)",               "Offline Vol" },                         //235
	{ "脱机电流(%)",               "Offline Cur" },                         //236
	{ " 5A",                       " 5A" },                                 //237
	{ " 7A",                       " 7A" },                                 //238
	{ "10A",                       "10A" },                                 //239
	{ "20A",                       "20A" },                                 //240
	{ "30A",                       "30A" },                                 //241
	{ "35A",                       "35A" },                                 //242
	{ "40A",                       "40A" },                                 //243

	{ "1组电池巡检配置",           "BAT1 Module Config" },                  //244
	{ "2组电池巡检配置",           "BAT2 Module Config" },                  //245
	{ "型号",                      "Type" },                                //246
	{ "数量(个)",                  "BXX Num" },                             //247
	{ "第1个数量",                 "1nd Num" },                             //248
	{ "第2个数量",                 "2nd Num" },                             //249
	{ "第3个数量",                 "3nd Num" },                             //250
	{ "第4个数量",                 "4nd Num" },                             //251
	{ "第5个数量",                 "5nd Num" },                             //252
	{ "B21",                       "B21" },                                 //253
	{ "B3",                        "B3" },                                  //254
	{ "B4",                        "B4" },                                  //255

	{ "常规开关状态配置",          "SWT State Config" },                    //256
	{ "告警输入",                  "Input Way" },                           //257
	{ "不测量",                    "Unmeasu" },                             //258
	{ "测量",                      "Measu  " },                             //259
	{ "常开",                      "Open " },                               //260
	{ "常闭",                      "Close" },                               //261

	{ "#馈线柜配置",               "#FDL Panel Config" },                   //262
	{ "馈线模块数量",              "FDL Num" },                             //263
	{ "#馈线柜",                   "#FDL Panel" },                          //264
	{ "#馈线模块",                 "#FDL Module" },                         //265
	{ "通讯地址",                  "Address" },                             //266
	{ "开关跳闸",                  "SWT Trip " },                           //267
	{ "跳闸输入",                  "Alarm In" },                            //268
	{ "开关状态",                  "SWT State" },                           //269
	{ "状态输入",                  "State In" },                            //270
	{ "支路电阻",                  "Branch Res" },                          //271
	{ "支路电流",                  "Branch Cur" },                          //272

	{ "交流门限设置",              "AC threshold value" },                  //273
	{ "过压点(V)",                 "AC OVP" },                              //274
	{ "欠压点(V)",                 "AC UVP" },                              //275
	{ "缺相点(V)",                 "AC PLP" },                              //276

	{ "直流门限设置",              "DC threshold value" },                  //277
	{ "合母过压(V)",               "PB OVP" },                              //278
	{ "合母欠压(V)",               "PB UVP" },                              //279
	{ "控母过压(V)",               "CB OVP" },                              //280
	{ "控母欠压(V)",               "CB UVP" },                              //281
	{ "母线过压(V)",               "BB OVP" },                              //282
	{ "母线欠压(V)",               "BB UVP" },                              //283

	{ "电池门限值设置",            "BAT threshold value" },                 //284
	{ "电池组过压(V)",             "BAT  OVP" },                            //285
	{ "电池组欠压(V)",             "BAT  UVP" },                            //286
	{ "单体过压值(V)",             "Cell OVP" },                            //287
	{ "单体欠压值(V)",             "Cell UVP" },                            //288
	{ "尾电池过压(V)",             "Tail OVP" },                            //289
	{ "尾电池欠压(V)",             "Tail UVP" },                            //290
	{ "充电过流(c10)",             "BAT  OIP" },                            //291

	{ "绝缘门限报警设置",          "INS threshold value" },                 //292
	{ "不平衡报警(V)",             "INS VUP(V)" },                          //293
	{ "报警电阻(KΩ)",              "INS URP(KΩ)" },                         //294

	{ "电池充电参数设置",          "BAT Parameter Setup" },                 //295
	{ "均充电压(V) ",              "EQU Volt(V)" },                         //296
	{ "浮充电压(V)",               "FLO Volt(V)" },                         //297
	{ "温补系数(mv/组)",           "Coeff(mv/g.℃)" },                      //298
	{ "充电限流(C10)",             "Lmt Cur(C10)" },                        //299

	{ "电池转浮充判据",            "BAT turn FLO criterion" },              //300
	{ "电流转换点(C10)",           "EtoF Cur(C10)" },                       //301
	{ "均充倒计时(min)",           "EtoF Time(m)" },                        //302
	{ "或",                        "or" },                                  //303
	{ "均充限时(min)",             "EtoF Protect(m)" },                     //304

	{ "电池转均充判据",            "BAT turn EQU criterion" },              //305
	{ "电流转换点(C10)",           "FtoE Cur(C10)" },                       //306
	{ "持续时间(sec)",             "Duration(s)" },                         //307
	{ "交流停电(min)",             "AC Stop(s)" },                          //308
	{ "周期均充(hor)",             "FtoE Period(h)" },                      //309

	{ "电池核容终止设置",          "BAT DIS end criterion" },               //310
	{ "放电时间(min)",             "Dis Time(m)" },                         //311
	{ "终止电压(v)",               "BAT Vol(V)" },                          //312
	{ "单体电压(v)",               "Cell Vol(V)" },                         //313

	{ "电池放电曲线设置",          "Discharge curve Setup" },               //314
	{ "0.1C10A(hor)",              "0.1C10A(hor)" },                        //315
	{ "0.2C10A(hor)",              "0.2C10A(hor)" },                        //316
	{ "0.3C10A(hor)",              "0.3C10A(hor)" },                        //317
	{ "0.4C10A(hor)",              "0.4C10A(hor)" },                        //318
	{ "0.5C10A(hor)",              "0.5C10A(hor)" },                        //319
	{ "0.6C10A(hor)",              "0.6C10A(hor)" },                        //320
	{ "0.7C10A(hor)",              "0.7C10A(hor)" },                        //321
	{ "0.8C10A(hor)",              "0.8C10A(hor)" },                        //322
	{ "0.9C10A(hor)",              "0.9C10A(hor)" },                        //323
	{ "1.0C10A(hor)",              "1.0C10A(hor)" },                        //324

	{ "电池充电控制方式",          "Charging control mode" },               //325
	{ "手动控制电池充电方式",      "Manu control mode" },                   //326
	{ "一组电池",                  "BATT1" },                               //327
	{ "整流模块开关机",            "Rect Module On/Off" },                  //328
	{ "#模块",                     "#Module" },                             //329

	{ "通信模块参数配置",          "CPS Module Config" },                   //330
	{ "UPS模块参数配置",           "UPS Module Config" },                   //331
	{ "输出限流(%)",               "Out Cur(%)" },                          //332
	{ "通信开关参数配置",          "CPS SWT Config" },                      //333
	{ "UPS开关参数配置",           "UPS SWT Config" },                      //334
	{ "通信开关",                  "CPS SWT" },                             //335
	{ "馈线模块",                  "FDL Module" },                          //336
	{ "UPS开关",                   "UPS SWT" },                             //337
	{ "馈线模块",                  "FDL Module" },                          //338
	{ "电流系数(A)",               "Cur Coeff" },                           //339

	{ "干接点输出设定",            "Fault Out Setup" },                     //340
	{ "(修改后需复位生效)",        "(The revised reset)" },                 //341
	{ "交流故障",                  "AC Fault" },                            //342
	{ "直流母线故障",              "DC Bus Fault" },                        //343
	{ "电池故障",                  "BAT Fault" },                           //344
	{ "绝缘故障",                  "INS Fault" },                           //345
	{ "充电模块故障",              "Rect Fault" },                          //346
	{ "熔断器故障",                "Fuse Fault" },                          //347
	{ "UPS|CPS故障",               "UPS|CPS Fault" },                       //348
	{ "馈线支路故障",              "Feed Fault" },                          //349
	{ "均充指示",                  "EQU Tip" },                             //350
	{ "核容指示",                  "DIS Tip" },                             //351

	{ "报警设定",                  "Alarm Config" },                        //352
	{ "蜂鸣器",                    "Buzzer" },                              //353
	{ "电流不平衡告警",            "Cur unbalance" },                       //354
	{ "次要告警",                  "Minor alarm" },                         //355	   
	{ "一般告警",                  "General alarm" },                       //356
	{ "报警",                      "Alarm" },                               //357
	{ "静音",                      "Mute" },                                //358
	{ "关闭",                      "Disabl" },                              //359
	{ "开启",                      "Enable" },                              //360
	{ "不保存",                    "Unsave" },                              //361
	{ "保存",                      "Save  " },                              //362

	{ "远程通讯设置",              "Remote Comm Setup" },                   //363
	{ "通讯速率",                  "Baud" },                                //364
	{ "奇偶校验",                  "Parity" },                              //365
	{ "B码对时",                   "IRIG-B" },                              //366
	{ "1200",                      "1200" },                                //367
	{ "2400",                      "2400" },                                //368
	{ "4800",                      "4800" },                                //369
	{ "9600",                      "9600" },                                //370
	{ "19200",                     "19200" },                               //371
	{ "奇校验",                    "Odd " },                                //372
	{ "偶校验",                    "Even" },                                //373
	{ "无校验",                    "None" },                                //374
	{ "MODBUS",                    "MODBUS" },                              //375
	{ "CDT",                       "CDT" },                                 //376
	{ "校准",                      "ADJUST" },                              //377
	{ "IEC103",                    "IEC103" },                              //378
	{ "无",                        "No " },                                 //379
	{ "有",                        "Yes" },                                 //380

	{ "RTC时间设定",               "RTC Time Setup" },                      //381
	{ "年",                        "Y" },                                   //382
	{ "月",                        "M" },                                   //383
	{ "日",                        "D" },                                   //384
	{ "时",                        "H" },                                   //385
	{ "分",                        "M" },                                   //386
	{ "秒",                        "S" },                                   //387
	{ "用户密码",                  "Password" },                            //388

	{ "历史告警记录清除",          "His Alarm Log Clean" },                 //389
	{ "操作后，当前所有",          "After operation, all  " },              //390
	{ "历史告警记录被清除",        "his Alarm log clean!  " },              //391

	{ "掉电未复归告警清除",        "Unrecovered Log Clean" },               //392
	{ "操作后，当前所有掉电",      "After operation, all  " },              //393
	{ "未复归告警记录被清除",      "unrecovered log clean!" },              //394

	{ "事件记录清除",              "Event Log Clean" },                     //395
	{ "操作后，当前所有",          "After operation, all  " },              //396
	{ "事件记录被清除",            "event log Clean!      " },              //397

	{ "电池容量恢复",              "BAT Capacity Recover" },                //398
	{ "操作后，恢复电池当前",      "After operation, BAT  " },              //399
	{ "容量为额定值",              "Capa recovery rating! " },              //400

	{ "设置参数恢复",              "Set param recover" },                   //401
	{ "请注意，操作后，",          "After operation, all  " },              //402
	{ "设置数据恢复默认值",        "param recover default!" },              //403

	{ "按ENTER键执行操作！",       "Press 'ENTER' do" },                    //404
	{ "按CANCEL键取消操作！",      "Press 'CANCEL' undo   " },              //405
	{ "操作已成功，",              "Operation to complete " },              //406                                           
	{ "请按CANCEL键返回！",        "Press 'CANCEL' return " },              //407

	{ "交流校准—输入实际值",       "AC calibration-input" },                //408
	{ "一路UV电压(V)",             "AC1 Uuv(V)" },                          //409
	{ "一路VW电压(V)",             "AC1 Uvw(V)" },                          //410
	{ "一路WU电压(V)",             "AC1 Uwu(V)" },                          //411
	{ "二路UV电压(V)",             "AC2 Uuv(V)" },                          //412
	{ "二路VW电压(V)",             "AC2 Uvw(V)" },                          //413
	{ "二路WU电压(V)",             "AC2 Uwu(V)" },                          //414

	{ "直流电压校准—",             "DC calibration-input" },                //415
	{ "输入实际值",                "actual value" },                        //416
	{ "负载电流校准—",             "Load Cur calibration-" },               //417
	{ "负载电流1(A)",              "Iload1(A)" },                           //418
	{ "负载电流2(A)",              "Iload2(A)" },                           //419
	{ "一组电池电流校准—",         "BAT1 Cur calibration-" },               //420
	{ "二组电池电流校准—",         "BAT2 Cur calibration-" },               //421
	{ "电池电流1(A)",              "Ibat1(A)" },                            //422
	{ "电池电流2(A)",              "Ibat2(A)" },                            //423

	{ "请等待...",                 "Please waiting..." },                   //424
	{ "校准成功！",                "Calibration sucess!" },                 //425
	{ "校准失败！",                "Calibration fail!" },                   //426
	{ "第一个电流点输入完成！",    "1nd Cur input finish! " },              //427
	{ "请按CANCEL键返回，然后",    "Press 'CANCEL' return!" },              //428
	{ "输入另一个点的电流值。",    "2nd Cur input finish! " },              //429
	
	{ "显示语言及显示方式",        "Display Config" },                      //430
	{ "语言",                      "Language" },                            //431
	{ "方式",                      "Display" },                             //432
	{ "中文",                      "Chinese" },                             //433
	{ "英文",                      "English" },                             //434
	{ "水平",                      "Horizontal" },                          //435
	{ "竖显",                      "Vertical" },                            //436

	{ "产品",                      "Prod" },                                //437
	{ "硬件",                      "Hard" },                                //438
	{ "软件",                      "Soft" },                                //439
	
	{ "电池正在核容/手动管理",     "Batt checking/Manual" },                //440
	{ "电池正在放电/手动管理",     "Batt discharge/Manual" },               //441
	
	{ "电池开关2",                 "BAT SWT2" },                            //442
	
	{ "SC12",                      "SC12" },                                //443
	{ "SC22",                      "SC22" },                                //444
	{ "SC32",                      "SC32" },                                //445
	
	{ "系统无此项功能！",          "Without this function" },               //446
	
	{ ".维护指南",                 ".Maintenance guide" },                  //447
	{ "维护指南:",                 "Maintenance:" },                        //448
	{ ".开通",                     ".System commissioning" },               //449
	{ ".通讯异常报警",             ".Communication alarm" },                //450
	{ ".过欠压报警",               ".Over/Under Vol alarm" },               //451
	{ ".绝缘报警",                 ".Insulation alarm" },                   //452
	{ ".后台通讯异常",             ".Remote Comm alarm" },                  //453
	{ ".联系厂家",                 ".Contact manufacturer" },               //454
	{ ".关于设备返修",             ".Equipment repair" },                   //455
//Check on-line is normal or not
//Check module address is correct
//Check the configuration parameters is correct	
	{ "开通",                      ".System commissioning" },               //456
	{ "开通前需按系统配线图检",    "Check on-line is norm-" },              //457
	{ "查屏间联线是否正常，各",    "al, and module addres-" },              //458
	{ "模块通讯地址是否按要求",    "s is correct, and the " },              //459
	{ "正确设置，监控设置菜单",    "configuration paramet-" },              //460
	{ "中的配置参数是否与系统",    "ers is correct.       " },              //461
	{ "实物配置相一致。",          "                      " },              //462
//Check the operation of equipment lamp is normal or not
//Communication line if there is loose or anomaly
//Device address is correct	
	{ "通讯异常报警",              ".Communication alarm  " },              //463
	{ "通讯异常报警先确认报警",    "Check the operation of" },              //464
	{ "装置，检查所报警的设备",    "equipment lamp is nor-" },              //465
	{ "运行或电源指示灯是否正",    "mal or not, and Commu-" },              //466
	{ "常；通讯线是否有松动及",    "nication line if ther-" },              //467
	{ "异常，可用万用表测量通",    "e is loose or anomaly," },              //468
	{ "讯线是否通；检查设备通",    "and Device address is " },              //469
	{ "讯地址设置是否正常；如",    "correct!              " },              //470
	{ "有多个相同的装置，可交",    "                      " },              //471
	{ "换位置来定位故障性质。",    "                      " },              //472
//Check the detailed monitoring alarm content
//To check that whether the measured value by the main monitoring and the actual value measured by multimeter consistent	
//Such as consistent, check alarm threshold value is reasonable
	{ "过欠压报警",                ".Over/Under Vol alarm " },              //473
	{ "确认故障内容，在当前报",    "Check the detailed mo-" },              //474
	{ "警中查找详细的报警内容，",  "nitoring alarm content," },             //475
	{ "再查看主监控显示的测量",    "whether the measured  " },              //476
	{ "值，再用万用表测量采集",    "value by monitoring a-" },              //477
	{ "模块相应端子的采样信号",    "nd the actual value m-" },              //478
	{ "值，是否与显示值相一致；",  "easured by multimeter " },              //479
	{ "如一致，查看相应的报警",    "consistent, threshold " },              //480
	{ "范围设限是否正确。",        "value is reasonable   " },              //481
//Check the main monitoring shows that positive and negative bus voltage to earth and resistance
//Monitoring shows that the bus voltage and multimeter measured bus voltage to ground is consistent	
//Measuring sensor output signal to calculate resistance to earth
	{ "绝缘报警",                  ".INSU drop alarm" },                    //482
	{ "查看主监控显示的正负母",    "Check the main monito-" },              //483
	{ "线对地电压及电阻，用万",    "ring shows that posit-" },              //484
	{ "用表示实测系统母线对地",    "ive and negative bus  " },              //485
	{ "电压是否与显示值相一致，",  "voltage to earth and  " },              //486
	{ "如一致，则说明测量正常，",  "resistance. Measuring " },              //487
	{ "再查看是否有支路报警，",    "sensor output signal  " },              //488
	{ "如有支路报警，则用万表",    "to calculate resistan-" },              //489
	{ "测量此支路的传感器输出",    "ce to earth.          " },              //490
	{ "信号；最后通过断开关的",    "                      " },              //491
	{ "形式确认报警是否正确。",    "                      " },              //492
//Check the communication protocol and baud rate set  by monitoring device is correct	
//Use the test software testing remote communication port communication is normal or not
	{ "后台通讯异常",              ".Remote Comm alarm" },                  //493
	{ "查看主监控装置所设置的",    "Check the communicati-" },              //494
	{ "通讯协议及波特率是否与",    "on protocol and baud  " },              //495
	{ "综自使用的相一致；检查",    "rate set  by monitori-" },              //496
	{ "综自通讯口接线是否正确；",  "ng device is correct. " },              //497
	{ "可用测试软件测试后台通",    "Use the test software " },              //498
	{ "讯口通讯是否正常。",        "testing remote port.  " },              //499
//The fault that can't handle by own , by telephone manufacturers remote guidance	
//Please provide the factory equipment model and equipment Numbers, so that manufacturers 
//find relevant order information, quickly and accurately remove the fault
	{ "联系厂家",                  ".Contact manufacturer" },               //500	
	{ "在现场不能自行处理的故",    "The fault that can't h" },              //501
	{ "障，可电话联系厂家进行",    "andle by own, Please p" },              //502
	{ "远程指导，请报给厂家设",    "rovide the equipment m" },              //503
	{ "备型号及设备编号，以方",    "odel and equipment No." },              //504
	{ "便厂家查找相关的订单信",    "so that manufacturers " },              //505
	{ "息，最好在现场与厂家联",    "quickly and accurately" },              //506
	{ "系，以方便快速准确的排",    "remove the fault.     " },              //507
	{ "除故障。",                  "                      " },              //508
//If confirmed equipment has been damaged,
//Please return to list describing field failure phenomenon, 
//with convenient maintenance accurate positioning fault point and related information collection	
	{ "关于设备返修",              ".Equipment repair" },                   //509
	{ "如确认设备已损坏，请在",    "If confirmed equipment" },              //510
	{ "返回单上描述现场故障现",    "has been damaged, Ple-" },              //511
	{ "象，以方便维修准确定位",    "ase return to list de-" },              //512
	{ "故障点及相关信息的收集。",  "scribing field failure" },              //513  
	                                                                        
	{ "交流窜直流电压",            "AC join to DC" },                       //514
	{ "交流窜直流报警(V)",         "AC2DC AP(V)" },                         //515

	{ "母线负对地电压(V)",         "P- Vol to GND(V)" },                    //516

	{ ".维护级配置",               ".Defend Level Config" },                //517
	
	{ "手动限流调节设置",          "Manu Lmt Cur Cfg" },                    //518
	{ "限流调节方式",              "Lmt Cur Mode" },                        //519
	{ "手动限流点(%)",             "Manu Lmt Cur(%)" },                     //520
	
	{ "通讯报警条件设置",          "Comm Offline Config" },                 //521
	{ "馈线检测模块",              "FDL Module" },                          //522
	{ "电池巡检",                  "BAT Module" },                          //523
	{ "整流模块",                  "Rect Module" },                         //524
	{ "通信模块",                  "CPS Module" },                          //525
	{ "逆变模块",                  "UPS Module" },                          //526
	{ "ATS模块",                   "ATS Module" },                          //527
	{ "多功能电表",                "ACM Module" },                          //528
	
	{ "工具",                      "Tool" },                                //529

	{ "5A",                        "5A" },                                  //530
	{ "7A",                        "7A" },                                  //531
	{ "10A",                       "10A" },                                 //532
	{ "20A",                       "20A" },                                 //533
	{ "30A",                       "30A" },                                 //534
	{ "35A",                       "35A" },                                 //535
	{ "40A",                       "40A" },                                 //536
	{ "50A",                       "50A" },                                 //537

	{ "MODBUS",                    "MODBUS" },                              //538
	{ "CAN",                       "CAN" },                                 //539

	{ "5A",                        "5A" },                                  //540
	{ "10A",                       "10A" },                                 //541
	{ "20A",                       "20A" },                                 //542
	{ "30A",                       "30A" },                                 //543
	{ "40A",                       "40A" },                                 //544
	{ "50A",                       "50A" },                                 //545
	{ "60A",                       "60A" },                                 //546
	{ "80A",                       "80A" },                                 //547
	{ "100A",                      "100A" },                                //548

	{ "绝缘测量参数配置",          "INSU Para Config" },                    //549
	{ "CAN速率",                   "CAN Baud" },                            //550	
	{ "测量模式",                  "Meas Way" },                            //551
	{ "电桥投切延时(S)",           "ChangeTime(S)" },                       //552
	{ "传感器量程(mA)",            "SensorRate(mA)" },                      //553
	{ "电压不平衡(S)",             "ConfirmTime(S)" },                      //554
	{ "绝缘测量周期(H)",           "MeasPeriod(H)" },                       //555
	{ "定期测量-时(H)",            "MeasHour(H)" },                         //556
	{ "定期测量-分(M)",            "MeasMinute(M)" },                       //557
	{ "125K",                      "125K" },                                //558
	{ "50K",                       "50K" },                                 //559
	{ "20K",                       "20K" },                                 //560
	{ "10K",                       "10K" },                                 //561
	{ "工程",                      "Normal" },                              //562
	{ "调试",                      "Debug " },                              //563

	{ "母线数据来源配置",          "Bus data source" },                     //564
	{ "D21模块(个)",               "D21 Num" },                             //565

	{ "电流零点修正设置",          "Zero curr correct" },                   //566
	{ "",                          "" },                           //567	
	{ "负载修正(A)",               "Load curr(A)" },                        //568	
	{ "",                          "" },                           //569
	{ "电池1修正(A)",              "Bat1 curr(A)" },                        //570
	{ "",                          "" },                           //571	
	{ "电池2修正(A)",              "Bat2 curr(A)" },                        //572
	{ "+",                         "+" },                                   //573
	{ "-",                         "-" },                                   //574

	{ "二组电池电压(V)",           "Batt2 Vol(V)" },                         //575
	{ "二组电池电流(A)",           "Batt2 Cur(A)" },                         //576
	{ "二组电池容量(Ah)",          "Batt2 Capa(Ah)" },                       //577
	{ "二段负载电流(A)",           "Load2 Cur(A)" },                         //578
	{ "二段合母电压(V)",           "PB2  Vol(V)" },                          //579
	{ "二段母线电压(V)",           "Bus2 Vol(V)" },                          //580
	{ "二段控母电压(V)",           "CB2  Vol(V)" },                          //581

	{ "直流柜DC10模块配置",        "DCS DC10 Cur Config" },                 //582
	{ "DC10模块(个)",              "DC10 Num" },                            //583
	{ "A1采集电压(mV)",            "A1 Cur Sample(mV)" },                   //584
	{ "A2采集电压(mV)",            "A2 Cur Sample(mV)" },                   //585
	{ "A3采集电压(mV)",            "A3 Cur Sample(mV)" },                   //586
	{ "A/S1电流量程(A)",           "A/S1 Cur Range(A)" },                   //587
	{ "A/S2电流量程(A)",           "A/S2 Cur Range(A)" },                   //588
	{ "A/S3电流量程(A)",           "A/S3 Cur Range(A)" },                   //589

	{ "二组电池",                  "BATT2" },                               //590

	{ ".交流系统配置",             ".AC System Config    " },               //591

	{ "交流柜DC10模块配置",        "ACS DC10 Cur Config" },                 //592
	{ "DC10模块(个)",              "DC10 Num" },                            //593
	{ "交流过压点(V)",             "ACS AC OVP(V)" },                       //594
	{ "交流欠压点(V)",             "ACS AC UVP(V)" },                       //595
	{ "交流缺相点(V)",             "ACS AC PLP(V)" },                       //596
	{ "A/S1电流量程(A)",           "A/S1 Cur Range(A)" },                   //597
	{ "A/S2电流量程(A)",           "A/S2 Cur Range(A)" },                   //598
	{ "A/S3电流量程(A)",           "A/S3 Cur Range(A)" },                   //599

	{ "交流馈出参数配置",          "ATS SWT Config" },                      //600
	{ "交流馈线柜",                "ACS FDL Panel" },                       //601
	{ "#馈线模块",                 "#FDL Module" },                         //602

	{ ".交流系统信息查询",         ".AC System Info" },                     //603

	{ "交流系统信息查询:",         "ACS Panel Info: " },                    //604
	{ ".交流母线信息查询",         ".AC Bus Info    " },                    //605
	{ ".交流开关状态查询",         ".ACS SWT State Info" },                 //606

	{ "交流输出开关状态",          "ACS Panel SWT State" },                 //607

	{ "DC10模块",                  "DC10 Module" },                         //608
	{ "RC10模块",                  "RC10 Module" },                         //609

	{ "RC10模块(个)",              "RC10 Module" },                         //610

	{ ".电操开关控制",             ".SWT Control" },                        //611

	{ "电操开关控制",              "Switch On/Off" },                       //612

	{ ".电操开关状态查询",         ".EC SWT State Query " },                //613
	{ "电操开关状态",              "EC SWT State" },                        //614
	
	{ "直流母线段数",              "DC Bus Num" },                          //615	
	{ "一段",                      "One Seg" },                             //616
	{ "两段",                      "Two Seg" },                             //617

	{ "绝缘主机地址配置 ",         "Insu Master Add" },                     //618
	{ "一段绝缘主机： ",           "1#Insu Master:" },                      //619
	{ "二段绝缘主机： ",           "2#Insu Master:" },                      //620

	{ "电流(A)",                   "AC Curr(A)" },                          //621
	{ "交流过压延时(S)",           "AC OV Delay(S)" },                   //622
};


/************************** 事件名称显示字符串定义 ********************************/
const STR_T g_s_batt_record[][2] =            //电池充放电转换记录名称
{
	{ "1#充电管理设为自动",          "1# Auto charging   " },
	{ "1#充电管理设为手动",          "1# Manu charging   " },
	{ "1#电池自动转浮充",            "1# Auto floating   " },
	{ "1#电池自动转均充",            "1# Auto equalizing " },
	{ "1#电池自动转核容",            "1# Auto discharging" },
	{ "1#电池手动转浮充",            "1# Manu floating   " },
	{ "1#电池手动转均充",            "1# Manu equalizing " },
	{ "1#电池手动转核容",            "1# Manu discharging" },
	
	{ "2#充电管理设为自动",          "2# Auto charging   " },
	{ "2#充电管理设为手动",          "2# Manu charging   " },
	{ "2#电池自动转浮充",            "2# Auto floating   " },
	{ "2#电池自动转均充",            "2# Auto equalizing " },
	{ "2#电池自动转核容",            "2# Auto discharging" },
	{ "2#电池手动转浮充",            "2# Manu floating   " },
	{ "2#电池手动转均充",            "2# Manu equalizing " },
	{ "2#电池手动转核容",            "2# Manu discharging" },
};

/************************** 电操开关名称显示字符串定义 ********************************/
const STR_T g_s_ctrl_swt_name[FACT_SWT_CTRL_MAX][2] =            //电操开关名称
{	//1#RC10电操开关
	{ "4S1开关",                   "4S1 Switch   "    },
	{ "4S2开关",                   "4S2 Switch   "    },
	{ "4S3开关",                   "4S3 Switch   "    },	
	{ "4S4开关",                   "4S4 Switch   "    },
	{ "5S1开关",                   "5S1 Switch   "    },
	{ "5S2开关",                   "5S2 Switch   "    },
	{ "5S3开关",                   "5S3 Switch   "    },
	{ "5S4开关",                   "5S4 Switch   "   },

	//2#RC10电操开关
	{ "5S5开关",                   "5S5 Switch   "    },
	{ "5S6开关",                   "5S6 Switch   "    },
	{ "S1开关",                    "S1 Switch   "     },
	{ "S2开关",                    "S2 Switch   "     },
	{ "S3开关",                    "S3 Switch   "     },
	{ "S4开关",                    "S4 Switch   "     },
	{ "S5开关",                    "S5 Switch   "     },
	{ "8S1开关",                   "8S1 Switch   "    },

	//3#RC10电操开关
	{ "8S2开关",                   "3S9 Switch   "    },
	{ "8S3开关",                  "3S10 Switch   "    },
	{ "8S4开关",                  "3S11 Switch   "    },
	{ "8S5开关",                  "3S12 Switch   "    },
	{ "未定义1",                  "Reserved 1	"	},
	{ "未定义2",                  "Reserved 2	"	},
	{ "未定义3",                  "Reserved 3	"	},
	{ "未定义4",                  "Reserved 4	"	},


//	//4#RC10电操开关	
//	{ "4S1开关",                   "4S1 Switch   "    },
//	{ "4S2开关",                   "4S2 Switch   "    },
//	{ "4S3开关",                   "4S3 Switch   "    },
//	{ "4S4开关",                   "4S4 Switch   "    },
//	{ "4S5开关",                   "4S5 Switch   "    },
//	{ "4S6开关",                   "4S6 Switch   "    },
//	{ "4S7开关",                   "4S7 Switch   "    },
//	{ "4S8开关",                   "4S8 Switch   "    },
//	
//	//5#RC10电操开关	
//	{ "4S9开关",                   "4S9 Switch   "    },
//	{ "4S10开关",                  "4S10 Switch   "    },
//	{ "4S11开关",                  "4S11 Switch   "    },
//	{ "4S12开关",                  "4S12 Switch   "    },
//	{ "4S13开关",                  "4S13 Switch   "    },
//	{ "4S14开关",                  "4S14 Switch   "    },
//	{ "4S15开关",                  "4S15 Switch   "    },
//	{ "4S16开关",                  "4S16 Switch   "    },
//	
//	//6#RC10电操开关	
//	{ "3S17开关",                  "3S17 Switch   "    },
//	{ "4S17开关",                  "4S17 Switch   "    },
//	{ "MLS开关",                   "MLS Switch   "    },
//	{ "未定义1",                   "Reserved 1   "   },
//	{ "未定义2",                   "Reserved 2   "   },
//	{ "未定义3",                   "Reserved 3   "   },
//	{ "未定义4",                   "Reserved 4   "   },
//	{ "未定义5",                   "Reserved 5   "   },

//	//7#RC10电操开关
//	{ "1HK开关",                   "1HK Switch   "   },
//	{ "2HK开关",                   "2HK Switch   "   },
//	{ "3HK开关",                   "3HK Switch   "   },
//	{ "未定义1",                   "Reserved 1   "    },
//	{ "未定义2",                   "Reserved 2   "    },
//	{ "未定义3",                   "Reserved 3   "    },
//	{ "未定义4",                   "Reserved 4   "    },
//	{ "未定义5",                   "Reserved 5   "    },
//	
//	//8#RC10电操开关
//	{ "AC 3S1开关",                "AC 3S1 Switch   "    },
//	{ "AC 3S2开关",                "AC 3S2 Switch   "    },
//	{ "AC 3S3开关",                "AC 3S3 Switch   "    },
//	{ "AC 3S4开关",                "AC 3S4 Switch   "    },
//	{ "AC 3S5开关",                "AC 3S5 Switch   "    },
//	{ "AC 3S6开关",                "AC 3S6 Switch   "    },
//	{ "AC 3S7开关",                "AC 3S7 Switch   "    },
//	{ "AC 3S8开关",                "AC 3S8 Switch   "    },

//	//9#RC10电操开关
//	{ "AC 3S9开关",                "AC 3S9 Switch   "    },
//	{ "AC 3S10开关",               "AC 3S10 Switch   "    },
//	{ "AC 3S11开关",               "AC 3S11 Switch   "    },
//	{ "AC 3S12开关",               "AC 3S12 Switch   "    },
//	{ "AC 3S13开关",               "AC 3S13 Switch   "    },
//	{ "AC 3S14开关",               "AC 3S14 Switch   "    },
//	{ "AC 3S15开关",               "AC 3S15 Switch   "    },
//	{ "AC 3S16开关",               "AC 3S16 Switch   "    },

//	//10#RC10电操开关
//	{ "AC 4S1开关",                "AC 4S1 Switch   "    },
//	{ "AC 4S2开关",                "AC 4S2 Switch   "    },
//	{ "AC 4S3开关",                "AC 4S3 Switch   "    },
//	{ "AC 4S4开关",                "AC 4S4 Switch   "    },
//	{ "AC 4S5开关",                "AC 4S5 Switch   "    },
//	{ "AC 4S6开关",                "AC 4S6 Switch   "    },
//	{ "AC 4S7开关",                "AC 4S7 Switch   "    },
//	{ "AC 4S8开关",                "AC 4S8 Switch   "    },

//	//11#RC10电操开关
//	{ "AC 4S9开关",                "AC 4S9 Switch   "    },
//	{ "AC 4S10开关",               "AC 4S10 Switch   "    },
//	{ "AC 4S11开关",               "AC 4S11 Switch   "    },
//	{ "AC 4S12开关",               "AC 4S12 Switch   "    },
//	{ "AC 4S13开关",               "AC 4S13 Switch   "    },
//	{ "AC 4S14开关",               "AC 4S14 Switch   "    },
//	{ "AC 4S15开关",               "AC 4S15 Switch   "    },
//	{ "未定义1",                   "Reserved 1   "    },
};
