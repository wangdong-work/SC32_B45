/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Usb.c
��    ����1.00
�������ڣ�2012-03-16
��    �ߣ�������
����������usbת�����豸����ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-16  1.00     ����
**************************************************************/

#include <lpc17xx.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#include "Delay.h"
#include "usbreg_lpc17xx.h"
#include "Usb.h"

#define USB_GET_STATUS			0x00
#define USB_CLEAR_FEATURE		0x01
#define USB_SET_FEATURE			0x03
#define USB_SET_ADDRESS			0x05
#define USB_GET_DESCRIPTOR		0x06
#define USB_SET_DESCRIPTOR		0x07
#define USB_GET_CONFIGURATION	0x08
#define USB_SET_CONFIGURATION	0x09
#define USB_GET_INTERFACE		0x0A
#define USB_SET_INTERFACE		0x0B
#define USB_SYNCH_FRAME			0x0C

#define USB_CDC_ACM_SET_LINE    0x20
#define USB_CDC_ACM_GET_LINE    0x21
#define USB_CDC_ACM_SET_CTL     0x22

#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_QUALIFIER		0x06


#define USB_MAX_PACKET         64

#define EP_CONTROL_LOG_NUM     0
#define EP_CONTROL_OUT_PHY_NUM 0
#define EP_CONTROL_IN_PHY_NUM  1

#define EP_COMM_LOG_NUM        1
#define EP_COMM_OUT_PHY_NUM    2
#define EP_COMM_IN_PHY_NUM     3

#define EP_DATA_LOG_NUM        2
#define EP_DATA_OUT_PHY_NUM    4
#define EP_DATA_IN_PHY_NUM     5

#define EP_INT_NUM ((1 << EP_CONTROL_OUT_PHY_NUM) | (1 << EP_CONTROL_IN_PHY_NUM) | (1 << EP_COMM_IN_PHY_NUM) | \
					(1 << EP_DATA_OUT_PHY_NUM) | (1 << EP_DATA_IN_PHY_NUM))

enum
{
	WAIT_NONE = 0,
	WAIT_ADDR_ACK,
	WAIT_CONFIG_ACK,
	WAIT_TX_NEXT_CONFIG_PACKET,
	WAIT_RX_SET_LINE_CODE,
};

#define USB_FIFO_SIZE (128+1)

typedef struct
{
	U8_T buf[USB_FIFO_SIZE];
	U32_T rd;
	U32_T wr;
}USB_FIFO_T;

typedef struct
{
	USB_FIFO_T t_rx;
	USB_FIFO_T t_tx;
	U8_T buf[USB_MAX_PACKET];
	U8_T addr;
	U8_T status;
	U8_T tx_flag;
}USB_DATA_T;



/*static struct usb_device_descriptor m_u8_usb_to_serial_device_desc = {
	.bLength =            0x12,             // 0x12
	.bDescriptorType =    0x01,             // 0x01
	.bcdUSB =             0x0101,           // 0x0101(usb1.1)����0x0200(usb2.0)
	.bDeviceClass =	      2,                // 2
	.bDeviceSubClass =    0,
	.bDeviceProtocol =    0,
	.bMaxPacketSize0 =    64,
	.idVendor =	          0x03EB,
	.idProduct =          0x6119,
	.bcdDevice =          0x0100,
	.iManufacturer =      0,
	.iProduct =           0,
	.iSerialNumber =      0,
	.bNumConfigurations = 1,
};

static struct usb_cdc_configure_descriptor m_u8_usb_to_serial_configure_desc = {

// Standard configuration descriptor
	.config = {
		.bLength = USB_DT_CONFIG_SIZE,     // 0x09
		.bDescriptorType = USB_DT_CONFIG,  // 0x02
		.wTotalLength = 67,
		.bNumInterfaces = 2,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes =	0xC0,
		.bMaxPower =		1,
	},

 // Communication class interface standard descriptor
	.comm_interface = {
		.bLength = USB_DT_INTERFACE_SIZE,         // 0x09
		.bDescriptorType = USB_DT_INTERFACE,      // 0x04
		.bInterfaceNumber = 0, 
		.bAlternateSetting = 0,
		.bNumEndpoints =	 1,
		.bInterfaceClass = 0x02,          // commucations interface class
		.bInterfaceSubClass = 0x02,        // abstract control modal
		.bInterfaceProtocol = 0,          // no protocol
		.iInterface = 0,
	},

// Class-specific header functional descriptor
	.header = {
		.bLength = sizeof(struct usb_cdc_header_desc),    // 0x05
		.bDescriptorType = USB_DT_CS_INTERFACE,           // 0x24
		.bDescriptorSubType = USB_CDC_HEADER_TYPE,        // 0x00
		.bcdCDC = __constant_cpu_to_le16(0x0110),          // 0x0110
	},

// Class-specific call management functional descriptor
	.call = {
		.bLength = sizeof(struct usb_cdc_call_mgmt_descriptor),   // 0x05
		.bDescriptorType = USB_DT_CS_INTERFACE,                   // 0x24
		.bDescriptorSubType = USB_CDC_CALL_MANAGEMENT_TYPE,       // 0x01
		.bmCapabilities =	1,
		.bDataInterface =	1,	//index of data interface 
	},

// Class-specific abstract control management functional descriptor
	.acm = {
		.bLength = sizeof(struct usb_cdc_acm_descriptor),      // 0x04
		.bDescriptorType = USB_DT_CS_INTERFACE,                // 0x24
		.bDescriptorSubType =	USB_CDC_ACM_TYPE,              // 0x02
		.bmCapabilities =	0x02,                              // support line
	},

// Class-specific union functional descriptor with one slave interface
	.union_desc = {
		.bLength = sizeof(struct usb_cdc_union_desc),    // 0x05
		.bDescriptorType = USB_DT_CS_INTERFACE,          // 0x24
		.bDescriptorSubType = USB_CDC_UNION_TYPE,        // 0x06
		.bMasterInterface0 =	0,	//index of control interface
		.bSlaveInterface0 =	1,	// index of data interface
	},

// Notification endpoint standard descriptor
	.notify_endpoint = {
		.bLength = USB_DT_ENDPOINT_SIZE,           // 0x07
		.bDescriptorType = USB_DT_ENDPOINT,        // 0x05
		.bEndpointAddress = (USB_DIR_IN | 0x01),   // 0x81
		.bmAttributes = USB_ENDPOINT_XFER_INT,     // 0x03
		.wMaxPacketSize = 64,
		.bInterval =	 10,                           // 10ms
	},

// Data class interface standard descriptor
	.data_interface = {
		.bLength = USB_DT_INTERFACE_SIZE,         // 0x09
		.bDescriptorType = USB_DT_INTERFACE,      // 0x04
		.bInterfaceNumber = 1,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CLASS_CDC_DATA,    // 0x0a
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	},

// Bulk-OUT endpoint standard descriptor
	.bulk_out = {
		.bLength = USB_DT_ENDPOINT_SIZE,           // 0x07
		.bDescriptorType = USB_DT_ENDPOINT,        // 0x05
		.bEndpointAddress = (USB_DIR_OUT | 0x02),  // 0x02
		.bmAttributes = USB_ENDPOINT_XFER_BULK,    // 0x02
		.wMaxPacketSize = 64,
		.bInterval =	 0,
	},

// Bulk-IN endpoint standard descriptor
	.bulk_in = {
		.bLength = USB_DT_ENDPOINT_SIZE,           // 0x07
		.bDescriptorType = USB_DT_ENDPOINT,        // 0x05
		.bEndpointAddress = (USB_DIR_IN | 0x03),   // 0x83
		.bmAttributes = USB_ENDPOINT_XFER_BULK,    // 0x02
		.wMaxPacketSize = 64,
		.bInterval =	 0,
	},
};*/


static const U8_T m_u8_usb_to_serial_device_desc[] =
{
	0x12, 0x01, 0x00, 0x02, 0x02, 0x00, 0x00, 0x40, 0xEB,
	0x03, 0x19, 0x61, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
};

static const U8_T m_u8_usb_to_serial_configure_desc[] =
{
	0x09, 0x02, 0x43, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x01,    // Standard configuration descriptor
	0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00,    // Communication class interface standard descriptor
	0x05, 0x24, 0x00, 0x10, 0x01,                            // Class-specific header functional descriptor
	0x05, 0x24, 0x01, 0x01, 0x01,                            // Class-specific call management functional descriptor
	0x04, 0x24, 0x02, 0x02,                                  // Class-specific abstract control management functional descriptor
	0x05, 0x24, 0x06, 0x00, 0x01,                            // Class-specific union functional descriptor with one slave interface
	0x07, 0x05, 0x81, 0x03, 0x40, 0x00, 0x0A,                // Notification endpoint standard descriptor
	0x09, 0x04, 0x01, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x00,    // Data class interface standard descriptor
	0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,                // Bulk-OUT endpoint standard descriptor
	0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00                 // Bulk-IN endpoint standard descriptor
};

/* Line Coding Structure from CDC spec 6.2.13 */
typedef struct
{
	U32_T dwDTERate;
	U8_T  bCharFormat;
#define USB_CDC_1_STOP_BITS			0
#define USB_CDC_1_5_STOP_BITS       1
#define USB_CDC_2_STOP_BITS			2

	U8_T  bParityType;
#define USB_CDC_NO_PARITY			0
#define USB_CDC_ODD_PARITY			1
#define USB_CDC_EVEN_PARITY			2
#define USB_CDC_MARK_PARITY			3
#define USB_CDC_SPACE_PARITY		4

	U8_T	bDataBits;
}USB_CDC_LINE_CODING_T;

static USB_CDC_LINE_CODING_T m_t_line_code =
{
	115200,
	USB_CDC_1_STOP_BITS,
	USB_CDC_NO_PARITY,
	8
};

static USB_DATA_T m_t_usb_data;

/*************************************************************
��������: u32_usb_usb_fifo_space		           				
��������: ����FIFO�Ŀ�д�ռ��ֽ���						
�������: pt_fifo -- ָ��Ҫ�����FIFO        		   				
�������: ��
����ֵ  ��FIFO�Ŀ�д�ֽ���														   				
**************************************************************/
static U32_T u32_usb_usb_fifo_space(USB_FIFO_T *pt_fifo)
{
	return ((USB_FIFO_SIZE + pt_fifo->rd - pt_fifo->wr - 1) % USB_FIFO_SIZE); 
}

/*************************************************************
��������: u32_usb_usb_fifo_count		           				
��������: ����FIFO�Ŀɶ��ֽ���						
�������: pt_fifo -- ָ��Ҫ�����FIFO        		   				
�������: ��
����ֵ  ��FIFO�Ŀɶ��ֽ���														   				
**************************************************************/
static U32_T u32_usb_usb_fifo_count(USB_FIFO_T *pt_fifo)
{
	return ((USB_FIFO_SIZE + pt_fifo->wr - pt_fifo->rd) % USB_FIFO_SIZE);
}

/*************************************************************
��������: v_usb_usb_write_cmd		           				
��������: ��usb�豸��������SIEд����						
�������: u32_cmd -- ������        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_write_cmd(U32_T u32_cmd)
{
	LPC_USB->USBDevIntClr = CCEMTY_INT;
	LPC_USB->USBCmdCode = u32_cmd;
	while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);
}

/*************************************************************
��������: v_usb_usb_write_cmd_data		           				
��������: ��usb�豸��������SIEд���������						
�������: u32_cmd -- ������
          u32_val -- ����        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_write_cmd_data(U32_T u32_cmd, U32_T u32_val)
{
	LPC_USB->USBDevIntClr = CCEMTY_INT;
	LPC_USB->USBCmdCode = u32_cmd;
	while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);

	LPC_USB->USBDevIntClr = CCEMTY_INT;
	LPC_USB->USBCmdCode = u32_val;
	while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);
}

/*************************************************************
��������: v_usb_usb_write_cmd_ep		           				
��������: ��usb�豸��������SIEд�˵�����						
�������: u32_ep_num -- ����˵��
          u32_cmd    -- ������        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_write_cmd_ep(U32_T u32_ep_num, U32_T u32_cmd)
{
	LPC_USB->USBDevIntClr = CCEMTY_INT;
	LPC_USB->USBCmdCode = CMD_SEL_EP(u32_ep_num);
	while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);

	LPC_USB->USBDevIntClr = CCEMTY_INT;
	LPC_USB->USBCmdCode = u32_cmd;
	while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);
}

/*************************************************************
��������: u32_usb_usb_read_cmd_data		           				
��������: ��usb�豸��������SIE������						
�������: u32_cmd -- ������        		   				
�������: ��
����ֵ  �����ض���������														   				
**************************************************************/
static U32_T u32_usb_usb_read_cmd_data(U32_T u32_cmd)
{
	LPC_USB->USBDevIntClr = CCEMTY_INT | CDFULL_INT;
	LPC_USB->USBCmdCode = u32_cmd;
	while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);

	return (LPC_USB->USBCmdData);
}

/*************************************************************
��������: u32_usb_usb_ep_readability		           				
��������: ��ѯusb�豸�������Ķ˵��Ƿ�ɶ�						
�������: u32_num -- ����˵��        		   				
�������: ��
����ֵ  ������0��ʾ���ɶ������ط�0��ʾ�ɶ�														   				
**************************************************************/
static U32_T u32_usb_usb_ep_readability(U32_T u32_num)
{
	U32_T val;

	v_usb_usb_write_cmd(CMD_SEL_EP(u32_num));
	val = u32_usb_usb_read_cmd_data(DAT_SEL_EP(u32_num));

	return (val & 0x01);

}

/*************************************************************
��������: u32_usb_usb_ep_writability		           				
��������: ��ѯusb�豸�������Ķ˵��Ƿ��д						
�������: u32_num -- ����˵��        		   				
�������: ��
����ֵ  ������0��ʾ����д�����ط�0��ʾ��д														   				
**************************************************************/
static U32_T u32_usb_usb_ep_writability(U32_T u32_num)
{
	U32_T val;

	v_usb_usb_write_cmd(CMD_SEL_EP(u32_num));
	val = u32_usb_usb_read_cmd_data(DAT_SEL_EP(u32_num));

	return ((val & 0x01) ? 0 : 1);
}

/*************************************************************
��������: u32_usb_usb_read_ep_len		           				
��������: ��ѯ�˵㻺������ĩ�����ֽ���						
�������: u32_num -- ����˵��        		   				
�������: ��
����ֵ  ��ĩ�����ֽ���														   				
**************************************************************/
static U32_T u32_usb_usb_read_ep_len(U32_T u32_num)
{
	U32_T cnt;

	LPC_USB->USBCtrl = ((u32_num << 2) | CTRL_RD_EN);

	do
	{
		cnt = LPC_USB->USBRxPLen;
	}while ((cnt & PKT_RDY) == 0);
	cnt &= PKT_LNGTH_MASK;

	LPC_USB->USBCtrl = 0;

	return cnt;
}

/*************************************************************
��������: u32_usb_usb_read_ep		           				
��������: ���˵㻺��������						
�������: u32_num -- ����˵��        		   				
�������: pu8_buf -- �����ȡ�������ݣ���������Ϊ������㹻�Ŀռ�
����ֵ  ����ȡ�����ֽ���														   				
**************************************************************/
static U32_T u32_usb_usb_read_ep(U32_T u32_num, U8_T *pu8_buf)
{
	U32_T cnt, n;

	LPC_USB->USBCtrl = ((u32_num<<2) | CTRL_RD_EN);

	do
	{
		cnt = LPC_USB->USBRxPLen;
	}while ((cnt & PKT_RDY) == 0);
	cnt &= PKT_LNGTH_MASK;

	for (n=0; n<(cnt+3)/4; n++)
	{
		*((U32_T *)pu8_buf) = LPC_USB->USBRxData;
		pu8_buf += 4;
	}

	LPC_USB->USBCtrl = 0;
	v_usb_usb_write_cmd_ep(u32_num*2, CMD_CLR_BUF);

	return cnt;
}

/*************************************************************
��������: v_usb_usb_write_ep		           				
��������: д���ݵ��˵㻺����						
�������: u32_num -- ����˵��        		   				
          pu8_buf -- ָ��Ҫд�������
          u32_cnt -- Ҫд����ֽ���
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_write_ep(U32_T u32_num, const U8_T *pu8_buf, U32_T u32_cnt)
{
	U32_T n;

	LPC_USB->USBCtrl = ((u32_num<<2) | CTRL_WR_EN);
	LPC_USB->USBTxPLen = u32_cnt;

	for (n=0; n<(u32_cnt+3)/4; n++)
	{
		LPC_USB->USBTxData = *((U32_T *)pu8_buf);
		pu8_buf += 4;
	}

	LPC_USB->USBCtrl = 0;
	v_usb_usb_write_cmd_ep(u32_num*2+1, CMD_VALID_BUF);
}

/*************************************************************
��������: v_usb_usb_set_address		           				
��������: ����USB�豸�ĵ�ַ						
�������: addr -- ��ַ        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_set_address(U32_T addr)
{
	v_usb_usb_write_cmd_data(CMD_SET_ADDR, DAT_WR_BYTE(DEV_EN | (addr&DEV_ADDR_MASK)));
	v_usb_usb_write_cmd_data(CMD_SET_ADDR, DAT_WR_BYTE(DEV_EN | (addr&DEV_ADDR_MASK)));   //�����Ա����������Σ�ֻ����һ�Σ��豸������
}

/*************************************************************
��������: v_usb_usb_reset		           				
��������: ��������USB�豸						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_reset(void)
{
	LPC_USB->USBDevIntClr = EP_RLZED_INT;
	LPC_USB->USBReEp |= (1 << EP_CONTROL_OUT_PHY_NUM);
	LPC_USB->USBEpInd    = EP_CONTROL_OUT_PHY_NUM;
	LPC_USB->USBMaxPSize = USB_MAX_PACKET;
	while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
	LPC_USB->USBDevIntClr = EP_RLZED_INT;

	LPC_USB->USBReEp |= (1 << EP_CONTROL_IN_PHY_NUM);
	LPC_USB->USBEpInd    = EP_CONTROL_IN_PHY_NUM;
	LPC_USB->USBMaxPSize = USB_MAX_PACKET;
	while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
	LPC_USB->USBDevIntClr = EP_RLZED_INT;

	LPC_USB->USBEpIntClr  = 0xFFFFFFFF;
	LPC_USB->USBDevIntClr = 0xFFFFFFFF;
	LPC_USB->USBEpIntEn   = EP_INT_NUM;
	LPC_USB->USBDevIntEn  = (DEV_STAT_INT | EP_SLOW_INT);

	v_usb_usb_set_address(0);
}

/*************************************************************
��������: v_usb_usb_connect		           				
��������: ����USB�豸��������ô˺���������USB�豸����������λ						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_connect(void)
{
	v_usb_usb_write_cmd_data(CMD_SET_DEV_STAT, DAT_WR_BYTE(DEV_CON));
}

/*************************************************************
��������: v_usb_usb_handle_setup		           				
��������: ����usb�豸��ö�ٹ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_handle_setup(void)
{
	U8_T *data = m_t_usb_data.buf;
	U8_T type;
	U32_T cnt, len;

	cnt = u32_usb_usb_read_ep(EP_CONTROL_LOG_NUM, data);
	if (cnt != 8)
		return;
	
	type = (data[0] & 0x60);

	switch (type)
	{
		case 0:
			if (data[1] == USB_GET_DESCRIPTOR)
			{
				len = (data[7]<<8) + data[6];

				if (data[3] == USB_DT_DEVICE)
				{

					if (len > 0x12)
						len = 0x12;

					v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, m_u8_usb_to_serial_device_desc, len);
				}

				else if (data[3]== USB_DT_CONFIG)
				{
					if (len > 67)
						len = 67;

					if (len > USB_MAX_PACKET)
					{
						len = USB_MAX_PACKET;
						m_t_usb_data.status = WAIT_TX_NEXT_CONFIG_PACKET;
					}

					v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, m_u8_usb_to_serial_configure_desc, len);
				}

				else
				{
					v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
				}
			}

			else if (data[1] == USB_SET_ADDRESS)
			{
				m_t_usb_data.addr = data[2];
				m_t_usb_data.status = WAIT_ADDR_ACK;
				v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
			}

			else if (data[1] == USB_SET_CONFIGURATION)
			{
				m_t_usb_data.status = WAIT_CONFIG_ACK;
				v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
			}

			else
			{
				v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
			}

			break;

		case 0x20:
			if (data[1] == USB_CDC_ACM_SET_LINE)
			{
				m_t_usb_data.status = WAIT_RX_SET_LINE_CODE;
			}

			else if (data[1] == USB_CDC_ACM_GET_LINE)
			{
				v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, (U8_T *)&m_t_line_code, 7);
			}

			else if (data[1] == USB_CDC_ACM_SET_CTL)
			{
				v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
			}

			else
			{
				v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
			}

			break;

		default:
			v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
			break;
	}
}

/*************************************************************
��������: v_usb_usb_ctl_rx_handler		           				
��������: ���ƶ˵������ɴ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_ctl_rx_handler(void)
{
	U8_T *data = m_t_usb_data.buf;
	U32_T cnt;

	cnt = u32_usb_usb_read_ep(EP_CONTROL_LOG_NUM, data);
	if (cnt != 7)
		return;

	if (m_t_usb_data.status == WAIT_RX_SET_LINE_CODE)
	{
		m_t_line_code.dwDTERate = (data[3]<<24) + (data[2]<<16) + (data[1]<<8) + data[0];
		m_t_line_code.bCharFormat = data[4];
		m_t_line_code.bParityType = data[5];
		m_t_line_code.bDataBits = data[6];
		m_t_usb_data.status = WAIT_NONE;

		v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, NULL, 0);
	}
}

/*************************************************************
��������: v_usb_usb_ctl_tx_handler		           				
��������: ���ƶ˵㷢����ɴ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_ctl_tx_handler(void)
{
	if (m_t_usb_data.status == WAIT_ADDR_ACK)
	{
		m_t_usb_data.status = WAIT_NONE;
		v_usb_usb_set_address(m_t_usb_data.addr);
	}

	else if (m_t_usb_data.status == WAIT_CONFIG_ACK)
	{
		m_t_usb_data.status = WAIT_NONE;

		LPC_USB->USBDevIntClr = EP_RLZED_INT;
		LPC_USB->USBReEp |= (1 << EP_COMM_IN_PHY_NUM);
		LPC_USB->USBEpInd    = EP_COMM_IN_PHY_NUM;
		LPC_USB->USBMaxPSize = USB_MAX_PACKET;
		while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
		LPC_USB->USBDevIntClr = EP_RLZED_INT;

		LPC_USB->USBReEp |= (1 << EP_DATA_OUT_PHY_NUM);
		LPC_USB->USBEpInd    = EP_DATA_OUT_PHY_NUM;
		LPC_USB->USBMaxPSize = USB_MAX_PACKET;
		while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
		LPC_USB->USBDevIntClr = EP_RLZED_INT;

		LPC_USB->USBReEp |= (1 << EP_DATA_IN_PHY_NUM);
		LPC_USB->USBEpInd    = EP_DATA_IN_PHY_NUM;
		LPC_USB->USBMaxPSize = USB_MAX_PACKET;
		while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
		LPC_USB->USBDevIntClr = EP_RLZED_INT;

		v_usb_usb_write_cmd_data(CMD_CFG_DEV, DAT_WR_BYTE(1));

		memset(&m_t_usb_data, 0, sizeof(m_t_usb_data));     //��ջ������ͷ��ͱ�־
	}

	else if (m_t_usb_data.status == WAIT_TX_NEXT_CONFIG_PACKET)
	{
		v_usb_usb_write_ep(EP_CONTROL_LOG_NUM, m_u8_usb_to_serial_configure_desc+USB_MAX_PACKET, 67-USB_MAX_PACKET);
		m_t_usb_data.status = WAIT_NONE;
	}
}

/*************************************************************
��������: v_usb_usb_data_rx_handler		           				
��������: ���ݶ˵������ɴ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_data_rx_handler(void)
{
	U8_T *data = m_t_usb_data.buf;
	U32_T cnt, len;
	USB_FIFO_T *fifo = &m_t_usb_data.t_rx;

	if (u32_usb_usb_ep_readability(EP_DATA_OUT_PHY_NUM) == 0)
		return;

	cnt = u32_usb_usb_read_ep_len(EP_DATA_LOG_NUM);
	if (u32_usb_usb_fifo_space(fifo) < cnt)
		return;

	cnt = u32_usb_usb_read_ep(EP_DATA_LOG_NUM, data);

	if (fifo->wr + cnt > USB_FIFO_SIZE)
	{
		len = USB_FIFO_SIZE - fifo->wr;
		memcpy(&(fifo->buf[fifo->wr]), data, len);
		memcpy(fifo->buf, data+len, cnt-len);
	}
	else
		memcpy(&(fifo->buf[fifo->wr]), data, cnt);
	
	fifo->wr = ((fifo->wr + cnt) % USB_FIFO_SIZE);
}

/*************************************************************
��������: v_usb_usb_data_tx_handler		           				
��������: ���ݶ˵㷢����ɴ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_usb_usb_data_tx_handler(void)
{
	U8_T *data = m_t_usb_data.buf;
	U32_T cnt, len;
	USB_FIFO_T *fifo = &m_t_usb_data.t_tx;
	
	if (u32_usb_usb_ep_writability(EP_DATA_OUT_PHY_NUM) == 0)
		return;

	cnt = u32_usb_usb_fifo_count(fifo);
	if (cnt == 0)
	{
		m_t_usb_data.tx_flag = 0;
		return;
	}
	else
	{
		m_t_usb_data.tx_flag = 1;
	}

	if (cnt > (USB_MAX_PACKET-1))                // ���������������ݣ�PC���ղ���
		cnt = USB_MAX_PACKET - 1;              // ����ÿ�η��͵�����ֽ���ΪUSB_MAX_PACKET - 1

	if (fifo->rd + cnt > USB_FIFO_SIZE)
	{
		len = USB_FIFO_SIZE - fifo->rd;
		memcpy(data, &(fifo->buf[fifo->rd]), len);
		memcpy(data+len, fifo->buf, cnt-len);
	}
	else
	{
		memcpy(data, &(fifo->buf[fifo->rd]), cnt);
	}

	fifo->rd = ((fifo->rd + cnt) % USB_FIFO_SIZE);
	
	v_usb_usb_write_ep(EP_DATA_LOG_NUM, data, cnt);
}

/*************************************************************
��������: USB_IRQHandler		           				
��������: usb�жϴ�����						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void USB_IRQHandler(void)
{
	U32_T reg, val;

	reg = LPC_USB->USBDevIntSt;                  // Device Interrupt Status

	if ((reg & DEV_STAT_INT) == DEV_STAT_INT)
	{
		LPC_USB->USBDevIntClr = DEV_STAT_INT;

		v_usb_usb_write_cmd(CMD_GET_DEV_STAT);
		val = u32_usb_usb_read_cmd_data(DAT_GET_DEV_STAT);
		if ((val & DEV_RST) == DEV_RST)
			v_usb_usb_reset();
	}

	if ((reg & EP_SLOW_INT) == EP_SLOW_INT)
	{
		reg = LPC_USB->USBEpIntSt;

		if ((reg & (1<<EP_CONTROL_OUT_PHY_NUM)) != 0)
		{
			LPC_USB->USBEpIntClr = (1 << EP_CONTROL_OUT_PHY_NUM);
        	while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
        	val = LPC_USB->USBCmdData;

			if ((val & EP_SEL_STP) != 0)
			{
				v_usb_usb_handle_setup();
			}
			else
			{
				v_usb_usb_ctl_rx_handler();
			}
				
		}

		if ((reg & (1<<EP_CONTROL_IN_PHY_NUM)) != 0)
		{
			LPC_USB->USBEpIntClr = (1 << EP_CONTROL_IN_PHY_NUM);
        	while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
        	val = LPC_USB->USBCmdData;

			v_usb_usb_ctl_tx_handler();
		}

		if ((reg & (1<<EP_COMM_IN_PHY_NUM)) != 0)
		{
			LPC_USB->USBEpIntClr = (1 << EP_COMM_IN_PHY_NUM);
        	while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
        	val = LPC_USB->USBCmdData;

			//����
		}

		if ((reg & (1<<EP_DATA_OUT_PHY_NUM)) != 0)
		{
			LPC_USB->USBEpIntClr = (1 << EP_DATA_OUT_PHY_NUM);
        	while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
        	val = LPC_USB->USBCmdData;

			v_usb_usb_data_rx_handler();
		}

		if ((reg & (1<<EP_DATA_IN_PHY_NUM)) != 0)
		{
			LPC_USB->USBEpIntClr = (1 << EP_DATA_IN_PHY_NUM);
        	while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
        	val = LPC_USB->USBCmdData;

			v_usb_usb_data_tx_handler();
		}

		LPC_USB->USBDevIntClr = EP_SLOW_INT;
	}
}

/*************************************************************
��������: v_usb_usb_init		           				
��������: ��ʼ��USB�豸������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_usb_usb_init(void)
{
	memset(&m_t_usb_data, 0, sizeof(m_t_usb_data));

	LPC_USB->USBClkCtrl = 0x1A;                  // Dev, AHB clock enable
	while ((LPC_USB->USBClkSt & 0x1A) != 0x1A);

	NVIC_EnableIRQ(USB_IRQn);                    // enable USB interrupt

	v_usb_usb_reset();
	v_usb_usb_connect();	                     // �趨�������������裬���ܲ���usb��λ�ж�
}

/*************************************************************
��������: u32_usb_usb_read		           				
��������: ��ȡusb���յ�������						
�������: u32_cnt  -- Ҫ��ȡ���ֽڸ���       		   				
�������: pu8_buf  -- ���ض�ȡ�������ݣ���������Ϊ������㹻�Ŀռ䣨���Ȳ�С��u32_cnt)
����ֵ  ������ʵ�ʶ�ȡ���ĸ�����������0���ʾ�����ݿ��Զ�ȡ          		   															   				
**************************************************************/
U32_T u32_usb_usb_read(U8_T *pu8_buf, U32_T u32_cnt)
{
	U32_T fifo_cnt, ret_cnt, len;
	USB_FIFO_T *fifo = &m_t_usb_data.t_rx;

	LPC_USB->USBDevIntEn  = 0;	        // �ر��ж�

	v_usb_usb_data_rx_handler();
	fifo_cnt = u32_usb_usb_fifo_count(fifo);
	if (fifo_cnt == 0)
	{
		ret_cnt = 0;
		goto out;
	}

	if (fifo_cnt > u32_cnt)
		ret_cnt = u32_cnt;
	else
		ret_cnt = fifo_cnt;

	if (fifo->rd + ret_cnt > USB_FIFO_SIZE)
	{
		len = USB_FIFO_SIZE - fifo->rd;
		memcpy(pu8_buf, &(fifo->buf[fifo->rd]), len);
		memcpy(pu8_buf+len, fifo->buf, ret_cnt-len);
	}
	else
		memcpy(pu8_buf, &(fifo->buf[fifo->rd]), ret_cnt);
	
	fifo->rd = ((fifo->rd + ret_cnt) % USB_FIFO_SIZE);

out:
	v_usb_usb_data_rx_handler();
	LPC_USB->USBDevIntEn  = (DEV_STAT_INT | EP_SLOW_INT);	   // ʹ���ж�

	return ret_cnt;	
}

/*************************************************************
��������: usb_write		           				
��������: д���ݵ�usb						
�������: pu8_buf  -- ָ��Ҫд�������
		  u32_cnt  -- Ҫд����ֽڸ���
�������: ��
����ֵ  ������ʵ��д��ĸ�����������0�򻺳���û�пռ��ˣ�����д��          		   															   				
**************************************************************/
U32_T u32_usb_usb_write(U8_T *pu8_buf, U32_T u32_cnt)
{
	U32_T fifo_space, ret_cnt, len;
	USB_FIFO_T *fifo = &m_t_usb_data.t_tx;

	LPC_USB->USBDevIntEn  = 0;    // �ر��ж�

	fifo_space = u32_usb_usb_fifo_space(fifo);
	if (fifo_space == 0)
	{
		ret_cnt = 0;
		goto out;
	}

	if (fifo_space > u32_cnt)
		ret_cnt = u32_cnt;
	else
		ret_cnt = fifo_space;

	if (fifo->wr + ret_cnt > USB_FIFO_SIZE)
	{
		len = USB_FIFO_SIZE - fifo->wr;
		memcpy(&(fifo->buf[fifo->wr]), pu8_buf, len);
		memcpy(fifo->buf, pu8_buf+len, ret_cnt-len);
	}
	else
		memcpy(&(fifo->buf[fifo->wr]), pu8_buf, ret_cnt);
	
	fifo->wr = ((fifo->wr + ret_cnt) % USB_FIFO_SIZE);

out:
	if (m_t_usb_data.tx_flag == 0)
		v_usb_usb_data_tx_handler();
	LPC_USB->USBDevIntEn  = (DEV_STAT_INT | EP_SLOW_INT);   // ʹ���ж�

	return ret_cnt;	

}

/*************************************************************
��������: u32_usb_debug_print		           				
��������: ���Դ�ӡ������ͨ��usbת���ڴ�ӡ�ַ���						
�������: fmt  -- ��ʽ���ַ���
		  ...  -- �ɱ�����б�
�������: ��
����ֵ  ������ʵ�ʴ�ӡ���ַ�����          		   															   				
**************************************************************/
U32_T u32_usb_debug_print(const char *fmt, ...)
{
	U32_T len;
	char *buf = (char *)(m_t_usb_data.buf);

	va_list arg;
	va_start(arg, fmt);

	memset(buf, 0, USB_MAX_PACKET);
	vsnprintf(buf, USB_MAX_PACKET, fmt, arg);
	va_end(arg);

	len = strlen(buf);

	return (u32_usb_usb_write((U8_T *)buf, len));
}
