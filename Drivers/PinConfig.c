/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：PinConfig.c
版    本：1.00
创建日期：2012-03-19
作    者：郭数理
功能描述：引脚配置初始化实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-19  1.00     创建
**************************************************************/

#include <lpc17xx.h>
#include "PinConfig.h"


//---<<< Use Configuration Wizard in Context Menu >>>---


//***************** Pin Function Select register ********************

//	<h> Pin Function Select register 0 (PINSEL0)
//		<o0.0..1> P0.0: P0.0 Function Select
//					<0=> GPIO Port 0.0
//					<1=> RD1
//					<2=> TXD3
//				    <3=> SDA1
//		<o0.2..3> P0.1: P0.1 Function Select
//					<0=> GPIO Port 0.1
//					<1=> TD1
//					<2=> RXD3 
//				    <3=> SCL1
//		<o0.4..5> P0.2: P0.2 Function Select
//					<0=> GPIO Port 0.2 
//					<1=> TXD0 
//					<2=> AD0.7 
//		<o0.6..7> P0.3: P0.3 Function Select
//					<0=> GPIO Port 0.3 
//					<1=> RXD0 
//					<2=> AD0.6 
//		<o0.8..9> P0.4: P0.4 Function Select
//					<0=> GPIO Port 0.4 
//					<1=> I2SRX_CLK 
//					<2=> RD2 
//				    <3=> CAP2.0
//		<o0.10..11> P0.5: P0.5 Function Select
//					<0=> GPIO Port 0.5 
//					<1=> I2SRX_WS 
//					<2=> TD2 
//				    <3=> CAP2.1
//		<o0.12..13> P0.6: P0.6 Function Select
//					<0=> GPIO Port 0.6 
//					<1=> I2SRX_SDA 
//					<2=> SSEL1 
//				    <3=> MAT2.0
//		<o0.14..15> P0.7: P0.7 Function Select
//					<0=> GPIO Port 0.7 
//					<1=> I2STX_CLK 
//					<2=> SCK1 
//				    <3=> MAT2.1
//		<o0.16..17> P0.8: P0.8 Function Select
//					<0=> GPIO Port 0.8 
//					<1=> I2STX_WS 
//					<2=> MISO1 
//				    <3=> MAT2.2
//		<o0.18..19> P0.9: P0.9 Function Select
//					<0=> GPIO Port 0.9 
//					<1=> I2STX_SDA 
//					<2=> MOSI1 
//				    <3=> MAT2.3
//		<o0.20..21> P0.10: P0.10 Function Select
//					<0=> GPIO Port 0.10 
//					<1=> TXD2 
//					<2=> SDA2 
//				    <3=> MAT3.0
//		<o0.22..23> P0.11: P0.11 Function Select
//					<0=> GPIO Port 0.11 
//					<1=> RXD2 
//					<2=> SCL2 
//				    <3=> MAT3.1
//		<o0.30..31> P0.15: P0.15 Function Select
//					<0=> GPIO Port 0.15 
//					<1=> TXD1 
//					<2=> SCK0 
//				    <3=> SCK
//	</h>


//	<h> Pin Function Select register 1 (PINSEL1)
//		<o1.0..1> P0.16: P0.16 Function Select
//					<0=> GPIO Port 0.16  
//					<1=> RXD1
//					<2=> SSEL0 
//				    <3=> SSEL
//		<o1.2..3> P0.17: P0.17 Function Select
//					<0=> GPIO Port 0.17 
//					<1=> CTS1 
//					<2=> MISO0  
//				    <3=> MISO
//		<o1.4..5> P0.18: P0.18 Function Select
//					<0=> GPIO Port 0.18 
//					<1=> DCD1 
//					<2=> MOSI0  
//				    <3=> MOSI
//		<o1.6..7> P0.19: P0.19 Function Select
//					<0=> GPIO Port 0.19  
//					<1=> DSR1   
//				    <3=> SDA1
//		<o1.8..9> P0.20: P0.20 Function Select
//					<0=> GPIO Port 0.20  
//					<1=> DTR1    
//				    <3=> SCL1
//		<o1.10..11> P0.21: P0.21 Function Select
//					<0=> GPIO Port 0.21  
//					<1=> RI1    
//				    <3=> RD1
//		<o1.12..13> P0.22: P0.22 Function Select
//					<0=> GPIO Port 0.22 
//					<1=> RTS1   
//				    <3=> TD1
//		<o1.14..15> P0.23: P0.23 Function Select
//					<0=> GPIO Port 0.23  
//					<1=> AD0.0  
//					<2=> I2SRX_CLK  
//				    <3=> CAP3.0
//		<o1.16..17> P0.24: P0.24 Function Select
//					<0=> GPIO Port 0.24   
//					<1=> AD0.1  
//					<2=> I2SRX_WS  
//				    <3=> CAP3.1
//		<o1.18..19> P0.25: P0.25 Function Select
//					<0=> GPIO Port 0.25  
//					<1=> AD0.2  
//					<2=> I2SRX_SDA  
//				    <3=> TXD3
//		<o1.20..21> P0.26: P0.26 Function Select
//					<0=> GPIO Port 0.26  
//					<1=> AD0.3  
//					<2=> AOUT  
//				    <3=> RXD3
//		<o1.22..23> P0.27: P0.27 Function Select
//					<0=> GPIO Port 0.27  
//					<1=> SDA0  
//					<2=> USB_SDA  
//		<o1.24..25> P0.28: P0.28 Function Select
//					<0=> GPIO Port 0.28  
//					<1=> SCL0  
//					<2=> USB_SCL
//		<o1.26..27> P0.29: P0.29 Function Select
//					<0=> GPIO Port 0.29 
//					<1=> USB_D+  
//		<o1.28..29> P0.30: P0.30 Function Select
//					<0=> GPIO Port 0.30  
//					<1=> USB_D-  
//	</h>


//	<h> Pin Function Select register 2 (PINSEL2)
//		<o2.0..1> P1.0: P1.0 Function Select
//					<0=> GPIO Port 1.0 
//					<1=> ENET_TXD0
//		<o2.2..3> P1.1: P1.1 Function Select
//					<0=> GPIO Port 1.1  
//					<1=> ENET_TXD1 
//		<o2.8..9> P1.4: P1.4 Function Select
//					<0=> GPIO Port 1.4   
//					<1=> ENET_TX_EN    
//		<o2.16..17> P1.8: P1.8 Function Select
//					<0=> GPIO Port 1.8    
//					<1=> ENET_CRS  
//		<o2.18..19> P1.9: P1.9 Function Select
//					<0=> GPIO Port 1.9   
//					<1=> ENET_RXD0  
//		<o2.20..21> P1.10: P1.10 Function Select
//					<0=> GPIO Port 1.10   
//					<1=> ENET_RXD1
//		<o2.28..29> P1.14: P1.14 Function Select
//					<0=> GPIO Port 1.14   
//					<1=> ENET_RX_ER
//		<o2.30..31> P1.15: P1.15 Function Select
//					<0=> GPIO Port 1.15    
//					<1=> ENET_REF_CLK   
//	</h>

//	<h> Pin Function Select register 3 (PINSEL3)
//		<o3.0..1> P1.16: P1.16 Function Select
//					<0=> GPIO Port 1.16   
//					<1=> ENET_MDC
//		<o3.2..3> P1.17: P1.17 Function Select
//					<0=> GPIO Port 1.17  
//					<1=> ENET_MDIO
//		<o3.4..5> P1.18: P1.18 Function Select
//					<0=> GPIO Port 1.18  
//					<1=> USB_UP_LED  
//					<2=> PWM1.1   
//				    <3=> CAP1.0
//		<o3.6..7> P1.19: P1.19 Function Select
//					<0=> GPIO Port 1.19   
//					<1=> MC0A 
//					<2=> USB_PPWR    
//				    <3=> CAP1.1
//		<o3.8..9> P1.20: P1.20 Function Select
//					<0=> GPIO Port 1.20   
//					<1=> MCFB0 
//					<2=> PWM1.2     
//				    <3=> SCK0
//		<o3.10..11> P1.21: P1.21 Function Select
//					<0=> GPIO Port 1.21   
//					<1=> MCABORT 
//					<2=> PWM1.3     
//				    <3=> SSEL0
//		<o3.12..13> P1.22: P1.22 Function Select
//					<0=> GPIO Port 1.22  
//					<1=> MC0B 
//					<2=> USB_PWRD    
//				    <3=> MAT1.0
//		<o3.14..15> P1.23: P1.23 Function Select
//					<0=> GPIO Port 1.23   
//					<1=> MCFB1  
//					<2=> PWM1.4   
//				    <3=> MISO0
//		<o3.16..17> P1.24: P1.24 Function Select
//					<0=> GPIO Port 1.24    
//					<1=> MCFB2   
//					<2=> PWM1.5   
//				    <3=> MOSI0
//		<o3.18..19> P1.25: P1.25 Function Select
//					<0=> GPIO Port 1.25   
//					<1=> MC1A   
//					<2=> CLKOUT   
//				    <3=> MAT1.1
//		<o3.20..21> P1.26: P1.26 Function Select
//					<0=> GPIO Port 1.26   
//					<1=> MC1B   
//					<2=> PWM1.6   
//				    <3=> CAP0.0
//		<o3.22..23> P1.27: P1.27 Function Select
//					<0=> GPIO Port 1.27   
//					<1=> CLKOUT
//					<2=> USB_OVRCR   
//					<3=> CAP0.1  
//		<o3.24..25> P1.28: P1.28 Function Select
//					<0=> GPIO Port 1.28   
//					<1=> MC2A   
//					<2=> PCAP1.0 
//					<3=> MAT0.0
//		<o3.26..27> P1.29: P1.29 Function Select
//					<0=> GPIO Port 1.29  
//					<1=> MC2B 
//					<2=> PCAP1.1 
//					<3=> MAT0.1  
//		<o3.28..29> P1.30: P1.30 Function Select
//					<0=> GPIO Port 1.30   
//					<2=> VBUS 
//					<3=> AD0.4
//		<o3.30..31> P1.31: P1.31 Function Select
//					<0=> GPIO Port 1.31   
//					<2=> SCK1  
//					<3=> AD0.5  
//	</h>


//	<h> Pin Function Select register 4 (PINSEL4)
//		<o4.0..1> P2.0: P2.0 Function Select
//					<0=> GPIO Port 2.0 
//					<1=> PWM1.1 
//					<2=> TXD1
//		<o4.2..3> P2.1: P2.1 Function Select
//					<0=> GPIO Port 2.1 
//					<1=> PWM1.2 
//					<2=> RXD1
//		<o4.4..5> P2.2: P2.2 Function Select
//					<0=> GPIO Port 2.2  
//					<1=> PWM1.3  
//					<2=> CTS1 
//		<o4.6..7> P2.3: P2.3 Function Select
//					<0=> GPIO Port 2.3  
//					<1=> PWM1.4
//					<2=> DCD1 
//		<o4.8..9> P2.4: P2.4 Function Select
//					<0=> GPIO Port 2.4  
//					<1=> PWM1.5  
//					<2=> DSR1
//		<o4.10..11> P2.5: P2.5 Function Select
//					<0=> GPIO Port 2.5
//					<1=> PWM1.6
//					<2=> DTR1
//		<o4.12..13> P2.6: P2.6 Function Select
//					<0=> GPIO Port 2.6
//					<1=> PCAP1.0
//					<2=> RI1
//		<o4.14..15> P2.7: P2.7 Function Select
//					<0=> GPIO Port 2.7
//					<1=> RD2
//					<2=> RTS1
//		<o4.16..17> P2.8: P2.8 Function Select
//					<0=> GPIO Port 2.8 
//					<1=> TD2 
//					<2=> TXD2 
//				    <3=> ENET_MDC
//		<o4.18..19> P2.9: P2.9 Function Select
//					<0=> GPIO Port 2.9 
//					<1=> USB_CONNECT 
//					<2=> RXD2 
//				    <3=> ENET_MDIO
//		<o4.20..21> P2.10: P2.10 Function Select
//					<0=> GPIO Port 2.10 
//					<1=> EINT0 
//					<2=> NMI 
//		<o4.22..23> P2.11: P2.11 Function Select
//					<0=> GPIO Port 2.11 
//					<1=> EINT1  
//				    <3=> I2STX_CLK
//		<o4.24..25> P2.12: P2.12 Function Select
//					<0=> GPIO Port 2.12 
//					<1=> EINT2  
//				    <3=> I2STX_WS
//		<o4.26..27> P2.13: P2.13 Function Select
//					<0=> GPIO Port 2.13 
//					<1=> EINT3  
//				    <3=> I2STX_SDA
//	</h>


//	<h> Pin Function Select register 7 (PINSEL7)
//		<o5.18..19> P3.25: P3.25 Function Select
//					<0=> GPIO Port 3.25    
//					<2=> MAT0.0   
//				    <3=> PWM1.2
//		<o5.20..21> P3.26: P3.26 Function Select
//					<0=> GPIO Port 3.26   
//					<1=> STCLK   
//					<2=> MAT0.1   
//				    <3=> PWM1.3 
//	</h>


//	<h> Pin Function Select register 9 (PINSEL9)
//		<o6.24..25> P4.28: P4.28 Function Select
//					<0=> GPIO Port 4.28
//					<1=> RX_MCLK    
//					<2=> MAT2.0   
//				    <3=> TXD3
//		<o6.26..27> P4.29: P4.29 Function Select
//					<0=> GPIO Port 4.29   
//					<1=> TX_MCLK
//					<2=> MAT2.1   
//				    <3=> RXD3 
//	</h>


//	<h> Pin Function Select register 10 (PINSEL10)
//		<o7.3> GPIO/TRACE: TPIU interface pins control
//					<0=> TPIU interface is disabled
//					<1=> TPIU interface is enabled. TPIU signals are available on the pins hosting them regardless of the PINSEL4 content
//	</h>

//********** Pin Function Select register end **********************



//********** Pin Mode select register ******************************

//	<h> Pin Mode Select register 0 (PINMODE0)
//		<o8.0..1> P0.00MODE: Port 0 pin 0 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.2..3> P0.01MODE: Port 0 pin 1 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.4..5> P0.02MODE: Port 0 pin 2 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.6..7> P0.03MODE: Port 0 pin 3 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//		<o8.8..9> P0.04MODE: Port 0 pin 4 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.10..11> P0.05MODE: Port 0 pin 5 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.12..13> P0.06MODE: Port 0 pin 6 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.14..15> P0.07MODE: Port 0 pin 7 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.16..17> P0.08MODE: Port 0 pin 8 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.18..19> P0.09MODE: Port 0 pin 9 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.20..21> P0.10MODE: Port 0 pin 10 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.22..23> P0.11MODE: Port 0 pin 11 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o8.30..31> P0.15MODE: Port 0 pin 15 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//	</h>


//	<h> Pin Mode Select register 1 (PINMODE1)
//		<o9.0..1> P0.16MODE: Port 0 pin 16 on-chip pull-up/down resistor control
//					<0=> apull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.2..3> P0.17MODE: Port 0 pin 17 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.4..5> P0.18MODE: Port 0 pin 18 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.6..7> P0.19MODE: Port 0 pin 19 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.8..9> P0.20MODE: Port 0 pin 20 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.10..11> P0.21MODE: Port 0 pin 21 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.12..13> P0.22MODE: Port 0 pin 22 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.14..15> P0.23MODE: Port 0 pin 23 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.16..17> P0.24MODE: Port 0 pin 24 on-chip pull-up/down resistor control
//					<0=> a pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.18..19> P0.25MODE: Port 0 pin 25 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o9.20..21> P0.26MODE: Port 0 pin 26 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//	</h>


//	<h> Pin Mode Select register 2 (PINMODE2)
//		<o10.0..1> P1.00MODE: Port 1 pin 0 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//		<o10.2..3> P1.01MODE: Port 1 pin 1 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//		<o10.8..9> P1.04MODE: Port 1 pin 4 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled   
//		<o10.16..17> P1.08MODE: Port 1 pin 8 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o10.18..19> P1.09MODE: Port 1 pin 9 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled  
//		<o10.20..21> P1.10MODE: Port 1 pin 10 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o10.28..29> P1.14MODE: Port 1 pin 14 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o10.30..31> P1.15MODE: Port 1 pin 15 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled   
//	</h>

//	<h> Pin Mode Select register 3 (PINMODE3)
//		<o11.0..1> P1.16MODE: Port 1 pin 16 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.2..3> P1.17MODE: Port 1 pin 17 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.4..5> P1.18MODE: Port 1 pin 18 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.6..7> P1.19MODE: Port 1 pin 19 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.8..9> P1.20MODE: Port 1 pin 20 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.10..11> P1.21MODE: Port 1 pin 21 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.12..13> P1.22MODE: Port 1 pin 22 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.14..15> P1.23MODE: Port 1 pin 23 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.16..17> P1.24MODE: Port 1 pin 24 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.18..19> P1.25MODE: Port 1 pin 25 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.20..21> P1.26MODE: Port 1 pin 26 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.22..23> P1.27MODE: Port 1 pin 27 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled  
//		<o11.24..25> P1.28MODE: Port 1 pin 28 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.26..27> P1.29MODE: Port 1 pin 29 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled  
//		<o11.28..29> P1.30MODE: Port 1 pin 30 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o11.30..31> P1.31MODE: Port 1 pin 31 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled  
//	</h>


//	<h> Pin Mode Select register 4 (PINMODE4)
//		<o12.0..1> P2.00MODE: Port 2 pin 0 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//		<o12.2..3> P2.01MODE: Port 2 pin 1 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.4..5> P2.02MODE: Port 2 pin 2 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.6..7> P2.03MODE: Port 2 pin 3 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//		<o12.8..9> P2.04MODE: Port 2 pin 4 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.10..11> P2.05MODE: Port 2 pin 5 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.12..13> P2.06MODE: Port 2 pin 6 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.14..15> P2.07MODE: Port 2 pin 7 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.16..17> P2.08MODE: Port 2 pin 8 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.18..19> P2.09MODE: Port 2 pin 9 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.20..21> P2.10MODE: Port 2 pin 10 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//		<o12.22..23> P2.11MODE: Port 2 pin 11 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.24..25> P2.12MODE: Port 2 pin 12 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o12.26..27> P2.13MODE: Port 2 pin 13 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//	</h>


//	<h> Pin Mode Select register 7 (PINMODE7)
//		<o13.18..19> P3.25MODE: Port 3 pin 25 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o13.20..21> P3.26MODE: Port 3 pin 26 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//	</h>


//	<h> Pin Mode Select register 9 (PINMODE9)
//		<o14.24..25> P4.28MODE: Port 4 pin 28 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled
//		<o14.26..27> P4.29MODE: Port 4 pin 29 on-chip pull-up/down resistor control
//					<0=> pull-up resistor enabled
//					<1=> repeater mode enabled
//					<2=> neither pull-up nor pull-down
//					<3=> pull-down resistor enabled 
//	</h>

//********** Pin Mode select register end ***************************



//********** Open Drain Pin Mode select register ********************

//	<h> Open Drain Pin Mode select register 0 (PINMODE_OD0)
//		<o15.0> P0.00OD: Port 0 pin 0 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.1> P0.01OD: Port 0 pin 1 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.2> P0.02OD: Port 0 pin 2 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.3> P0.03OD: Port 0 pin 3 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.4> P0.04OD: Port 0 pin 4 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.5> P0.05OD: Port 0 pin 5 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.6> P0.06OD: Port 0 pin 6 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.7> P0.07OD: Port 0 pin 7 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.8> P0.08OD: Port 0 pin 8 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.9> P0.09OD: Port 0 pin 9 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.10> P0.10OD: Port 0 pin 10 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.11> P0.11OD: Port 0 pin 11 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.15> P0.15OD: Port 0 pin 15 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.16> P0.16OD: Port 0 pin 16 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.17> P0.17OD: Port 0 pin 17 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.18> P0.18OD: Port 0 pin 18 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.19> P0.19OD: Port 0 pin 19 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.20> P0.20OD: Port 0 pin 20 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.21> P0.21OD: Port 0 pin 21 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.22> P0.22OD: Port 0 pin 22 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.23> P0.23OD: Port 0 pin 23 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.24> P0.24OD: Port 0 pin 24 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.25> P0.25OD: Port 0 pin 25 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.26> P0.26OD: Port 0 pin 26 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.29> P0.29OD: Port 0 pin 29 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o15.30> P0.30OD: Port 0 pin 30 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//	</h>


//	<h> Open Drain Pin Mode select register 1 (PINMODE_OD1)
//		<o16.0> P1.00OD: Port 1 pin 0 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.1> P1.01OD: Port 1 pin 1 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.4> P1.04OD: Port 1 pin 4 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.8> P1.08OD: Port 1 pin 8 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.9> P1.09OD: Port 1 pin 9 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.10> P1.10OD: Port 1 pin 10 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.14> P1.14OD: Port 1 pin 14 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.15> P1.15OD: Port 1 pin 15 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.16> P1.16OD: Port 1 pin 16 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.17> P1.17OD: Port 1 pin 17 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.18> P1.18OD: Port 1 pin 18 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.19> P1.19OD: Port 1 pin 19 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.20> P1.20OD: Port 1 pin 20 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.21> P1.21OD: Port 1 pin 21 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.22> P1.22OD: Port 1 pin 22 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.23> P1.23OD: Port 1 pin 23 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.24> P1.24OD: Port 1 pin 24 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.25> P1.25OD: Port 1 pin 25 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.26> P1.26OD: Port 1 pin 26 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.27> P1.27OD: Port 1 pin 27 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.28> P1.28OD: Port 1 pin 28 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.29> P1.29OD: Port 1 pin 29 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.30> P1.30OD: Port 1 pin 30 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o16.31> P1.31OD: Port 1 pin 31 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//	</h>


//	<h> Open Drain Pin Mode select register 2 (PINMODE_OD2)
//		<o17.0> P2.00OD: Port 2 pin 0 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.1> P2.01OD: Port 2 pin 1 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.2> P2.02OD: Port 2 pin 2 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.3> P2.03OD: Port 2 pin 3 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.4> P2.04OD: Port 2 pin 4 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.5> P2.05OD: Port 2 pin 5 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.6> P2.06OD: Port 2 pin 6 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.7> P2.07OD: Port 2 pin 7 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.8> P2.08OD: Port 2 pin 8 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.9> P2.09OD: Port 2 pin 9 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.10> P2.10OD: Port 2 pin 10 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.11> P2.11OD: Port 2 pin 11 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.12> P2.12OD: Port 2 pin 12 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o17.13> P2.13OD: Port 2 pin 13 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//	</h>


//	<h> Open Drain Pin Mode select register 3 (PINMODE_OD3)
//		<o18.25> P3.25OD: Port 3 pin 25 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o18.26> P3.26OD: Port 3 pin 26 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//	</h>


//	<h> Open Drain Pin Mode select register 4 (PINMODE_OD4)
//		<o19.28> P4.28OD: Port 4 pin 28 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//		<o19.29> P4.29OD: Port 4 pin 29 open drain mode control
//					<0=> normal mode
//					<1=> open drain mode
//	</h>

//********** Open Drain Pin Mode select register  end ***************



//********** I2C Pin Configuration register *************************

//	<h> I2C Pin Configuration register (I2CPADCFG)
//		<o20.0> SDADRV0: Drive mode control for the SDA0 pin, P0.27
//					<0=> The SDA0 pin is in the standard drive mode
//					<1=> The SDA0 pin is in Fast Mode Plus drive mode
//		<o20.1> SDAI2C0: I2C mode control for the SDA0 pin, P0.27
//					<0=> The SDA0 pin has I2C glitch filtering and slew rate control enabled
//					<1=> The SDA0 pin has I2C glitch filtering and slew rate control disabled
//		<o20.2> SCLDRV0: Drive mode control for the SCL0 pin, P0.28
//					<0=> The SCL0 pin is in the standard drive mode
//					<1=> The SCL0 pin is in Fast Mode Plus drive mode
//		<o20.3> SCLI2C0: I2C mode control for the SCL0 pin, P0.28
//					<0=> The SCL0 pin has I2C glitch filtering and slew rate control enabled
//					<1=> The SCL0 pin has I2C glitch filtering and slew rate control disabled
//	</h>

//********** I2C Pin Configuration register end *********************


#define PINSEL0_VAL 0xC05AAA55	   // 0
#define PINSEL1_VAL 0x140003FC	   // 1
#define PINSEL2_VAL 0x50150105	   // 2
#define PINSEL3_VAL 0x0003C305	   // 3
#define PINSEL4_VAL 0x0050000A	   // 4
#define PINSEL7_VAL 0x00000000	   // 5
#define PINSEL9_VAL 0x00000000	   // 6
#define PINSEL10_VAL 0x00000000	   // 7

#define PINMODE0_VAL 0x80AAAAAA	   // 8
#define PINMODE1_VAL 0x002AAAAA	   // 9
#define PINMODE2_VAL 0xA02A020A	   // 10
#define PINMODE3_VAL 0xAAAAAAAA	   // 11
#define PINMODE4_VAL 0x0AAAAAAA	   // 12
#define PINMODE7_VAL 0x00280000	   // 13
#define PINMODE9_VAL 0x0A000000    // 14

#define PINMODE_OD0_VAL 0x00000000 //15
#define PINMODE_OD1_VAL 0x00000000 //16
#define PINMODE_OD2_VAL 0x00000000 //17
#define PINMODE_OD3_VAL 0x00000000 //18
#define PINMODE_OD4_VAL 0x00000000 //19

#define I2CPADCFG_VAL 0x00000000   //20



//---<<< end of configuration section >>>---


/*************************************************************
函数名称: v_pcfg_pin_config		           				
函数功能: 端口功能配置函数，应在系统初始化时调用，
		  并放在其它驱动初始函数前面调用，为各驱动配置好相关的端口功能						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_pcfg_pin_config(void)
{
	LPC_PINCON->PINSEL0 = PINSEL0_VAL;
	LPC_PINCON->PINSEL1 = PINSEL1_VAL;
	LPC_PINCON->PINSEL2 = PINSEL2_VAL;
	LPC_PINCON->PINSEL3 = PINSEL3_VAL;
	LPC_PINCON->PINSEL4 = PINSEL4_VAL;
	LPC_PINCON->PINSEL7 = PINSEL7_VAL;
	LPC_PINCON->PINSEL9 = PINSEL9_VAL;
	LPC_PINCON->PINSEL10 = PINSEL10_VAL;

	LPC_PINCON->PINMODE0 = PINMODE0_VAL;
	LPC_PINCON->PINMODE1 = PINMODE1_VAL;
	LPC_PINCON->PINMODE2 = PINMODE2_VAL;
	LPC_PINCON->PINMODE3 = PINMODE3_VAL;
	LPC_PINCON->PINMODE4 = PINMODE4_VAL;
	LPC_PINCON->PINMODE7 = PINMODE7_VAL;
	LPC_PINCON->PINMODE9 = PINMODE9_VAL;

	LPC_PINCON->PINMODE_OD0 = PINMODE_OD0_VAL;
	LPC_PINCON->PINMODE_OD1 = PINMODE_OD1_VAL;
	LPC_PINCON->PINMODE_OD2 = PINMODE_OD2_VAL;
	LPC_PINCON->PINMODE_OD3 = PINMODE_OD3_VAL;
	LPC_PINCON->PINMODE_OD4 = PINMODE_OD4_VAL;

	LPC_PINCON->I2CPADCFG = I2CPADCFG_VAL;
}
