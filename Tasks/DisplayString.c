/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����DisplayString.c
��    ����1.00
�������ڣ�2012-05-09
��    �ߣ�������
������������ʾ�ַ�������

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-05-09  1.00     ����
**************************************************************/

#include "Type.h"
#include "ShareDataStruct.h"
#include "DisplayString.h"


/************************** ������ʾ�ַ������� ********************************/
const STR_T g_s_string[][2] =
{
	{ "�����¶�(��)",              "Temperature(��)" },                     //0
	{ "һ���ص�ѹ(V)",           "Batt1 Vol(V)" },                        //1
	{ "һ���ص���(A)",           "Batt1 Cur(A)" },                        //2
	{ "һ��������(Ah)",          "Batt1 Capa(Ah)" },                      //3
	{ "һ�θ��ص���(A)",           "Load1 Cur(A)" },                        //4
	{ "һ�κ�ĸ��ѹ(V)",           "PB1  Vol(V)" },                         //5
	{ "һ��ĸ�ߵ�ѹ(V)",           "Bus1 Vol(V)" },                         //6
	{ "һ�ο�ĸ��ѹ(V)",           "CB1  Vol(V)" },                         //7
	{ "������ڸ���/�Զ�����",     "Batt FLO/Auto" },                       //8
	{ "������ھ���/�Զ�����",     "Batt EQU/Auto" },                       //9
	{ "������ں���/�Զ�����",     "Batt CHK/Auto" },                       //10
	{ "������ڷŵ�/�Զ�����",     "Batt DIS/Auto" },                       //11
	{ "������ڸ���/�ֶ�����",     "Batt FLO/Manu" },                       //12
	{ "������ھ���/�ֶ�����",     "Batt EQU/Manu" },                       //13
	{ "ϵͳ����",                  "Sys normal" },                          //14
	{ "ϵͳ�澯/��������",         "Sys alarm/AC fault" },                  //15   
	{ "ϵͳ�澯/ֱ��ĸ�߹���",     "Sys alarm/DC bus fault" },              //16
	{ "ϵͳ�澯/��ع���",         "Sys alarm/Batt fault" },                //17
	{ "ϵͳ�澯/��Ե����",         "Sys alarm/Insu fault" },                //18
	{ "ϵͳ�澯/���ģ�����",     "Sys alarm/Rect fault" },                //19
	{ "ϵͳ�澯/�۶�������",       "Sys alarm/Fuse fault" },                //20
	{ "ϵͳ�澯/UPS|CPS����",      "Sys alarm/UPS|CPS fault" },             //21
	{ "ϵͳ�澯/����֧·����",     "Sys alarm/Feeder fault" },              //22

	{ "��ѡ��:",                   "Select please:" },                      //23
	{ ".ֱ��ϵͳ��Ϣ��ѯ",         ".DC  System Info" },                    //24
	{ ".ͨ��ϵͳ��Ϣ��ѯ",         ".CPS System Info" },                    //25
	{ ".UPSϵͳ��Ϣ��ѯ",          ".UPS System Info" },                    //26
	{ ".��ǰ�澯��ѯ",             ".CUR Alarm  Info" },                    //27
	{ ".��ʷ�澯��ѯ",             ".HIS Alarm  Info" },                    //28
	{ ".����δ����澯��ѯ",       ".UNR Alarm  Info" },                    //29
	{ ".�¼���¼��ѯ",             ".Event Log  Info" },                    //30
	{ ".��������",                 ".Parameter Setup" },                    //31
	{ ".����",                     ".About" },                              //32

	{ "ֱ��ϵͳ��Ϣ��ѯ:",         "DC System Info:       " },              //33
	{ ".������Ϣ��ѯ",             ".AC Module Data Query " },              //34
	{ ".���״̬��ѯ",             ".BAT Group Data Query " },              //35
	{ ".һ��ĸ�߾�Ե��ѯ",         ".BUS1 Insu Data Query " },              //36
	{ ".����ģ����Ϣ",             ".REC Module Data Query" },              //37
	{ ".ϵͳ����״̬��ѯ",         ".SWT State Data Query " },              //38
	{ ".һ��������Ϣ��ѯ",         ".FDL1 Panel Data Query" },              //39

	{ "һ�����߹��ѯ:",           "SEG1 FDL panel Info:  " },              //40
	{ ".1#���߹���Ϣ��ѯ",         ".1#FDL Panel Info     " },              //41
	{ ".2#���߹���Ϣ��ѯ",         ".2#FDL Panel Info     " },              //42
	{ ".3#���߹���Ϣ��ѯ",         ".3#FDL Panel Info     " },              //43
	{ ".4#���߹���Ϣ��ѯ",         ".4#FDL Panel Info     " },              //44
	{ ".����ĸ�߾�Ե��ѯ",         ".BUS2 Insu Data Query " },              //45
	{ ".����������Ϣ��ѯ",         ".FDL2 Panel Data Query" },              //46
	{ "�������߹��ѯ:",           "SEG2 FDL panel Info:  " },              //47
	{ "���������(��) ",           "Cabinet Num2          " },              //48

	{ "ͨ��ϵͳ��Ϣ��ѯ:",         "CPS Panel Info:       " },              //49
	{ ".DC/DCģ����Ϣ��ѯ",        ".DC/DC Module Info    " },              //50
	{ ".ͨ�ſ���״̬��ѯ",         ".CPS SWT State Info   " },              //51

	{ "UPSϵͳ��Ϣ��ѯ��",         "UPS Panel Info:       " },              //52
	{ ".UPSģ����Ϣ��ѯ",          ".UPS Module Info      " },              //53
	{ ".UPSϵͳ����״̬��ѯ",      ".UPS SWT State Info   " },              //54

	{ "��ѡ��:",                   "Select please:        " },              //55
	{ ".ֱ��ϵͳ����",             ".DC  System Config    " },              //56
	{ ".ͨ��ϵͳ����",             ".CPS System Config    " },              //57
	{ ".UPSϵͳ����",              ".UPS System Config    " },              //58
	{ ".�澯�趨",                 ".SYS Alarm  Config    " },              //59
	{ ".��̨ͨѶ",                 ".Remote Comm Config   " },              //60
	{ ".ʱ������",                 ".Time&Password Setup  " },              //61
	{ ".ϵͳ����",                 ".System Control       " },              //62
	{ ".�����ָ�",                 ".Parameter Recovery   " },              //63
	{ ".����У׼",                 ".Parameter Calibrate  " },              //64
	{ ".��ʾ����",                 ".Display Setup        " },              //65

	{ "ֱ��ϵͳ����:",             "DC System Config:     " },              //66
	{ ".ֱ������",                 ".DC Config            " },              //67
	{ ".һ�����߹�����",           ".SEG1 FDL Panel Config" },              //68
	{ ".����ֵ����",               ".ThresholdValue Config" },              //69
	{ ".��س�ŵ�����",           ".BAT Manage Config    " },              //70
	{ ".��缰ģ�����",           ".Charge Control       " },              //71

	{ "һ�����߹�����:",           "SEG1 FDL Panel Setup: " },              //72
	{ ".1#���߹�����",             ".1#FDL Panel Config   " },              //73
	{ ".2#���߹�����",             ".2#FDL Panel Config   " },              //74
	{ ".3#���߹�����",             ".3#FDL Panel Config   " },              //75
	{ ".4#���߹�����",             ".4#FDL Panel Config   " },              //76
	{ "�������߹�����:",           "SEG2 FDL Panel Setup: " },              //77
	{ "���ζԵص�ѹ����(V)",       "SEG2 Vol to GND(V)    " },              //78
	{ "���ζԵص������(K��)",      "SEG2 Res to GND(K��)   " },              //79
	{ ".�������߹�����",           ".SEG2 FDL Panel Config" },              //80

	{ "ϵͳδ���ô�����Ϣ��",      "Not config this info, " },              //81
	{ "������Ҫ�����ã�",          "Reconfigure please!   " },              //82

	{ "������������",              "AC Realtime Data" },                    //83
	{ "����һ·�ߵ�ѹ",            "AC1 Line Voltage" },                    //84
	{ "������·�ߵ�ѹ",            "AC2 Line Voltage" },                    //85
	{ "UV��ѹ(V)",                 "Uuv(V)" },                              //86
	{ "VW��ѹ(V)",                 "Uvw(V)" },                              //87
	{ "WU��ѹ(V)",                 "Uwu(V)" },                              //88
	{ "����һ·���ѹ",            "AC1 Phase Voltage" },                   //89
	{ "������·���ѹ",            "AC2 Phase Voltage" },                   //90
	{ "��ѹ(V)",                   "Voltage(V)" },                          //91

	{ "�������������",            "BAT Realtime Data" },                   //92
	{ "��ǰ����(Ah)",              "BAT Capa(Ah)" },                        //93
	{ "��ص�ѹ(V)",               "BAT Vol(V)" },                          //94
	{ "��ص���(A)",               "BAT Cur(A)" },                          //95
	{ "���״̬",                  "BAT State" },                           //96
	{ "�Ѹ���(hor)",               "FLO Time(h)" },                         //97
	{ "�Ѿ���(min)",               "EQU Time(m)" },                         //98
	{ "�Ѻ���(min)",               "DIS Time(m)" },                         //99
	{ "����",                      "FLO" },                                 //100
	{ "����",                      "EQU" },                                 //101
	{ "����",                      "DIS" },                                 //102

	{ "һ��������",              "BAT1 Data" },                           //103
	{ "����������",              "BAT2 Data" },                           //104
	{ "��ص���(A)",               "BAT Cur(A)" },                          //105
	{ "��ǰ����(Ah)",              "BAT Capa(Ah)" },                        //106

	{ "һ����Ѳ������",          "BAT1 Cell Data" },                      //107
	{ "������Ѳ������",          "BAT2 Cell Data" },                      //108
	{ "�����ѹ���ֵ��Ϣ",        "BAT Cell Vmax" },                       //109
	{ "�����ѹ���ֵ��Ϣ",        "BAT Cell Vmin" },                       //110
	{ "������",                  "Cell No." },                            //111
	{ "��ѹֵ(V)",                 "Cell Vol" },                            //112
	{ "һ�鵥���ѹ����(V)",       "BAT1 Cell Vol(V)" },                    //113
	{ "���鵥���ѹ����(V)",       "BAT2 Cell Vol(V)" },                    //114

	{ "һ�ζԵص�ѹ����(V)",       "SEG1 Vol to GND(V)" },                  //115
	{ "��ĸ���Եص�ѹ",            "PB Vol to GND" },                       //116
	{ "��ĸ���Եص�ѹ",            "CB Vol to GND" },                       //117
	{ "ĸ�����Եص�ѹ",            "P+ Vol to GND" },                       //118
	{ "ĸ�߸��Եص�ѹ",            "P- Vol to GND" },                       //119
	{ "һ�ζԵص������(K��)",      "SEG1 Res to GND(K��)" },                 //120
	{ "���Եص���",                "Res+ to GND" },                         //121
	{ "���Եص���",                "Res- to GND" },                         //122

	{ "#����ģ����Ϣ",             "#Rect Module Info" },                   //123
	{ "#DC/DCģ����Ϣ",            "#DC/DC Module Info" },                  //124
	{ "�����ѹ(V)",               "Out Vol(V)" },                          //125
	{ "�������(A)",               "Out Cur(A)" },                          //126
	{ "������(%)",                 "Lmt Cur(%)" },                          //127
	{ "���Ʒ�ʽ",                  "Control Way" },                         //128
	{ "ģ��״̬",                  "State" },                               //129
	{ "�Զ�",                      "Auto" },                                //130
	{ "�ֶ�",                      "Manu" },                                //131
	{ "����",                      "On " },                                 //132
	{ "�ػ�",                      "Off" },                                 //133
	{ "�쳣",                      "Fault" },                               //134

	{ "ֱ��ϵͳ����״̬",          "DC System SWT State" },                 //135
	{ "��������",                "Charger SWT" },                         //136
	{ "��ؿ���1",                 "BAT SWT1" },                            //137
	{ "ĸ������",                  "Bus tie SWT" },                         //138		   
	{ "��������1",                 "AC SWT1" },                             //139
	{ "��������2",                 "AC SWT2" },                             //140
	{ "�Ͽ�",                      "Off" },                                 //141
	{ "�պ�",                      "ON " },                                 //142

	{ "#���߹��������״̬",       "#FDL Panel SWT State" },                //143
	{ "ͨ���������״̬",          "CPS Panel SWT State" },                 //144
	{ "UPSϵͳ�������״̬",       "UPS Panel SWT State" },                 //145
	{ "#�������",                 "#SWT State" },                          //146
	{ "����״̬",                  "SWT State" },                           //147
	{ "��Ե����(K��)",              "Insu Res(K��)" },                        //148
	{ "�������(A)",               "Out Cur(A)" },                          //149
	{ "������״̬",                "Sensor State" },                        //150
	{ "---",                       "---" },                                 //151
	{ "��",                        "Off" },                                 //152
	{ "��",                        "On " },                                 //153
	{ "---",                       "---" },                                 //154
	{ "����",                      "Normal" },                              //155
	{ "�쳣",                      "Fault" },                               //156

	{ "#UPSģ����Ϣ ",             "#UPS Module Info" },                    //157
	{ "���Ƶ��(Hz)",              "Out Freq(Hz)" },                        //158
	{ "����ѹ(V)",               "Invert Vol(V)" },                       //159
	{ "��·��ѹ(V)",               "Bypass Vol(V)" },                       //160
	{ "����й�(KW)",              "Out power(KW)" },                       //161
	{ "���ڹ���(KVA)",             "App power(KVA)" },                      //162
	{ "��������",                  "Power Rate" },                          //163
	{ "������ʽ",                  "Work Way " },                           //164
	{ "����/���",                 "On /INV" },                             //165
	{ "����/��·",                 "On /BYP" },                             //166
	{ "�ػ�",                      "Off " },                                //167
	{ "״̬",                      "State" },                               //168
	{ "����",                      "Normal" },                              //169
	{ "����",                      "Alarm" },                               //170

	{ "��ǰ�澯: ",                "Current Alarm: " },                     //171
	{ "��ʷ�澯: ",                "History Alarm: " },                     //172
	{ "����δ����澯: ",          "Unrecovered Alarm: " },                 //173
	{ "�¼���¼: ",                "Event Log: " },                         //174
	{ "����:",                     "Strt:" },                               //175
	{ "����:",                     "End: " },                               //176
	{ "ϵͳ��ǰ�������޹���",      "System is normal," },                   //177
	{ "��¼��",                    "no fault log��" },                      //178
	{ "����ʷ���ϼ�¼��Ϣ��",      "No history fault log��" },              //179
	{ "�޵���δ�������",          "No Recovery fault log" },               //180
	{ "��¼��Ϣ��",                "Log information��" },                   //181
	{ "���¼���¼��Ϣ��",          "No Event Log info��" },                 //182
	{ "���ݼ�¼������",            "Record isn't complete" },               //183

	{ "��������������:",           "Please input password:" },              //184
	{ "����������������룡",    "mistake, enter again��" },              //185
	
	{ "�������ó�����ʾ",          "Setting transfinit tip" },              //186
	{ "���������������ֵ��",      "Para beyond the value," },              //187
	{ "��ESC�����ء�",             "Press 'ESC' back." },                   //188
	{ "�������ֵ: ",              "Set the max: " },                       //189
	{ "������Сֵ: ",              "Set the min: " },                       //190

	{ "ֱ��ϵͳ",                  "DC System" },                           //191
	{ "ֱ����ѹ�ȼ�",              "DC Vol level" },                        //192
	{ "�Ͽ�ĸ�ֶ�",                "PB Subsection" },                       //193
	{ "�������",                  "Silicon chain" },                       //194
	{ "��ĸ���(V)",               "CB Out(V)" },                           //195
	{ "һ�������(��) ",           "Cabinet Num1" },                        //196
	{ "220V",                      "220V" },                                //197
	{ "110V",                      "110V" },                                //198
	{ "���ֶ�",                    "Multiple" },                            //199
	{ "�ֶ�",                      "Single" },                              //200
	{ "��",                        "None" },                                //201
	{ "5��4V",                     "5L/4V" },                               //202
	{ "5��7V",                     "5L/7V" },                               //203
	{ "7��3V",                     "7L/3V" },                               //204
	{ "7��5V",                     "7L/5V" },                               //205

	{ "������������",              "AC Para Config" },                      //206
	{ "��������",                  "Phase Num" },                           //207
	{ "����·��",                  "Input Num" },                           //208
	{ "��������",                  "AC Control" },                          //209
	{ "����",                      "PH3 " },                                //210
	{ "����",                      "PH1 " },                                //211
	{ "��",                        "None" },                                //212
	{ "1·",                       "1nd " },                                //213
	{ "2·",                       "2nd " },                                //214
	{ "һ·����",                  "1nd pri" },                             //215
	{ "�̶�һ·",                  "1nd fix" },                             //216
	{ "��·����",                  "2nd pri" },                             //217
	{ "�̶���·",                  "2nd fix" },                             //218

	{ "�������",                  "Battery config" },                      //219
	{ "�������",                  "Group Num" },                           //220
	{ "��������(Ah)",              "Capa(Ah)" },                            //221

	{ "�����ɼ���������",          "Cur Sample config" },                   //222
	{ "�����ɼ�(mV)",              "Cur Sample(mV)" },                      //223
	{ "75",                        "75" },                                  //224
	{ "50",                        "50" },                                  //225
	{ "��������(A)",               "Load Range" },                          //226
	{ "���1����(A)",              "BAT1 Range" },                          //227
	{ "���2����(A)",              "BAT2 Range" },                          //228
	{ "֧·����������ϵ��",        "Sensor coefficient" },                  //229
	{ "����(A)",                   "Range(A)" },                            //230

	{ "����ģ���������",          "Rect Module config" },                  //231
	{ "����(��)",                  "Num" },                                 //232
	{ "�����",                  "Rated Cur" },                           //233
	{ "ͨѶЭ��",                  "Comm Pro" },                            //234
	{ "�ѻ���ѹ(V)",               "Offline Vol" },                         //235
	{ "�ѻ�����(%)",               "Offline Cur" },                         //236
	{ " 5A",                       " 5A" },                                 //237
	{ " 7A",                       " 7A" },                                 //238
	{ "10A",                       "10A" },                                 //239
	{ "20A",                       "20A" },                                 //240
	{ "30A",                       "30A" },                                 //241
	{ "35A",                       "35A" },                                 //242
	{ "40A",                       "40A" },                                 //243

	{ "1����Ѳ������",           "BAT1 Module Config" },                  //244
	{ "2����Ѳ������",           "BAT2 Module Config" },                  //245
	{ "�ͺ�",                      "Type" },                                //246
	{ "����(��)",                  "BXX Num" },                             //247
	{ "��1������",                 "1nd Num" },                             //248
	{ "��2������",                 "2nd Num" },                             //249
	{ "��3������",                 "3nd Num" },                             //250
	{ "��4������",                 "4nd Num" },                             //251
	{ "��5������",                 "5nd Num" },                             //252
	{ "B21",                       "B21" },                                 //253
	{ "B3",                        "B3" },                                  //254
	{ "B4",                        "B4" },                                  //255

	{ "���濪��״̬����",          "SWT State Config" },                    //256
	{ "�澯����",                  "Input Way" },                           //257
	{ "������",                    "Unmeasu" },                             //258
	{ "����",                      "Measu  " },                             //259
	{ "����",                      "Open " },                               //260
	{ "����",                      "Close" },                               //261

	{ "#���߹�����",               "#FDL Panel Config" },                   //262
	{ "����ģ������",              "FDL Num" },                             //263
	{ "#���߹�",                   "#FDL Panel" },                          //264
	{ "#����ģ��",                 "#FDL Module" },                         //265
	{ "ͨѶ��ַ",                  "Address" },                             //266
	{ "������բ",                  "SWT Trip " },                           //267
	{ "��բ����",                  "Alarm In" },                            //268
	{ "����״̬",                  "SWT State" },                           //269
	{ "״̬����",                  "State In" },                            //270
	{ "֧·����",                  "Branch Res" },                          //271
	{ "֧·����",                  "Branch Cur" },                          //272

	{ "������������",              "AC threshold value" },                  //273
	{ "��ѹ��(V)",                 "AC OVP" },                              //274
	{ "Ƿѹ��(V)",                 "AC UVP" },                              //275
	{ "ȱ���(V)",                 "AC PLP" },                              //276

	{ "ֱ����������",              "DC threshold value" },                  //277
	{ "��ĸ��ѹ(V)",               "PB OVP" },                              //278
	{ "��ĸǷѹ(V)",               "PB UVP" },                              //279
	{ "��ĸ��ѹ(V)",               "CB OVP" },                              //280
	{ "��ĸǷѹ(V)",               "CB UVP" },                              //281
	{ "ĸ�߹�ѹ(V)",               "BB OVP" },                              //282
	{ "ĸ��Ƿѹ(V)",               "BB UVP" },                              //283

	{ "�������ֵ����",            "BAT threshold value" },                 //284
	{ "������ѹ(V)",             "BAT  OVP" },                            //285
	{ "�����Ƿѹ(V)",             "BAT  UVP" },                            //286
	{ "�����ѹֵ(V)",             "Cell OVP" },                            //287
	{ "����Ƿѹֵ(V)",             "Cell UVP" },                            //288
	{ "β��ع�ѹ(V)",             "Tail OVP" },                            //289
	{ "β���Ƿѹ(V)",             "Tail UVP" },                            //290
	{ "������(c10)",             "BAT  OIP" },                            //291

	{ "��Ե���ޱ�������",          "INS threshold value" },                 //292
	{ "��ƽ�ⱨ��(V)",             "INS VUP(V)" },                          //293
	{ "��������(K��)",              "INS URP(K��)" },                         //294

	{ "��س���������",          "BAT Parameter Setup" },                 //295
	{ "�����ѹ(V) ",              "EQU Volt(V)" },                         //296
	{ "�����ѹ(V)",               "FLO Volt(V)" },                         //297
	{ "�²�ϵ��(mv/��)",           "Coeff(mv/g.��)" },                      //298
	{ "�������(C10)",             "Lmt Cur(C10)" },                        //299

	{ "���ת�����о�",            "BAT turn FLO criterion" },              //300
	{ "����ת����(C10)",           "EtoF Cur(C10)" },                       //301
	{ "���䵹��ʱ(min)",           "EtoF Time(m)" },                        //302
	{ "��",                        "or" },                                  //303
	{ "������ʱ(min)",             "EtoF Protect(m)" },                     //304

	{ "���ת�����о�",            "BAT turn EQU criterion" },              //305
	{ "����ת����(C10)",           "FtoE Cur(C10)" },                       //306
	{ "����ʱ��(sec)",             "Duration(s)" },                         //307
	{ "����ͣ��(min)",             "AC Stop(s)" },                          //308
	{ "���ھ���(hor)",             "FtoE Period(h)" },                      //309

	{ "��غ�����ֹ����",          "BAT DIS end criterion" },               //310
	{ "�ŵ�ʱ��(min)",             "Dis Time(m)" },                         //311
	{ "��ֹ��ѹ(v)",               "BAT Vol(V)" },                          //312
	{ "�����ѹ(v)",               "Cell Vol(V)" },                         //313

	{ "��طŵ���������",          "Discharge curve Setup" },               //314
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

	{ "��س����Ʒ�ʽ",          "Charging control mode" },               //325
	{ "�ֶ����Ƶ�س�緽ʽ",      "Manu control mode" },                   //326
	{ "һ����",                  "BATT1" },                               //327
	{ "����ģ�鿪�ػ�",            "Rect Module On/Off" },                  //328
	{ "#ģ��",                     "#Module" },                             //329

	{ "ͨ��ģ���������",          "CPS Module Config" },                   //330
	{ "UPSģ���������",           "UPS Module Config" },                   //331
	{ "�������(%)",               "Out Cur(%)" },                          //332
	{ "ͨ�ſ��ز�������",          "CPS SWT Config" },                      //333
	{ "UPS���ز�������",           "UPS SWT Config" },                      //334
	{ "ͨ�ſ���",                  "CPS SWT" },                             //335
	{ "����ģ��",                  "FDL Module" },                          //336
	{ "UPS����",                   "UPS SWT" },                             //337
	{ "����ģ��",                  "FDL Module" },                          //338
	{ "����ϵ��(A)",               "Cur Coeff" },                           //339

	{ "�ɽӵ�����趨",            "Fault Out Setup" },                     //340
	{ "(�޸ĺ��踴λ��Ч)",        "(The revised reset)" },                 //341
	{ "��������",                  "AC Fault" },                            //342
	{ "ֱ��ĸ�߹���",              "DC Bus Fault" },                        //343
	{ "��ع���",                  "BAT Fault" },                           //344
	{ "��Ե����",                  "INS Fault" },                           //345
	{ "���ģ�����",              "Rect Fault" },                          //346
	{ "�۶�������",                "Fuse Fault" },                          //347
	{ "UPS|CPS����",               "UPS|CPS Fault" },                       //348
	{ "����֧·����",              "Feed Fault" },                          //349
	{ "����ָʾ",                  "EQU Tip" },                             //350
	{ "����ָʾ",                  "DIS Tip" },                             //351

	{ "�����趨",                  "Alarm Config" },                        //352
	{ "������",                    "Buzzer" },                              //353
	{ "������ƽ��澯",            "Cur unbalance" },                       //354
	{ "��Ҫ�澯",                  "Minor alarm" },                         //355	   
	{ "һ��澯",                  "General alarm" },                       //356
	{ "����",                      "Alarm" },                               //357
	{ "����",                      "Mute" },                                //358
	{ "�ر�",                      "Disabl" },                              //359
	{ "����",                      "Enable" },                              //360
	{ "������",                    "Unsave" },                              //361
	{ "����",                      "Save  " },                              //362

	{ "Զ��ͨѶ����",              "Remote Comm Setup" },                   //363
	{ "ͨѶ����",                  "Baud" },                                //364
	{ "��żУ��",                  "Parity" },                              //365
	{ "B���ʱ",                   "IRIG-B" },                              //366
	{ "1200",                      "1200" },                                //367
	{ "2400",                      "2400" },                                //368
	{ "4800",                      "4800" },                                //369
	{ "9600",                      "9600" },                                //370
	{ "19200",                     "19200" },                               //371
	{ "��У��",                    "Odd " },                                //372
	{ "żУ��",                    "Even" },                                //373
	{ "��У��",                    "None" },                                //374
	{ "MODBUS",                    "MODBUS" },                              //375
	{ "CDT",                       "CDT" },                                 //376
	{ "У׼",                      "ADJUST" },                              //377
	{ "IEC103",                    "IEC103" },                              //378
	{ "��",                        "No " },                                 //379
	{ "��",                        "Yes" },                                 //380

	{ "RTCʱ���趨",               "RTC Time Setup" },                      //381
	{ "��",                        "Y" },                                   //382
	{ "��",                        "M" },                                   //383
	{ "��",                        "D" },                                   //384
	{ "ʱ",                        "H" },                                   //385
	{ "��",                        "M" },                                   //386
	{ "��",                        "S" },                                   //387
	{ "�û�����",                  "Password" },                            //388

	{ "��ʷ�澯��¼���",          "His Alarm Log Clean" },                 //389
	{ "�����󣬵�ǰ����",          "After operation, all  " },              //390
	{ "��ʷ�澯��¼�����",        "his Alarm log clean!  " },              //391

	{ "����δ����澯���",        "Unrecovered Log Clean" },               //392
	{ "�����󣬵�ǰ���е���",      "After operation, all  " },              //393
	{ "δ����澯��¼�����",      "unrecovered log clean!" },              //394

	{ "�¼���¼���",              "Event Log Clean" },                     //395
	{ "�����󣬵�ǰ����",          "After operation, all  " },              //396
	{ "�¼���¼�����",            "event log Clean!      " },              //397

	{ "��������ָ�",              "BAT Capacity Recover" },                //398
	{ "�����󣬻ָ���ص�ǰ",      "After operation, BAT  " },              //399
	{ "����Ϊ�ֵ",              "Capa recovery rating! " },              //400

	{ "���ò����ָ�",              "Set param recover" },                   //401
	{ "��ע�⣬������",          "After operation, all  " },              //402
	{ "�������ݻָ�Ĭ��ֵ",        "param recover default!" },              //403

	{ "��ENTER��ִ�в�����",       "Press 'ENTER' do" },                    //404
	{ "��CANCEL��ȡ��������",      "Press 'CANCEL' undo   " },              //405
	{ "�����ѳɹ���",              "Operation to complete " },              //406                                           
	{ "�밴CANCEL�����أ�",        "Press 'CANCEL' return " },              //407

	{ "����У׼������ʵ��ֵ",       "AC calibration-input" },                //408
	{ "һ·UV��ѹ(V)",             "AC1 Uuv(V)" },                          //409
	{ "һ·VW��ѹ(V)",             "AC1 Uvw(V)" },                          //410
	{ "һ·WU��ѹ(V)",             "AC1 Uwu(V)" },                          //411
	{ "��·UV��ѹ(V)",             "AC2 Uuv(V)" },                          //412
	{ "��·VW��ѹ(V)",             "AC2 Uvw(V)" },                          //413
	{ "��·WU��ѹ(V)",             "AC2 Uwu(V)" },                          //414

	{ "ֱ����ѹУ׼��",             "DC calibration-input" },                //415
	{ "����ʵ��ֵ",                "actual value" },                        //416
	{ "���ص���У׼��",             "Load Cur calibration-" },               //417
	{ "���ص���1(A)",              "Iload1(A)" },                           //418
	{ "���ص���2(A)",              "Iload2(A)" },                           //419
	{ "һ���ص���У׼��",         "BAT1 Cur calibration-" },               //420
	{ "�����ص���У׼��",         "BAT2 Cur calibration-" },               //421
	{ "��ص���1(A)",              "Ibat1(A)" },                            //422
	{ "��ص���2(A)",              "Ibat2(A)" },                            //423

	{ "��ȴ�...",                 "Please waiting..." },                   //424
	{ "У׼�ɹ���",                "Calibration sucess!" },                 //425
	{ "У׼ʧ�ܣ�",                "Calibration fail!" },                   //426
	{ "��һ��������������ɣ�",    "1nd Cur input finish! " },              //427
	{ "�밴CANCEL�����أ�Ȼ��",    "Press 'CANCEL' return!" },              //428
	{ "������һ����ĵ���ֵ��",    "2nd Cur input finish! " },              //429
	
	{ "��ʾ���Լ���ʾ��ʽ",        "Display Config" },                      //430
	{ "����",                      "Language" },                            //431
	{ "��ʽ",                      "Display" },                             //432
	{ "����",                      "Chinese" },                             //433
	{ "Ӣ��",                      "English" },                             //434
	{ "ˮƽ",                      "Horizontal" },                          //435
	{ "����",                      "Vertical" },                            //436

	{ "��Ʒ",                      "Prod" },                                //437
	{ "Ӳ��",                      "Hard" },                                //438
	{ "���",                      "Soft" },                                //439
	
	{ "������ں���/�ֶ�����",     "Batt checking/Manual" },                //440
	{ "������ڷŵ�/�ֶ�����",     "Batt discharge/Manual" },               //441
	
	{ "��ؿ���2",                 "BAT SWT2" },                            //442
	
	{ "SC12",                      "SC12" },                                //443
	{ "SC22",                      "SC22" },                                //444
	{ "SC32",                      "SC32" },                                //445
	
	{ "ϵͳ�޴���ܣ�",          "Without this function" },               //446
	
	{ ".ά��ָ��",                 ".Maintenance guide" },                  //447
	{ "ά��ָ��:",                 "Maintenance:" },                        //448
	{ ".��ͨ",                     ".System commissioning" },               //449
	{ ".ͨѶ�쳣����",             ".Communication alarm" },                //450
	{ ".��Ƿѹ����",               ".Over/Under Vol alarm" },               //451
	{ ".��Ե����",                 ".Insulation alarm" },                   //452
	{ ".��̨ͨѶ�쳣",             ".Remote Comm alarm" },                  //453
	{ ".��ϵ����",                 ".Contact manufacturer" },               //454
	{ ".�����豸����",             ".Equipment repair" },                   //455
//Check on-line is normal or not
//Check module address is correct
//Check the configuration parameters is correct	
	{ "��ͨ",                      ".System commissioning" },               //456
	{ "��ͨǰ�谴ϵͳ����ͼ��",    "Check on-line is norm-" },              //457
	{ "�����������Ƿ���������",    "al, and module addres-" },              //458
	{ "ģ��ͨѶ��ַ�Ƿ�Ҫ��",    "s is correct, and the " },              //459
	{ "��ȷ���ã�������ò˵�",    "configuration paramet-" },              //460
	{ "�е����ò����Ƿ���ϵͳ",    "ers is correct.       " },              //461
	{ "ʵ��������һ�¡�",          "                      " },              //462
//Check the operation of equipment lamp is normal or not
//Communication line if there is loose or anomaly
//Device address is correct	
	{ "ͨѶ�쳣����",              ".Communication alarm  " },              //463
	{ "ͨѶ�쳣������ȷ�ϱ���",    "Check the operation of" },              //464
	{ "װ�ã�������������豸",    "equipment lamp is nor-" },              //465
	{ "���л��Դָʾ���Ƿ���",    "mal or not, and Commu-" },              //466
	{ "����ͨѶ���Ƿ����ɶ���",    "nication line if ther-" },              //467
	{ "�쳣���������ñ����ͨ",    "e is loose or anomaly," },              //468
	{ "Ѷ���Ƿ�ͨ������豸ͨ",    "and Device address is " },              //469
	{ "Ѷ��ַ�����Ƿ���������",    "correct!              " },              //470
	{ "�ж����ͬ��װ�ã��ɽ�",    "                      " },              //471
	{ "��λ������λ�������ʡ�",    "                      " },              //472
//Check the detailed monitoring alarm content
//To check that whether the measured value by the main monitoring and the actual value measured by multimeter consistent	
//Such as consistent, check alarm threshold value is reasonable
	{ "��Ƿѹ����",                ".Over/Under Vol alarm " },              //473
	{ "ȷ�Ϲ������ݣ��ڵ�ǰ��",    "Check the detailed mo-" },              //474
	{ "���в�����ϸ�ı������ݣ�",  "nitoring alarm content," },             //475
	{ "�ٲ鿴�������ʾ�Ĳ���",    "whether the measured  " },              //476
	{ "ֵ���������ñ�����ɼ�",    "value by monitoring a-" },              //477
	{ "ģ����Ӧ���ӵĲ����ź�",    "nd the actual value m-" },              //478
	{ "ֵ���Ƿ�����ʾֵ��һ�£�",  "easured by multimeter " },              //479
	{ "��һ�£��鿴��Ӧ�ı���",    "consistent, threshold " },              //480
	{ "��Χ�����Ƿ���ȷ��",        "value is reasonable   " },              //481
//Check the main monitoring shows that positive and negative bus voltage to earth and resistance
//Monitoring shows that the bus voltage and multimeter measured bus voltage to ground is consistent	
//Measuring sensor output signal to calculate resistance to earth
	{ "��Ե����",                  ".INSU drop alarm" },                    //482
	{ "�鿴�������ʾ������ĸ",    "Check the main monito-" },              //483
	{ "�߶Եص�ѹ�����裬����",    "ring shows that posit-" },              //484
	{ "�ñ�ʾʵ��ϵͳĸ�߶Ե�",    "ive and negative bus  " },              //485
	{ "��ѹ�Ƿ�����ʾֵ��һ�£�",  "voltage to earth and  " },              //486
	{ "��һ�£���˵������������",  "resistance. Measuring " },              //487
	{ "�ٲ鿴�Ƿ���֧·������",    "sensor output signal  " },              //488
	{ "����֧·�������������",    "to calculate resistan-" },              //489
	{ "������֧·�Ĵ��������",    "ce to earth.          " },              //490
	{ "�źţ����ͨ���Ͽ��ص�",    "                      " },              //491
	{ "��ʽȷ�ϱ����Ƿ���ȷ��",    "                      " },              //492
//Check the communication protocol and baud rate set  by monitoring device is correct	
//Use the test software testing remote communication port communication is normal or not
	{ "��̨ͨѶ�쳣",              ".Remote Comm alarm" },                  //493
	{ "�鿴�����װ�������õ�",    "Check the communicati-" },              //494
	{ "ͨѶЭ�鼰�������Ƿ���",    "on protocol and baud  " },              //495
	{ "����ʹ�õ���һ�£����",    "rate set  by monitori-" },              //496
	{ "����ͨѶ�ڽ����Ƿ���ȷ��",  "ng device is correct. " },              //497
	{ "���ò���������Ժ�̨ͨ",    "Use the test software " },              //498
	{ "Ѷ��ͨѶ�Ƿ�������",        "testing remote port.  " },              //499
//The fault that can't handle by own , by telephone manufacturers remote guidance	
//Please provide the factory equipment model and equipment Numbers, so that manufacturers 
//find relevant order information, quickly and accurately remove the fault
	{ "��ϵ����",                  ".Contact manufacturer" },               //500	
	{ "���ֳ��������д���Ĺ�",    "The fault that can't h" },              //501
	{ "�ϣ��ɵ绰��ϵ���ҽ���",    "andle by own, Please p" },              //502
	{ "Զ��ָ�����뱨��������",    "rovide the equipment m" },              //503
	{ "���ͺż��豸��ţ��Է�",    "odel and equipment No." },              //504
	{ "�㳧�Ҳ�����صĶ�����",    "so that manufacturers " },              //505
	{ "Ϣ��������ֳ��볧����",    "quickly and accurately" },              //506
	{ "ϵ���Է������׼ȷ����",    "remove the fault.     " },              //507
	{ "�����ϡ�",                  "                      " },              //508
//If confirmed equipment has been damaged,
//Please return to list describing field failure phenomenon, 
//with convenient maintenance accurate positioning fault point and related information collection	
	{ "�����豸����",              ".Equipment repair" },                   //509
	{ "��ȷ���豸���𻵣�����",    "If confirmed equipment" },              //510
	{ "���ص��������ֳ�������",    "has been damaged, Ple-" },              //511
	{ "���Է���ά��׼ȷ��λ",    "ase return to list de-" },              //512
	{ "���ϵ㼰�����Ϣ���ռ���",  "scribing field failure" },              //513  
	                                                                        
	{ "������ֱ����ѹ",            "AC join to DC" },                       //514
	{ "������ֱ������(V)",         "AC2DC AP(V)" },                         //515

	{ "ĸ�߸��Եص�ѹ(V)",         "P- Vol to GND(V)" },                    //516

	{ ".ά��������",               ".Defend Level Config" },                //517
	
	{ "�ֶ�������������",          "Manu Lmt Cur Cfg" },                    //518
	{ "�������ڷ�ʽ",              "Lmt Cur Mode" },                        //519
	{ "�ֶ�������(%)",             "Manu Lmt Cur(%)" },                     //520
	
	{ "ͨѶ������������",          "Comm Offline Config" },                 //521
	{ "���߼��ģ��",              "FDL Module" },                          //522
	{ "���Ѳ��",                  "BAT Module" },                          //523
	{ "����ģ��",                  "Rect Module" },                         //524
	{ "ͨ��ģ��",                  "CPS Module" },                          //525
	{ "���ģ��",                  "UPS Module" },                          //526
	{ "ATSģ��",                   "ATS Module" },                          //527
	{ "�๦�ܵ��",                "ACM Module" },                          //528
	
	{ "����",                      "Tool" },                                //529

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

	{ "��Ե������������",          "INSU Para Config" },                    //549
	{ "CAN����",                   "CAN Baud" },                            //550	
	{ "����ģʽ",                  "Meas Way" },                            //551
	{ "����Ͷ����ʱ(S)",           "ChangeTime(S)" },                       //552
	{ "����������(mA)",            "SensorRate(mA)" },                      //553
	{ "��ѹ��ƽ��(S)",             "ConfirmTime(S)" },                      //554
	{ "��Ե��������(H)",           "MeasPeriod(H)" },                       //555
	{ "���ڲ���-ʱ(H)",            "MeasHour(H)" },                         //556
	{ "���ڲ���-��(M)",            "MeasMinute(M)" },                       //557
	{ "125K",                      "125K" },                                //558
	{ "50K",                       "50K" },                                 //559
	{ "20K",                       "20K" },                                 //560
	{ "10K",                       "10K" },                                 //561
	{ "����",                      "Normal" },                              //562
	{ "����",                      "Debug " },                              //563

	{ "ĸ��������Դ����",          "Bus data source" },                     //564
	{ "D21ģ��(��)",               "D21 Num" },                             //565

	{ "���������������",          "Zero curr correct" },                   //566
	{ "",                          "" },                           //567	
	{ "��������(A)",               "Load curr(A)" },                        //568	
	{ "",                          "" },                           //569
	{ "���1����(A)",              "Bat1 curr(A)" },                        //570
	{ "",                          "" },                           //571	
	{ "���2����(A)",              "Bat2 curr(A)" },                        //572
	{ "+",                         "+" },                                   //573
	{ "-",                         "-" },                                   //574

	{ "�����ص�ѹ(V)",           "Batt2 Vol(V)" },                         //575
	{ "�����ص���(A)",           "Batt2 Cur(A)" },                         //576
	{ "����������(Ah)",          "Batt2 Capa(Ah)" },                       //577
	{ "���θ��ص���(A)",           "Load2 Cur(A)" },                         //578
	{ "���κ�ĸ��ѹ(V)",           "PB2  Vol(V)" },                          //579
	{ "����ĸ�ߵ�ѹ(V)",           "Bus2 Vol(V)" },                          //580
	{ "���ο�ĸ��ѹ(V)",           "CB2  Vol(V)" },                          //581

	{ "ֱ����DC10ģ������",        "DCS DC10 Cur Config" },                 //582
	{ "DC10ģ��(��)",              "DC10 Num" },                            //583
	{ "A1�ɼ���ѹ(mV)",            "A1 Cur Sample(mV)" },                   //584
	{ "A2�ɼ���ѹ(mV)",            "A2 Cur Sample(mV)" },                   //585
	{ "A3�ɼ���ѹ(mV)",            "A3 Cur Sample(mV)" },                   //586
	{ "A/S1��������(A)",           "A/S1 Cur Range(A)" },                   //587
	{ "A/S2��������(A)",           "A/S2 Cur Range(A)" },                   //588
	{ "A/S3��������(A)",           "A/S3 Cur Range(A)" },                   //589

	{ "������",                  "BATT2" },                               //590

	{ ".����ϵͳ����",             ".AC System Config    " },               //591

	{ "������DC10ģ������",        "ACS DC10 Cur Config" },                 //592
	{ "DC10ģ��(��)",              "DC10 Num" },                            //593
	{ "������ѹ��(V)",             "ACS AC OVP(V)" },                       //594
	{ "����Ƿѹ��(V)",             "ACS AC UVP(V)" },                       //595
	{ "����ȱ���(V)",             "ACS AC PLP(V)" },                       //596
	{ "A/S1��������(A)",           "A/S1 Cur Range(A)" },                   //597
	{ "A/S2��������(A)",           "A/S2 Cur Range(A)" },                   //598
	{ "A/S3��������(A)",           "A/S3 Cur Range(A)" },                   //599

	{ "����������������",          "ATS SWT Config" },                      //600
	{ "�������߹�",                "ACS FDL Panel" },                       //601
	{ "#����ģ��",                 "#FDL Module" },                         //602

	{ ".����ϵͳ��Ϣ��ѯ",         ".AC System Info" },                     //603

	{ "����ϵͳ��Ϣ��ѯ:",         "ACS Panel Info: " },                    //604
	{ ".����ĸ����Ϣ��ѯ",         ".AC Bus Info    " },                    //605
	{ ".��������״̬��ѯ",         ".ACS SWT State Info" },                 //606

	{ "�����������״̬",          "ACS Panel SWT State" },                 //607

	{ "DC10ģ��",                  "DC10 Module" },                         //608
	{ "RC10ģ��",                  "RC10 Module" },                         //609

	{ "RC10ģ��(��)",              "RC10 Module" },                         //610

	{ ".��ٿ��ؿ���",             ".SWT Control" },                        //611

	{ "��ٿ��ؿ���",              "Switch On/Off" },                       //612

	{ ".��ٿ���״̬��ѯ",         ".EC SWT State Query " },                //613
	{ "��ٿ���״̬",              "EC SWT State" },                        //614
	
	{ "ֱ��ĸ�߶���",              "DC Bus Num" },                          //615	
	{ "һ��",                      "One Seg" },                             //616
	{ "����",                      "Two Seg" },                             //617

	{ "��Ե������ַ���� ",         "Insu Master Add" },                     //618
	{ "һ�ξ�Ե������ ",           "1#Insu Master:" },                      //619
	{ "���ξ�Ե������ ",           "2#Insu Master:" },                      //620

	{ "����(A)",                   "AC Curr(A)" },                          //621
	{ "������ѹ��ʱ(S)",           "AC OV Delay(S)" },                   //622
};


/************************** �¼�������ʾ�ַ������� ********************************/
const STR_T g_s_batt_record[][2] =            //��س�ŵ�ת����¼����
{
	{ "1#��������Ϊ�Զ�",          "1# Auto charging   " },
	{ "1#��������Ϊ�ֶ�",          "1# Manu charging   " },
	{ "1#����Զ�ת����",            "1# Auto floating   " },
	{ "1#����Զ�ת����",            "1# Auto equalizing " },
	{ "1#����Զ�ת����",            "1# Auto discharging" },
	{ "1#����ֶ�ת����",            "1# Manu floating   " },
	{ "1#����ֶ�ת����",            "1# Manu equalizing " },
	{ "1#����ֶ�ת����",            "1# Manu discharging" },
	
	{ "2#��������Ϊ�Զ�",          "2# Auto charging   " },
	{ "2#��������Ϊ�ֶ�",          "2# Manu charging   " },
	{ "2#����Զ�ת����",            "2# Auto floating   " },
	{ "2#����Զ�ת����",            "2# Auto equalizing " },
	{ "2#����Զ�ת����",            "2# Auto discharging" },
	{ "2#����ֶ�ת����",            "2# Manu floating   " },
	{ "2#����ֶ�ת����",            "2# Manu equalizing " },
	{ "2#����ֶ�ת����",            "2# Manu discharging" },
};

/************************** ��ٿ���������ʾ�ַ������� ********************************/
const STR_T g_s_ctrl_swt_name[FACT_SWT_CTRL_MAX][2] =            //��ٿ�������
{	//1#RC10��ٿ���
	{ "4S1����",                   "4S1 Switch   "    },
	{ "4S2����",                   "4S2 Switch   "    },
	{ "4S3����",                   "4S3 Switch   "    },	
	{ "4S4����",                   "4S4 Switch   "    },
	{ "5S1����",                   "5S1 Switch   "    },
	{ "5S2����",                   "5S2 Switch   "    },
	{ "5S3����",                   "5S3 Switch   "    },
	{ "5S4����",                   "5S4 Switch   "   },

	//2#RC10��ٿ���
	{ "5S5����",                   "5S5 Switch   "    },
	{ "5S6����",                   "5S6 Switch   "    },
	{ "S1����",                    "S1 Switch   "     },
	{ "S2����",                    "S2 Switch   "     },
	{ "S3����",                    "S3 Switch   "     },
	{ "S4����",                    "S4 Switch   "     },
	{ "S5����",                    "S5 Switch   "     },
	{ "8S1����",                   "8S1 Switch   "    },

	//3#RC10��ٿ���
	{ "8S2����",                   "3S9 Switch   "    },
	{ "8S3����",                  "3S10 Switch   "    },
	{ "8S4����",                  "3S11 Switch   "    },
	{ "8S5����",                  "3S12 Switch   "    },
	{ "δ����1",                  "Reserved 1	"	},
	{ "δ����2",                  "Reserved 2	"	},
	{ "δ����3",                  "Reserved 3	"	},
	{ "δ����4",                  "Reserved 4	"	},


//	//4#RC10��ٿ���	
//	{ "4S1����",                   "4S1 Switch   "    },
//	{ "4S2����",                   "4S2 Switch   "    },
//	{ "4S3����",                   "4S3 Switch   "    },
//	{ "4S4����",                   "4S4 Switch   "    },
//	{ "4S5����",                   "4S5 Switch   "    },
//	{ "4S6����",                   "4S6 Switch   "    },
//	{ "4S7����",                   "4S7 Switch   "    },
//	{ "4S8����",                   "4S8 Switch   "    },
//	
//	//5#RC10��ٿ���	
//	{ "4S9����",                   "4S9 Switch   "    },
//	{ "4S10����",                  "4S10 Switch   "    },
//	{ "4S11����",                  "4S11 Switch   "    },
//	{ "4S12����",                  "4S12 Switch   "    },
//	{ "4S13����",                  "4S13 Switch   "    },
//	{ "4S14����",                  "4S14 Switch   "    },
//	{ "4S15����",                  "4S15 Switch   "    },
//	{ "4S16����",                  "4S16 Switch   "    },
//	
//	//6#RC10��ٿ���	
//	{ "3S17����",                  "3S17 Switch   "    },
//	{ "4S17����",                  "4S17 Switch   "    },
//	{ "MLS����",                   "MLS Switch   "    },
//	{ "δ����1",                   "Reserved 1   "   },
//	{ "δ����2",                   "Reserved 2   "   },
//	{ "δ����3",                   "Reserved 3   "   },
//	{ "δ����4",                   "Reserved 4   "   },
//	{ "δ����5",                   "Reserved 5   "   },

//	//7#RC10��ٿ���
//	{ "1HK����",                   "1HK Switch   "   },
//	{ "2HK����",                   "2HK Switch   "   },
//	{ "3HK����",                   "3HK Switch   "   },
//	{ "δ����1",                   "Reserved 1   "    },
//	{ "δ����2",                   "Reserved 2   "    },
//	{ "δ����3",                   "Reserved 3   "    },
//	{ "δ����4",                   "Reserved 4   "    },
//	{ "δ����5",                   "Reserved 5   "    },
//	
//	//8#RC10��ٿ���
//	{ "AC 3S1����",                "AC 3S1 Switch   "    },
//	{ "AC 3S2����",                "AC 3S2 Switch   "    },
//	{ "AC 3S3����",                "AC 3S3 Switch   "    },
//	{ "AC 3S4����",                "AC 3S4 Switch   "    },
//	{ "AC 3S5����",                "AC 3S5 Switch   "    },
//	{ "AC 3S6����",                "AC 3S6 Switch   "    },
//	{ "AC 3S7����",                "AC 3S7 Switch   "    },
//	{ "AC 3S8����",                "AC 3S8 Switch   "    },

//	//9#RC10��ٿ���
//	{ "AC 3S9����",                "AC 3S9 Switch   "    },
//	{ "AC 3S10����",               "AC 3S10 Switch   "    },
//	{ "AC 3S11����",               "AC 3S11 Switch   "    },
//	{ "AC 3S12����",               "AC 3S12 Switch   "    },
//	{ "AC 3S13����",               "AC 3S13 Switch   "    },
//	{ "AC 3S14����",               "AC 3S14 Switch   "    },
//	{ "AC 3S15����",               "AC 3S15 Switch   "    },
//	{ "AC 3S16����",               "AC 3S16 Switch   "    },

//	//10#RC10��ٿ���
//	{ "AC 4S1����",                "AC 4S1 Switch   "    },
//	{ "AC 4S2����",                "AC 4S2 Switch   "    },
//	{ "AC 4S3����",                "AC 4S3 Switch   "    },
//	{ "AC 4S4����",                "AC 4S4 Switch   "    },
//	{ "AC 4S5����",                "AC 4S5 Switch   "    },
//	{ "AC 4S6����",                "AC 4S6 Switch   "    },
//	{ "AC 4S7����",                "AC 4S7 Switch   "    },
//	{ "AC 4S8����",                "AC 4S8 Switch   "    },

//	//11#RC10��ٿ���
//	{ "AC 4S9����",                "AC 4S9 Switch   "    },
//	{ "AC 4S10����",               "AC 4S10 Switch   "    },
//	{ "AC 4S11����",               "AC 4S11 Switch   "    },
//	{ "AC 4S12����",               "AC 4S12 Switch   "    },
//	{ "AC 4S13����",               "AC 4S13 Switch   "    },
//	{ "AC 4S14����",               "AC 4S14 Switch   "    },
//	{ "AC 4S15����",               "AC 4S15 Switch   "    },
//	{ "δ����1",                   "Reserved 1   "    },
};
