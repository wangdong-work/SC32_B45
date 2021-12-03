/************************************************************
Copyright (C), 2012-2020, ����Ӣ����Ƽ��������޹�˾
�� �� ����Type.h
��    ����1.00
�������ڣ�2012-04-27
��    �ߣ�������
��������������׼�����������¶���

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-04-27  1.00     ����
**************************************************************/

#ifndef __TYPE_H_
#define __TYPE_H_


typedef   unsigned char     BOOL_T; 
typedef   unsigned char     U8_T; 
typedef   signed   char     S8_T; 
typedef   unsigned short    U16_T; 
typedef   signed   short    S16_T; 
typedef   unsigned int      U32_T; 
typedef   signed   int      S32_T; 
typedef   float             F32_T; 
typedef   double            F64_T; 
typedef   char *            STR_T;
 
#define BYTE           S8_T
#define UBYTE          U8_T
#define WORD           S16_T
#define UWORD          U16_T
#define LONG           S32_T
#define ULONG          U32_T
#define LLONG          S64_T
#define ULLONG         U64_T

#endif
